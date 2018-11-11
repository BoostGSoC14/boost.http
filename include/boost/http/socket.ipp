namespace boost {
namespace http {

namespace detail {

template <class Headers>
bool has_connection_close(const Headers &headers)
{
    typedef basic_string_view<typename Headers::mapped_type::value_type>
        string_view_type;

    auto range = headers.equal_range("connection");
    for (; range.first != range.second ; ++range.first) {
        if (header_value_any_of((*range.first).second,
                                [](const string_view_type &v) {
                                    return iequals(v, "close");
                                })) {
            return true;
        }
    }

    return false;
}

} // namespace detail

template<class Socket, class Settings>
bool basic_socket<Socket, Settings>::is_open() const
{
    return channel.is_open() && is_open_;
}

template<class Socket, class Settings>
read_state basic_socket<Socket, Settings>::read_state() const
{
    return istate;
}

template<class Socket, class Settings>
write_state basic_socket<Socket, Settings>::write_state() const
{
    return writer_helper.state;
}

template<class Socket, class Settings>
bool basic_socket<Socket, Settings>::write_response_native_stream() const
{
    return modern_http;
}

template<class Socket, class Settings>
typename basic_socket<Socket, Settings>::executor_type
basic_socket<Socket, Settings>::get_executor()
{
    return channel.get_executor();
}

template<class Socket, class Settings>
template<class Request, class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(system::error_code))
basic_socket<Socket, Settings>::async_read_request(Request &request,
                                                   CompletionToken &&token)
{
    static_assert(is_request_message<Request>::value,
                  "Request must fulfill the Request concept");

    boost::asio::async_completion<CompletionToken, void(system::error_code)>
        init{token};

    switch (parser.which()) {
    case 0: // none
        parser = req_parser{};
        modern_http = true; // ignore `lock_client_to_http10`
        break;
    case 1: // req_parser
        break;
    case 2: // res_parser
        invoke_handler(std::move(init.completion_handler),
                       http_errc::wrong_direction);
        return init.result.get();
    }

    if (istate != http::read_state::empty) {
        invoke_handler(std::move(init.completion_handler),
                       http_errc::out_of_order);
        return init.result.get();
    }

    request.method().clear();
    request.target().clear();
    clear_message(request);
    writer_helper = http::write_state::finished;
    schedule_on_async_read_message<false>(std::move(init.completion_handler),
                                          request);

    return init.result.get();
}

template<class Socket, class Settings>
template<class Response, class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(system::error_code))
basic_socket<Socket, Settings>::async_read_response(Response &response,
                                                    CompletionToken &&token)
{
    static_assert(is_response_message<Response>::value,
                  "Response must fulfill the Response concept");

    boost::asio::async_completion<CompletionToken, void(system::error_code)>
        init{token};

    switch (parser.which()) {
    case 0: // none
        parser = res_parser{};
        break;
    case 1: // req_parser
        invoke_handler(std::move(init.completion_handler),
                       http_errc::wrong_direction);
        return init.result.get();
    case 2: // res_parser
        break;
    }

    if (istate != http::read_state::empty) {
        invoke_handler(std::move(init.completion_handler),
                       http_errc::out_of_order);
        return init.result.get();
    }

    response.reason_phrase().clear();
    clear_message(response);
    schedule_on_async_read_message<false>(std::move(init.completion_handler),
                                          response);

    return init.result.get();
}

template<class Socket, class Settings>
template<class Message, class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(system::error_code))
basic_socket<Socket, Settings>::async_read_some(Message &message,
                                                CompletionToken &&token)
{
    static_assert(is_message<Message>::value,
                  "Message must fulfill the Message concept");

    boost::asio::async_completion<CompletionToken, void(system::error_code)>
        init{token};

    if (istate != http::read_state::message_ready) {
        invoke_handler(std::move(init.completion_handler),
                       http_errc::out_of_order);
        return init.result.get();
    }

    schedule_on_async_read_message<false>(std::move(init.completion_handler),
                                          message);

    return init.result.get();
}

template<class Socket, class Settings>
template<class Message, class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken,
                              void(system::error_code, std::size_t))
basic_socket<Socket, Settings>::async_read_chunkext(
    Message &message, typename Message::headers_type &chunkext,
    CompletionToken &&token
)
{
    static_assert(is_message<Message>::value,
                  "Message must fulfill the Message concept");

    boost::asio::async_completion<
        CompletionToken, void(system::error_code, std::size_t)> init{token};

    if (istate != http::read_state::message_ready) {
        invoke_handler(std::move(init.completion_handler),
                       http_errc::out_of_order,
                       0);
        return init.result.get();
    }

    chunkext.clear();
    schedule_on_async_read_message<true>(std::move(init.completion_handler),
                                         message, &chunkext);

    return init.result.get();
}

template<class Socket, class Settings>
template<class Message, class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(system::error_code))
basic_socket<Socket, Settings>::async_read_trailers(Message &message,
                                                    CompletionToken &&token)
{
    static_assert(is_message<Message>::value,
                  "Message must fulfill the Message concept");

    boost::asio::async_completion<CompletionToken, void(system::error_code)>
        init{token};

    if (istate != http::read_state::body_ready) {
        invoke_handler(std::move(init.completion_handler),
                       http_errc::out_of_order);
        return init.result.get();
    }

    schedule_on_async_read_message<false>(std::move(init.completion_handler),
                                          message);

    return init.result.get();
}

