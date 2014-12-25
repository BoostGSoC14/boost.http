namespace boost {
namespace http {

template<class Socket>
read_state basic_socket<Socket>::read_state() const
{
    return istate;
}

template<class Socket>
write_state basic_socket<Socket>::write_state() const
{
    return writer_helper.state;
}

template<class Socket>
bool basic_socket<Socket>::write_response_native_stream() const
{
    return flags & HTTP_1_1;
}

template<class Socket>
template<class String, class Message, class CompletionToken>
typename asio::async_result<
    typename asio::handler_type<CompletionToken,
                                void(system::error_code)>::type>::type
basic_socket<Socket>
::async_read_request(String &method, String &path, Message &message,
                     CompletionToken &&token)
{
    typedef typename asio::handler_type<
        CompletionToken, void(system::error_code)>::type Handler;

    Handler handler(std::forward<CompletionToken>(token));

    asio::async_result<Handler> result(handler);

    if (istate != http::read_state::empty) {
        handler(system::error_code{http_errc::out_of_order});
        return result.get();
    }

    method.clear();
    path.clear();
    schedule_on_async_read_message<READY>(handler, message, &method, &path);

    return result.get();
}

template<class Socket>
template<class Message, class CompletionToken>
typename asio::async_result<
    typename asio::handler_type<CompletionToken,
                                void(system::error_code)>::type>::type
basic_socket<Socket>::async_read_some(Message &message, CompletionToken &&token)
{
    typedef typename asio::handler_type<
        CompletionToken, void(system::error_code)>::type Handler;

    Handler handler(std::forward<CompletionToken>(token));

    asio::async_result<Handler> result(handler);

    if (istate != http::read_state::message_ready) {
        handler(system::error_code{http_errc::out_of_order});
        return result.get();
    }

    schedule_on_async_read_message<DATA>(handler, message);

    return result.get();
}

template<class Socket>
template<class Message, class CompletionToken>
typename asio::async_result<
    typename asio::handler_type<CompletionToken,
                                void(system::error_code)>::type>::type
basic_socket<Socket>::async_read_trailers(Message &message,
                                          CompletionToken &&token)
{
    typedef typename asio::handler_type<
        CompletionToken, void(system::error_code)>::type Handler;

    Handler handler(std::forward<CompletionToken>(token));

    asio::async_result<Handler> result(handler);

    if (istate != http::read_state::body_ready) {
        handler(system::error_code{http_errc::out_of_order});
        return result.get();
    }

    schedule_on_async_read_message<END>(handler, message);

    return result.get();
}

template<class Socket>
template<class StringRef, class Message, class CompletionToken>
typename asio::async_result<
    typename asio::handler_type<CompletionToken,
                                void(system::error_code)>::type>::type
basic_socket<Socket>
::async_write_response(std::uint_fast16_t status_code,
                       const StringRef &reason_phrase, const Message &message,
                       CompletionToken &&token)
{
    using detail::string_literal_buffer;
    typedef typename asio::handler_type<
        CompletionToken, void(system::error_code)>::type Handler;

    Handler handler(std::forward<CompletionToken>(token));
    asio::async_result<Handler> result(handler);

    if (!writer_helper.write_message()) {
        handler(system::error_code{http_errc::out_of_order});
        return result.get();
    }

    auto crlf = string_literal_buffer("\r\n");
    auto sep = string_literal_buffer(": ");
    bool implicit_content_length
        = (message.headers().find("content-length") != message.headers().end())
        || (status_code / 100 == 1) || (status_code == 204)
        || (connect_request && (status_code / 100 == 2));

    // because we don't create multiple responses at once with HTTP/1.1
    // pipelining, it's safe to use this "shared state"
    content_length_buffer = std::to_string(status_code) + ' ';
    std::string::size_type content_length_delim = content_length_buffer.size();

    // because we don't create multiple responses at once with HTTP/1.1
    // pipelining, it's safe to use this "shared state"
    content_length_buffer += std::to_string(message.body().size());

    const auto nbuffer_pieces =
        // Start line (http version + status code + reason phrase) + CRLF
        4
        // Headers
        // Each header is 4 buffer pieces: key + sep + value + crlf
        + 4 * message.headers().size()
        // Extra content-length header uses 3 pieces
        + (implicit_content_length ? 0 : 3)
        // Extra CRLF for end of headers
        + 1
        // And finally, the message body
        + (implicit_content_length ? 0 : 1);

    // TODO (C++14): replace by dynarray
    std::vector<asio::const_buffer> buffers(nbuffer_pieces);

    buffers.push_back(string_literal_buffer((flags & HTTP_1_1)
                                            ? "HTTP/1.1 " : "HTTP/1.0 "));
    buffers.push_back(asio::buffer(content_length_buffer.data(),
                                   content_length_delim));
    buffers.push_back(asio::buffer(reason_phrase.data(), reason_phrase.size()));
    buffers.push_back(crlf);

    for (const auto &header: message.headers()) {
        buffers.push_back(asio::buffer(header.first));
        buffers.push_back(sep);
        buffers.push_back(asio::buffer(header.second));
        buffers.push_back(crlf);
    }

    if (!implicit_content_length) {
        buffers.push_back(string_literal_buffer("content-length: "));
        buffers.push_back(asio::buffer(content_length_buffer.data()
                                       + content_length_delim,
                                       content_length_buffer.size()
                                       - content_length_delim));
        buffers.push_back(crlf);
    }

    buffers.push_back(crlf);

    if (!implicit_content_length)
        buffers.push_back(asio::buffer(message.body()));

    auto flags = this->flags;
    asio::async_write(channel, buffers,
                      [handler,flags]
                      (const system::error_code &ec, std::size_t) mutable {
        if (ec || flags & KEEP_ALIVE)
            handler(ec);
        else
            handler(system::error_code{http_errc::stream_finished});
    });

    return result.get();
}

template<class Socket>
template<class CompletionToken>
typename asio::async_result<
    typename asio::handler_type<CompletionToken,
                                void(system::error_code)>::type>::type
basic_socket<Socket>
::async_write_response_continue(CompletionToken &&token)
{
    typedef typename asio::handler_type<
        CompletionToken, void(system::error_code)>::type Handler;

    Handler handler(std::forward<CompletionToken>(token));

    asio::async_result<Handler> result(handler);

    if (!writer_helper.write_continue()) {
        handler(system::error_code{http_errc::out_of_order});
        return result.get();
    }

    asio::async_write(channel,
                      detail::string_literal_buffer("HTTP/1.1 100"
                                                    " Continue\r\n\r\n"),
                      [handler]
                      (const system::error_code &ec, std::size_t) mutable {
        handler(ec);
    });

    return result.get();
}

template<class Socket>
template<class StringRef, class Message, class CompletionToken>
typename asio::async_result<
    typename asio::handler_type<CompletionToken,
                                void(system::error_code)>::type>::type
basic_socket<Socket>
::async_write_response_metadata(std::uint_fast16_t status_code,
                                const StringRef &reason_phrase,
                                const Message &message, CompletionToken &&token)
{
    using detail::string_literal_buffer;
    typedef typename asio::handler_type<
        CompletionToken, void(system::error_code)>::type Handler;

    Handler handler(std::forward<CompletionToken>(token));

    asio::async_result<Handler> result(handler);

    if (!writer_helper.write_metadata()) {
        handler(system::error_code{http_errc::out_of_order});
        return result.get();
    }

    if (flags & HTTP_1_1 == 0) {
        handler(system::error_code{http_errc::native_stream_unsupported});
        return result.get();
    }

    auto crlf = string_literal_buffer("\r\n");
    auto sep = string_literal_buffer(": ");

    // because we don't create multiple responses at once with HTTP/1.1
    // pipelining, it's safe to use this "shared state"
    content_length_buffer = std::to_string(status_code) + ' ';

    const auto nbuffer_pieces =
        // Start line (http version + status code + reason phrase) + CRLF
        4
        // Headers
        // Each header is 4 buffer pieces: key + sep + value + crlf
        + 4 * message.headers().size()
        // Extra transfer-encoding header and extra CRLF for end of headers
        + 1;

    // TODO (C++14): replace by dynarray
    std::vector<asio::const_buffer> buffers(nbuffer_pieces);

    buffers.push_back(string_literal_buffer((flags & HTTP_1_1)
                                            ? "HTTP/1.1 " : "HTTP/1.0 "));
    buffers.push_back(asio::buffer(content_length_buffer));
    buffers.push_back(asio::buffer(reason_phrase.data(), reason_phrase.size()));
    buffers.push_back(crlf);

    for (const auto &header: message.headers()) {
        buffers.push_back(asio::buffer(header.first));
        buffers.push_back(sep);
        buffers.push_back(asio::buffer(header.second));
        buffers.push_back(crlf);
    }

    buffers.push_back(string_literal_buffer("transfer-encoding: chunked\r\n"
                                            "\r\n"));

    asio::async_write(channel, buffers,
                      [handler]
                      (const system::error_code &ec, std::size_t) mutable {
        handler(ec);
    });

    return result.get();
}

template<class Socket>
template<class Message, class CompletionToken>
typename asio::async_result<
    typename asio::handler_type<CompletionToken,
                                void(system::error_code)>::type>::type
basic_socket<Socket>::async_write(const Message &message,
                                  CompletionToken &&token)
{
    using detail::string_literal_buffer;
    typedef typename asio::handler_type<
        CompletionToken, void(system::error_code)>::type Handler;

    Handler handler(std::forward<CompletionToken>(token));

    asio::async_result<Handler> result(handler);

    if (!writer_helper.write()) {
        handler(system::error_code{http_errc::out_of_order});
        return result.get();
    }

    if (message.body().size() == 0) {
        handler(system::error_code{});
        return result.get();
    }

    auto crlf = string_literal_buffer("\r\n");

    {
        std::ostringstream ostr;
        ostr << std::hex << message.body().size();
        // because we don't create multiple responses at once with HTTP/1.1
        // pipelining, it's safe to use this "shared state"
        content_length_buffer = ostr.str();
    }

    std::array<boost::asio::const_buffer, 4> buffers = {
        asio::buffer(content_length_buffer),
        crlf,
        asio::buffer(message.body()),
        crlf
    };

    asio::async_write(channel, buffers,
                      [handler]
                      (const system::error_code &ec, std::size_t) mutable {
        handler(ec);
    });

    return result.get();
}

template<class Socket>
template<class Message, class CompletionToken>
typename asio::async_result<
    typename asio::handler_type<CompletionToken,
                                void(system::error_code)>::type>::type
basic_socket<Socket>::async_write_trailers(const Message &message,
                                           CompletionToken &&token)
{
    using detail::string_literal_buffer;
    typedef typename asio::handler_type<
        CompletionToken, void(system::error_code)>::type Handler;

    Handler handler(std::forward<CompletionToken>(token));

    asio::async_result<Handler> result(handler);

    if (!writer_helper.write_trailers()) {
        handler(system::error_code{http_errc::out_of_order});
        return result.get();
    }

    auto last_chunk = string_literal_buffer("0\r\n");
    auto crlf = string_literal_buffer("\r\n");
    auto sep = string_literal_buffer(": ");

    const auto nbuffer_pieces =
        // last_chunk
        1
        // Trailers
        // Each header is 4 buffer pieces: key + sep + value + crlf
        + 4 * message.trailers().size()
        // Final CRLF for end of trailers
        + 1;

    // TODO (C++14): replace by dynarray
    std::vector<asio::const_buffer> buffers(nbuffer_pieces);

    buffers.push_back(last_chunk);

    for (const auto &header: message.trailers()) {
        buffers.push_back(asio::buffer(header.first));
        buffers.push_back(sep);
        buffers.push_back(asio::buffer(header.second));
        buffers.push_back(crlf);
    }

    buffers.push_back(crlf);

    auto flags = this->flags;
    asio::async_write(channel, buffers,
                      [handler,flags]
                      (const system::error_code &ec, std::size_t) mutable {
        if (ec || flags & KEEP_ALIVE)
            handler(ec);
        else
            handler(system::error_code{http_errc::stream_finished});
    });

    return result.get();
}

template<class Socket>
template<class CompletionToken>
typename asio::async_result<
    typename asio::handler_type<CompletionToken,
                                void(system::error_code)>::type>::type
basic_socket<Socket>
::async_write_end_of_message(CompletionToken &&token)
{
    using detail::string_literal_buffer;
    typedef typename asio::handler_type<
        CompletionToken, void(system::error_code)>::type Handler;

    Handler handler(std::forward<CompletionToken>(token));

    asio::async_result<Handler> result(handler);

    if (!writer_helper.end()) {
        handler(system::error_code{http_errc::out_of_order});
        return result.get();
    }

    auto last_chunk = string_literal_buffer("0\r\n\r\n");

    auto flags = this->flags;
    asio::async_write(channel, last_chunk,
                      [handler,flags]
                      (const system::error_code &ec, std::size_t) mutable {
        if (ec || flags & KEEP_ALIVE)
            handler(ec);
        else
            handler(system::error_code{http_errc::stream_finished});
    });

    return result.get();
}

template<class Socket>
template<class... Args>
basic_socket<Socket>
::basic_socket(boost::asio::io_service &io_service, Args&&... args)
    : channel(io_service, std::forward<Args>(args)...)
    , istate(http::read_state::empty)
    , writer_helper(http::write_state::empty)
{
    detail::init(parser);
    parser.data = this;
}

template<class Socket>
basic_socket<Socket>::basic_socket(boost::asio::io_service &io_service) :
    channel(io_service),
    buffer_if_not_provided(new char[BOOST_HTTP_SOCKET_DEFAULT_BUFFER_SIZE]),
    istate(http::read_state::empty),
    buffer(boost::asio::buffer(buffer_if_not_provided,
                               BOOST_HTTP_SOCKET_DEFAULT_BUFFER_SIZE)),
    writer_helper(http::write_state::empty)
{
    static_assert(BOOST_HTTP_SOCKET_DEFAULT_BUFFER_SIZE != 0,
                  "buffers must not be 0-sized");

    detail::init(parser);
    parser.data = this;
}

template<class Socket>
template<class... Args, class>
basic_socket<Socket>::basic_socket(Args&&... args)
    : channel(std::forward<Args>(args)...)
    , buffer_if_not_provided(new char[BOOST_HTTP_SOCKET_DEFAULT_BUFFER_SIZE])
    , istate(http::read_state::empty)
    , buffer(boost::asio::buffer(buffer_if_not_provided,
                                 BOOST_HTTP_SOCKET_DEFAULT_BUFFER_SIZE))
    , writer_helper(http::write_state::empty)
{
    static_assert(BOOST_HTTP_SOCKET_DEFAULT_BUFFER_SIZE != 0,
                  "buffers must not be 0-sized");

    detail::init(parser);
    parser.data = this;
}

template<class Socket>
basic_socket<Socket>::~basic_socket()
{
    delete[] buffer_if_not_provided;
}

template<class Socket>
Socket &basic_socket<Socket>::next_layer()
{
    return channel;
}

template<class Socket>
const Socket &basic_socket<Socket>::next_layer() const
{
    return channel;
}

template<class Socket>
template<int target, class Message, class Handler, class String>
void basic_socket<Socket>
::schedule_on_async_read_message(Handler &handler, Message &message,
                                 String *method, String *path)
{
    if (used_size) {
        // Have cached some bytes from a previous read
        on_async_read_message<target>(std::move(handler), method, path, message,
                                         system::error_code{}, 0);
    } else {
        // TODO (C++14): move in lambda capture list
        channel.async_read_some(asio::buffer(&read_buffer[used_size],
                                             read_buffer.size() - used_size),
                                [this,handler,method,path,&message]
                                (const system::error_code &ec,
                                 std::size_t bytes_transferred) mutable {
            on_async_read_message<target>(std::move(handler), method, path,
                                          message, ec, bytes_transferred);
        });
    }
}

template<class Socket>
template<int target, class Message, class Handler, class String>
void basic_socket<Socket>
::on_async_read_message(Handler handler, String *method, String *path,
                        Message &message, const system::error_code &ec,
                        std::size_t bytes_transferred)
{
    using detail::string_literal_buffer;

    if (ec) {
        clear_buffer();
        handler(ec);
        return;
    }

    used_size += bytes_transferred;
    current_method = reinterpret_cast<void*>(method);
    current_path = reinterpret_cast<void*>(path);
    current_message = reinterpret_cast<void*>(&message);
    auto nparsed = detail::execute(parser, settings<Message, String>(),
                                   read_buffer.data(),
                                   used_size);

    if (parser.http_errno) {
        system::error_code ignored_ec;

        if (parser.http_errno
            == int(detail::parser_error::cb_headers_complete)) {
            clear_buffer();
            clear_message(message);

            auto error_message
                = string_literal_buffer("HTTP/1.1 505 HTTP Version Not"
                                        " Supported\r\n"
                                        "Content-Length: 48\r\n"
                                        "Connection: close\r\n"
                                        "\r\n"
                                        "This server only supports HTTP/1.0 and"
                                        " HTTP/1.1\n");
            asio::async_write(channel, asio::buffer(error_message),
                              [this,handler](system::error_code ignored_ec,
                                             std::size_t bytes_transferred)
                              mutable {
                handler(system::error_code{http_errc::parsing_error});
            });
            return;
        } else if (parser.http_errno
                   == int(detail::parser_error::cb_message_complete)) {
            /* After an error is set, http_parser enter in an invalid state
               and needs to be reset. */
            detail::init(parser);
        } else {
            clear_buffer();
            handler(system::error_code(http_errc::parsing_error));
            return;
        }
    }

    std::copy_n(&read_buffer[nparsed], used_size - nparsed, read_buffer.begin());

    used_size -= nparsed;

    if (target == READY && flags & READY) {
        flags &= ~READY;
        handler(system::error_code{});
    } else if (target == DATA && flags & (DATA|END)) {
        flags &= ~(READY|DATA);
        handler(system::error_code{});
    } else if (target == END && flags & END) {
        flags &= ~(READY|DATA|END);
        handler(system::error_code{});
    } else {
        if (used_size == read_buffer.size()) {
            handler(system::error_code{http_errc::buffer_exhausted});
            return;
        }

        // TODO (C++14): move in lambda capture list
        channel.async_read_some(asio::buffer(&read_buffer[used_size],
                                             read_buffer.size() - used_size),
                                [this,handler,method,path,&message]
                                (const system::error_code &ec,
                                 std::size_t bytes_transferred) mutable {
            on_async_read_message<target>(std::move(handler), method, path,
                                          message, ec, bytes_transferred);
        });
    }
}

template<class Socket>
template<class Message, class String>
detail::http_parser_settings basic_socket<Socket>::settings()
{
    http_parser_settings settings;

    settings.on_message_begin = on_message_begin<Message>;
    settings.on_url = on_url<Message, String>;
    settings.on_header_field = on_header_field<Message>;
    settings.on_header_value = on_header_value<Message>;
    settings.on_headers_complete = on_headers_complete<Message, String>;
    settings.on_body = on_body<Message>;
    settings.on_message_complete = on_message_complete<Message>;

    return settings;
}

template<class Socket>
template<class Message>
int basic_socket<Socket>::on_message_begin(http_parser *parser)
{
    auto socket = reinterpret_cast<basic_socket*>(parser->data);
    auto message = reinterpret_cast<Message*>(socket->current_message);
    socket->flags = 0;
    socket->use_trailers = false;
    clear_message(*message);
    return 0;
}

template<class Socket>
template<class Message, class String>
int basic_socket<Socket>::on_url(http_parser *parser, const char *at,
                                           std::size_t size)
{
    auto socket = reinterpret_cast<basic_socket*>(parser->data);
    auto path = reinterpret_cast<String*>(socket->current_path);
    path->append(at, size);
    return 0;
}

template<class Socket>
template<class Message>
int basic_socket<Socket>
::on_header_field(http_parser *parser, const char *at, std::size_t size)
{
    using std::transform;
    auto tolower = [](int ch) -> int { return std::tolower(ch); };

    auto socket = reinterpret_cast<basic_socket*>(parser->data);
    auto message = reinterpret_cast<Message*>(socket->current_message);
    auto &field = socket->last_header.first;
    auto &value = socket->last_header.second;

    if (value.size() /* last header piece was value */) {
        if (!socket->use_trailers)
            message->headers().insert(socket->last_header);
        else
            message->trailers().insert(socket->last_header);
        value.clear();

        field.replace(0, field.size(), at, size);
        transform(field.begin(), field.end(), field.begin(), tolower);
    } else {
        auto offset = field.size();
        field.append(at, size);
        auto begin = field.begin() + offset;
        transform(begin, field.end(), begin, tolower);
    }

    return 0;
}

template<class Socket>
template<class Message>
int basic_socket<Socket>
::on_header_value(http_parser *parser, const char *at, std::size_t size)
{
    auto socket = reinterpret_cast<basic_socket*>(parser->data);
    auto &value = socket->last_header.second;
    value.append(at, size);
    return 0;
}

template<class Socket>
template<class Message, class String>
int basic_socket<Socket>::on_headers_complete(http_parser *parser)
{
    auto socket = reinterpret_cast<basic_socket*>(parser->data);
    auto message = reinterpret_cast<Message*>(socket->current_message);

    {
        auto method = reinterpret_cast<String*>(socket->current_method);
        using detail::constchar_helper;
        static const constchar_helper methods[] = {
            "DELETE",
            "GET",
            "HEAD",
            "POST",
            "PUT",
            "CONNECT",
            "OPTIONS",
            "TRACE",
            "COPY",
            "LOCK",
            "MKCOL",
            "MOVE",
            "PROPFIND",
            "PROPPATCH",
            "SEARCH",
            "UNLOCK",
            "REPORT",
            "MKACTIVITY",
            "CHECKOUT",
            "MERGE",
            "M-SEARCH",
            "NOTIFY",
            "SUBSCRIBE",
            "UNSUBSCRIBE",
            "PATCH",
            "PURGE"
        };
        const auto &m = methods[parser->method];
        method->append(m.data, m.size);
        socket->connect_request = parser->method == 5;
    }

    {
        auto handle_error = [](){
            /* WARNING: if you update the code and another error condition
               become possible, you'll be in trouble, because, as I write
               this comment, there is **NOT** a non-hacky way to notify
               different error conditions through the callback return value
               (Ryan Dahl's parser limitation, probably designed in favor of
               lower memory consumption) and then you'll need to call the
               type erased user handler from this very function. One
               solution with minimal performance impact to this future
               problem is presented below.

               First, update this function signature to also remember the
               erased type:

               ```cpp
               template<class Message, class Handler>
               static int on_headers_complete(http_parser *parser)
               ```

               Now add the following member to this class:

               ```cpp
               void *handler;
               ```

               Before the execute function is called, update the previously
               declared member:

               ```cpp
               this->handler = &handler;
               ```
               Now use the following to call the type erased user function
               from this very function:

               ```cpp
               auto &handler = *reinterpret_cast<Handler*>(socket->handler);
               handler(system::error_code{http_errc::parsing_error});
               ``` */
            /* We don't use http_parser_pause, because it looks error-prone:
               <https://github.com/joyent/http-parser/issues/97>. */
            return -1;
        };
        switch (parser->http_major) {
        case 1:
            if (parser->http_minor != 0)
                socket->flags |= HTTP_1_1;

            break;
        default:
            return handle_error();
        }
    }

    message->headers().insert(socket->last_header);
    socket->last_header.first.clear();
    socket->last_header.second.clear();
    socket->use_trailers = true;
    socket->istate = http::read_state::message_ready;
    socket->flags |= READY;

    return 0;
}

template<class Socket>
template<class Message>
int basic_socket<Socket>
::on_body(http_parser *parser, const char *data, std::size_t size)
{
    auto socket = reinterpret_cast<basic_socket*>(parser->data);
    auto message = reinterpret_cast<Message*>(socket->current_message);
    auto begin = reinterpret_cast<const std::uint8_t*>(data);
    message->body().insert(message->body().end(), begin, begin + size);
    socket->flags |= DATA;

    if (detail::body_is_final(*parser))
        socket->istate = http::read_state::body_ready;

    return 0;
}

template<class Socket>
template<class Message>
int basic_socket<Socket>::on_message_complete(http_parser *parser)
{
    auto socket = reinterpret_cast<basic_socket*>(parser->data);
    auto message = reinterpret_cast<Message*>(socket->current_message);
    message->trailers().insert(socket->last_header);
    socket->last_header.first.clear();
    socket->last_header.second.clear();
    socket->istate = http::read_state::empty;
    socket->use_trailers = false;
    socket->flags |= END;

    if (detail::should_keep_alive(*parser))
        socket->flags |= KEEP_ALIVE;

    /* To avoid passively parsing pipelined message ahead-of-asked, we
       signalize error to stop parsing. */
    return parser->upgrade ? -1 : 0;
}

template<class Socket>
void basic_socket<Socket>::clear_buffer()
{
    istate = http::read_state::empty;
    writer_helper.state = http::write_state::empty;
    used_size = 0;
    detail::init(parser);
}

template<class Socket>
template<class Message>
void basic_socket<Socket>::clear_message(Message &message)
{
    message.headers().clear();
    message.body().clear();
    message.trailers().clear();
}

} // namespace boost
} // namespace http
