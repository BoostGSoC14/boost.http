namespace boost {
namespace http {

namespace detail {

template <class Headers>
bool has_connection_close(const Headers &headers)
{
    typedef basic_string_ref<typename Headers::mapped_type::value_type>
        string_ref_type;

    auto range = headers.equal_range("connection");
    for (; range.first != range.second ; ++range.first) {
        if (header_value_any_of((*range.first).second,
                                [](const string_ref_type &v) {
                                    return iequals(v, "close");
                                })) {
            return true;
        }
    }

    return false;
}

} // namespace detail

template<class Socket>
bool basic_socket<Socket>::is_open() const
{
    return channel.is_open() && is_open_;
}

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
    return modern_http;
}

template<class Socket>
asio::io_service &basic_socket<Socket>::get_io_service()
{
    return channel.get_io_service();
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
    static_assert(is_message<Message>::value,
                  "Message must fulfill the Message concept");

    typedef typename asio::handler_type<
        CompletionToken, void(system::error_code)>::type Handler;

    Handler handler(std::forward<CompletionToken>(token));

    asio::async_result<Handler> result(handler);

    if (istate != http::read_state::empty) {
        invoke_handler(std::forward<decltype(handler)>(handler),
                       http_errc::out_of_order);
        return result.get();
    }

    method.clear();
    path.clear();
    clear_message(message);
    writer_helper = http::write_state::finished;
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
    static_assert(is_message<Message>::value,
                  "Message must fulfill the Message concept");

    typedef typename asio::handler_type<
        CompletionToken, void(system::error_code)>::type Handler;

    Handler handler(std::forward<CompletionToken>(token));

    asio::async_result<Handler> result(handler);

    if (istate != http::read_state::message_ready) {
        invoke_handler(std::forward<decltype(handler)>(handler),
                       http_errc::out_of_order);
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
    static_assert(is_message<Message>::value,
                  "Message must fulfill the Message concept");

    typedef typename asio::handler_type<
        CompletionToken, void(system::error_code)>::type Handler;

    Handler handler(std::forward<CompletionToken>(token));

    asio::async_result<Handler> result(handler);

    if (istate != http::read_state::body_ready) {
        invoke_handler(std::forward<decltype(handler)>(handler),
                       http_errc::out_of_order);
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
    static_assert(is_message<Message>::value,
                  "Message must fulfill the Message concept");

    using detail::string_literal_buffer;
    typedef typename asio::handler_type<
        CompletionToken, void(system::error_code)>::type Handler;

    Handler handler(std::forward<CompletionToken>(token));
    asio::async_result<Handler> result(handler);

    if (!writer_helper.write_message()) {
        invoke_handler(std::forward<decltype(handler)>(handler),
                       http_errc::out_of_order);
        return result.get();
    }

    auto crlf = string_literal_buffer("\r\n");
    auto sep = string_literal_buffer(": ");
    bool implicit_content_length
        = (message.headers().find("content-length") != message.headers().end())
        || (status_code / 100 == 1) || (status_code == 204)
        || (connect_request && (status_code / 100 == 2));
    auto has_connection_close = detail::has_connection_close(message.headers());

    if (has_connection_close)
        keep_alive = KEEP_ALIVE_CLOSE_READ;

    auto use_connection_close_buf = (keep_alive == KEEP_ALIVE_CLOSE_READ)
        && !has_connection_close;

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
        // If user didn't provided "connection: close"
        + (use_connection_close_buf ? 1 : 0)
        // Each header is 4 buffer pieces: key + sep + value + crlf
        + 4 * message.headers().size()
        // Extra content-length header uses 3 pieces
        + (implicit_content_length ? 0 : 3)
        // Extra CRLF for end of headers
        + 1
        // And finally, the message body
        + (implicit_content_length ? 0 : 1);

    // TODO (C++14): replace by dynarray
    std::vector<asio::const_buffer> buffers;
    buffers.reserve(nbuffer_pieces);

    buffers.push_back(modern_http ? string_literal_buffer("HTTP/1.1 ")
                      : string_literal_buffer("HTTP/1.0 "));
    buffers.push_back(asio::buffer(content_length_buffer.data(),
                                   content_length_delim));
    buffers.push_back(asio::buffer(reason_phrase.data(), reason_phrase.size()));
    buffers.push_back(crlf);

    if (use_connection_close_buf)
        buffers.push_back(string_literal_buffer("connection: close\r\n"));

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

    asio::async_write(channel, buffers,
                      [handler,this]
                      (const system::error_code &ec, std::size_t) mutable {
        is_open_ = keep_alive == KEEP_ALIVE_KEEP_ALIVE_READ;
        if (!is_open_)
            channel.close();
        handler(ec);
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
        invoke_handler(std::forward<decltype(handler)>(handler),
                       http_errc::out_of_order);
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
    static_assert(is_message<Message>::value,
                  "Message must fulfill the Message concept");

    using detail::string_literal_buffer;
    typedef typename asio::handler_type<
        CompletionToken, void(system::error_code)>::type Handler;

    Handler handler(std::forward<CompletionToken>(token));

    asio::async_result<Handler> result(handler);

    {
        auto prev = writer_helper.state;
        if (!writer_helper.write_metadata()) {
            invoke_handler(std::forward<decltype(handler)>(handler),
                           http_errc::out_of_order);
            return result.get();
        }

        if (!modern_http) {
            writer_helper = prev;
            invoke_handler(std::forward<decltype(handler)>(handler),
                           http_errc::native_stream_unsupported);
            return result.get();
        }
    }

    auto crlf = string_literal_buffer("\r\n");
    auto sep = string_literal_buffer(": ");
    auto has_connection_close = detail::has_connection_close(message.headers());

    if (has_connection_close)
        keep_alive = KEEP_ALIVE_CLOSE_READ;

    auto use_connection_close_buf = (keep_alive == KEEP_ALIVE_CLOSE_READ)
        && !has_connection_close;

    // because we don't create multiple responses at once with HTTP/1.1
    // pipelining, it's safe to use this "shared state"
    content_length_buffer = std::to_string(status_code) + ' ';

    const auto nbuffer_pieces =
        // Start line (http version + status code + reason phrase) + CRLF
        4
        // Headers
        // If user didn't provided "connection: close"
        + (use_connection_close_buf ? 1 : 0)
        // Each header is 4 buffer pieces: key + sep + value + crlf
        + 4 * message.headers().size()
        // Extra transfer-encoding header and extra CRLF for end of headers
        + 1;

    // TODO (C++14): replace by dynarray
    std::vector<asio::const_buffer> buffers;
    buffers.reserve(nbuffer_pieces);

    buffers.push_back(modern_http ? string_literal_buffer("HTTP/1.1 ")
                      : string_literal_buffer("HTTP/1.0 "));
    buffers.push_back(asio::buffer(content_length_buffer));
    buffers.push_back(asio::buffer(reason_phrase.data(), reason_phrase.size()));
    buffers.push_back(crlf);

    if (use_connection_close_buf)
        buffers.push_back(string_literal_buffer("connection: close\r\n"));

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
    static_assert(is_message<Message>::value,
                  "Message must fulfill the Message concept");

    using detail::string_literal_buffer;
    typedef typename asio::handler_type<
        CompletionToken, void(system::error_code)>::type Handler;

    Handler handler(std::forward<CompletionToken>(token));

    asio::async_result<Handler> result(handler);

    if (!writer_helper.write()) {
        invoke_handler(std::forward<decltype(handler)>(handler),
                       http_errc::out_of_order);
        return result.get();
    }

    if (message.body().size() == 0) {
        invoke_handler(std::forward<decltype(handler)>(handler));
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
    static_assert(is_message<Message>::value,
                  "Message must fulfill the Message concept");

    using detail::string_literal_buffer;
    typedef typename asio::handler_type<
        CompletionToken, void(system::error_code)>::type Handler;

    Handler handler(std::forward<CompletionToken>(token));

    asio::async_result<Handler> result(handler);

    if (!writer_helper.write_trailers()) {
        invoke_handler(std::forward<decltype(handler)>(handler),
                       http_errc::out_of_order);
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
    std::vector<asio::const_buffer> buffers;
    buffers.reserve(nbuffer_pieces);

    buffers.push_back(last_chunk);

    for (const auto &header: message.trailers()) {
        buffers.push_back(asio::buffer(header.first));
        buffers.push_back(sep);
        buffers.push_back(asio::buffer(header.second));
        buffers.push_back(crlf);
    }

    buffers.push_back(crlf);

    asio::async_write(channel, buffers,
                      [handler,this]
                      (const system::error_code &ec, std::size_t) mutable {
        is_open_ = keep_alive == KEEP_ALIVE_KEEP_ALIVE_READ;
        if (!is_open_)
            channel.close();
        handler(ec);
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
        invoke_handler(std::forward<decltype(handler)>(handler),
                       http_errc::out_of_order);
        return result.get();
    }

    auto last_chunk = string_literal_buffer("0\r\n\r\n");

    asio::async_write(channel, last_chunk,
                      [handler,this]
                      (const system::error_code &ec, std::size_t) mutable {
        is_open_ = keep_alive == KEEP_ALIVE_KEEP_ALIVE_READ;
        if (!is_open_)
            channel.close();
        handler(ec);
    });

    return result.get();
}

template<class Socket>
basic_socket<Socket>
::basic_socket(boost::asio::io_service &io_service,
               boost::asio::mutable_buffer inbuffer) :
    channel(io_service),
    istate(http::read_state::empty),
    buffer(inbuffer),
    writer_helper(http::write_state::empty)
{
    if (asio::buffer_size(buffer) == 0)
        throw std::invalid_argument("buffers must not be 0-sized");
}

template<class Socket>
template<class... Args>
basic_socket<Socket>
::basic_socket(boost::asio::mutable_buffer inbuffer, Args&&... args)
    : channel(std::forward<Args>(args)...)
    , istate(http::read_state::empty)
    , buffer(inbuffer)
    , writer_helper(http::write_state::empty)
{
    if (asio::buffer_size(buffer) == 0)
        throw std::invalid_argument("buffers must not be 0-sized");
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
void basic_socket<Socket>::open()
{
    is_open_ = true;
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
        channel.async_read_some(asio::buffer(buffer + used_size),
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
    if (ec) {
        clear_buffer();
        handler(ec);
        return;
    }

    used_size += bytes_transferred;
    if (expecting_field) {
        /* We complicate field management to avoid allocations. The field name
           MUST occupy the initial bytes on the buffer. The bytes that
           immediately follow should be the field value (i.e. remove any `skip`
           that appears between name and value). */
        parser.set_buffer(asio::buffer(buffer + field_name_size,
                                       used_size - field_name_size));
    } else {
        parser.set_buffer(asio::buffer(buffer, used_size));
    }

    std::size_t nparsed = expecting_field ? field_name_size : 0;
    std::size_t field_name_begin = 0;
    int flags = 0;

    /* Buffer management is simplified by making `error_insufficient_data` the
       only way to break out of this loop (on non-error paths). */
    do {
        parser.next();
        switch (parser.code()) {
        case token::code::error_insufficient_data:
            // break of for loop completely
            continue;
        case token::code::error_set_method:
            BOOST_HTTP_DETAIL_UNREACHABLE("client API not implemented yet");
            break;
        case token::code::error_use_another_connection:
            BOOST_HTTP_DETAIL_UNREACHABLE("client API not implemented yet");
            break;
        case token::code::error_invalid_data:
        case token::code::error_no_host:
        case token::code::error_invalid_content_length:
        case token::code::error_content_length_overflow:
        case token::code::error_invalid_transfer_encoding:
        case token::code::error_chunk_size_overflow:
            // TODO: send HTTP reply before proceeding
            {
                clear_buffer();
                handler(system::error_code(http_errc::parsing_error));
                return;
            }
        case token::code::skip:
            break;
        case token::code::method:
            use_trailers = false;
            {
                auto value = parser.value<token::method>();
                connect_request = value == "CONNECT";
                *method = String(value.data(), value.size());
            }
            break;
        case token::code::request_target:
            {
                auto value = parser.value<token::request_target>();
                *path = String(value.data(), value.size());
            }
            break;
        case token::code::version:
            {
                auto value = parser.value<token::version>();
                modern_http = value > 0;
                keep_alive = KEEP_ALIVE_UNKNOWN;
            }
            break;
        case token::code::status_code:
            use_trailers = false;
            BOOST_HTTP_DETAIL_UNREACHABLE("unimplemented not exposed feature");
            break;
        case token::code::reason_phrase:
            BOOST_HTTP_DETAIL_UNREACHABLE("unimplemented not exposed feature");
            break;
        case token::code::field_name:
            {
                auto buf_view = asio::buffer_cast<char*>(buffer);
                field_name_begin = nparsed;
                field_name_size = parser.token_size();

                for (std::size_t i = 0 ; i != field_name_size ; ++i) {
                    auto &ch = buf_view[field_name_begin + i];
                    ch = std::tolower(ch);
                }

                expecting_field = true;
            }
            break;
        case token::code::field_value:
            {
                typedef typename Message::headers_type::key_type NameT;
                typedef typename Message::headers_type::mapped_type ValueT;

                auto buf_view = asio::buffer_cast<char*>(buffer);
                auto name = string_ref(buf_view + field_name_begin,
                                       field_name_size);
                auto value = parser.value<token::field_value>();

                if (modern_http || (name != "expect" && name != "upgrade")) {
                    if (name == "connection") {
                        switch (keep_alive) {
                        case KEEP_ALIVE_UNKNOWN:
                        case KEEP_ALIVE_KEEP_ALIVE_READ:
                            header_value_any_of(value, [&](string_ref v) {
                                    if (iequals(v, "close")) {
                                        keep_alive = KEEP_ALIVE_CLOSE_READ;
                                        return true;
                                    }

                                    if (iequals(v, "keep-alive"))
                                        keep_alive = KEEP_ALIVE_KEEP_ALIVE_READ;

                                    return false;
                                });
                            break;
                        case KEEP_ALIVE_CLOSE_READ:
                            break;
                        }
                    }

                    (use_trailers ? message.trailers() : message.headers())
                        .emplace(NameT(name.data(), name.size()),
                                 ValueT(value.data(), value.size()));
                }

                expecting_field = false;
            }
            break;
        case token::code::end_of_headers:
            istate = http::read_state::message_ready;
            flags |= READY;
            writer_helper = http::write_state::empty;

            {
                auto er = message.headers().equal_range("expect");
                if (std::distance(er.first, er.second) > 1)
                    message.headers().erase(er.first, er.second);
            }

            if (keep_alive == KEEP_ALIVE_UNKNOWN) {
                keep_alive = modern_http
                    ? KEEP_ALIVE_KEEP_ALIVE_READ : KEEP_ALIVE_CLOSE_READ;
            }
            break;
        case token::code::body_chunk:
            {
                auto value = parser.value<token::body_chunk>();
                auto begin = asio::buffer_cast<const std::uint8_t*>(value);
                auto size = asio::buffer_size(value);
                message.body().insert(message.body().end(), begin, begin + size);
                flags |= DATA;
            }
            break;
        case token::code::end_of_body:
            istate = http::read_state::body_ready;
            use_trailers = true;
            break;
        case token::code::end_of_message:
            istate = http::read_state::empty;
            flags |= END;
            parser.set_buffer(asio::buffer(buffer + nparsed,
                                           parser.token_size()));
            break;
        }

        nparsed += parser.token_size();
    } while (parser.code() != token::code::error_insufficient_data);

    if (!expecting_field) {
        auto buf_view = asio::buffer_cast<char*>(buffer);
        std::copy_n(buf_view + nparsed, used_size - nparsed, buf_view);
        used_size -= nparsed;
    } else if(nparsed != 0) {
        auto buf_view = asio::buffer_cast<char*>(buffer);
        std::copy_n(buf_view + field_name_begin, field_name_size, buf_view);
        std::copy_n(buf_view + nparsed, used_size - nparsed,
                    buf_view + field_name_size);
        used_size -= nparsed;
        used_size += field_name_size;
    }
    nparsed = 0;

    if (target == READY && flags & READY) {
        handler(system::error_code{});
    } else if (target == DATA && flags & (DATA|END)) {
        handler(system::error_code{});
    } else if (target == END && flags & END) {
        handler(system::error_code{});
    } else {
        if (used_size == asio::buffer_size(buffer)) {
            /* TODO: use `expected_token()` to reply with appropriate "... too
               long" status code */
            handler(system::error_code{http_errc::buffer_exhausted});
            return;
        }

        // TODO (C++14): move in lambda capture list
        channel.async_read_some(asio::buffer(buffer + used_size),
                                [this,handler,method,path,&message]
                                (const system::error_code &ec,
                                 std::size_t bytes_transferred) mutable {
            on_async_read_message<target>(std::move(handler), method, path,
                                          message, ec, bytes_transferred);
        });
    }
}

template<class Socket>
void basic_socket<Socket>::clear_buffer()
{
    istate = http::read_state::empty;
    writer_helper.state = http::write_state::empty;
    used_size = 0;
    parser.reset();
    expecting_field = false;
}

template<class Socket>
template<class Message>
void basic_socket<Socket>::clear_message(Message &message)
{
    message.headers().clear();
    message.body().clear();
    message.trailers().clear();
}

template<class Socket>
template <typename Handler,
          typename ErrorCode>
void basic_socket<Socket>::invoke_handler(Handler&& handler,
                                          ErrorCode error)
{
    channel.get_io_service().post
        ([handler, error] () mutable
         {
             handler(make_error_code(error));
         });
}

template<class Socket>
template <class Handler>
void basic_socket<Socket>::invoke_handler(Handler&& handler)
{
    channel.get_io_service().post
        ([handler] () mutable
         {
             handler(system::error_code{});
         });
}

} // namespace boost
} // namespace http