template<class Socket, class Settings>
template<class Response, class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(system::error_code))
basic_socket<Socket, Settings>
::async_write_response(const Response &response, CompletionToken &&token)
{
    static_assert(is_response_message<Response>::value,
                  "Response must fulfill the Response concept");

    boost::asio::async_completion<CompletionToken, void(system::error_code)>
        init{token};

    if (parser.which() != 1) { //< !req_parser
        invoke_handler(std::move(init.completion_handler),
                       http_errc::wrong_direction);
        return init.result.get();
    }

    auto prev_state = writer_helper.state;
    if (!writer_helper.write_message()) {
        invoke_handler(std::move(init.completion_handler),
                       http_errc::out_of_order);
        return init.result.get();
    }
    istate = http::read_state::empty;

    const auto status_code = response.status_code();
    const auto &reason_phrase = response.reason_phrase();
    const auto &headers = response.headers();

    bool implicit_content_length
        = (headers.find("content-length") != headers.end())
        || (status_code / 100 == 1) || (status_code == 204)
        || (connect_request && (status_code / 100 == 2));
    auto has_connection_close = detail::has_connection_close(headers);

    if (has_connection_close)
        keep_alive = KEEP_ALIVE_CLOSE_READ;

    auto use_connection_close_buf = (keep_alive == KEEP_ALIVE_CLOSE_READ)
        && !has_connection_close;

    const bool copy_body
        = (response.body().size() < Settings::body_copy_threshold)
        || (response.body().size() == 0);

    std::size_t size =
        // Start line (http version + status code + reason phrase) + CRLF
        string_view("HTTP/1.1 NNN ").size() + reason_phrase.size() + 2
        // If user didn't provided "connection: close"
        + (use_connection_close_buf
           ? string_view("connection: close\r\n").size()
           : 0)
        // Headers are computed later
        + 0
        // Extra CRLF for end of headers
        + 2
        // And finally, the message body
        + ((implicit_content_length || !copy_body)
           ? 0 : response.body().size());
    std::size_t content_length_ndigits = 0;

    // Headers
    for (const auto &e: headers) {
        // Each header is 4 buffer pieces: key + sep + value + CRLF
        // sep (": ") + CRLF is always 4 bytes.
        size += 4 + e.first.size() + e.second.size();
    }

    // "content-length" header
    if (!implicit_content_length) {
        content_length_ndigits
            = detail::count_decdigits(response.body().size());
        size += string_view("content-length: \r\n").size()
            + content_length_ndigits;
    }

    char *buf = NULL;
    std::size_t idx = 0;
    try {
        buf = reinterpret_cast<char*>(boost::asio::asio_handler_allocate(
            size, &init.completion_handler)
        );
    } catch (const std::bad_alloc&) {
        writer_helper.state = prev_state;
        throw;
    } catch (...) {
        assert(false);
    }

    {
        string_view x;

        if (modern_http)
            x = "HTTP/1.1 ";
        else
            x = "HTTP/1.0 ";

        std::memcpy(buf + idx, x.data(), x.size());
        idx += x.size();
    }

    buf[idx++] = (status_code / 100) + '0';
    buf[idx++] = (status_code / 10) % 10 + '0';
    buf[idx++] = (status_code % 10) + '0';
    buf[idx++] = ' ';

    std::memcpy(buf + idx, reason_phrase.data(), reason_phrase.size());
    idx += reason_phrase.size();

    std::memcpy(buf + idx, "\r\n", 2);
    idx += 2;

    if (use_connection_close_buf) {
        string_view x = "connection: close\r\n";
        std::memcpy(buf + idx, x.data(), x.size());
        idx += x.size();
    }

    for (const auto &e: headers) {
        std::memcpy(buf + idx, e.first.data(), e.first.size());
        idx += e.first.size();

        std::memcpy(buf + idx, ": ", 2);
        idx += 2;

        std::memcpy(buf + idx, e.second.data(), e.second.size());
        idx += e.second.size();

        std::memcpy(buf + idx, "\r\n", 2);
        idx += 2;
    }

    if (!implicit_content_length) {
        string_view x("content-length: ");
        std::memcpy(buf + idx, x.data(), x.size());
        idx += x.size();

        auto body_size = response.body().size();
        for (auto i = content_length_ndigits ; i != 0 ; --i) {
            buf[idx + i - 1] = (body_size % 10) + '0';
            body_size /= 10;
        }
        idx += content_length_ndigits;

        std::memcpy(buf + idx, "\r\n", 2);
        idx += 2;
    }

    std::memcpy(buf + idx, "\r\n", 2);
    idx += 2;

    if (!implicit_content_length && copy_body) {
        std::copy(response.body().begin(), response.body().end(), buf + idx);
        idx += response.body().size();
    }

    assert(size == idx);

    auto handler = std::move(init.completion_handler);
    if (copy_body) {
        boost::asio::async_write(
            channel, boost::asio::buffer(buf, size),
            [handler,buf,size,this]
            (const system::error_code &ec, std::size_t) mutable {
                boost::asio::asio_handler_deallocate(buf, size, &handler);
                is_open_ = keep_alive == KEEP_ALIVE_KEEP_ALIVE_READ;
                if (!is_open_)
                    channel.lowest_layer().close();
                handler(ec);
            }
        );
    } else {
        auto body_buf = boost::asio::buffer(
            response.body().data(), response.body().size()
        );
        boost::asio::async_write(
            channel, boost::asio::buffer(buf, size),
            [handler,buf,size,body_buf,this]
            (const system::error_code &ec, std::size_t) mutable {
                boost::asio::asio_handler_deallocate(buf, size, &handler);
                if (ec) {
                    handler(ec);
                    return;
                }
                boost::asio::async_write(
                    channel, body_buf,
                    [handler,this]
                    (const system::error_code &ec, std::size_t) mutable {
                        is_open_ = keep_alive == KEEP_ALIVE_KEEP_ALIVE_READ;
                        if (!is_open_)
                            channel.lowest_layer().close();
                        handler(ec);
                    }
                );
            }
        );
    }

    return init.result.get();
}

template<class Socket, class Settings>
template<class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(system::error_code))
basic_socket<Socket, Settings>
::async_write_response_continue(CompletionToken &&token)
{
    boost::asio::async_completion<CompletionToken, void(system::error_code)>
        init{token};

    if (!writer_helper.write_continue()) {
        invoke_handler(std::move(init.completion_handler),
                       http_errc::out_of_order);
        return init.result.get();
    }

    auto handler = std::move(init.completion_handler);
    boost::asio::async_write(
        channel, detail::string_literal_buffer("HTTP/1.1 100 Continue\r\n\r\n"),
        [handler](const system::error_code &ec, std::size_t) mutable {
            handler(ec);
        }
    );

    return init.result.get();
}

template<class Socket, class Settings>
template<class Response, class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(system::error_code))
basic_socket<Socket, Settings>
::async_write_response_metadata(const Response &response,
                                CompletionToken &&token)
{
    static_assert(is_response_message<Response>::value,
                  "Response must fulfill the Response concept");

    boost::asio::async_completion<CompletionToken, void(system::error_code)>
        init{token};

    if (parser.which() != 1) { //< !req_parser
        invoke_handler(std::move(init.completion_handler),
                       http_errc::wrong_direction);
        return init.result.get();
    }

    auto prev_state = writer_helper.state;
    if (!writer_helper.write_metadata()) {
        invoke_handler(std::move(init.completion_handler),
                       http_errc::out_of_order);
        return init.result.get();
    }

    if (!modern_http) {
        writer_helper = prev_state;
        invoke_handler(std::move(init.completion_handler),
                       http_errc::native_stream_unsupported);
        return init.result.get();
    }

    const auto status_code = response.status_code();
    const auto &reason_phrase = response.reason_phrase();
    const auto &headers = response.headers();

    auto has_connection_close = detail::has_connection_close(headers);

    if (has_connection_close)
        keep_alive = KEEP_ALIVE_CLOSE_READ;

    auto use_connection_close_buf = (keep_alive == KEEP_ALIVE_CLOSE_READ)
        && !has_connection_close;

    std::size_t size =
        // Start line (http version + status code + reason phrase) + CRLF
        string_view("HTTP/1.1 NNN ").size() + reason_phrase.size() + 2
        // "transfer-encoding" header
        + string_view("transfer-encoding: chunked\r\n").size()
        // If user didn't provided "connection: close"
        + (use_connection_close_buf
           ? string_view("connection: close\r\n").size()
           : 0)
        // Headers are computed later
        + 0
        // Final CRLF
        + 2;

    // Headers
    for (const auto &e: headers) {
        // Each header is 4 buffer pieces: key + sep + value + CRLF
        // sep (": ") + CRLF is always 4 bytes.
        size += 4 + e.first.size() + e.second.size();
    }

    char *buf = NULL;
    std::size_t idx = 0;
    try {
        buf = reinterpret_cast<char*>(boost::asio::asio_handler_allocate(
            size, &init.completion_handler)
        );
    } catch (const std::bad_alloc&) {
        writer_helper.state = prev_state;
        throw;
    } catch (...) {
        assert(false);
    }

    {
        string_view x = "HTTP/1.1 ";
        std::memcpy(buf + idx, x.data(), x.size());
        idx += x.size();
    }

    buf[idx++] = (status_code / 100) + '0';
    buf[idx++] = (status_code / 10) % 10 + '0';
    buf[idx++] = (status_code % 10) + '0';
    buf[idx++] = ' ';

    std::memcpy(buf + idx, reason_phrase.data(), reason_phrase.size());
    idx += reason_phrase.size();

    std::memcpy(buf + idx, "\r\n", 2);
    idx += 2;

    if (use_connection_close_buf) {
        string_view x = "connection: close\r\n";
        std::memcpy(buf + idx, x.data(), x.size());
        idx += x.size();
    }

    for (const auto &e: headers) {
        std::memcpy(buf + idx, e.first.data(), e.first.size());
        idx += e.first.size();

        std::memcpy(buf + idx, ": ", 2);
        idx += 2;

        std::memcpy(buf + idx, e.second.data(), e.second.size());
        idx += e.second.size();

        std::memcpy(buf + idx, "\r\n", 2);
        idx += 2;
    }

    {
        string_view x = "transfer-encoding: chunked\r\n\r\n";
        std::memcpy(buf + idx, x.data(), x.size());
        idx += x.size();
    }

    assert(size == idx);

    auto handler = std::move(init.completion_handler);
    boost::asio::async_write(
        channel, boost::asio::buffer(buf, size),
        [handler,buf,size]
        (const system::error_code &ec, std::size_t) mutable {
            boost::asio::asio_handler_deallocate(buf, size, &handler);
            handler(ec);
        }
    );

    return init.result.get();
}

