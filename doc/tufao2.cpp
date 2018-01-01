inline bool WebSocketHttpClient::execute(QByteArray &chunk)
{
    if (errored)
        return false;

    parser.set_buffer(asio::buffer(chunk.data(), chunk.size()));

    std::size_t nparsed = 0;

    while(parser.code() != http::token::code::error_insufficient_data) {
        switch(parser.code()) {
        case http::token::code::error_set_method:
            qFatal("unreachable: we did call `set_method`");
            break;
        case http::token::code::error_use_another_connection:
            errored = true;
            return false;
        case http::token::code::error_invalid_data:
            errored = true;
            return false;
        case http::token::code::error_no_host:
            qFatal("unreachable");
            break;
        case http::token::code::error_invalid_content_length:
            errored = true;
            return false;
        case http::token::code::error_content_length_overflow:
            errored = true;
            return false;
        case http::token::code::error_invalid_transfer_encoding:
            errored = true;
            return false;
        case http::token::code::error_chunk_size_overflow:
            errored = true;
            return false;
        case http::token::code::skip:
            break;
        case http::token::code::method:
            qFatal("unreachable");
            break;
        case http::token::code::request_target:
            qFatal("unreachable");
            break;
        case http::token::code::version:
            if (parser.value<http::token::version>() == 0) {
                errored = true;
                return false;
            }

            break;
        case http::token::code::status_code:
            status_code = parser.value<http::token::status_code>();
            if (status_code != 101) {
                errored = true;
                return false;
            }

            parser.set_method("GET");
            break;
        case http::token::code::reason_phrase:
            break;
        case http::token::code::field_name:
            {
                auto value = parser.value<http::token::field_name>();
                lastHeader = QByteArray(value.data(), value.size());
            }
            break;
        case http::token::code::field_value:
            {
                auto value = parser.value<http::token::field_value>();
                QByteArray header(value.data(), value.size());
                headers.insert(lastHeader, std::move(header));
                lastHeader.clear();
            }
            break;
        case http::token::code::end_of_headers:
            break;
        case http::token::code::body_chunk:
            break;
        case http::token::code::end_of_body:
            break;
        case http::token::code::trailer_name:
            break;
        case http::token::code::trailer_value:
            break;
        case http::token::code::end_of_message:
            ready = true;
            parser.set_buffer(asio::buffer(chunk.data() + nparsed,
                                           parser.token_size()));
            break;
        }

        nparsed += parser.token_size();
        parser.next();
    }
    nparsed += parser.token_size();
    parser.next();
    chunk.remove(0, nparsed);

    if (ready && headers.contains("Upgrade"))
        return true;

    return false;
}
