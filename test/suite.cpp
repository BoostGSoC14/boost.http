#ifdef NDEBUG
#undef NDEBUG
#endif

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <boost/http/reader.hpp>

namespace asio = boost::asio;
namespace http = boost::http;

template<std::size_t N>
asio::const_buffer my_buffer(const char (&in)[N])
{
    return asio::buffer(in, N - 1);
}

TEST_CASE("Utility test functions", "[misc]")
{
    REQUIRE(asio::buffer_size(my_buffer("")) == 0);
    REQUIRE(asio::buffer_size(my_buffer("Nu")) == 2);
}

TEST_CASE("Unreachable macro", "[detail]")
{
#define BOOST_HTTP_SPONSOR "[random string]: anarchy is coming"
    try {
        BOOST_HTTP_DETAIL_UNREACHABLE(BOOST_HTTP_SPONSOR);
    } catch(const char *ex) {
        REQUIRE(!boost::find_first(ex, BOOST_HTTP_SPONSOR).empty());
        return;
    }
    throw "shouldn't happen";
#undef BOOST_HTTP_SPONSOR
}

TEST_CASE("decode_transfer_encoding", "[detail]")
{
    // REMAINDER: there can never be beginning or trailing OWS in header fields

    using http::detail::CHUNKED_NOT_FOUND;
    using http::detail::CHUNKED_AT_END;
    using http::detail::CHUNKED_INVALID;
    using http::detail::decode_transfer_encoding;

    // CHUNKED_NOT_FOUND

    CHECK(decode_transfer_encoding("") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(", ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(", ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",,   ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(", , ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,   ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",,,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(", ,,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",,, ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",,,   ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",, ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",,   ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(", ,, ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,   ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",, ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",,   ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(", , ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(", ,   ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   , ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,   ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",, , ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",,   , ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",, ,   ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",,   ,   ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(", , , ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,   ,   ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chunked chunked") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chunked a") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a chunked") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a chunked a") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a#chunked#a") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chunked#chunked") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chuNKed cHUnked") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chUNked a") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a CHunked") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a chunkED a") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a#cHunKed#a") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chuNDeD#CHunkeD") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,chunked chunked")
          == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,chunked a") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,a chunked") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,a chunked a") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,a#chunked#a") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,chunked#chunked")
          == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,chuNKed cHUnked")
          == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,chUNked a") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,a CHunked") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,a chunkED a") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,a#cHunKed#a") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding(",   ,,chuNDeD#CHunkeD")
          == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chunked chunked,  ,,")
          == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chunked a,  ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a chunked,  ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a chunked a,  ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a#chunked#a,  ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chunked#chunked,  ,,")
          == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chuNKed cHUnked,  ,,")
          == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chUNked a,  ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a CHunked,  ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a chunkED a,  ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a#cHunKed#a,  ,,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chuNDeD#CHunkeD,  ,,")
          == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chunked chunked  ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chunked a  ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a chunked  ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a chunked a  ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a#chunked#a  ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chunked#chunked  ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chuNKed cHUnked  ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chUNked a  ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a CHunked  ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a chunkED a  ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("a#cHunKed#a  ,") == CHUNKED_NOT_FOUND);
    CHECK(decode_transfer_encoding("chuNDeD#CHunkeD  ,") == CHUNKED_NOT_FOUND);

    // CHUNKED_INVALID

    CHECK(decode_transfer_encoding("chunded,chunked") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chUNded,CHunked") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunded ,CHUNKED") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("CHUNDED   ,chunked") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("CHUNDed, cHUnked") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("ChUNDed,   cHUnKed") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunded,say what") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunded, say what") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunded,   say what") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunded ,say what") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunded   ,say what") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,chunded,chunked") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,chUNded,CHunked") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,chunded ,CHUNKED")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,CHUNDED   ,chunked")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,CHUNDed, cHUnked")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,ChUNDed,   cHUnKed")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,chunded,say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,CHUNDED, say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,chunded,   say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,CHUNDED ,say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   chunded   ,say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   chunDED,chunked") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   chUNded,CHunked") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   chunded ,CHUNKED")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   CHUNDED   ,chunked")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   CHUNDed, cHUnked")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   ChUNDed,   cHUnKed")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   chunded,say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   CHUNDED, say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   chunded,   say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   CHUNDED ,say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   chunded   ,say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunded,chunked,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chUNded,CHunked,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunded ,CHUNKED,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("CHUNDED   ,chunked,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("CHUNDed, cHUnked,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("ChUNDed,   cHUnKed,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunded,say what,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunded, say what,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunded,   say what,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunded ,say what,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunded   ,say what,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunded,chunked ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chUNded,CHunked ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunded ,CHUNKED ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("CHUNDED   ,chunked ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("CHUNDed, cHUnked ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("ChUNDed,   cHUnKed ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunded,say what ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunded, say what ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunded,   say what ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunded ,say what ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunded   ,say what ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunded,chunked   ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chUNded,CHunked   ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunded ,CHUNKED   ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("CHUNDED   ,chunked   ,")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("CHUNDed, cHUnked   ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("ChUNDed,   cHUnKed   ,")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunded,say what   ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunded, say what   ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunded,   say what   ,")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunded ,say what   ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunded   ,say what   ,")
          == CHUNKED_INVALID);

    // CHUNKED_AT_END

    CHECK(decode_transfer_encoding("chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("Chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunkeD") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("CHUNKED") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("cHunKed") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding(",chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding(", chUnked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding(",   chunkEd") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding(",,   chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding(", ,chunKed") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding(",   ,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding(",  ,,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunked,") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunkEd ,") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunked   ,") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunKed   ,,") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("cHunked, ,") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunkeD,   ,") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("cHunked,,   ,") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("and then there was light,chunked")
          == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("let there be rock  ,chunked")
          == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("punk is dead,  chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("punk is not dead,,chunked")
          == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("a ,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("a, chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("#chunked#,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("#chunked,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunked#,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("#chunked# ,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("#chunked ,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunked# ,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("#chunked#   ,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunked#   ,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("#chunked   ,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("#chunked#, chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("#chunked, chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunked#, chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("#chunked#,   chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunked#,   chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("#chunked,   chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("a chunked b,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("a chunked,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunked b,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("a chunked b ,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("a chunked ,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunked b ,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("a chunked b   ,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunked b   ,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("a chunked   ,chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("a chunked b, chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("a chunked, chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunked b, chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("a chunked b,   chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("chunked b,   chunked") == CHUNKED_AT_END);
    CHECK(decode_transfer_encoding("a chunked,   chunked") == CHUNKED_AT_END);
}

TEST_CASE("from_hex_string", "[detail]")
{
    
}

TEST_CASE("Parse 2 simple pipelined non-fragmented/whole requests",
          "[parser,good]")
{
    http::request_reader parser;

    REQUIRE(parser.code() == http::token::code::error_insufficient_data);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token() == http::token::code::method);

    parser.set_buffer(my_buffer("GET / HTTP/1.1\r\n"
                                "host: aliceinthewonderland.com\t \r\n"
                                "\r\n"

                                "POST /upload HTTP/1.1\r\n"
                                "Content-length: 4\r\n"
                                "host:thelastringbearer.org\r\n"
                                "\r\n"
                                "ping"

                                "POST http://notheaven.onion/ HTTP/1.1\r\n"
                                "Host: playwithme.onion\r\n"
                                "X-Pants: On\r\n"
                                "Transfer-Encoding: chunked\r\n"
                                "\r\n"

                                "4\r\n"
                                "Wiki\r\n"

                                "5\r\n"
                                "pedia\r\n"

                                "e\r\n"
                                " in\r\n\r\nchunks.\r\n"

                                "0\r\n"
                                "\r\n"

                                "POST http://notheaven.onion/ HTTP/1.1\r\n"
                                "Host: playwithme.onion\r\n"
                                "X-Pants: On\r\n"
                                "Transfer-Encoding: chunked\r\n"
                                "\r\n"

                                "4\r\n"
                                "Wiki\r\n"

                                "5\r\n"
                                "pedia\r\n"

                                "e\r\n"
                                " in\r\n\r\nchunks.\r\n"

                                "0\r\n"
                                "Content-MD5: \t  \t\t   25b83662323c397c9944a8"
                                "a7b3fef7ab    \t\t\t\t   \t \r\n"
                                "\r\n"));

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
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_value);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_value);
    REQUIRE(parser.token_size() == 26);
    REQUIRE(parser.value<http::token::field_value>()
            == "aliceinthewonderland.com");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_message);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::method);

    // # SECOND REQUEST

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
    REQUIRE(parser.token_size() == 7);
    REQUIRE(parser.value<http::token::request_target>() == "/upload");
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
    REQUIRE(parser.token_size() == 14);
    REQUIRE(parser.value<http::token::field_name>() == "Content-length");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_value);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_value);
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.value<http::token::field_value>() == "4");
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
    REQUIRE(parser.token_size() == 1);
    REQUIRE(parser.expected_token() == http::token::code::field_value);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_value);
    REQUIRE(parser.token_size() == 21);
    REQUIRE(parser.value<http::token::field_value>()
            == "thelastringbearer.org");
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
    REQUIRE(parser.token_size() == 4);
    {
        asio::const_buffer buf = parser.value<http::token::body_chunk>();
        REQUIRE(asio::buffer_size(buf) == 4);

        const char *body = asio::buffer_cast<const char*>(buf);
        REQUIRE(body[0] == 'p');
        REQUIRE(body[1] == 'i');
        REQUIRE(body[2] == 'n');
        REQUIRE(body[3] == 'g');
    }
    REQUIRE(parser.expected_token() == http::token::code::method);

    // THIRD REQUEST

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
    REQUIRE(parser.token_size() == 23);
    REQUIRE(parser.value<http::token::request_target>()
            == "http://notheaven.onion/");
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
    REQUIRE(parser.value<http::token::field_name>() == "Host");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_value);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_value);
    REQUIRE(parser.token_size() == 16);
    REQUIRE(parser.value<http::token::field_value>() == "playwithme.onion");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
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
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::body_chunk);

    parser.next();

    REQUIRE(parser.code() == http::token::code::body_chunk);
    REQUIRE(parser.token_size() == 4);
    {
        asio::const_buffer buf = parser.value<http::token::body_chunk>();
        REQUIRE(asio::buffer_size(buf) == 4);

        const char *body = asio::buffer_cast<const char*>(buf);
        REQUIRE(body[0] == 'W');
        REQUIRE(body[1] == 'i');
        REQUIRE(body[2] == 'k');
        REQUIRE(body[3] == 'i');
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

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::body_chunk);

    parser.next();

    REQUIRE(parser.code() == http::token::code::body_chunk);
    REQUIRE(parser.token_size() == 5);
    {
        asio::const_buffer buf = parser.value<http::token::body_chunk>();
        REQUIRE(asio::buffer_size(buf) == 5);

        const char *body = asio::buffer_cast<const char*>(buf);
        REQUIRE(body[0] == 'p');
        REQUIRE(body[1] == 'e');
        REQUIRE(body[2] == 'd');
        REQUIRE(body[3] == 'i');
        REQUIRE(body[4] == 'a');
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

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::body_chunk);

    parser.next();

    REQUIRE(parser.code() == http::token::code::body_chunk);
    REQUIRE(parser.token_size() == 14);
    {
        asio::const_buffer buf = parser.value<http::token::body_chunk>();
        REQUIRE(asio::buffer_size(buf) == 14);

        const char *body = asio::buffer_cast<const char*>(buf);
        REQUIRE(body[0] == ' ');
        REQUIRE(body[1] == 'i');
        REQUIRE(body[2] == 'n');
        REQUIRE(body[3] == '\r');
        REQUIRE(body[4] == '\n');
        REQUIRE(body[5] == '\r');
        REQUIRE(body[6] == '\n');
        REQUIRE(body[7] == 'c');
        REQUIRE(body[8] == 'h');
        REQUIRE(body[9] == 'u');
        REQUIRE(body[10] == 'n');
        REQUIRE(body[11] == 'k');
        REQUIRE(body[12] == 's');
        REQUIRE(body[13] == '.');
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

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_message);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::method);

    // FOURTH REQUEST

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
    REQUIRE(parser.token_size() == 23);
    REQUIRE(parser.value<http::token::request_target>()
            == "http://notheaven.onion/");
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
    REQUIRE(parser.value<http::token::field_name>() == "Host");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_value);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_value);
    REQUIRE(parser.token_size() == 16);
    REQUIRE(parser.value<http::token::field_value>() == "playwithme.onion");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
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
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::body_chunk);

    parser.next();

    REQUIRE(parser.code() == http::token::code::body_chunk);
    REQUIRE(parser.token_size() == 4);
    {
        asio::const_buffer buf = parser.value<http::token::body_chunk>();
        REQUIRE(asio::buffer_size(buf) == 4);

        const char *body = asio::buffer_cast<const char*>(buf);
        REQUIRE(body[0] == 'W');
        REQUIRE(body[1] == 'i');
        REQUIRE(body[2] == 'k');
        REQUIRE(body[3] == 'i');
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

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::body_chunk);

    parser.next();

    REQUIRE(parser.code() == http::token::code::body_chunk);
    REQUIRE(parser.token_size() == 5);
    {
        asio::const_buffer buf = parser.value<http::token::body_chunk>();
        REQUIRE(asio::buffer_size(buf) == 5);

        const char *body = asio::buffer_cast<const char*>(buf);
        REQUIRE(body[0] == 'p');
        REQUIRE(body[1] == 'e');
        REQUIRE(body[2] == 'd');
        REQUIRE(body[3] == 'i');
        REQUIRE(body[4] == 'a');
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

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::body_chunk);

    parser.next();

    REQUIRE(parser.code() == http::token::code::body_chunk);
    REQUIRE(parser.token_size() == 14);
    {
        asio::const_buffer buf = parser.value<http::token::body_chunk>();
        REQUIRE(asio::buffer_size(buf) == 14);

        const char *body = asio::buffer_cast<const char*>(buf);
        REQUIRE(body[0] == ' ');
        REQUIRE(body[1] == 'i');
        REQUIRE(body[2] == 'n');
        REQUIRE(body[3] == '\r');
        REQUIRE(body[4] == '\n');
        REQUIRE(body[5] == '\r');
        REQUIRE(body[6] == '\n');
        REQUIRE(body[7] == 'c');
        REQUIRE(body[8] == 'h');
        REQUIRE(body[9] == 'u');
        REQUIRE(body[10] == 'n');
        REQUIRE(body[11] == 'k');
        REQUIRE(body[12] == 's');
        REQUIRE(body[13] == '.');
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

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    // shit happens

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_name);
    REQUIRE(parser.token_size() == 11);
    REQUIRE(parser.value<http::token::field_name>() == "Content-MD5");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 10);
    REQUIRE(parser.expected_token() == http::token::code::field_value);

    parser.next();

    REQUIRE(parser.code() == http::token::code::field_value);
    REQUIRE(parser.token_size() == 45);
    REQUIRE(parser.value<http::token::field_value>()
            == "25b83662323c397c9944a8a7b3fef7ab");
    REQUIRE(parser.expected_token() == http::token::code::skip);

    parser.next();

    REQUIRE(parser.code() == http::token::code::skip);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

    // and shit is overcome

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_message);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::method);
}
