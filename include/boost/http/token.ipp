/* Copyright (c) 2018 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

namespace boost {
namespace http {
namespace token {

inline symbol::value symbol::convert(code::value v)
{
    switch (v) {
    case code::error_insufficient_data:
    case code::error_set_method:
    case code::error_use_another_connection:
    case code::error_invalid_data:
    case code::error_no_host:
    case code::error_invalid_content_length:
    case code::error_content_length_overflow:
    case code::error_invalid_transfer_encoding:
    case code::error_chunk_size_overflow:
        return error;
    case code::skip:
        return skip;
    case code::method:
        return method;
    case code::request_target:
        return request_target;
    case code::version:
        return version;
    case code::status_code:
        return status_code;
    case code::reason_phrase:
        return reason_phrase;
    case code::field_name:
        return field_name;
    case code::field_value:
        return field_value;
    case code::end_of_headers:
        return end_of_headers;
    case code::chunk_ext:
        return chunk_ext;
    case code::body_chunk:
        return body_chunk;
    case code::end_of_body:
        return end_of_body;
    case code::trailer_name:
        return trailer_name;
    case code::trailer_value:
        return trailer_value;
    case code::end_of_message:
        return end_of_message;
    }
}

inline category::value category::convert(code::value v)
{
    switch (v) {
    case code::error_insufficient_data:
    case code::error_set_method:
    case code::error_use_another_connection:
    case code::error_invalid_data:
    case code::error_no_host:
    case code::error_invalid_content_length:
    case code::error_content_length_overflow:
    case code::error_invalid_transfer_encoding:
    case code::error_chunk_size_overflow:
    case code::skip:
        return status;
    case code::method:
    case code::request_target:
    case code::version:
    case code::status_code:
    case code::reason_phrase:
    case code::field_name:
    case code::field_value:
    case code::chunk_ext:
    case code::body_chunk:
    case code::trailer_name:
    case code::trailer_value:
        return data;
    case code::end_of_headers:
    case code::end_of_body:
    case code::end_of_message:
        return structural;
    }
}

inline category::value category::convert(symbol::value v)
{
    switch (v) {
    case symbol::error:
    case symbol::skip:
        return status;
    case symbol::method:
    case symbol::request_target:
    case symbol::version:
    case symbol::status_code:
    case symbol::reason_phrase:
    case symbol::field_name:
    case symbol::field_value:
    case symbol::chunk_ext:
    case symbol::body_chunk:
    case symbol::trailer_name:
    case symbol::trailer_value:
        return data;
    case symbol::end_of_headers:
    case symbol::end_of_body:
    case symbol::end_of_message:
        return structural;
    }
}

} // namespace token
} // namespace http
} // namespace boost