template<class Socket, class Settings>
template<class Request, class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(system::error_code))
basic_socket<Socket, Settings>
::async_write_request(const Request &request, CompletionToken &&token)
{
    static_assert(is_request_message<Request>::value,
                  "Request must fulfill the Request concept");

    boost::asio::async_completion<CompletionToken, void(system::error_code)>
        init{token};

    switch (parser.which()) {
    case 0: // none
        parser = res_parser{};
        break;
    case 1: // req_parser
        invoke_handler(std::move(init.completion_handler),
                       http_errc::wrong_direction);
        return init.result.get();
    case 2: // res_parser
        break;
    }

    if (write_state() != http::write_state::empty) {
        invoke_handler(std::move(init.completion_handler),
                       http_errc::out_of_order);
        return init.result.get();
    }

    const auto &method = request.method();
    const auto &target = request.target();
    const auto &headers = request.headers();

    {
        SentMethod sent_method;
        if (method == "HEAD") {
            sent_method = HEAD;
        } else if (method == "CONNECT") {
            sent_method = CONNECT;
        } else {
            sent_method = OTHER_METHOD;
        }
        sent_requests.push_back(sent_method);
    }

    const bool copy_body
        = (request.body().size() < Settings::body_copy_threshold)
        || (request.body().size() == 0);

    std::size_t size =
        // Request line
        method.size() + 1 + target.size() + string_view(" HTTP/1.x\r\n").size()
        // Headers are computed later
        + 0
        // Final CRLF
        + 2
        // Body
        + (copy_body ? request.body().size() : 0);
    std::size_t content_length_ndigits = 0;

    // Headers
    for (const auto &e: headers) {
        // Each header is 4 buffer pieces: key + sep + value + CRLF
        // sep (": ") + CRLF is always 4 bytes.
        size += 4 + e.first.size() + e.second.size();
    }

    // "content-length" header
    if (request.body().size()) {
        content_length_ndigits
            = detail::count_decdigits(request.body().size());
        size += string_view("content-length: \r\n").size()
            + content_length_ndigits;
    }

    char *buf = NULL;
    std::size_t idx = 0;
    try {
        buf = reinterpret_cast<char*>(boost::asio::asio_handler_allocate(
            size, &init.completion_handler)
        );
    } catch (const std::bad_alloc&) {
        sent_requests.pop_back();
        throw;
    } catch (...) {
        assert(false);
    }

    std::memcpy(buf + idx, method.data(), method.size());
    idx += method.size();

    buf[idx++] = ' ';

    std::memcpy(buf + idx, target.data(), target.size());
    idx += target.size();

    {
        string_view x;

        if (modern_http)
            x = " HTTP/1.1\r\n";
        else
            x = " HTTP/1.0\r\n";

        std::memcpy(buf + idx, x.data(), x.size());
        idx += x.size();
    }

    if (request.body().size()) {
        string_view x("content-length: ");
        std::memcpy(buf + idx, x.data(), x.size());
        idx += x.size();

        auto body_size = request.body().size();
        for (auto i = content_length_ndigits ; i != 0 ; --i) {
            buf[idx + i - 1] = (body_size % 10) + '0';
            body_size /= 10;
        }
        idx += content_length_ndigits;

        std::memcpy(buf + idx, "\r\n", 2);
        idx += 2;
    }

    for (const auto &e: headers) {
        std::memcpy(buf + idx, e.first.data(), e.first.size());
        idx += e.first.size();

        std::memcpy(buf + idx, ": ", 2);
        idx += 2;

        std::memcpy(buf + idx, e.second.data(), e.second.size());
        idx += e.second.size();

        std::memcpy(buf + idx, "\r\n", 2);
        idx += 2;
    }

    std::memcpy(buf + idx, "\r\n", 2);
    idx += 2;

    if (copy_body) {
        std::copy(request.body().begin(), request.body().end(), buf + idx);
        idx += request.body().size();
    }
    assert(size == idx);

    auto handler = std::move(init.completion_handler);
    if (copy_body) {
        boost::asio::async_write(
            channel, boost::asio::buffer(buf, size),
            [handler,buf,size]
            (const system::error_code &ec, std::size_t) mutable {
                boost::asio::asio_handler_deallocate(buf, size, &handler);
                handler(ec);
            }
        );
    } else {
        auto body_buf
            = boost::asio::buffer(request.body().data(), request.body().size());
        boost::asio::async_write(
            channel, boost::asio::buffer(buf, size),
            [handler,buf,size,body_buf,this]
            (const system::error_code &ec, std::size_t) mutable {
                boost::asio::asio_handler_deallocate(buf, size, &handler);
                if (ec) {
                    handler(ec);
                    return;
                }
                boost::asio::async_write(
                    channel, body_buf,
                    [handler]
                    (const system::error_code &ec, std::size_t) mutable {
                        handler(ec);
                    }
                );
            }
        );
    }

    return init.result.get();
}

