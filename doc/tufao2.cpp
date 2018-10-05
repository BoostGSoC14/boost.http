inline bool WebSocketHttpClient::execute(QByteArray &chunk)
{
    if (errored)
        return false;

    parser.set_buffer(asio::buffer(chunk.data(), chunk.size()));

    while(parser.code() != http::token::code::error_insufficient_data) {
        switch(parser.symbol()) {
        case http::token::symbol::error:
            errored = true;
            return false;
        case http::token::symbol::skip:
            break;
        case http::token::symbol::method:
            qFatal("unreachable");
            break;
        case http::token::symbol::request_target:
            qFatal("unreachable");
            break;
        case http::token::symbol::version:
            if (parser.value<http::token::version>() == 0) {
                errored = true;
                return false;
            }

            break;
        case http::token::symbol::status_code:
            status_code = parser.value<http::token::status_code>();
            if (status_code != 101) {
                errored = true;
                return false;
            }

            parser.set_method("GET");
            break;
        case http::token::symbol::reason_phrase:
            break;
        case http::token::symbol::field_name:
            {
                auto value = parser.value<http::token::field_name>();
                lastHeader = QByteArray(value.data(), value.size());
            }
            break;
        case http::token::symbol::field_value:
            {
                auto value = parser.value<http::token::field_value>();
                QByteArray header(value.data(), value.size());
                headers.insert(lastHeader, std::move(header));
                lastHeader.clear();
            }
            break;
        case http::token::symbol::end_of_headers:
            break;
        case http::token::symbol::body_chunk:
            break;
        case http::token::symbol::end_of_body:
            break;
        case http::token::symbol::trailer_name:
            break;
        case http::token::symbol::trailer_value:
            break;
        case http::token::symbol::end_of_message:
            ready = true;
            chunk.remove(0, parser.parsed_count());
            parser.set_buffer(asio::buffer(chunk.data(),
                                           parser.token_size()));
            break;
        }

        parser.next();
    }
    chunk.remove(0, parser.parsed_count());

    if (ready && headers.contains("Upgrade"))
        return true;

    return false;
}
