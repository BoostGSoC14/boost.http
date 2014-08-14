#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <boost/http/algorithm/header.hpp>

#include <iostream>

template<class Target, class String>
Target from_decimal_string(const String &value)
{
    return boost::http::detail::from_decimal_string<Target>(value.begin(),
                                                            value.end());
}

template<class Target, unsigned N>
Target from_decimal_string(const char (&value)[N])
{
    return boost::http::detail::from_decimal_string<Target>(value,
                                                            value + N - 1);
}

template<class Y, class Mo, class D, class H, class Mi, class S>
boost::posix_time::ptime make_datetime(Y y, Mo mo, D d, H h, Mi mi, S s)
{
    using namespace boost::posix_time;
    return ptime(boost::gregorian::date(y, mo, d), time_duration(h, mi, s));
}

void reset(boost::posix_time::ptime &datetime)
{
    datetime = boost::posix_time::ptime();
}

BOOST_AUTO_TEST_CASE(from_decimal_string_case) {
    // Just need to test up to 4 digits
    BOOST_CHECK(from_decimal_string<int>("") == 0);
    BOOST_CHECK(from_decimal_string<int>("0") == 0);
    BOOST_CHECK(from_decimal_string<int>("000") == 0);
    BOOST_CHECK(from_decimal_string<int>("001") == 1);
    BOOST_CHECK(from_decimal_string<int>("1") == 1);
    BOOST_CHECK(from_decimal_string<int>("10") == 10);
    BOOST_CHECK(from_decimal_string<int>("010") == 10);
    BOOST_CHECK(from_decimal_string<int>("0010") == 10);
    BOOST_CHECK(from_decimal_string<int>("11") == 11);
    BOOST_CHECK(from_decimal_string<int>("011") == 11);
    BOOST_CHECK(from_decimal_string<int>("0011") == 11);
    BOOST_CHECK(from_decimal_string<int>("1000") == 1000);
    BOOST_CHECK(from_decimal_string<int>("2020") == 2020);
    BOOST_CHECK(from_decimal_string<int>("1984") == 1984);
    BOOST_CHECK(from_decimal_string<int>("1337") == 1337);
    BOOST_CHECK(from_decimal_string<int>("42") == 42);
}

