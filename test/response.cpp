#ifdef NDEBUG
#undef NDEBUG
#endif

#define CATCH_CONFIG_MAIN
#include "common.hpp"
#include <boost/http/reader/response.hpp>

namespace asio = boost::asio;
namespace http = boost::http;
namespace reader = http::reader;

TEST_CASE("Parse a few pipelined non-fragmented/whole responses",
          "[parser,good]")
{
    http::reader::response parser;

    REQUIRE(parser.code() == http::token::code::error_insufficient_data);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.set_buffer(my_buffer("HTTP/1.1 200 OK\r\n"
                                "Server: Boost master race\r\n"
                                "Content-Length: 0\r\n"
                                "\r\n"

                                // SECOND RESPONSE
                                "HTTP/1.1 200 OK\r\n"
                                "Transfer-Encoding: chunked\r\n"
                                "\r\n"

                                // THIRD RESPONSE
                                "HTTP/1.1 200 OK\r\n"
                                "Transfer-Encoding: chunked\r\n"
                                "\r\n"));

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 7);
    REQUIRE(parser.expected_token() == http::token::code::version);

    parser.next();

    REQUIRE(parser.code() == http::token::code::version);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.value<http::token::version>() == 1);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.expected_token() == http::token::code::status_code);

    parser.next();

    REQUIRE(parser.code() == http::token::code::status_code);
    REQUIRE(parser.token_size() == 3);
    REQUIRE(parser.value<http::token::status_code>() == 200);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.set_method("GET");

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.expected_token() == http::token::code::reason_phrase);

    parser.next();

    REQUIRE(parser.code() == http::token::code::reason_phrase);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.value<http::token::reason_phrase>() == "OK");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_name);
    REQUIRE(parser.token_size() == 6);
    REQUIRE(parser.value<http::token::field_name>() == "Server");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_value);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_value);
    REQUIRE(parser.token_size() == 17);
    REQUIRE(parser.value<http::token::field_value>() == "Boost master race");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_name);
    REQUIRE(parser.token_size() == 14);
    REQUIRE(parser.value<http::token::field_name>() == "Content-Length");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_value);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_value);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.value<http::token::field_value>() == "0");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_headers);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::end_of_body);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_body);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token() == http::token::code::end_of_message);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_message);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    // SECOND RESPONSE

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 7);
    REQUIRE(parser.expected_token() == http::token::code::version);

    parser.next();

    REQUIRE(parser.code() == http::token::code::version);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.value<http::token::version>() == 1);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.expected_token() == http::token::code::status_code);

    parser.next();

    REQUIRE(parser.code() == http::token::code::status_code);
    REQUIRE(parser.token_size() == 3);
    REQUIRE(parser.value<http::token::status_code>() == 200);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.set_method("HEAD");

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.expected_token() == http::token::code::reason_phrase);

    parser.next();

    REQUIRE(parser.code() == http::token::code::reason_phrase);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.value<http::token::reason_phrase>() == "OK");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_name);
    REQUIRE(parser.token_size() == 17);
    REQUIRE(parser.value<http::token::field_name>() == "Transfer-Encoding");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_value);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_value);
    REQUIRE(parser.token_size() == 7);
    REQUIRE(parser.value<http::token::field_value>() == "chunked");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_headers);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::end_of_body);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_body);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token() == http::token::code::end_of_message);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_message);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    // THIRD RESPONSE

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 7);
    REQUIRE(parser.expected_token() == http::token::code::version);

    parser.next();

    REQUIRE(parser.code() == http::token::code::version);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.value<http::token::version>() == 1);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.expected_token() == http::token::code::status_code);

    parser.next();

    REQUIRE(parser.code() == http::token::code::status_code);
    REQUIRE(parser.token_size() == 3);
    REQUIRE(parser.value<http::token::status_code>() == 200);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.set_method("CONNECT");

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.expected_token() == http::token::code::reason_phrase);

    parser.next();

    REQUIRE(parser.code() == http::token::code::reason_phrase);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.value<http::token::reason_phrase>() == "OK");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_name);
    REQUIRE(parser.token_size() == 17);
    REQUIRE(parser.value<http::token::field_name>() == "Transfer-Encoding");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_value);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_value);
    REQUIRE(parser.token_size() == 7);
    REQUIRE(parser.value<http::token::field_value>() == "chunked");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_headers);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::end_of_body);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_body);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token() == http::token::code::end_of_message);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_message);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token()
            == http::token::code::error_use_another_connection);

    parser.next();

    REQUIRE(parser.code() == http::token::code::error_use_another_connection);
}