template<class Socket, class Settings>
template<class Request, class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(system::error_code))
basic_socket<Socket, Settings>
::async_write_request_metadata(const Request &request, CompletionToken &&token)
{
    static_assert(is_request_message<Request>::value,
                  "Request must fulfill the Request concept");

    boost::asio::async_completion<CompletionToken, void(system::error_code)>
        init{token};

    switch (parser.which()) {
    case 0: // none
        parser = res_parser{};
        break;
    case 1: // req_parser
        invoke_handler(std::move(init.completion_handler),
                       http_errc::wrong_direction);
        return init.result.get();
    case 2: // res_parser
        break;
    }

    if (write_state() != http::write_state::empty) {
        invoke_handler(std::move(init.completion_handler),
                       http_errc::out_of_order);
        return init.result.get();
    }

    if (writer_helper.state != write_state::empty) {
        invoke_handler(std::move(init.completion_handler),
                       http_errc::out_of_order);
        return init.result.get();
    }

    if (!modern_http) {
        invoke_handler(std::move(init.completion_handler),
                       http_errc::native_stream_unsupported);
        return init.result.get();
    }

    writer_helper.state = write_state::metadata_issued;

    const auto &method = request.method();
    const auto &target = request.target();
    const auto &headers = request.headers();

    {
        SentMethod sent_method;
        if (method == "HEAD") {
            sent_method = HEAD;
        } else if (method == "CONNECT") {
            sent_method = CONNECT;
        } else {
            sent_method = OTHER_METHOD;
        }
        sent_requests.push_back(sent_method);
    }

    std::size_t size =
        // Request line
        method.size() + 1 + target.size() + string_view(" HTTP/1.1\r\n").size()
        // "transfer-encoding" header
        + string_view("transfer-encoding: chunked\r\n").size()
        // Headers are computed later
        + 0
        // Final CRLF
        + 2;

    // Headers
    for (const auto &e: headers) {
        // Each header is 4 buffer pieces: key + sep + value + CRLF
        // sep (": ") + CRLF is always 4 bytes.
        size += 4 + e.first.size() + e.second.size();
    }

    char *buf = NULL;
    std::size_t idx = 0;
    try {
        buf = reinterpret_cast<char*>(boost::asio::asio_handler_allocate(
            size, &init.completion_handler)
        );
    } catch (const std::bad_alloc&) {
        sent_requests.pop_back();
        writer_helper.state = write_state::empty;
        throw;
    } catch (...) {
        assert(false);
    }

    std::memcpy(buf + idx, method.data(), method.size());
    idx += method.size();

    buf[idx++] = ' ';

    std::memcpy(buf + idx, target.data(), target.size());
    idx += target.size();

    {
        string_view x = " HTTP/1.1\r\n" "transfer-encoding: chunked\r\n";
        std::memcpy(buf + idx, x.data(), x.size());
        idx += x.size();
    }

    for (const auto &e: headers) {
        std::memcpy(buf + idx, e.first.data(), e.first.size());
        idx += e.first.size();

        std::memcpy(buf + idx, ": ", 2);
        idx += 2;

        std::memcpy(buf + idx, e.second.data(), e.second.size());
        idx += e.second.size();

        std::memcpy(buf + idx, "\r\n", 2);
        idx += 2;
    }

    std::memcpy(buf + idx, "\r\n", 2);
    idx += 2;

    assert(size == idx);

    auto handler = std::move(init.completion_handler);
    boost::asio::async_write(
        channel, boost::asio::buffer(buf, size),
        [handler,buf,size](const system::error_code &ec, std::size_t) mutable {
            boost::asio::asio_handler_deallocate(buf, size, &handler);
            handler(ec);
        }
    );

    return init.result.get();
}

template<class Socket, class Settings>
template<class Message, class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(system::error_code))
basic_socket<Socket, Settings>::async_write(const Message &message,
                                            CompletionToken &&token)
{
    static_assert(is_message<Message>::value,
                  "Message must fulfill the Message concept");

    using detail::string_literal_buffer;

    boost::asio::async_completion<CompletionToken, void(system::error_code)>
        init{token};

    auto prev_state = writer_helper.state;
    if (!writer_helper.write()) {
        invoke_handler(std::move(init.completion_handler),
                       http_errc::out_of_order);
        return init.result.get();
    }

    if (message.body().size() == 0) {
        invoke_handler(std::move(init.completion_handler));
        return init.result.get();
    }

    const bool copy_body
        = message.body().size() < Settings::body_copy_threshold;

    std::size_t content_length_nhexdigits
        = detail::count_hexdigits(message.body().size());
    std::size_t size =
        // hex string
        content_length_nhexdigits
        // crlf
        + 2
        // body chunk + crlf
        + (copy_body
           ? (message.body().size() + 2)
           : 0);

    char *buf = NULL;
    std::size_t idx = 0;
    try {
        buf = reinterpret_cast<char*>(boost::asio::asio_handler_allocate(
            size, &init.completion_handler)
        );
    } catch (const std::bad_alloc&) {
        writer_helper.state = prev_state;
        throw;
    } catch (...) {
        assert(false);
    }

    {
        auto body_size = message.body().size();
        for (auto i = content_length_nhexdigits ; i != 0 ; --i) {
            char hexdigit = body_size % 16;
            if (hexdigit > 10)
                hexdigit += 'a' - 10;
            else
                hexdigit += '0';
            buf[idx + i - 1] = hexdigit;
            body_size /= 16;
        }
        idx += content_length_nhexdigits;
    }

    std::memcpy(buf + idx, "\r\n", 2);
    idx += 2;

    if (copy_body) {
        std::copy(message.body().begin(), message.body().end(), buf + idx);
        idx += message.body().size();

        std::memcpy(buf + idx, "\r\n", 2);
        idx += 2;
    }

    assert(size == idx);

    auto handler = std::move(init.completion_handler);
    if (copy_body) {
        boost::asio::async_write(
            channel, boost::asio::buffer(buf, size),
            [handler,buf,size]
            (const system::error_code &ec, std::size_t) mutable {
                boost::asio::asio_handler_deallocate(buf, size, &handler);
                handler(ec);
            }
        );
    } else {
        auto body_buf
            = boost::asio::buffer(message.body().data(), message.body().size());
        boost::asio::async_write(
            channel, boost::asio::buffer(buf, size),
            [handler,buf,size,body_buf,this]
            (const system::error_code &ec, std::size_t) mutable {
                boost::asio::asio_handler_deallocate(buf, size, &handler);
                if (ec) {
                    handler(ec);
                    return;
                }
                boost::asio::async_write(
                    channel, body_buf,
                    [handler,this]
                    (const system::error_code &ec, std::size_t) mutable {
                        if (ec) {
                            handler(ec);
                            return;
                        }
                        auto crlf = string_literal_buffer("\r\n");
                        boost::asio::async_write(
                            channel, crlf,
                            [handler](const system::error_code &ec,
                                      std::size_t) mutable {
                                handler(ec);
                            }
                        );
                    }
                );
            }
        );
    }

    return init.result.get();
}

