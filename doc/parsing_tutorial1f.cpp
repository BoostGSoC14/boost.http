void my_socket_consumer::on_socket_callback(asio::buffer data)
{
    using namespace http::token;
    using token::code;

    buffer.push_back(data);
    request_reader.set_buffer(buffer);

    std::size_t nparsed = 0;

    while (request_reader.code() != code::error_insufficient_data
           && request_reader.code() != code::end_of_message) {
        switch (request_reader.code()) {
        // ...
        case code::skip:
            break;
        case code::method:
            method = request_reader.value<token::method>();
            break;
        case code::request_target:
            request_target = request_reader.value<token::request_target>();
            break;
        case code::version:
            version = request_reader.value<token::version>();
            break;
        case code::field_name:
        case code::trailer_name:
            last_header = request_reader.value<token::field_name>();
            break;
        case code::field_value: //< NEW
        case code::trailer_value: //< NEW
            // NEW
            headers.emplace(last_header,
                            request_reader.value<token::field_value>());
        }

        nparsed += request_reader.token_size();
        request_reader.next();
    }
    nparsed += request_reader.token_size();
    request_reader.next();
    buffer.erase(0, nparsed);

    if (request_reader.code() == code::error_insufficient_data)
        return;

    ready();
}