TEST_CASE("Parse a single message with no body", "[parser,good]")
{
    http::reader::response parser;

    REQUIRE(parser.code() == http::token::code::error_insufficient_data);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    /* DO NOT increase buffer length. This test is intended to make sure parser
       doesn't stay stuck after headers waiting for some byte before emitting
       delimiters. */
    parser.set_buffer(my_buffer("HTTP/1.1 200 OK\r\n"
                                "Server: Boost master race\r\n"
                                "Content-Length: 0\r\n"
                                "\r\n"));

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 7);
    REQUIRE(parser.expected_token() == http::token::code::version);

    parser.next();

    REQUIRE(parser.code() == http::token::code::version);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.value<http::token::version>() == 1);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.expected_token() == http::token::code::status_code);

    parser.next();

    REQUIRE(parser.code() == http::token::code::status_code);
    REQUIRE(parser.token_size() == 3);
    REQUIRE(parser.value<http::token::status_code>() == 200);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.set_method("GET");
    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.expected_token() == http::token::code::reason_phrase);

    parser.next();

    REQUIRE(parser.code() == http::token::code::reason_phrase);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.value<http::token::reason_phrase>() == "OK");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_name);
    REQUIRE(parser.token_size() == 6);
    REQUIRE(parser.value<http::token::field_name>() == "Server");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_value);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_value);
    REQUIRE(parser.token_size() == 17);
    REQUIRE(parser.value<http::token::field_value>() == "Boost master race");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_name);
    REQUIRE(parser.token_size() == 14);
    REQUIRE(parser.value<http::token::field_name>() == "Content-Length");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_value);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_value);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.value<http::token::field_value>() == "0");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_headers);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::end_of_body);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_body);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token() == http::token::code::end_of_message);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_message);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token() == http::token::code::skip);
}