template<class Socket, class Settings>
template<class Message, class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(system::error_code))
basic_socket<Socket, Settings>::async_write_chunkext(
    const Message &message, const typename Message::headers_type &chunkext,
    CompletionToken &&token
)
{
    static_assert(is_message<Message>::value,
                  "Message must fulfill the Message concept");

    using detail::string_literal_buffer;

    boost::asio::async_completion<CompletionToken, void(system::error_code)>
        init{token};

    auto prev_state = writer_helper.state;
    if (!writer_helper.write()) {
        invoke_handler(std::move(init.completion_handler),
                       http_errc::out_of_order);
        return init.result.get();
    }

    if (message.body().size() == 0) {
        invoke_handler(std::move(init.completion_handler));
        return init.result.get();
    }

    const bool copy_body
        = message.body().size() < Settings::body_copy_threshold;

    std::size_t content_length_nhexdigits
        = detail::count_hexdigits(message.body().size());
    std::size_t size =
        // hex string
        content_length_nhexdigits
        // crlf
        + 2
        // body chunk + crlf
        + (copy_body
           ? (message.body().size() + 2)
           : 0);

    for (const auto &e: chunkext) {
        // Each chunkext is 4 buffer pieces: SEMICOLON + key + EQUALS_SIGN +
        // value. EQUALS_SIGN + value is optional.
        //
        // SEMICOLON + EQUALS_SIGN is always 2 bytes.
        if (e.second.size())
            size += 2 + e.first.size() + e.second.size();
        else
            size += 1 + e.first.size();
    }

    char *buf = NULL;
    std::size_t idx = 0;
    try {
        buf = reinterpret_cast<char*>(boost::asio::asio_handler_allocate(
            size, &init.completion_handler)
        );
    } catch (const std::bad_alloc&) {
        writer_helper.state = prev_state;
        throw;
    } catch (...) {
        assert(false);
    }

    {
        auto body_size = message.body().size();
        for (auto i = content_length_nhexdigits ; i != 0 ; --i) {
            char hexdigit = body_size % 16;
            if (hexdigit > 10)
                hexdigit += 'a' - 10;
            else
                hexdigit += '0';
            buf[idx + i - 1] = hexdigit;
            body_size /= 16;
        }
        idx += content_length_nhexdigits;
    }

    for (const auto &e: chunkext) {
        buf[idx++] = ';';
        std::memcpy(buf + idx, e.first.data(), e.first.size());
        idx += e.first.size();
        if (e.second.size()) {
            buf[idx++] = '=';
            std::memcpy(buf + idx, e.second.data(), e.second.size());
            idx += e.second.size();
        }
    }

    std::memcpy(buf + idx, "\r\n", 2);
    idx += 2;

    if (copy_body) {
        std::copy(message.body().begin(), message.body().end(), buf + idx);
        idx += message.body().size();

        std::memcpy(buf + idx, "\r\n", 2);
        idx += 2;
    }

    assert(size == idx);

    auto handler = std::move(init.completion_handler);
    if (copy_body) {
        boost::asio::async_write(
            channel, boost::asio::buffer(buf, size),
            [handler,buf,size]
            (const system::error_code &ec, std::size_t) mutable {
                boost::asio::asio_handler_deallocate(buf, size, &handler);
                handler(ec);
            }
        );
    } else {
        auto body_buf
            = boost::asio::buffer(message.body().data(), message.body().size());
        boost::asio::async_write(
            channel, boost::asio::buffer(buf, size),
            [handler,buf,size,body_buf,this]
            (const system::error_code &ec, std::size_t) mutable {
                boost::asio::asio_handler_deallocate(buf, size, &handler);
                if (ec) {
                    handler(ec);
                    return;
                }
                boost::asio::async_write(
                    channel, body_buf,
                    [handler,this]
                    (const system::error_code &ec, std::size_t) mutable {
                        if (ec) {
                            handler(ec);
                            return;
                        }
                        auto crlf = string_literal_buffer("\r\n");
                        boost::asio::async_write(
                            channel, crlf,
                            [handler](const system::error_code &ec,
                                      std::size_t) mutable {
                                handler(ec);
                            }
                        );
                    }
                );
            }
        );
    }

    return init.result.get();
}

template<class Socket, class Settings>
template<class Message, class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(system::error_code))
basic_socket<Socket, Settings>::async_write_trailers(const Message &message,
                                                     CompletionToken &&token)
{
    static_assert(is_message<Message>::value,
                  "Message must fulfill the Message concept");

    boost::asio::async_completion<CompletionToken, void(system::error_code)>
        init{token};

    auto prev_rstate = istate;
    auto prev_wstate = writer_helper.state;
    if (!writer_helper.write_trailers()) {
        invoke_handler(std::move(init.completion_handler),
                       http_errc::out_of_order);
        return init.result.get();
    }

    switch (parser.which()) {
    case 0: // none
        assert(false);
        break;
    case 1: // req_parser
        istate = http::read_state::empty;
        break;
    case 2: // res_parser
        writer_helper.state = write_state::empty;
        break;
    }

    std::size_t size =
        // last_chunk
        string_view("0\r\n").size()
        // Trailers are computed later
        + 0
        // Final CRLF for end of trailers
        + 2;

    for (const auto &e: message.trailers()) {
        // Each header is 4 buffer pieces: key + sep + value + crlf
        // sep (": ") + crlf is always 4 bytes.
        size += 4 + e.first.size() + e.second.size();
    }

    char *buf = NULL;
    std::size_t idx = 0;
    try {
        buf = reinterpret_cast<char*>(boost::asio::asio_handler_allocate(
            size, &init.completion_handler)
        );
    } catch (const std::bad_alloc&) {
        istate = prev_rstate;
        writer_helper.state = prev_wstate;
        throw;
    } catch (...) {
        assert(false);
    }

    std::memcpy(buf + idx, "0\r\n", 3);
    idx += 3;

    for (const auto &e: message.trailers()) {
        std::memcpy(buf + idx, e.first.data(), e.first.size());
        idx += e.first.size();

        std::memcpy(buf + idx, ": ", 2);
        idx += 2;

        std::memcpy(buf + idx, e.second.data(), e.second.size());
        idx += e.second.size();

        std::memcpy(buf + idx, "\r\n", 2);
        idx += 2;
    }

    std::memcpy(buf + idx, "\r\n", 2);
    idx += 2;

    auto handler = std::move(init.completion_handler);
    boost::asio::async_write(
        channel, boost::asio::buffer(buf, size),
        [handler,buf,size,this]
        (const system::error_code &ec, std::size_t) mutable {
            boost::asio::asio_handler_deallocate(buf, size, &handler);
            is_open_ = keep_alive == KEEP_ALIVE_KEEP_ALIVE_READ;
            if (!is_open_)
                channel.lowest_layer().close();
            handler(ec);
        }
    );

    return init.result.get();
}

template<class Socket, class Settings>
template<class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(system::error_code))
basic_socket<Socket, Settings>
::async_write_end_of_message(CompletionToken &&token)
{
    using detail::string_literal_buffer;

    boost::asio::async_completion<CompletionToken, void(system::error_code)>
        init{token};

    if (!writer_helper.end()) {
        invoke_handler(std::move(init.completion_handler),
                       http_errc::out_of_order);
        return init.result.get();
    }

    switch (parser.which()) {
    case 0: // none
        assert(false);
        break;
    case 1: // req_parser
        istate = http::read_state::empty;
        break;
    case 2: // res_parser
        writer_helper.state = write_state::empty;
        break;
    }

    auto last_chunk = string_literal_buffer("0\r\n\r\n");

    auto handler = std::move(init.completion_handler);
    boost::asio::async_write(
        channel, last_chunk,
        [handler,this](const system::error_code &ec, std::size_t) mutable {
            is_open_ = keep_alive == KEEP_ALIVE_KEEP_ALIVE_READ;
            if (!is_open_)
                channel.lowest_layer().close();
            handler(ec);
        }
    );

    return init.result.get();
}

template<class Socket, class Settings>
basic_socket<Socket, Settings>
::basic_socket(boost::asio::io_context &io_context,
               boost::asio::mutable_buffer inbuffer) :
    channel(io_context),
    istate(http::read_state::empty),
    buffer(inbuffer),
    writer_helper(http::write_state::empty)
{
    if (buffer.size() == 0)
        throw std::invalid_argument("buffers must not be 0-sized");
}

