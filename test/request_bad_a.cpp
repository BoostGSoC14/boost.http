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

    /* Now the `reader` has passed over all possible body types and if there is
       any dirty state stored, it's probably exposed by now already. Hence, it's
       the perfect time to test invalid streams. */

    SECTION("Invalid method: space preceding") {
        my_copy(buf, idx,
                " GET / HTTP/1.1\r\n"
                "host: burn.burn\r\n"
                "\r\n");

        parser.next();

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Invalid method: initial byte is not a token char") {
        my_copy(buf, idx,
                "@GET / HTTP/1.1\r\n"
                "host: burn.burn\r\n"
                "\r\n");

        parser.next();

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Invalid request line: no SP after method") {
        my_copy(buf, idx,
                "#GET@ / HTTP/1.1\r\n"
                "host: burn.burn\r\n"
                "\r\n");

        parser.next();

        REQUIRE(parser.code() == http::token::code::method);
        REQUIRE(parser.token_size() == 4);
        REQUIRE(parser.value<http::token::method>() == "#GET");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Invalid request-target: space preceding") {
        my_copy(buf, idx,
                "GET  / HTTP/1.1\r\n"
                "host: burn.burn\r\n"
                "\r\n");

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

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Invalid request-target: initial byte is invalid") {
        my_copy(buf, idx,
                "GET </ HTTP/1.1\r\n"
                "host: burn.burn\r\n"
                "\r\n");

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

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Invalid request-target: no SP after request-target") {
        my_copy(buf, idx,
                "GET /< HTTP/1.1\r\n"
                "host: burn.burn\r\n"
                "\r\n");

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

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Invalid HTTP version #1") {
        my_copy(buf, idx,
                "GET / HtTP/1.1\r\n"
                "host: burn.burn\r\n"
                "\r\n");

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

        /* Another correct behaviour would be to emit the SP skip token before
           erroring out, but skip is an opaque element to the user and we can
           merge them (or keep them separate) as much as we like. */

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Invalid HTTP version #2") {
        /* HTTP 0.9 is too old and not supported. */
        my_copy(buf, idx,
                "GET / HTTP/0.9\r\n"
                "host: burn.burn\r\n"
                "\r\n");

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

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Invalid HTTP version #3") {
        /* HTTP 0.9 is too old and not supported. */
        my_copy(buf, idx,
                "GET / HTTP/0.9\r\n"
                "host: burn.burn\r\n"
                "\r\n");

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

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Invalid HTTP version #4") {
        /* > The HTTP version number consists of two decimal digits separated by
           > a "." (period or decimal point). The first digit ("major version")
           > indicates the HTTP messaging syntax, whereas the second digit
           > ("minor version") indicates the highest minor version within that
           > major version to which the sender is conformant and able to
           > understand for future communication. The minor version advertises
           > the sender's communication capabilities even when the sender is
           > only using a backwards-compatible subset of the protocol, thereby
           > letting the recipient know that more advanced features can be used
           > in response (by servers) or in future requests (by clients).
           -- Section 2.6 of RFC7230

           HTTP 2.0 was released already and it uses a binary format. Under no
           rules a change in the major version retains compatibility with older
           parsers. */
        my_copy(buf, idx,
                "GET / HTTP/2.0\r\n"
                "host: burn.burn\r\n"
                "\r\n");

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

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Invalid HTTP version #5") {
        /* This is a "good" test case. I just want to be sure HTTP 1.2 will be
           supported (although it is very unlikely to be ever released). */
        my_copy(buf, idx,
                "GET / HTTP/1.2");

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
        REQUIRE(parser.token_size() ==  8);
        REQUIRE(parser.expected_token() == http::token::code::version);

        parser.next();

        REQUIRE(parser.code() == http::token::code::version);
        REQUIRE(parser.token_size() == 1);
        REQUIRE(parser.value<http::token::version>() == 2);
        REQUIRE(parser.expected_token() == http::token::code::skip);
    }

    SECTION("Extra space after HTTP version") {
        my_copy(buf, idx,
                "GET / HTTP/1.1 \r\n"
                "host: burn.burn\r\n"
                "\r\n");

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

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Invalid data after HTTP version") {
        my_copy(buf, idx,
                "GET / HTTP/1.1\r \n"
                "host: burn.burn\r\n"
                "\r\n");

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

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Forbidden whitespace before field name") {
        my_copy(buf, idx,
                "GET / HTTP/1.1\r\n"
                " host: burn.burn\r\n"
                "\r\n");

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

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Invalid field name") {
        my_copy(buf, idx,
                "GET / HTTP/1.1\r\n"
                "@host: burn.burn\r\n"
                "\r\n");

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

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Empty field name") {
        my_copy(buf, idx,
                "GET / HTTP/1.1\r\n"
                ": burn.burn\r\n"
                "\r\n");

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

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Forbidden whitespace after field name") {
        my_copy(buf, idx,
                "GET / HTTP/1.1\r\n"
                "host : burn.burn\r\n"
                "\r\n");

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
        REQUIRE(parser.value<http::token::field_name>() == "host");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Invalid data after empty field value") {
        my_copy(buf, idx,
                "GET / HTTP/1.1\r\n"
                "host:     ""\x7F""burn.burn\r\n"
                "\r\n");

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
        REQUIRE(parser.value<http::token::field_name>() == "host");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 6);
        REQUIRE(parser.expected_token() == http::token::code::field_value);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_value);
        REQUIRE(parser.token_size() == 0);
        REQUIRE(parser.value<http::token::field_value>() == "");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Invalid data after field value") {
        my_copy(buf, idx,
                "GET / HTTP/1.1\r\n"
                "host:     burn.burn\x7F\r\n"
                "\r\n");

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

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Invalid CRLF after field value") {
        my_copy(buf, idx,
                "GET / HTTP/1.1\r\n"
                "host:     burn.burn\r \n"
                "\r\n");

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

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("Obsolete line folding in field value") {
        /* Deprecated (respond with 400 bad request) in section 2.3.4 of
           RFC7230. */
        my_copy(buf, idx,
                "GET / HTTP/1.1\r\n"
                "host:     burn.burn\r\n"
                " spanned over multiple lines\r\n"
                "\r\n");

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

        REQUIRE(parser.code() == http::token::code::error_invalid_data);
    }

    SECTION("HTTP/1.0 with no host **is** okay") {
        my_copy(buf, idx,
                "GET / HTTP/1.0\r\n"
                "\r\n");

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
        REQUIRE(parser.value<http::token::version>() == 0);
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
    }

    SECTION("HTTP/1.1 with no host is **NOT** okay") {
        my_copy(buf, idx,
                "GET / HTTP/1.1\r\n"
                "\r\n");

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

        REQUIRE(parser.code() == http::token::code::error_no_host);
    }

    SECTION("HTTP/1.2 with no host is **NOT** okay") {
        my_copy(buf, idx,
                "GET / HTTP/1.2\r\n"
                "\r\n");

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
        REQUIRE(parser.value<http::token::version>() == 2);
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_name);

        parser.next();

        REQUIRE(parser.code() == http::token::code::error_no_host);
    }

    SECTION("Multiple Content-Length #1") {
        my_copy(buf, idx,
                "POST / HTTP/1.1\r\n"
                "host:     burn.burn\r\n"
                "Content-Length: 2, 3\r\n"
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
        REQUIRE(parser.token_size() == 14);
        REQUIRE(parser.value<http::token::field_name>() == "Content-Length");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_value);

        parser.next();

        REQUIRE(parser.code()
                == http::token::code::error_invalid_content_length);
    }

    SECTION("Multiple Content-Length #2") {
        my_copy(buf, idx,
                "POST / HTTP/1.1\r\n"
                "host:     burn.burn\r\n"
                "Content-Length: 2\r\n"
                "contenT-lengtH: 3\r\n"
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

        REQUIRE(parser.code()
                == http::token::code::error_invalid_content_length);
    }

    SECTION("Invalid Content-Length #1") {
        my_copy(buf, idx,
                "POST / HTTP/1.1\r\n"
                "host:     burn.burn\r\n"
                "Content-Length: -2\r\n"
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
        REQUIRE(parser.token_size() == 14);
        REQUIRE(parser.value<http::token::field_name>() == "Content-Length");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_value);

        parser.next();

        REQUIRE(parser.code()
                == http::token::code::error_invalid_content_length);
    }

    SECTION("Invalid Content-Length #2") {
        my_copy(buf, idx,
                "POST / HTTP/1.1\r\n"
                "host:     burn.burn\r\n"
                "Content-Length: B\r\n"
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
        REQUIRE(parser.token_size() == 14);
        REQUIRE(parser.value<http::token::field_name>() == "Content-Length");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_value);

        parser.next();

        REQUIRE(parser.code()
                == http::token::code::error_invalid_content_length);
    }

    SECTION("Invalid Content-Length #3") {
        my_copy(buf, idx,
                "POST / HTTP/1.1\r\n"
                "host:     burn.burn\r\n"
                "Content-Length: 3F\r\n"
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
        REQUIRE(parser.token_size() == 14);
        REQUIRE(parser.value<http::token::field_name>() == "Content-Length");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_value);

        parser.next();

        REQUIRE(parser.code()
                == http::token::code::error_invalid_content_length);
    }

    SECTION("Content-Length overflow") {
        my_copy(buf, idx,
                "POST / HTTP/1.1\r\n"
                "host:     burn.burn\r\n"
                "Content-Length: 99999999999999999999999999999\r\n"
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
        REQUIRE(parser.token_size() == 14);
        REQUIRE(parser.value<http::token::field_name>() == "Content-Length");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_value);

        parser.next();

        REQUIRE(parser.code()
                == http::token::code::error_content_length_overflow);
    }

    SECTION("Invalid Transfer-Encoding #1") {
        my_copy(buf, idx,
                "POST / HTTP/1.1\r\n"
                "host:     burn.burn\r\n"
                "Transfer-Encoding: chunked,not,at,the,end\r\n"
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

        REQUIRE(parser.code()
                == http::token::code::error_invalid_transfer_encoding);
    }

    SECTION("Invalid Transfer-Encoding #2") {
        my_copy(buf, idx,
                "POST / HTTP/1.1\r\n"
                "host:     burn.burn\r\n"
                "Transfer-Encoding: chunked i am not\r\n"
                "transfeR-encodinG: chunked,not,at,the,end\r\n"
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
        REQUIRE(parser.token_size() == 16);
        REQUIRE(parser.value<http::token::field_value>() == "chunked i am not");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_name);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_name);
        REQUIRE(parser.token_size() == 17);
        REQUIRE(parser.value<http::token::field_name>() == "transfeR-encodinG");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_value);

        parser.next();

        REQUIRE(parser.code()
                == http::token::code::error_invalid_transfer_encoding);
    }

    SECTION("Invalid Transfer-Encoding #3") {
        my_copy(buf, idx,
                "POST / HTTP/1.1\r\n"
                "host:     burn.burn\r\n"
                "Transfer-Encoding: chunked i am not\r\n"
                "transfeR-encodinG: chunked\r\n"
                "transfeR-Encoding: not,at,the,end\r\n"
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
        REQUIRE(parser.token_size() == 16);
        REQUIRE(parser.value<http::token::field_value>() == "chunked i am not");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_name);

        parser.next();

        REQUIRE(parser.code() == http::token::code::field_name);
        REQUIRE(parser.token_size() == 17);
        REQUIRE(parser.value<http::token::field_name>() == "transfeR-encodinG");
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

        REQUIRE(parser.code()
                == http::token::code::error_invalid_transfer_encoding);
    }

    SECTION("Invalid Transfer-Encoding #4") {
        my_copy(buf, idx,
                "POST / HTTP/1.1\r\n"
                "host:     burn.burn\r\n"
                "Transfer-Encoding: chunked , chunked\r\n"
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

        REQUIRE(parser.code()
                == http::token::code::error_invalid_transfer_encoding);
    }

    SECTION("Invalid Transfer-Encoding #5") {
        my_copy(buf, idx,
                "POST / HTTP/1.1\r\n"
                "host:     burn.burn\r\n"
                "Transfer-Encoding: vini patented encoding\r\n"
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
        REQUIRE(parser.token_size() == 22);
        REQUIRE(parser.value<http::token::field_value>()
                == "vini patented encoding");
        REQUIRE(parser.expected_token() == http::token::code::skip);

        parser.next();

        REQUIRE(parser.code() == http::token::code::skip);
        REQUIRE(parser.token_size() == 2);
        REQUIRE(parser.expected_token() == http::token::code::field_name);

        parser.next();

        REQUIRE(parser.code()
                == http::token::code::error_invalid_transfer_encoding);
    }
}
