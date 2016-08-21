#ifdef NDEBUG
#undef NDEBUG
#endif

#define CATCH_CONFIG_MAIN
#include "common.hpp"
#include <boost/http/reader/request.hpp>

namespace asio = boost::asio;
namespace http = boost::http;
namespace reader = http::reader;

TEST_CASE("Parse a few (good,bad) pipelined non-fragmented/whole requests",
          "[parser,bad]")
{
    /* A really large buffer just to be sure it'll be enough to hold the second
       request. The invalid bytes that follow shouldn't matter as this parser is
       incremental and MUST **NOT** consume tokens ahead of asked (and it'll
       error sooner anyway with the bad request). */
    std::vector<char> buf(5000);

    // Index of the second request
    std::size_t idx;

    {
        const char req[]
            = ("GET / HTTP/1.1\r\n"
               "hOST: gaimanoncopyright.thoughts\r\n"
               "\r\n");
        idx = sizeof(req) - 1;

        my_copy(buf, 0, req);
    }

    http::reader::request parser;

    REQUIRE(parser.code() == http::token::code::error_insufficient_data);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token() == http::token::code::method);

    parser.set_buffer(asio::buffer(buf));
    parser.next();

    REQUIRE(parser.code() == http::token::code::method);
    REQUIRE(parser.token_size() == 3);
    REQUIRE(parser.value<http::token::method>() == "GET");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.expected_token() == http::token::code::request_target);

    parser.next();

    REQUIRE(parser.code() == http::token::code::request_target);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.value<http::token::request_target>() == "/");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 8);
    REQUIRE(parser.expected_token() == http::token::code::version);

    parser.next();

    REQUIRE(parser.code() == http::token::code::version);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.value<http::token::version>() == 1);
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_name);
    REQUIRE(parser.token_size() == 4);
    REQUIRE(parser.value<http::token::field_name>() == "hOST");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_value);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_value);
    REQUIRE(parser.token_size() == 26);
    REQUIRE(parser.value<http::token::field_value>()
            == "gaimanoncopyright.thoughts");
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
    REQUIRE(parser.expected_token() == http::token::code::method);

    SECTION("Just to be sure 0000 is accepted as last chunk") {
        my_copy(buf, idx,
                "POST / HTTP/1.1\r\n"
                "host:     burn.burn\r\n"
                "Transfer-Encoding: chunked\r\n"
                "\r\n"

                "00000000000000000000000000000000000000000000\r\n"
                "\r\n");

        parser.next();

        REQUIRE(parser.code() == http::token::code::method);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::method>() == "POST");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.expected_token() == http::token::code::request_target);

        parser.next();

        REQUIRE(parser.code() == http::token::code::request_target);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::request_target>() == "/");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 8);
        REQUIRE(parser.expected_token() == http::token::code::version);

        parser.next();

        REQUIRE(parser.code() == http::token::code::version);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::version>() == 1);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_name);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_name);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::field_name>() == "host");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 6);
        REQUIRE(parser.expected_token() == http::token::code::field_value);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_value);
        REQUIRE(parser.token_size() == 9);
        REQUIRE(parser.value<http::token::field_value>() == "burn.burn");
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
        REQUIRE(parser.token_size() == 44);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::end_of_body);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_name);

        parser.next();

        REQUIRE(parser.code() == http::token::code::end_of_message);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::method);
    }

    SECTION("Invalid chunk size") {
        /* We don't use a special error code for invalid chunk size as we use
           for Content-Length for the same reason we don't use special error
           codes for most tokens, compactness and usefulness. Content-Length is
           special because it is an exposed header field the user can check. */
        my_copy(buf, idx,
                "POST / HTTP/1.1\r\n"
                "host:     burn.burn\r\n"
                "Transfer-Encoding: chunked\r\n"
                "\r\n"

                "-4\r\n"
                "Wiki\r\n"

                "0\r\n"
                "\r\n");

        parser.next();

        REQUIRE(parser.code() == http::token::code::method);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::method>() == "POST");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.expected_token() == http::token::code::request_target);

        parser.next();

        REQUIRE(parser.code() == http::token::code::request_target);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::request_target>() == "/");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 8);
        REQUIRE(parser.expected_token() == http::token::code::version);

        parser.next();

        REQUIRE(parser.code() == http::token::code::version);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::version>() == 1);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_name);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_name);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::field_name>() == "host");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 6);
        REQUIRE(parser.expected_token() == http::token::code::field_value);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_value);
        REQUIRE(parser.token_size() == 9);
        REQUIRE(parser.value<http::token::field_value>() == "burn.burn");
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

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Chunk size overflow") {
        my_copy(buf, idx,
                "POST / HTTP/1.1\r\n"
                "host:     burn.burn\r\n"
                "Transfer-Encoding: chunked\r\n"
                "\r\n"

                "999999999999999999999999999999999999999999\r\n"
                "Wiki\r\n"

                "0\r\n"
                "\r\n");

        parser.next();

        REQUIRE(parser.code() == http::token::code::method);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::method>() == "POST");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.expected_token() == http::token::code::request_target);

        parser.next();

        REQUIRE(parser.code() == http::token::code::request_target);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::request_target>() == "/");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 8);
        REQUIRE(parser.expected_token() == http::token::code::version);

        parser.next();

        REQUIRE(parser.code() == http::token::code::version);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::version>() == 1);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_name);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_name);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::field_name>() == "host");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 6);
        REQUIRE(parser.expected_token() == http::token::code::field_value);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_value);
        REQUIRE(parser.token_size() == 9);
        REQUIRE(parser.value<http::token::field_value>() == "burn.burn");
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

        REQUIRE(parser.code() == http::token::code::error_chunk_size_overflow);
    }

    SECTION("Invalid chunk-ext") {
        my_copy(buf, idx,
                "POST / HTTP/1.1\r\n"
                "host:     burn.burn\r\n"
                "Transfer-Encoding: chunked\r\n"
                "\r\n"

                "4;""\x7F""\r\n"
                "Wiki\r\n"

                "0\r\n"
                "\r\n");

        parser.next();

        REQUIRE(parser.code() == http::token::code::method);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::method>() == "POST");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.expected_token() == http::token::code::request_target);

        parser.next();

        REQUIRE(parser.code() == http::token::code::request_target);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::request_target>() == "/");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 8);
        REQUIRE(parser.expected_token() == http::token::code::version);

        parser.next();

        REQUIRE(parser.code() == http::token::code::version);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::version>() == 1);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_name);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_name);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::field_name>() == "host");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 6);
        REQUIRE(parser.expected_token() == http::token::code::field_value);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_value);
        REQUIRE(parser.token_size() == 9);
        REQUIRE(parser.value<http::token::field_value>() == "burn.burn");
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
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Invalid CRLF after chunk-ext") {
        my_copy(buf, idx,
                "POST / HTTP/1.1\r\n"
                "host:     burn.burn\r\n"
                "Transfer-Encoding: chunked\r\n"
                "\r\n"

                "4;FSF=good;ms=evil\r \n"
                "Wiki\r\n"

                "0\r\n"
                "\r\n");

        parser.next();

        REQUIRE(parser.code() == http::token::code::method);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::method>() == "POST");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.expected_token() == http::token::code::request_target);

        parser.next();

        REQUIRE(parser.code() == http::token::code::request_target);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::request_target>() == "/");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 8);
        REQUIRE(parser.expected_token() == http::token::code::version);

        parser.next();

        REQUIRE(parser.code() == http::token::code::version);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::version>() == 1);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_name);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_name);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::field_name>() == "host");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 6);
        REQUIRE(parser.expected_token() == http::token::code::field_value);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_value);
        REQUIRE(parser.token_size() == 9);
        REQUIRE(parser.value<http::token::field_value>() == "burn.burn");
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
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 17);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Invalid CRLF after chunk data") {
        my_copy(buf, idx,
                "POST / HTTP/1.1\r\n"
                "host:     burn.burn\r\n"
                "Transfer-Encoding: chunked\r\n"
                "\r\n"

                "4;FSF=good;ms=evil\r\n"
                "Wiki\r \n"

                "0\r\n"
                "\r\n");

        parser.next();

        REQUIRE(parser.code() == http::token::code::method);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::method>() == "POST");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.expected_token() == http::token::code::request_target);

        parser.next();

        REQUIRE(parser.code() == http::token::code::request_target);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::request_target>() == "/");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 8);
        REQUIRE(parser.expected_token() == http::token::code::version);

        parser.next();

        REQUIRE(parser.code() == http::token::code::version);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::version>() == 1);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_name);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_name);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::field_name>() == "host");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 6);
        REQUIRE(parser.expected_token() == http::token::code::field_value);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_value);
        REQUIRE(parser.token_size() == 9);
        REQUIRE(parser.value<http::token::field_value>() == "burn.burn");
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
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 17);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::body_chunk);

        parser.next();

        REQUIRE(parser.code() == http::token::code::body_chunk);
        REQUIRE(parser.token_size() == 4);
        {
            asio::const_buffer buf = parser.value<http::token::body_chunk>();
            REQUIRE(asio::buffer_size(buf) == 4);
            const char *view = asio::buffer_cast<const char*>(buf);
            REQUIRE(view[0] == 'W');
            REQUIRE(view[1] == 'i');
            REQUIRE(view[2] == 'k');
            REQUIRE(view[3] == 'i');
        }
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Forbidden whitespace before trailer name") {
        my_copy(buf, idx,
                "POST / HTTP/1.1\r\n"
                "host:     burn.burn\r\n"
                "Transfer-Encoding: chunked\r\n"
                "\r\n"

                "4;FSF=good;ms=evil\r\n"
                "Wiki\r\n"

                "0\r\n"
                " X-Pants: Off\r\n"
                "\r\n");

        parser.next();

        REQUIRE(parser.code() == http::token::code::method);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::method>() == "POST");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.expected_token() == http::token::code::request_target);

        parser.next();

        REQUIRE(parser.code() == http::token::code::request_target);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::request_target>() == "/");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 8);
        REQUIRE(parser.expected_token() == http::token::code::version);

        parser.next();

        REQUIRE(parser.code() == http::token::code::version);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::version>() == 1);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_name);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_name);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::field_name>() == "host");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 6);
        REQUIRE(parser.expected_token() == http::token::code::field_value);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_value);
        REQUIRE(parser.token_size() == 9);
        REQUIRE(parser.value<http::token::field_value>() == "burn.burn");
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
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 17);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::body_chunk);

        parser.next();

        REQUIRE(parser.code() == http::token::code::body_chunk);
        REQUIRE(parser.token_size() == 4);
        {
            asio::const_buffer buf = parser.value<http::token::body_chunk>();
            REQUIRE(asio::buffer_size(buf) == 4);
            const char *view = asio::buffer_cast<const char*>(buf);
            REQUIRE(view[0] == 'W');
            REQUIRE(view[1] == 'i');
            REQUIRE(view[2] == 'k');
            REQUIRE(view[3] == 'i');
        }
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::end_of_body);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_name);

        parser.next();

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Invalid trailer name") {
        my_copy(buf, idx,
                "POST / HTTP/1.1\r\n"
                "host:     burn.burn\r\n"
                "Transfer-Encoding: chunked\r\n"
                "\r\n"

                "4;FSF=good;ms=evil\r\n"
                "Wiki\r\n"

                "0\r\n"
                "@X-Pants: Off\r\n"
                "\r\n");

        parser.next();

        REQUIRE(parser.code() == http::token::code::method);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::method>() == "POST");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.expected_token() == http::token::code::request_target);

        parser.next();

        REQUIRE(parser.code() == http::token::code::request_target);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::request_target>() == "/");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 8);
        REQUIRE(parser.expected_token() == http::token::code::version);

        parser.next();

        REQUIRE(parser.code() == http::token::code::version);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::version>() == 1);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_name);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_name);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::field_name>() == "host");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 6);
        REQUIRE(parser.expected_token() == http::token::code::field_value);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_value);
        REQUIRE(parser.token_size() == 9);
        REQUIRE(parser.value<http::token::field_value>() == "burn.burn");
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
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 17);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::body_chunk);

        parser.next();

        REQUIRE(parser.code() == http::token::code::body_chunk);
        REQUIRE(parser.token_size() == 4);
        {
            asio::const_buffer buf = parser.value<http::token::body_chunk>();
            REQUIRE(asio::buffer_size(buf) == 4);
            const char *view = asio::buffer_cast<const char*>(buf);
            REQUIRE(view[0] == 'W');
            REQUIRE(view[1] == 'i');
            REQUIRE(view[2] == 'k');
            REQUIRE(view[3] == 'i');
        }
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::end_of_body);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_name);

        parser.next();

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Empty trailer name") {
        my_copy(buf, idx,
                "POST / HTTP/1.1\r\n"
                "host:     burn.burn\r\n"
                "Transfer-Encoding: chunked\r\n"
                "\r\n"

                "4;FSF=good;ms=evil\r\n"
                "Wiki\r\n"

                "0\r\n"
                ": Off\r\n"
                "\r\n");

        parser.next();

        REQUIRE(parser.code() == http::token::code::method);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::method>() == "POST");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.expected_token() == http::token::code::request_target);

        parser.next();

        REQUIRE(parser.code() == http::token::code::request_target);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::request_target>() == "/");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 8);
        REQUIRE(parser.expected_token() == http::token::code::version);

        parser.next();

        REQUIRE(parser.code() == http::token::code::version);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::version>() == 1);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_name);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_name);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::field_name>() == "host");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 6);
        REQUIRE(parser.expected_token() == http::token::code::field_value);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_value);
        REQUIRE(parser.token_size() == 9);
        REQUIRE(parser.value<http::token::field_value>() == "burn.burn");
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
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 17);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::body_chunk);

        parser.next();

        REQUIRE(parser.code() == http::token::code::body_chunk);
        REQUIRE(parser.token_size() == 4);
        {
            asio::const_buffer buf = parser.value<http::token::body_chunk>();
            REQUIRE(asio::buffer_size(buf) == 4);
            const char *view = asio::buffer_cast<const char*>(buf);
            REQUIRE(view[0] == 'W');
            REQUIRE(view[1] == 'i');
            REQUIRE(view[2] == 'k');
            REQUIRE(view[3] == 'i');
        }
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::end_of_body);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_name);

        parser.next();

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Forbidden whitespace after trailer name") {
        my_copy(buf, idx,
                "POST / HTTP/1.1\r\n"
                "host:     burn.burn\r\n"
                "Transfer-Encoding: chunked\r\n"
                "\r\n"

                "4;FSF=good;ms=evil\r\n"
                "Wiki\r\n"

                "0\r\n"
                "X-Pants : Off\r\n"
                "\r\n");

        parser.next();

        REQUIRE(parser.code() == http::token::code::method);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::method>() == "POST");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.expected_token() == http::token::code::request_target);

        parser.next();

        REQUIRE(parser.code() == http::token::code::request_target);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::request_target>() == "/");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 8);
        REQUIRE(parser.expected_token() == http::token::code::version);

        parser.next();

        REQUIRE(parser.code() == http::token::code::version);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::version>() == 1);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_name);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_name);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::field_name>() == "host");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 6);
        REQUIRE(parser.expected_token() == http::token::code::field_value);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_value);
        REQUIRE(parser.token_size() == 9);
        REQUIRE(parser.value<http::token::field_value>() == "burn.burn");
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
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 17);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::body_chunk);

        parser.next();

        REQUIRE(parser.code() == http::token::code::body_chunk);
        REQUIRE(parser.token_size() == 4);
        {
            asio::const_buffer buf = parser.value<http::token::body_chunk>();
            REQUIRE(asio::buffer_size(buf) == 4);
            const char *view = asio::buffer_cast<const char*>(buf);
            REQUIRE(view[0] == 'W');
            REQUIRE(view[1] == 'i');
            REQUIRE(view[2] == 'k');
            REQUIRE(view[3] == 'i');
        }
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::end_of_body);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_name);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_name);
        REQUIRE(parser.token_size() == 7);
        REQUIRE(parser.value<http::token::field_name>() == "X-Pants");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Invalid data after empty trailer value") {
        my_copy(buf, idx,
                "POST / HTTP/1.1\r\n"
                "host:     burn.burn\r\n"
                "Transfer-Encoding: chunked\r\n"
                "\r\n"

                "4;FSF=good;ms=evil\r\n"
                "Wiki\r\n"

                "0\r\n"
                "X-Pants:""\x7F""Off\r\n"
                "\r\n");

        parser.next();

        REQUIRE(parser.code() == http::token::code::method);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::method>() == "POST");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.expected_token() == http::token::code::request_target);

        parser.next();

        REQUIRE(parser.code() == http::token::code::request_target);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::request_target>() == "/");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 8);
        REQUIRE(parser.expected_token() == http::token::code::version);

        parser.next();

        REQUIRE(parser.code() == http::token::code::version);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::version>() == 1);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_name);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_name);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::field_name>() == "host");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 6);
        REQUIRE(parser.expected_token() == http::token::code::field_value);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_value);
        REQUIRE(parser.token_size() == 9);
        REQUIRE(parser.value<http::token::field_value>() == "burn.burn");
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
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 17);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::body_chunk);

        parser.next();

        REQUIRE(parser.code() == http::token::code::body_chunk);
        REQUIRE(parser.token_size() == 4);
        {
            asio::const_buffer buf = parser.value<http::token::body_chunk>();
            REQUIRE(asio::buffer_size(buf) == 4);
            const char *view = asio::buffer_cast<const char*>(buf);
            REQUIRE(view[0] == 'W');
            REQUIRE(view[1] == 'i');
            REQUIRE(view[2] == 'k');
            REQUIRE(view[3] == 'i');
        }
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::end_of_body);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_name);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_name);
        REQUIRE(parser.token_size() == 7);
        REQUIRE(parser.value<http::token::field_name>() == "X-Pants");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.expected_token() == http::token::code::field_value);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_value);
        REQUIRE(parser.token_size() == 0);
        REQUIRE(parser.value<http::token::field_value>() == "");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Invalid data after trailer value") {
        my_copy(buf, idx,
                "POST / HTTP/1.1\r\n"
                "host:     burn.burn\r\n"
                "Transfer-Encoding: chunked\r\n"
                "\r\n"

                "4;FSF=good;ms=evil\r\n"
                "Wiki\r\n"

                "0\r\n"
                "X-Pants:On""\x7F""\r\n"
                "\r\n");

        parser.next();

        REQUIRE(parser.code() == http::token::code::method);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::method>() == "POST");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.expected_token() == http::token::code::request_target);

        parser.next();

        REQUIRE(parser.code() == http::token::code::request_target);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::request_target>() == "/");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 8);
        REQUIRE(parser.expected_token() == http::token::code::version);

        parser.next();

        REQUIRE(parser.code() == http::token::code::version);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::version>() == 1);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_name);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_name);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::field_name>() == "host");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 6);
        REQUIRE(parser.expected_token() == http::token::code::field_value);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_value);
        REQUIRE(parser.token_size() == 9);
        REQUIRE(parser.value<http::token::field_value>() == "burn.burn");
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
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 17);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::body_chunk);

        parser.next();

        REQUIRE(parser.code() == http::token::code::body_chunk);
        REQUIRE(parser.token_size() == 4);
        {
            asio::const_buffer buf = parser.value<http::token::body_chunk>();
            REQUIRE(asio::buffer_size(buf) == 4);
            const char *view = asio::buffer_cast<const char*>(buf);
            REQUIRE(view[0] == 'W');
            REQUIRE(view[1] == 'i');
            REQUIRE(view[2] == 'k');
            REQUIRE(view[3] == 'i');
        }
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::end_of_body);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_name);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_name);
        REQUIRE(parser.token_size() == 7);
        REQUIRE(parser.value<http::token::field_name>() == "X-Pants");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.expected_token() == http::token::code::field_value);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_value);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.value<http::token::field_value>() == "On");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Invalid CRLF after trailer value") {
        my_copy(buf, idx,
                "POST / HTTP/1.1\r\n"
                "host:     burn.burn\r\n"
                "Transfer-Encoding: chunked\r\n"
                "\r\n"

                "4;FSF=good;ms=evil\r\n"
                "Wiki\r\n"

                "0\r\n"
                "X-Pants:On\r \n"
                "\r\n");

        parser.next();

        REQUIRE(parser.code() == http::token::code::method);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::method>() == "POST");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.expected_token() == http::token::code::request_target);

        parser.next();

        REQUIRE(parser.code() == http::token::code::request_target);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::request_target>() == "/");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 8);
        REQUIRE(parser.expected_token() == http::token::code::version);

        parser.next();

        REQUIRE(parser.code() == http::token::code::version);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::version>() == 1);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_name);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_name);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::field_name>() == "host");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 6);
        REQUIRE(parser.expected_token() == http::token::code::field_value);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_value);
        REQUIRE(parser.token_size() == 9);
        REQUIRE(parser.value<http::token::field_value>() == "burn.burn");
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
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 17);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::body_chunk);

        parser.next();

        REQUIRE(parser.code() == http::token::code::body_chunk);
        REQUIRE(parser.token_size() == 4);
        {
            asio::const_buffer buf = parser.value<http::token::body_chunk>();
            REQUIRE(asio::buffer_size(buf) == 4);
            const char *view = asio::buffer_cast<const char*>(buf);
            REQUIRE(view[0] == 'W');
            REQUIRE(view[1] == 'i');
            REQUIRE(view[2] == 'k');
            REQUIRE(view[3] == 'i');
        }
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::end_of_body);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_name);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_name);
        REQUIRE(parser.token_size() == 7);
        REQUIRE(parser.value<http::token::field_name>() == "X-Pants");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.expected_token() == http::token::code::field_value);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_value);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.value<http::token::field_value>() == "On");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Obsolete line folding in trailer value") {
        my_copy(buf, idx,
                "POST / HTTP/1.1\r\n"
                "host:     burn.burn\r\n"
                "Transfer-Encoding: chunked\r\n"
                "\r\n"

                "4;FSF=good;ms=evil\r\n"
                "Wiki\r\n"

                "0\r\n"
                "X-Pants: On\r\n"
                " spanned over multiple lines\r\n"
                "\r\n");

        parser.next();

        REQUIRE(parser.code() == http::token::code::method);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::method>() == "POST");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.expected_token() == http::token::code::request_target);

        parser.next();

        REQUIRE(parser.code() == http::token::code::request_target);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::request_target>() == "/");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 8);
        REQUIRE(parser.expected_token() == http::token::code::version);

        parser.next();

        REQUIRE(parser.code() == http::token::code::version);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::version>() == 1);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_name);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_name);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::field_name>() == "host");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 6);
        REQUIRE(parser.expected_token() == http::token::code::field_value);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_value);
        REQUIRE(parser.token_size() == 9);
        REQUIRE(parser.value<http::token::field_value>() == "burn.burn");
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
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 17);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::body_chunk);

        parser.next();

        REQUIRE(parser.code() == http::token::code::body_chunk);
        REQUIRE(parser.token_size() == 4);
        {
            asio::const_buffer buf = parser.value<http::token::body_chunk>();
            REQUIRE(asio::buffer_size(buf) == 4);
            const char *view = asio::buffer_cast<const char*>(buf);
            REQUIRE(view[0] == 'W');
            REQUIRE(view[1] == 'i');
            REQUIRE(view[2] == 'k');
            REQUIRE(view[3] == 'i');
        }
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::end_of_body);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_name);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_name);
        REQUIRE(parser.token_size() == 7);
        REQUIRE(parser.value<http::token::field_name>() == "X-Pants");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_value);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_value);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.value<http::token::field_value>() == "On");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_name);

        parser.next();

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }
}