TEST_CASE("Test varying body types", "[parser,good]")
{
    http::reader::response parser;

    REQUIRE(parser.code() == http::token::code::error_insufficient_data);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.set_buffer(my_buffer("HTTP/1.1 200 OK\r\n"
                                "Server: Boost master race\r\n"
                                "Content-Length: 2\r\n"
                                "\r\n"

                                "HI"

                                // SECOND RESPONSE
                                "HTTP/1.1 200 OK\r\n"
                                "Transfer-Encoding: chunked\r\n"
                                "\r\n"

                                "1\r\n"
                                "a\r\n"

                                "0\r\n"
                                "\r\n"

                                // THIRD RESPONSE
                                "HTTP/1.1 200 OK\r\n"
                                "\r\n"
                                "Raphinha legal"));

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 7);
    REQUIRE(parser.expected_token() == http::token::code::version);

    parser.next();

    REQUIRE(parser.code() == http::token::code::version);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.value<http::token::version>() == 1);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.expected_token() == http::token::code::status_code);

    parser.next();

    REQUIRE(parser.code() == http::token::code::status_code);
    REQUIRE(parser.token_size() == 3);
    REQUIRE(parser.value<http::token::status_code>() == 200);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.set_method("GET");

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.expected_token() == http::token::code::reason_phrase);

    parser.next();

    REQUIRE(parser.code() == http::token::code::reason_phrase);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.value<http::token::reason_phrase>() == "OK");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_name);
    REQUIRE(parser.token_size() == 6);
    REQUIRE(parser.value<http::token::field_name>() == "Server");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_value);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_value);
    REQUIRE(parser.token_size() == 17);
    REQUIRE(parser.value<http::token::field_value>() == "Boost master race");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_name);
    REQUIRE(parser.token_size() == 14);
    REQUIRE(parser.value<http::token::field_name>() == "Content-Length");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_value);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_value);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.value<http::token::field_value>() == "2");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_headers);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::body_chunk);

    parser.next();

    REQUIRE(parser.code() == http::token::code::body_chunk);
    REQUIRE(parser.token_size() == 2);
    {
        asio::const_buffer body = parser.value<http::token::body_chunk>();
        REQUIRE(body.size() == 2);
        const char *buf_view = static_cast<const char*>(body.data());
        REQUIRE(buf_view[0] == 'H');
        REQUIRE(buf_view[1] == 'I');
    }
    REQUIRE(parser.expected_token() == http::token::code::end_of_body);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_body);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token() == http::token::code::end_of_message);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_message);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    // SECOND RESPONSE

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 7);
    REQUIRE(parser.expected_token() == http::token::code::version);

    parser.next();

    REQUIRE(parser.code() == http::token::code::version);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.value<http::token::version>() == 1);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.expected_token() == http::token::code::status_code);

    parser.next();

    REQUIRE(parser.code() == http::token::code::status_code);
    REQUIRE(parser.token_size() == 3);
    REQUIRE(parser.value<http::token::status_code>() == 200);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.set_method("GET");

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.expected_token() == http::token::code::reason_phrase);

    parser.next();

    REQUIRE(parser.code() == http::token::code::reason_phrase);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.value<http::token::reason_phrase>() == "OK");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_name);
    REQUIRE(parser.token_size() == 17);
    REQUIRE(parser.value<http::token::field_name>() == "Transfer-Encoding");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_value);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_value);
    REQUIRE(parser.token_size() == 7);
    REQUIRE(parser.value<http::token::field_value>() == "chunked");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_headers);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.expected_token() == http::token::code::chunk_ext);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::body_chunk);

    parser.next();

    REQUIRE(parser.code() == http::token::code::body_chunk);
    REQUIRE(parser.token_size() == 1);
    {
        asio::const_buffer body = parser.value<http::token::body_chunk>();
        REQUIRE(body.size() == 1);
        REQUIRE(*(static_cast<const char*>(body.data())) == 'a');
    }
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.expected_token() == http::token::code::chunk_ext);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_body);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::trailer_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_message);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    // THIRD RESPONSE

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 7);
    REQUIRE(parser.expected_token() == http::token::code::version);

    parser.next();

    REQUIRE(parser.code() == http::token::code::version);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.value<http::token::version>() == 1);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.expected_token() == http::token::code::status_code);

    parser.next();

    REQUIRE(parser.code() == http::token::code::status_code);
    REQUIRE(parser.token_size() == 3);
    REQUIRE(parser.value<http::token::status_code>() == 200);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.set_method("GET");

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.expected_token() == http::token::code::reason_phrase);

    parser.next();

    REQUIRE(parser.code() == http::token::code::reason_phrase);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.value<http::token::reason_phrase>() == "OK");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_headers);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::body_chunk);

    parser.next();

    REQUIRE(parser.code() == http::token::code::body_chunk);
    REQUIRE(parser.token_size() == 14);
    {
        asio::const_buffer body = parser.value<http::token::body_chunk>();
        REQUIRE(body.size() == 14);
        const char *buf_view = static_cast<const char*>(body.data());
        REQUIRE(buf_view[0] == 'R');
        REQUIRE(buf_view[1] == 'a');
        REQUIRE(buf_view[2] == 'p');
        REQUIRE(buf_view[3] == 'h');
        REQUIRE(buf_view[4] == 'i');
        REQUIRE(buf_view[5] == 'n');
        REQUIRE(buf_view[6] == 'h');
        REQUIRE(buf_view[7] == 'a');
        REQUIRE(buf_view[8] == ' ');
        REQUIRE(buf_view[9] == 'l');
        REQUIRE(buf_view[10] == 'e');
        REQUIRE(buf_view[11] == 'g');
        REQUIRE(buf_view[12] == 'a');
        REQUIRE(buf_view[13] == 'l');
    }
    REQUIRE(parser.expected_token() == http::token::code::body_chunk);

    parser.next();

    REQUIRE(parser.code() == http::token::code::error_insufficient_data);

    parser.puteof();

    REQUIRE(parser.code() == http::token::code::error_insufficient_data);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_body);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token() == http::token::code::end_of_message);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_message);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token()
            == http::token::code::error_use_another_connection);

    parser.next();

    REQUIRE(parser.code() == http::token::code::error_use_another_connection);
}