template<class Socket, class Settings>
template<class... Args>
basic_socket<Socket, Settings>
::basic_socket(boost::asio::mutable_buffer inbuffer, Args&&... args)
    : channel(std::forward<Args>(args)...)
    , istate(http::read_state::empty)
    , buffer(inbuffer)
    , writer_helper(http::write_state::empty)
{
    if (buffer.size() == 0)
        throw std::invalid_argument("buffers must not be 0-sized");
}

template<class Socket, class Settings>
Socket &basic_socket<Socket, Settings>::next_layer()
{
    return channel;
}

template<class Socket, class Settings>
const Socket &basic_socket<Socket, Settings>::next_layer() const
{
    return channel;
}

template<class Socket, class Settings>
void basic_socket<Socket, Settings>::open()
{
    is_open_ = true;
}

template<class Socket, class Settings>
boost::asio::const_buffer basic_socket<Socket, Settings>::upgrade_head() const
{
#ifndef BOOST_HTTP_UPGRADE_HEAD_DISABLE_CHECK
    // res_parser
    if (parser.which() != 2)
        throw std::logic_error("`upgrade_head` only makes sense in HTTP client"
                               " mode");
#endif // BOOST_HTTP_UPGRADE_HEAD_DISABLE_CHECK

    return boost::asio::buffer(buffer, used_size);
}

template<class Socket, class Settings>
void basic_socket<Socket, Settings>::lock_client_to_http10()
{
    // req_parser
    if (parser.which() == 1) {
        // server-mode, nothing to do
        return;
    }

    modern_http = false;
}

template<class Socket, class Settings>
template<bool enable_chunkext, class Message, class Handler>
void basic_socket<Socket, Settings>
::schedule_on_async_read_message(Handler &&handler, Message &message,
                                 typename Message::headers_type *chunkext)
{
    if (enable_chunkext)
        assert(chunkext != NULL);
    else
        assert(chunkext == NULL);

    bool server_mode;
    if (is_request_message<Message>::value
        && !is_response_message<Message>::value) {
        server_mode = true;
    } else if (is_response_message<Message>::value
               && !is_request_message<Message>::value) {
        server_mode = false;
    } else {
        switch (parser.which()) {
        case 0: // none
            assert(false);
            break;
        case 1: // req_parser
            server_mode = true;
            break;
        case 2: // res_parser
            server_mode = false;
            break;
        }
    }

    if (used_size) {
        // Have cached some bytes from a previous read
        if (server_mode) {
            on_async_read_message<true, enable_chunkext, req_parser>(
                std::forward<Handler>(handler), message, chunkext,
                system::error_code{}, 0
            );
        } else {
            on_async_read_message<false, enable_chunkext, res_parser>(
                std::forward<Handler>(handler), message, chunkext,
                system::error_code{}, 0
            );
        }
    } else {
        if (server_mode) {
            // TODO (C++14): move in lambda capture list
            channel.async_read_some(
                boost::asio::buffer(buffer + used_size),
                [this,handler,&message,&chunkext](
                    const system::error_code &ec, std::size_t bytes_transferred
                ) mutable {
                    on_async_read_message<true, enable_chunkext, req_parser>(
                        std::move(handler), message, chunkext, ec,
                        bytes_transferred
                    );
                }
            );
        } else {
            // TODO (C++14): move in lambda capture list
            channel.async_read_some(
                boost::asio::buffer(buffer + used_size),
                [this,handler,&message,&chunkext](
                    const system::error_code &ec, std::size_t bytes_transferred
                ) mutable {
                    on_async_read_message<false, enable_chunkext, res_parser>(
                        std::move(handler), message, chunkext, ec,
                        bytes_transferred
                    );
                }
            );
        }
    }
}

namespace detail {

template<bool server_mode, class Message, class Parser>
typename std::enable_if<!is_request_message<Message>::value || !server_mode,
                        boost::string_view>::type
fill_method(Message &request, const Parser &parser)
{
    return ""; //< no-op
}

template<bool server_mode, class Request, class Parser>
typename std::enable_if<is_request_message<Request>::value && server_mode,
                        boost::string_view>::type
fill_method(Request &request, const Parser &parser)
{
    auto value = parser.template value<token::method>();
    request.method().assign(value.data(), value.size());
    return value;
}

template<bool server_mode, class Message, class Parser>
typename std::enable_if<!is_request_message<Message>::value || !server_mode>
::type
fill_target(Message &request, const Parser &parser)
{} //< no-op

template<bool server_mode, class Request, class Parser>
typename std::enable_if<is_request_message<Request>::value && server_mode>::type
fill_target(Request &request, const Parser &parser)
{
    auto value = parser.template value<token::request_target>();
    request.target().assign(value.data(), value.size());
}

template<bool server_mode, class Message, class Parser>
typename std::enable_if<!is_response_message<Message>::value || server_mode,
                        std::uint_least16_t>
::type
fill_status_code(Message&, Parser&, boost::string_view)
{
    // no-op
    return 0;
}

template<bool server_mode, class Response, class Parser>
typename std::enable_if<is_response_message<Response>::value && !server_mode,
                        std::uint_least16_t>
::type
fill_status_code(Response &response, Parser &parser,
                 boost::string_view sent_method)
{
    auto value = parser.template value<token::status_code>();
    response.status_code() = value;
    parser.set_method(sent_method);
    return value;
}

template<bool server_mode, class Message, class Parser>
typename std::enable_if<!is_response_message<Message>::value || server_mode>
::type
fill_reason_phrase(Message&, const Parser&)
{} //< no-op

template<bool server_mode, class Response, class Parser>
typename std::enable_if<is_response_message<Response>::value && !server_mode>
::type
fill_reason_phrase(Response &response, const Parser &parser)
{
    auto value = parser.template value<token::reason_phrase>();
    response.reason_phrase().assign(value.data(), value.size());
}

template<bool server_mode, class Parser>
typename std::enable_if<server_mode>::type
puteof(Parser&)
{} //< no-op

template<bool server_mode, class Parser>
typename std::enable_if<!server_mode>::type
puteof(Parser &parser)
{
    parser.puteof();
}

template<bool enable_chunkext>
struct call_with_chunkext;

template<>
struct call_with_chunkext<false>
{
    template<class Handler, class T>
    static void call(Handler &handler, T)
    {
        handler(system::error_code{});
    }

    template<class Handler, class EC, class T>
    static void call(Handler &handler, EC ec, T)
    {
        handler(ec);
    }
};

template<>
struct call_with_chunkext<true>
{
    template<class Handler, class T>
    static void call(Handler &handler, T val)
    {
        handler(system::error_code{}, val);
    }

    template<class Handler, class EC, class T>
    static void call(Handler &handler, EC ec, T val)
    {
        handler(ec, val);
    }
};

} // namespace detail

template<class Socket, class Settings>
template<bool server_mode, bool enable_chunkext, class Parser, class Message,
         class Handler>
