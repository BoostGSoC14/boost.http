#include "catch.hpp"
#include <boost/http/token.hpp>
#include <vector>
#include <boost/asio/buffer.hpp>

namespace Catch {
    std::string toString(boost::http::token::code::value value) {
        switch (value) {
        case boost::http::token::code::end_of_message:
            return "end_of_message";
        case boost::http::token::code::error_set_method:
            return "error_set_method";
        case boost::http::token::code::error_use_another_connection:
            return "error_use_another_connection";
        case boost::http::token::code::skip:
            return "skip";
        case boost::http::token::code::error_insufficient_data:
            return "error_insufficient_data";
        case boost::http::token::code::error_invalid_data:
            return "error_invalid_data";
        case boost::http::token::code::error_no_host:
            return "error_no_host";
        case boost::http::token::code::error_invalid_content_length:
            return "error_invalid_content_length";
        case boost::http::token::code::error_content_length_overflow:
            return "error_content_length_overflow";
        case boost::http::token::code::error_invalid_transfer_encoding:
            return "error_invalid_transfer_encoding";
        case boost::http::token::code::error_chunk_size_overflow:
            return "error_chunk_size_overflow";
        case boost::http::token::code::field_name:
            return "field_name";
        case boost::http::token::code::field_value:
            return "field_value";
        case boost::http::token::code::body_chunk:
            return "body_chunk";
        case boost::http::token::code::end_of_headers:
            return "end_of_headers";
        case boost::http::token::code::end_of_body:
            return "end_of_body";
        case boost::http::token::code::method:
            return "method";
        case boost::http::token::code::request_target:
            return "request_target";
        case boost::http::token::code::version:
            return "version";
        case boost::http::token::code::status_code:
            return "status_code";
        case boost::http::token::code::reason_phrase:
            return "reason_phrase";
        }
    }
}

template<std::size_t N>
boost::asio::const_buffer my_buffer(const char (&in)[N])
{
    return boost::asio::buffer(in, N - 1);
}

template<std::size_t N, class CharT>
void my_copy(std::vector<CharT> &out, std::size_t out_idx, const char (&in)[N])
{
    for (std::size_t i = 0 ; i != N - 1 ; ++i)
        out[i + out_idx] = in[i];
}
