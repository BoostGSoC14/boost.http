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

template<std::size_t N, class CharT>
void my_copy(std::vector<CharT> &out, std::size_t out_idx, const char (&in)[N])
{
    for (std::size_t i = 0 ; i != N - 1 ; ++i)
        out[i + out_idx] = in[i];
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
    using http::detail::DECSTRING_INVALID;
    using http::detail::DECSTRING_OK;
    using http::detail::DECSTRING_OVERFLOW;
    using http::detail::from_decimal_string;

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

TEST_CASE("from_hex_string", "[detail]")
{
    using http::detail::HEXSTRING_INVALID;
    using http::detail::HEXSTRING_OK;
    using http::detail::HEXSTRING_OVERFLOW;
    using http::detail::from_hex_string;

    uint16_t out;

    // HEXSTRING_INVALID

    CHECK(from_hex_string("", out) == HEXSTRING_INVALID);
    CHECK(from_hex_string("-0", out) == HEXSTRING_INVALID);
    CHECK(from_hex_string("-1", out) == HEXSTRING_INVALID);
    CHECK(from_hex_string("g", out) == HEXSTRING_INVALID);
    CHECK(from_hex_string("z", out) == HEXSTRING_INVALID);
    CHECK(from_hex_string("$", out) == HEXSTRING_INVALID);
    CHECK(from_hex_string("#", out) == HEXSTRING_INVALID);
    CHECK(from_hex_string("0g", out) == HEXSTRING_INVALID);
    CHECK(from_hex_string("0$", out) == HEXSTRING_INVALID);
    CHECK(from_hex_string("2z", out) == HEXSTRING_INVALID);

    // HEXSTRING_OVERFLOW

    CHECK(from_hex_string("10000", out) == HEXSTRING_OVERFLOW);
    CHECK(from_hex_string("010000", out) == HEXSTRING_OVERFLOW);
    CHECK(from_hex_string("0000010000", out) == HEXSTRING_OVERFLOW);
    CHECK(from_hex_string("Ff003", out) == HEXSTRING_OVERFLOW);

    // HEXSTRING_OK

    out = 2;
    REQUIRE(from_hex_string("fFfF", out) == HEXSTRING_OK);
    REQUIRE(out == 0xFFFF);
    out = 2;
    REQUIRE(from_hex_string("FfFf", out) == HEXSTRING_OK);
    REQUIRE(out == 0xFFFF);
    out = 2;
    REQUIRE(from_hex_string("0ffff", out) == HEXSTRING_OK);
    REQUIRE(out == 0xFFFF);
    out = 2;
    REQUIRE(from_hex_string("000000000000000ffff", out) == HEXSTRING_OK);
    REQUIRE(out == 0xFFFF);
    out = 2;
    REQUIRE(from_hex_string("1234", out) == HEXSTRING_OK);
    REQUIRE(out == 0x1234);
    out = 2;
    REQUIRE(from_hex_string("aBcD", out) == HEXSTRING_OK);
    REQUIRE(out == 0xABCD);
    out = 2;
    REQUIRE(from_hex_string("0000dCba", out) == HEXSTRING_OK);
    REQUIRE(out == 0xDCBA);
    out = 2;
    REQUIRE(from_hex_string("89aB", out) == HEXSTRING_OK);
    REQUIRE(out == 0x89AB);
    out = 2;
    REQUIRE(from_hex_string("00000bA98", out) == HEXSTRING_OK);
    REQUIRE(out == 0xBA98);
    out = 2;
    REQUIRE(from_hex_string("0", out) == HEXSTRING_OK);
    REQUIRE(out == 0);
    out = 2;
    REQUIRE(from_hex_string("000000000000000000", out) == HEXSTRING_OK);
    REQUIRE(out == 0);
    out = 2;
    REQUIRE(from_hex_string("12", out) == HEXSTRING_OK);
    REQUIRE(out == 0x12);
    out = 2;
    REQUIRE(from_hex_string("012", out) == HEXSTRING_OK);
    REQUIRE(out == 0x12);
    out = 2;
    REQUIRE(from_hex_string("0012", out) == HEXSTRING_OK);
    REQUIRE(out == 0x12);
    out = 2;
    REQUIRE(from_hex_string("00012", out) == HEXSTRING_OK);
    REQUIRE(out == 0x12);
    out = 2;
    REQUIRE(from_hex_string("000000012", out) == HEXSTRING_OK);
    REQUIRE(out == 0x12);
    out = 2;
    REQUIRE(from_hex_string("a", out) == HEXSTRING_OK);
    REQUIRE(out == 0xA);
    out = 2;
    REQUIRE(from_hex_string("A", out) == HEXSTRING_OK);
    REQUIRE(out == 0xA);
    out = 2;
    REQUIRE(from_hex_string("b", out) == HEXSTRING_OK);
    REQUIRE(out == 0xB);
    out = 2;
    REQUIRE(from_hex_string("B", out) == HEXSTRING_OK);
    REQUIRE(out == 0xB);
    out = 2;
    REQUIRE(from_hex_string("c", out) == HEXSTRING_OK);
    REQUIRE(out == 0xC);
    out = 2;
    REQUIRE(from_hex_string("C", out) == HEXSTRING_OK);
    REQUIRE(out == 0xC);
    out = 2;
    REQUIRE(from_hex_string("d", out) == HEXSTRING_OK);
    REQUIRE(out == 0xD);
    out = 2;
    REQUIRE(from_hex_string("D", out) == HEXSTRING_OK);
    REQUIRE(out == 0xD);
    out = 2;
    REQUIRE(from_hex_string("e", out) == HEXSTRING_OK);
    REQUIRE(out == 0xE);
    out = 2;
    REQUIRE(from_hex_string("E", out) == HEXSTRING_OK);
    REQUIRE(out == 0xE);
    out = 2;
    REQUIRE(from_hex_string("f", out) == HEXSTRING_OK);
    REQUIRE(out == 0xF);
    out = 2;
    REQUIRE(from_hex_string("F", out) == HEXSTRING_OK);
    REQUIRE(out == 0xF);
    out = 2;
    REQUIRE(from_hex_string("4", out) == HEXSTRING_OK);
    REQUIRE(out == 4);
    out = 2;
    REQUIRE(from_hex_string("04", out) == HEXSTRING_OK);
    REQUIRE(out == 4);
    out = 2;
    REQUIRE(from_hex_string("00000000005", out) == HEXSTRING_OK);
    REQUIRE(out == 5);
}

TEST_CASE("Parse a few pipelined non-fragmented/whole requests",
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

    http::request_reader parser;

    REQUIRE(parser.code() == http::token::code::error_insufficient_data);
    REQUIRE(parser.token_size() == 0);
    REQUIRE(parser.expected_token() == http::token::code::method);

    parser.set_buffer(asio::buffer(buf));

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

    REQUIRE(parser.code() == http::token::code::end_of_message);
    REQUIRE(parser.token_size() == 2);
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
}