TEST_CASE("puteof", "[parser,good]")
{
    http::reader::response parser;

    REQUIRE(parser.code() == http::token::code::error_insufficient_data);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.set_buffer(my_buffer("HTTP/1.1 200 OK\r\n"
                                "\r\n"
                                "jaqueta"));

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 7);
    REQUIRE(parser.expected_token() == http::token::code::version);

    parser.next();

    REQUIRE(parser.code() == http::token::code::version);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.value<http::token::version>() == 1);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.expected_token() == http::token::code::status_code);

    parser.next();

    REQUIRE(parser.code() == http::token::code::status_code);
    REQUIRE(parser.token_size() == 3);
    REQUIRE(parser.value<http::token::status_code>() == 200);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.set_method("GET");

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.expected_token() == http::token::code::reason_phrase);

    parser.next();

    REQUIRE(parser.code() == http::token::code::reason_phrase);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.value<http::token::reason_phrase>() == "OK");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_headers);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::body_chunk);

    parser.next();

    REQUIRE(parser.code() == http::token::code::body_chunk);
    REQUIRE(parser.token_size() == 7);
    {
        asio::const_buffer body = parser.value<http::token::body_chunk>();
        REQUIRE(body.size() == 7);
        const char *buf_view = static_cast<const char*>(body.data());
        REQUIRE(buf_view[0] == 'j');
        REQUIRE(buf_view[1] == 'a');
        REQUIRE(buf_view[2] == 'q');
        REQUIRE(buf_view[3] == 'u');
        REQUIRE(buf_view[4] == 'e');
        REQUIRE(buf_view[5] == 't');
        REQUIRE(buf_view[6] == 'a');
    }
    REQUIRE(parser.expected_token() == http::token::code::body_chunk);

    parser.next();

    REQUIRE(parser.code() == http::token::code::error_insufficient_data);

    parser.puteof();

    REQUIRE(parser.code() == http::token::code::error_insufficient_data);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_body);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token() == http::token::code::end_of_message);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_message);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token()
            == http::token::code::error_use_another_connection);

    parser.next();

    REQUIRE(parser.code() == http::token::code::error_use_another_connection);
}

TEST_CASE("puteof2", "[parser,good]")
{
    http::reader::response parser;

    REQUIRE(parser.code() == http::token::code::error_insufficient_data);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.set_buffer(my_buffer("HTTP/1.0 200 OK\r\n"
                                "\r\n"
                                "jaqueta"));

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 7);
    REQUIRE(parser.expected_token() == http::token::code::version);

    parser.next();

    REQUIRE(parser.code() == http::token::code::version);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.value<http::token::version>() == 0);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.expected_token() == http::token::code::status_code);

    parser.next();

    REQUIRE(parser.code() == http::token::code::status_code);
    REQUIRE(parser.token_size() == 3);
    REQUIRE(parser.value<http::token::status_code>() == 200);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.set_method("GET");

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.expected_token() == http::token::code::reason_phrase);

    parser.next();

    REQUIRE(parser.code() == http::token::code::reason_phrase);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.value<http::token::reason_phrase>() == "OK");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_headers);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::body_chunk);

    parser.next();

    REQUIRE(parser.code() == http::token::code::body_chunk);
    REQUIRE(parser.token_size() == 7);
    {
        asio::const_buffer body = parser.value<http::token::body_chunk>();
        REQUIRE(body.size() == 7);
        const char *buf_view = static_cast<const char*>(body.data());
        REQUIRE(buf_view[0] == 'j');
        REQUIRE(buf_view[1] == 'a');
        REQUIRE(buf_view[2] == 'q');
        REQUIRE(buf_view[3] == 'u');
        REQUIRE(buf_view[4] == 'e');
        REQUIRE(buf_view[5] == 't');
        REQUIRE(buf_view[6] == 'a');
    }
    REQUIRE(parser.expected_token() == http::token::code::body_chunk);

    parser.next();

    REQUIRE(parser.code() == http::token::code::error_insufficient_data);

    parser.puteof();

    REQUIRE(parser.code() == http::token::code::error_insufficient_data);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_body);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token() == http::token::code::end_of_message);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_message);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token()
            == http::token::code::error_use_another_connection);

    parser.next();

    REQUIRE(parser.code() == http::token::code::error_use_another_connection);
}

