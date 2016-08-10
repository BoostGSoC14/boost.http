#ifdef NDEBUG
#undef NDEBUG
#endif

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <boost/http/reader/request.hpp>

namespace asio = boost::asio;
namespace http = boost::http;
namespace reader = http::reader;

template<std::size_t N>
asio::const_buffer my_buffer(const char (&in)[N])
{
    return asio::buffer(in, N - 1);
}

template<std::size_t N, class CharT>
void my_copy(std::vector<CharT> &out, std::size_t out_idx, const char (&in)[N])
{
    for (std::size_t i = 0 ; i != N - 1 ; ++i)
        out[i + out_idx] = in[i];
}

TEST_CASE("Utility test functions", "[misc]")
{
    CHECK(asio::buffer_size(my_buffer("")) == 0);
    CHECK(asio::buffer_size(my_buffer("Nu")) == 2);

    {
        std::vector<char> buf(10, 'c');
        my_copy(buf, 0, "a");
        REQUIRE(buf[0] == 'a');
        REQUIRE(buf[1] == 'c');
        my_copy(buf, 2, "bb");
        REQUIRE(buf[0] == 'a');
        REQUIRE(buf[1] == 'c');
        REQUIRE(buf[2] == 'b');
        REQUIRE(buf[3] == 'b');
        REQUIRE(buf[4] == 'c');
        my_copy(buf, 0, "");
        REQUIRE(buf[0] == 'a');
    }
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

    using http::reader::detail::CHUNKED_NOT_FOUND;
    using http::reader::detail::CHUNKED_AT_END;
    using http::reader::detail::CHUNKED_INVALID;
    using http::reader::detail::decode_transfer_encoding;

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

    CHECK(decode_transfer_encoding("chunked,chunked") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chUNked,CHunked") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked ,CHUNKED") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("CHUNKED   ,chunked") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("CHUNKed, cHUnked") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("ChUNKed,   cHUnKed") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked,say what") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked, say what") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked,   say what") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked ,say what") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked   ,say what") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,chunked,chunked") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,chUNked,CHunked") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,chunked ,CHUNKED")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,CHUNKED   ,chunked")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,CHUNKed, cHUnked")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,ChUNKed,   cHUnKed")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,chunKed,say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,CHUNKED, say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,chunked,   say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(",   ,,CHUNKED ,say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   chunked   ,say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   chunKED,chunked") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   chUNked,CHunked") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   chunked ,CHUNKED")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   CHUNKED   ,chunked")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   CHUNKed, cHUnked")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   ChUNKed,   cHUnKed")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   chunked,say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   CHUNKED, say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   chunked,   say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   CHUNKED ,say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding(", ,   chunked   ,say what")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked,chunked,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chUNked,CHunked,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked ,CHUNKED,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("CHUNKED   ,chunked,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("CHUNKed, cHUnked,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("ChUNKed,   cHUnKed,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked,say what,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked, say what,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked,   say what,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked ,say what,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked   ,say what,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked,chunked ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chUNked,CHunked ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked ,CHUNKED ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("CHUNKED   ,chunked ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("CHUNKed, cHUnked ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("ChUNKed,   cHUnKed ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked,say what ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked, say what ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked,   say what ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked ,say what ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked   ,say what ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked,chunked   ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chUNked,CHunked   ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked ,CHUNKED   ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("CHUNKED   ,chunked   ,")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("CHUNKed, cHUnked   ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("ChUNKed,   cHUnKed   ,")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked,say what   ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked, say what   ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked,   say what   ,")
          == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked ,say what   ,") == CHUNKED_INVALID);
    CHECK(decode_transfer_encoding("chunked   ,say what   ,")
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


TEST_CASE("from_decimal_string", "[detail]")
{
    using http::reader::detail::DECSTRING_INVALID;
    using http::reader::detail::DECSTRING_OK;
    using http::reader::detail::DECSTRING_OVERFLOW;
    using http::reader::detail::from_decimal_string;

    uint8_t out;

    // DECSTRING_INVALID

    CHECK(from_decimal_string("-0", out) == DECSTRING_INVALID);
    CHECK(from_decimal_string("-1", out) == DECSTRING_INVALID);
    CHECK(from_decimal_string("a", out) == DECSTRING_INVALID);
    CHECK(from_decimal_string("B", out) == DECSTRING_INVALID);
    CHECK(from_decimal_string("$", out) == DECSTRING_INVALID);
    CHECK(from_decimal_string("#", out) == DECSTRING_INVALID);
    CHECK(from_decimal_string("0d", out) == DECSTRING_INVALID);
    CHECK(from_decimal_string("0E", out) == DECSTRING_INVALID);
    CHECK(from_decimal_string("2z", out) == DECSTRING_INVALID);

    // DECSTRING_OVERFLOW

    CHECK(from_decimal_string("256", out) == DECSTRING_OVERFLOW);
    CHECK(from_decimal_string("0256", out) == DECSTRING_OVERFLOW);
    CHECK(from_decimal_string("00000256", out) == DECSTRING_OVERFLOW);
    CHECK(from_decimal_string("1000", out) == DECSTRING_OVERFLOW);
    CHECK(from_decimal_string("0001000", out) == DECSTRING_OVERFLOW);

    // DECSTRING_OK

    out = 67;
    REQUIRE(from_decimal_string("0", out) == DECSTRING_OK);
    REQUIRE(out == 0);
    out = 67;
    REQUIRE(from_decimal_string("00000000", out) == DECSTRING_OK);
    REQUIRE(out == 0);
    out = 67;
    REQUIRE(from_decimal_string("1", out) == DECSTRING_OK);
    REQUIRE(out == 1);
    out = 67;
    REQUIRE(from_decimal_string("01", out) == DECSTRING_OK);
    REQUIRE(out == 1);
    out = 67;
    REQUIRE(from_decimal_string("000001", out) == DECSTRING_OK);
    REQUIRE(out == 1);
    out = 67;
    REQUIRE(from_decimal_string("10", out) == DECSTRING_OK);
    REQUIRE(out == 10);
    out = 67;
    REQUIRE(from_decimal_string("010", out) == DECSTRING_OK);
    REQUIRE(out == 10);
    out = 67;
    REQUIRE(from_decimal_string("0000000010", out) == DECSTRING_OK);
    REQUIRE(out == 10);
    out = 67;
    REQUIRE(from_decimal_string("32", out) == DECSTRING_OK);
    REQUIRE(out == 32);
    out = 67;
    REQUIRE(from_decimal_string("255", out) == DECSTRING_OK);
    REQUIRE(out == 255);
    out = 67;
    REQUIRE(from_decimal_string("000000000255", out) == DECSTRING_OK);
    REQUIRE(out == 255);

    /* from_decimal_string is only used to parse the Content-Length header
       field. Header fiels are never allowed to be empty. Therefore,
       implementation can be simplified and assume empty strings are never
       passed as argument.

       Currently, the simplification initializes `out` with 0 and then proceed
       to remove leading 0s. If there are no remaining leading 0s, function
       returns. */
    out = 1;
    REQUIRE(from_decimal_string("", out) == DECSTRING_OK);
    REQUIRE(out == 0);
}

TEST_CASE("Parse a few pipelined non-fragmented/whole requests",
          "[parser,good]")
{
    http::reader::request parser;

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
    REQUIRE(parser.expected_token() == http::token::code::end_of_body);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_body);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token() == http::token::code::end_of_message);

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_message);
    REQUIRE(parser.token_size() == 0);
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

    REQUIRE(parser.code() == http::token::code::end_of_body);
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

    REQUIRE(parser.code() == http::token::code::end_of_body);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::field_name);

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

    parser.next();

    REQUIRE(parser.code() == http::token::code::end_of_message);
    REQUIRE(parser.token_size() == 2);
    REQUIRE(parser.expected_token() == http::token::code::method);
}

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

    SECTION("Invalid field value") {
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

    SECTION("Invalid trailer value") {
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