void basic_socket<Socket, Settings>
::on_async_read_message(Handler &&handler, Message &message,
                        typename Message::headers_type *chunkext,
                        const system::error_code &ec,
                        std::size_t bytes_transferred)
{
    using detail::string_literal_buffer;

    if (ec) {
        if (ec == system::error_code{boost::asio::error::eof} && !server_mode) {
            Parser &parser = get<Parser>(this->parser);
            detail::puteof<server_mode>(parser);
            is_open_ = false;
        } else {
            clear_buffer();
            detail::call_with_chunkext<enable_chunkext>::call(handler, ec, 0);
            return;
        }
    }

    Parser &parser = get<Parser>(this->parser);

    used_size += bytes_transferred;
    parser.set_buffer(boost::asio::buffer(buffer, used_size));

    bool loop = true;
    bool cb_ready = false;
    uint_least64_t cb_value = 0;

    while (loop) {
        switch (parser.code()) {
        case token::code::error_insufficient_data:
            loop = false;
            continue;
        case token::code::error_set_method:
            assert(false);
            break;
        case token::code::error_use_another_connection:
            assert(false);
            break;
        case token::code::error_invalid_data:
            {
                clear_buffer();

                auto error_message
                    = string_literal_buffer("HTTP/1.1 400 Bad Request\r\n"
                                            "Content-Length: 13\r\n"
                                            "Connection: close\r\n"
                                            "\r\n"
                                            "Invalid data\n");
                if (server_mode) {
                    boost::asio::async_write(
                        channel, boost::asio::buffer(error_message),
                        [handler](system::error_code /*ignored_ec*/,
                                  std::size_t /*bytes_transferred*/)
                        mutable {
                            detail::call_with_chunkext<enable_chunkext>::call(
                                handler, http_errc::parsing_error, 0);
                        }
                    );
                } else {
                    detail::call_with_chunkext<enable_chunkext>::call(
                        handler, http_errc::parsing_error, 0);
                }
                return;
            }
        case token::code::error_no_host:
            {
                clear_buffer();

                auto error_message
                    = string_literal_buffer("HTTP/1.1 400 Bad Request\r\n"
                                            "Content-Length: 13\r\n"
                                            "Connection: close\r\n"
                                            "\r\n"
                                            "Host missing\n");
                if (server_mode) {
                    boost::asio::async_write(
                        channel, boost::asio::buffer(error_message),
                        [handler](system::error_code /*ignored_ec*/,
                                  std::size_t /*bytes_transferred*/)
                        mutable {
                            detail::call_with_chunkext<enable_chunkext>::call(
                                handler, http_errc::parsing_error, 0);
                        }
                    );
                } else {
                    detail::call_with_chunkext<enable_chunkext>::call(
                        handler, http_errc::parsing_error, 0);
                }
                return;
            }
        case token::code::error_invalid_content_length:
        case token::code::error_content_length_overflow:
            {
                clear_buffer();

                auto error_message
                    = string_literal_buffer("HTTP/1.1 400 Bad Request\r\n"
                                            "Content-Length: 23\r\n"
                                            "Connection: close\r\n"
                                            "\r\n"
                                            "Invalid content-length\n");
                if (server_mode) {
                    boost::asio::async_write(
                        channel, boost::asio::buffer(error_message),
                        [handler](system::error_code /*ignored_ec*/,
                                  std::size_t /*bytes_transferred*/)
                        mutable {
                            detail::call_with_chunkext<enable_chunkext>::call(
                                handler, http_errc::parsing_error, 0);
                        }
                    );
                } else {
                    detail::call_with_chunkext<enable_chunkext>::call(
                        handler, http_errc::parsing_error, 0);
                }
                return;
            }
        case token::code::error_invalid_transfer_encoding:
            {
                clear_buffer();

                auto error_message
                    = string_literal_buffer("HTTP/1.1 400 Bad Request\r\n"
                                            "Content-Length: 25\r\n"
                                            "Connection: close\r\n"
                                            "\r\n"
                                            "Invalid transfer-encoding\n");
                if (server_mode) {
                    boost::asio::async_write(
                        channel, boost::asio::buffer(error_message),
                        [handler](system::error_code /*ignored_ec*/,
                                  std::size_t /*bytes_transferred*/)
                        mutable {
                            detail::call_with_chunkext<enable_chunkext>::call(
                                handler, http_errc::parsing_error, 0);
                        }
                    );
                } else {
                    detail::call_with_chunkext<enable_chunkext>::call(
                        handler, http_errc::parsing_error, 0);
                }
                return;
            }
        case token::code::error_chunk_size_overflow:
            {
                clear_buffer();

                auto error_message
                    = string_literal_buffer("HTTP/1.1 400 Bad Request\r\n"
                                            "Content-Length: 25\r\n"
                                            "Connection: close\r\n"
                                            "\r\n"
                                            "Can't process chunk size\n");
                if (server_mode) {
                    boost::asio::async_write(
                        channel, boost::asio::buffer(error_message),
                        [handler](system::error_code /*ignored_ec*/,
                                  std::size_t /*bytes_transferred*/)
                        mutable {
                            detail::call_with_chunkext<enable_chunkext>::call(
                                handler, http_errc::parsing_error, 0);
                        }
                    );
                } else {
                    detail::call_with_chunkext<enable_chunkext>::call(
                        handler, http_errc::parsing_error, 0);
                }
                return;
            }
        case token::code::skip:
            break;
        case token::code::method:
            {
                auto value = detail::fill_method<server_mode>(message, parser);
                connect_request = value == "CONNECT";
            }
            break;
        case token::code::request_target:
            detail::fill_target<server_mode>(message, parser);
            break;
        case token::code::version:
            {
                auto value = parser.template value<token::version>();
                if (value == 0)
                    modern_http = false;
                keep_alive = KEEP_ALIVE_UNKNOWN;
            }
            break;
        case token::code::status_code:
            {
                boost::string_view sent_method;
                if (sent_requests.size() == 0) {
                    clear_buffer();
                    detail::call_with_chunkext<enable_chunkext>::call(
                        handler, http_errc::parsing_error, 0);
                    return;
                }

                switch (sent_requests.front()) {
                case HEAD:
                    sent_method = "HEAD";
                    break;
                case CONNECT:
                    sent_method = "CONNECT";
                    break;
                case OTHER_METHOD:
                    sent_method = "GET";
                }
                auto sc = detail::fill_status_code<server_mode>(message, parser,
                                                                sent_method);
                // non-informational response
                if (sc / 100 != 1) {
                    // “The 1xx (Informational) class of status code indicates
                    // an interim response for communicating connection status
                    // or request progress prior to completing the requested
                    // action and sending a final response.” — RFC7231
                    //
                    // Therefore, informational responses do *not* mark the
                    // transition to a different request-response pair.
                    sent_requests.erase(sent_requests.begin());
                }
            }
            break;
        case token::code::reason_phrase:
            detail::fill_reason_phrase<server_mode>(message, parser);
            break;
        case token::code::field_name:
        case token::code::trailer_name:
            {
                typedef typename Message::headers_type::key_type NameT;
                typedef typename Message::headers_type::mapped_type ValueT;

                bool use_trailers = parser.code() == token::code::trailer_name;

                // Header name
                {
                    auto buf_view = static_cast<char*>(buffer.data());
                    std::size_t field_name_begin = parser.parsed_count();
                    std::size_t field_name_size = parser.token_size();

                    for (std::size_t i = 0 ; i != field_name_size ; ++i) {
                        auto &ch = buf_view[field_name_begin + i];
                        ch = std::tolower(ch);
                    }
                }

                // Header value
                Parser parser_copy(parser);
                parser_copy.next();

                if (parser_copy.code() == token::code::skip)
                    parser_copy.next();

                switch (parser_copy.code()) {
                case token::code::field_value:
                case token::code::trailer_value:
                    // do nothing
                    break;
                case token::code::error_insufficient_data:
                    loop = false;
                    continue;
                default:
                    // Some error. Re-use `parser.code()` error handling.
                    parser.next();
                    continue;
                }

                auto name = parser.template value<token::field_name>();
                auto value = parser_copy.template value<token::field_value>();

                if (modern_http || (name != "expect" && name != "upgrade")) {
                    if (name == "connection") {
                        switch (keep_alive) {
                        case KEEP_ALIVE_UNKNOWN:
                        case KEEP_ALIVE_KEEP_ALIVE_READ:
                            header_value_any_of(value, [&](string_view v) {
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

                parser = parser_copy;
            }
            break;
        case token::code::field_value:
        case token::code::trailer_value:
            BOOST_HTTP_DETAIL_UNREACHABLE("handled within `field_name` case");
            break;
        case token::code::end_of_headers:
            istate = http::read_state::message_ready;
            cb_ready = true;
            if (server_mode) {
                writer_helper = http::write_state::empty;
            }

            {
                auto er = message.headers().equal_range("expect");
                if (er.first != er.second) {
                    auto x = er.first;
                    ++x;
                    // (std::distance(er.first, er.second) > 1
                    if (x != er.second)
                        message.headers().erase(er.first, er.second);
                }
            }

            if (keep_alive == KEEP_ALIVE_UNKNOWN) {
                keep_alive = modern_http
                    ? KEEP_ALIVE_KEEP_ALIVE_READ : KEEP_ALIVE_CLOSE_READ;
            }
            break;
        case token::code::chunk_ext:
            if (enable_chunkext) {
                assert(chunkext != NULL);

                auto value = parser.template value<token::chunk_ext>();
                using view_type = decltype(value.ext);
                using view_size_t = typename view_type::size_type;
                static_assert(std::is_unsigned<view_size_t>::value,
                              "we rely on unsigned integer overflow");
                assert(value.ext.size() > 0);

                {
                    view_size_t i = 0;
                    while (i < value.ext.size() && value.ext[i] != ';') ++i;
                    value.ext.remove_prefix(
                        (i + 1 > value.ext.size()) ? value.ext.size() : i + 1);
                }

                view_type name;
                bool skip = false;
                for (view_size_t i = 0 ; i != value.ext.size() ; ++i) {
                    if (value.ext[i] == '=') {
                        name = value.ext.substr(0, i);
                        if (name.size() == 0)
                            skip = true;

                        value.ext.remove_prefix(i + 1);
                        i = std::numeric_limits<view_size_t>::max();
                    } else if (value.ext[i] == ';') {
                        view_type val;
                        (name.size() > 0 ? val : name) = value.ext.substr(0, i);
                        if (skip) {
                            name = view_type{};
                            skip = false;
                        }
                        if (name.size() > 0) {
                            chunkext->emplace(name, val);
                            name.clear();
                        }

                        value.ext.remove_prefix(i + 1);
                        i = std::numeric_limits<view_size_t>::max();
                    }
                }
                if (value.ext.size() > 0) {
                    view_type val;
                    (name.size() > 0 ? val : name) = value.ext;
                    if (!skip && name.size() > 0)
                        chunkext->emplace(name, val);
                }

                if (!chunkext->empty()) {
                    cb_value = value.chunk_size;
                    cb_ready = true;
                    loop = false;
                }
            }
            break;
        case token::code::body_chunk:
            {
                auto value = parser.template value<token::body_chunk>();
                auto begin = static_cast<const std::uint8_t*>(value.data());
                auto size = value.size();
                message.body().insert(message.body().end(), begin, begin + size);
                cb_ready = true;
            }
            break;
        case token::code::end_of_body:
            istate = http::read_state::body_ready;
            break;
        case token::code::end_of_message:
            if (server_mode) {
                istate = http::read_state::finished;
            } else {
                istate = http::read_state::empty;
                if (!modern_http || keep_alive == KEEP_ALIVE_CLOSE_READ)
                    is_open_ = false;
            }
            cb_ready = true;
            loop = false;
            continue;
        }
        parser.next();
    }

    {
        auto buf_view = static_cast<char*>(buffer.data());
        auto nparsed = parser.parsed_count();
        if (parser.code() == token::code::end_of_message) {
            const auto token_size = parser.token_size();
            parser.set_buffer(
                boost::asio::buffer(buffer + nparsed, parser.token_size())
            );
            parser.next();
            assert(parser.code() == token::code::error_use_another_connection
                   || parser.code() == token::code::error_insufficient_data);
            nparsed += token_size;
        }
        std::copy_n(buf_view + nparsed, used_size - nparsed, buf_view);
        used_size -= nparsed;
    }

    if (cb_ready) {
        detail::call_with_chunkext<enable_chunkext>::call(handler, cb_value);
    } else {
        if (used_size == buffer.size()) {
            /* TODO: use `expected_token()` to reply with appropriate "... too
               long" status code */
            detail::call_with_chunkext<enable_chunkext>::call(
                handler, http_errc::buffer_exhausted, 0);
            return;
        }

        // TODO (C++14): move in lambda capture list
        channel.async_read_some(
            boost::asio::buffer(buffer + used_size),
            [this,handler,&message,chunkext](
                const system::error_code &ec, std::size_t bytes_transferred
            ) mutable {
                on_async_read_message<server_mode, enable_chunkext, Parser>(
                    std::move(handler), message, chunkext, ec, bytes_transferred
                );
            }
        );
    }
}

template<class Socket, class Settings>
void basic_socket<Socket, Settings>::clear_buffer()
{
    istate = http::read_state::empty;
    writer_helper.state = http::write_state::empty;
    used_size = 0;
    parser = none;
    sent_requests.clear();
    modern_http = true;
}

template<class Socket, class Settings>
template<class Message>
void basic_socket<Socket, Settings>::clear_message(Message &message)
{
    message.headers().clear();
    message.body().clear();
    message.trailers().clear();
}

template<class Socket, class Settings>
template <typename Handler,
          typename ErrorCode>
void basic_socket<Socket, Settings>::invoke_handler(Handler&& handler,
                                                    ErrorCode error)
{
    auto ex(boost::asio::get_associated_executor(handler, get_executor()));
    auto alloc(boost::asio::get_associated_allocator(handler));
    ex.post([handler, error] () mutable { handler(make_error_code(error)); },
            alloc);
}

template<class Socket, class Settings>
template <class Handler>
void basic_socket<Socket, Settings>::invoke_handler(Handler&& handler)
{
    auto ex(boost::asio::get_associated_executor(handler, get_executor()));
    auto alloc(boost::asio::get_associated_allocator(handler));
    ex.post([handler] () mutable { handler(system::error_code{}); }, alloc);
}

template<class Socket, class Settings>
template<class Handler, class ErrorCode, class Value>
void basic_socket<Socket, Settings>::invoke_handler(Handler &&handler,
                                                    ErrorCode error,
                                                    Value value)
{
    auto ex(boost::asio::get_associated_executor(handler, get_executor()));
    auto alloc(boost::asio::get_associated_allocator(handler));
    ex.post([handler, error, value]() mutable {
                handler(make_error_code(error), value);
            },
            alloc);
}

} // namespace boost
} // namespace http