TEST_CASE("eof without puteof", "[parser,good]")
{
    http::reader::response parser;

    REQUIRE(parser.code() == http::token::code::error_insufficient_data);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.set_buffer(my_buffer("HTTP/1.0 200 OK\r\n"
                                "content-length: 7\r\n"
                                "\r\n"
                                "jaqueta"));

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 7);
    REQUIRE(parser.expected_token() == http::token::code::version);

    parser.next();

    REQUIRE(parser.code() == http::token::code::version);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.value<http::token::version>() == 0);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.expected_token() == http::token::code::status_code);

    parser.next();

    REQUIRE(parser.code() == http::token::code::status_code);
    REQUIRE(parser.token_size() == 3);
    REQUIRE(parser.value<http::token::status_code>() == 200);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.set_method("GET");

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.expected_token() == http::token::code::reason_phrase);

    parser.next();

    REQUIRE(parser.code() == http::token::code::reason_phrase);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.value<http::token::reason_phrase>() == "OK");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_name);
    REQUIRE(parser.token_size() == 14);
    REQUIRE(parser.value<http::token::field_name>() == "content-length");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_value);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_value);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.value<http::token::field_value>() == "7");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_headers);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::body_chunk);

    parser.next();

    REQUIRE(parser.code() == http::token::code::body_chunk);
    REQUIRE(parser.token_size() == 7);
    {
        asio::const_buffer body = parser.value<http::token::body_chunk>();
        REQUIRE(body.size() == 7);
        const char *buf_view = static_cast<const char*>(body.data());
        REQUIRE(buf_view[0] == 'j');
        REQUIRE(buf_view[1] == 'a');
        REQUIRE(buf_view[2] == 'q');
        REQUIRE(buf_view[3] == 'u');
        REQUIRE(buf_view[4] == 'e');
        REQUIRE(buf_view[5] == 't');
        REQUIRE(buf_view[6] == 'a');
    }
    REQUIRE(parser.expected_token() == http::token::code::end_of_body);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_body);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token() == http::token::code::end_of_message);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_message);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token()
            == http::token::code::error_use_another_connection);

    parser.next();

    REQUIRE(parser.code() == http::token::code::error_use_another_connection);
}