BOOST_AUTO_TEST_CASE(rfc1123_rfc1036_asctime) {
    using namespace boost::posix_time;
    using boost::http::detail::rfc1123;
    using boost::http::detail::rfc1036;
    using boost::http::detail::asctime;
    using boost::string_ref;

    ptime datetime;

    // "additive" (correct inputs are correct outputs) tests
    BOOST_REQUIRE(rfc1123(string_ref("Sun, 06 Nov 1994 08:49:37 GMT"),
                          datetime));
    BOOST_REQUIRE(datetime == make_datetime(1994, 11, 6, 8, 49, 37));

    reset(datetime);
    BOOST_REQUIRE(datetime == ptime());

    BOOST_REQUIRE(rfc1036(string_ref("Sunday, 06-Nov-94 08:49:37 GMT"),
                          datetime));
    BOOST_REQUIRE(datetime == make_datetime(1994, 11, 6, 8, 49, 37));

    reset(datetime);

    BOOST_REQUIRE(asctime(string_ref("Sun Nov  6 08:49:37 1994"), datetime));
    BOOST_REQUIRE(datetime == make_datetime(1994, 11, 6, 8, 49, 37));

    BOOST_REQUIRE(rfc1123(string_ref("Tue, 01 Dec 1903 00:00:00 GMT"),
                          datetime));
    BOOST_REQUIRE(datetime == make_datetime(1903, 12, 1, 0, 0, 0));

    reset(datetime);

    BOOST_REQUIRE(rfc1036(string_ref("Tuesday, 01-Dec-03 00:00:00 GMT"),
                          datetime));
    BOOST_REQUIRE(datetime == make_datetime(1903, 12, 1, 0, 0, 0));

    reset(datetime);

    BOOST_REQUIRE(asctime(string_ref("Tue Dec  1 00:00:00 1903"),
                          datetime));
    BOOST_REQUIRE(datetime == make_datetime(1903, 12, 1, 0, 0, 0));

    // "negative" (invalid input is rejected) tests
    BOOST_CHECK(!rfc1123(string_ref("Sun, 06 Nov 0001 08:49:37 GMT"),
                         datetime)); // boost::date range limitation
    BOOST_CHECK(!rfc1123(string_ref("Thu, 31 Nov 1994 08:49:37 GMT"),
                         datetime));
    BOOST_CHECK(!rfc1123(string_ref("Sun, 06 Nov 1994 24:49:37 GMT"),
                         datetime));
    BOOST_CHECK(!rfc1123(string_ref("Sun, 06 Nov 1994 08:60:37 GMT"),
                         datetime));
    BOOST_CHECK(!rfc1123(string_ref("Sun, 06 Nov 1994 08:49:61 GMT"),
                         datetime));
    BOOST_CHECK(!rfc1123(string_ref("Sun, 06 Nov 1994 08:49:37 GMT"
                                    "Sun, 06 Nov 1994 08:49:37 GMT"),
                         datetime));
    BOOST_CHECK(!rfc1123(string_ref("Sunday, 06-Nov-94 08:49:37 GMT"),
                         datetime));
    BOOST_CHECK(!rfc1123(string_ref("Sun Nov  6 08:49:37 1994"), datetime));
    BOOST_CHECK(!rfc1123(string_ref("All your base are belong to us"),
                         datetime));

    BOOST_CHECK(!rfc1036(string_ref("Thursday, 31-Nov-94 08:49:37 GMT"),
                         datetime));
    BOOST_CHECK(!rfc1036(string_ref("Sunday, 06-Nov-94 24:49:37 GMT"),
                         datetime));
    BOOST_CHECK(!rfc1036(string_ref("Sunday, 06-Nov-94 08:60:37 GMT"),
                         datetime));
    BOOST_CHECK(!rfc1036(string_ref("Sunday, 06-Nov-94 08:49:61 GMT"),
                         datetime));
    BOOST_CHECK(!rfc1036(string_ref("Sunday, 06-Nov-94 08:49:37 GMT"
                                    "Sunday, 06-Nov-94 08:49:37 GMT"),
                         datetime));
    BOOST_CHECK(!rfc1036(string_ref("Sun, 06 Nov 1994 08:49:37 GMT"),
                         datetime));
    BOOST_CHECK(!rfc1036(string_ref("Sun Nov  6 08:49:37 1994"), datetime));
    BOOST_CHECK(!rfc1036(string_ref("All your base are belong to us"),
                         datetime));

    BOOST_CHECK(!asctime(string_ref("Sun Dec  1 00:00:00 0001"),
                          datetime)); // boost::date range limitation
    BOOST_CHECK(!asctime(string_ref("Thu Nov 31 08:49:37 1994"),
                          datetime));
    BOOST_CHECK(!asctime(string_ref("Tue Dec  1 24:00:00 1903"),
                          datetime));
    BOOST_CHECK(!asctime(string_ref("Tue Dec  1 00:60:00 1903"),
                          datetime));
    BOOST_CHECK(!asctime(string_ref("Tue Dec  1 00:00:61 1903"),
                          datetime));
    BOOST_CHECK(!asctime(string_ref("Tue Dec  1 00:00:00 1903"
                                    "Tue Dec  1 00:00:00 1903"),
                          datetime));
    BOOST_CHECK(!asctime(string_ref("Sun, 06 Nov 1994 08:49:37 GMT"),
                         datetime));
    BOOST_CHECK(!asctime(string_ref("Sunday, 06-Nov-94 08:49:37 GMT"),
                         datetime));
    BOOST_CHECK(!asctime(string_ref("All your base are belong to us"),
                         datetime));
}

BOOST_AUTO_TEST_CASE(header_to_ptime_case) {
    using namespace boost::posix_time;
    using boost::http::header_to_ptime;
    using boost::string_ref;

    ptime datetime;

    BOOST_CHECK(header_to_ptime(string_ref("Sun, 06 Nov 1994 08:49:37 GMT"))
                == make_datetime(1994, 11, 6, 8, 49, 37));

    reset(datetime);

    BOOST_REQUIRE(datetime == ptime());

    BOOST_CHECK(header_to_ptime(string_ref("Sunday, 06-Nov-94 08:49:37 GMT"))
                == make_datetime(1994, 11, 6, 8, 49, 37));

    reset(datetime);

    BOOST_CHECK(header_to_ptime(string_ref("Sun Nov  6 08:49:37 1994"))
                == make_datetime(1994, 11, 6, 8, 49, 37));

    BOOST_CHECK(header_to_ptime(string_ref("All your base are belong to us"))
                .is_not_a_date_time());
}
