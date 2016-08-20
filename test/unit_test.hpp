/* This code is based on the Minimal Boost C++ Libraries emulation layer by
   Niall Douglas:
   https://github.com/ned14/boost-lite/blob/master/include/boost/test/unit_test.hpp

   Take care. */

#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_PREFIX_ALL
#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#define BOOST_TEST_MESSAGE(msg) CATCH_INFO(msg)
#define BOOST_WARN_MESSAGE(pred, msg) \
    if(!(pred))                       \
        CATCH_WARN(msg)
#define BOOST_FAIL(msg) CATCH_FAIL(msg)
#define BOOST_CHECK_MESSAGE(pred, msg) \
    if(!(pred))                        \
        CATCH_INFO(msg)

#define BOOST_CHECK(expr) CATCH_CHECK(expr)
#define BOOST_CHECK_THROWS(expr) CATCH_CHECK_THROWS(expr)
#define BOOST_CHECK_THROW(expr, type) CATCH_CHECK_THROWS_AS(expr, type)
#define BOOST_CHECK_NO_THROW(expr) CATCH_CHECK_NOTHROW(expr)

#define BOOST_CHECK_EQUAL(lhs, rhs) CATCH_CHECK((lhs) == (rhs))

#define BOOST_REQUIRE(expr) CATCH_REQUIRE(expr)
#define BOOST_REQUIRE_THROWS(expr) CATCH_REQUIRE_THROWS(expr)
#define BOOST_CHECK_REQUIRE(expr, type) CATCH_REQUIRE_THROWS_AS(expr, type)
#define BOOST_REQUIRE_NO_THROW(expr) CATCH_REQUIRE_NOTHROW(expr)

#define BOOST_CHECK_EXCEPTION(statement, exception, predicate) \
    do {                                                       \
        bool captured = false;                                 \
        try {                                                  \
            statement;                                         \
        } catch (exception &e) {                               \
            BOOST_CHECK(predicate(e));                         \
            captured = true;                                   \
        }                                                      \
        BOOST_CHECK(captured);                                 \
    } while(0)

#define BOOST_CATCH_AUTO_TEST_CASE_NAME(name) #name
#define BOOST_AUTO_TEST_CASE(test_name) CATCH_TEST_CASE(BOOST_CATCH_AUTO_TEST_CASE_NAME(test_name), "")
