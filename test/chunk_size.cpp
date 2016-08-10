#ifdef NDEBUG
#undef NDEBUG
#endif

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <boost/http/syntax/chunk_size.hpp>

namespace http = boost::http;
namespace syntax = http::syntax;

typedef syntax::chunk_size<char> chunk_size;
typedef chunk_size::result result;

result from_hex_string(boost::string_ref in, uint16_t &out) {
    return chunk_size::decode(in, out);
}

TEST_CASE("from_hex_string", "[syntax]")
{
    result HEXSTRING_INVALID = result::invalid;
    result HEXSTRING_OK = result::ok;
    result HEXSTRING_OVERFLOW = result::overflow;

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