TEST_CASE("eof before end", "[parser,bad]")
{
    http::reader::response parser;

    REQUIRE(parser.code() == http::token::code::error_insufficient_data);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.set_buffer(my_buffer("HTTP/1.0 200 OK\r\n"
                                "content-length: 7\r\n"
                                "\r\n"
                                "jaquet"));

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 7);
    REQUIRE(parser.expected_token() == http::token::code::version);

    parser.next();

    REQUIRE(parser.code() == http::token::code::version);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.value<http::token::version>() == 0);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.expected_token() == http::token::code::status_code);

    parser.next();

    REQUIRE(parser.code() == http::token::code::status_code);
    REQUIRE(parser.token_size() == 3);
    REQUIRE(parser.value<http::token::status_code>() == 200);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.set_method("GET");

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.expected_token() == http::token::code::reason_phrase);

    parser.next();

    REQUIRE(parser.code() == http::token::code::reason_phrase);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.value<http::token::reason_phrase>() == "OK");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_name);
    REQUIRE(parser.token_size() == 14);
    REQUIRE(parser.value<http::token::field_name>() == "content-length");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_value);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_value);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.value<http::token::field_value>() == "7");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_headers);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::body_chunk);

    parser.next();

    REQUIRE(parser.code() == http::token::code::body_chunk);
    REQUIRE(parser.token_size() == 6);
    {
        asio::const_buffer body = parser.value<http::token::body_chunk>();
        REQUIRE(body.size() == 6);
        const char *buf_view = static_cast<const char*>(body.data());
        REQUIRE(buf_view[0] == 'j');
        REQUIRE(buf_view[1] == 'a');
        REQUIRE(buf_view[2] == 'q');
        REQUIRE(buf_view[3] == 'u');
        REQUIRE(buf_view[4] == 'e');
        REQUIRE(buf_view[5] == 't');
    }
    REQUIRE(parser.expected_token() == http::token::code::body_chunk);

    parser.next();

    REQUIRE(parser.code() == http::token::code::error_insufficient_data);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token() == http::token::code::body_chunk);

    parser.puteof();
    parser.next();

    REQUIRE(parser.code() == http::token::code::error_insufficient_data);
    REQUIRE(parser.token_size() == 0);
}

TEST_CASE("response smuggling", "[parser,attack]")
{
    // The layer above can detect the smuggling attempt and take the appropriate
    // measures. At the parser layer here, we can understand the server was
    // tricked, and the message that should be delivered — server's message or
    // forgery attempt — is obviously the server's (which in the real-world it
    // is likely to be garbage anyway, but it is for the next layer to
    // resolve). HTTP/1.0 servers know no encoding.
    http::reader::response parser;

    REQUIRE(parser.code() == http::token::code::error_insufficient_data);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.set_buffer(my_buffer("HTTP/1.0 200 OK\r\n"
                                "content-length: 7\r\n"
                                "transfer-encoding: chunked\r\n"
                                "\r\n"
                                "jaqueta"));

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 7);
    REQUIRE(parser.expected_token() == http::token::code::version);

    parser.next();

    REQUIRE(parser.code() == http::token::code::version);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.value<http::token::version>() == 0);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.expected_token() == http::token::code::status_code);

    parser.next();

    REQUIRE(parser.code() == http::token::code::status_code);
    REQUIRE(parser.token_size() == 3);
    REQUIRE(parser.value<http::token::status_code>() == 200);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.set_method("GET");

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.expected_token() == http::token::code::reason_phrase);

    parser.next();

    REQUIRE(parser.code() == http::token::code::reason_phrase);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.value<http::token::reason_phrase>() == "OK");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_name);
    REQUIRE(parser.token_size() == 14);
    REQUIRE(parser.value<http::token::field_name>() == "content-length");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_value);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_value);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.value<http::token::field_value>() == "7");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_name);
    REQUIRE(parser.token_size() == 17);
    REQUIRE(parser.value<http::token::field_name>() == "transfer-encoding");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_value);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_value);
    REQUIRE(parser.token_size() == 7);
    REQUIRE(parser.value<http::token::field_value>() == "chunked");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_headers);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::body_chunk);

    parser.next();

    REQUIRE(parser.code() == http::token::code::body_chunk);
    REQUIRE(parser.token_size() == 7);
    {
        asio::const_buffer body = parser.value<http::token::body_chunk>();
        REQUIRE(body.size() == 7);
        const char *buf_view = static_cast<const char*>(body.data());
        REQUIRE(buf_view[0] == 'j');
        REQUIRE(buf_view[1] == 'a');
        REQUIRE(buf_view[2] == 'q');
        REQUIRE(buf_view[3] == 'u');
        REQUIRE(buf_view[4] == 'e');
        REQUIRE(buf_view[5] == 't');
        REQUIRE(buf_view[6] == 'a');
    }
    REQUIRE(parser.expected_token() == http::token::code::end_of_body);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_body);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token() == http::token::code::end_of_message);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_message);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token()
            == http::token::code::error_use_another_connection);

    parser.next();

    REQUIRE(parser.code() == http::token::code::error_use_another_connection);
}
