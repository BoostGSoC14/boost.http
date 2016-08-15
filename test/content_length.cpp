#ifdef NDEBUG
#undef NDEBUG
#endif

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <boost/http/syntax/content_length.hpp>

namespace http = boost::http;
namespace syntax = http::syntax;

typedef syntax::content_length<char> content_length;
typedef content_length::result result;

result from_decimal_string(boost::string_ref in, uint8_t &out) {
    return content_length::decode(in, out);
}

TEST_CASE("from_decimal_string", "[detail]")
{
    result DECSTRING_INVALID = result::invalid;
    result DECSTRING_OK = result::ok;
    result DECSTRING_OVERFLOW = result::overflow;

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
    out = 1;
    REQUIRE(from_decimal_string("", out) == DECSTRING_INVALID);
    REQUIRE(out == 1);
}
