#include "unit_test.hpp"

#include <boost/utility/string_ref.hpp>
#include <boost/http/algorithm/header.hpp>

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
    /* Just need to test up to 4 digits **IF** the using code is
       datetime-related */
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

BOOST_AUTO_TEST_CASE(from_decimal_string_overflow_case) {
    // first cases that shouldn't fail
    BOOST_CHECK(from_decimal_string<std::uint8_t>("255") == 255);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("254") == 254);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("253") == 253);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("252") == 252);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("251") == 251);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("250") == 250);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("249") == 249);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("248") == 248);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("247") == 247);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("246") == 246);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("245") == 245);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("244") == 244);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("0") == 0);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("1") == 1);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("2") == 2);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("3") == 3);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("4") == 4);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("5") == 5);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("6") == 6);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("7") == 7);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("8") == 8);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("9") == 9);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("10") == 10);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("11") == 11);

    BOOST_CHECK(from_decimal_string<std::uint8_t>("000000000000000255") == 255);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("000000000000000254") == 254);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("000000000000000253") == 253);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("000000000000000252") == 252);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("000000000000000251") == 251);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("000000000000000250") == 250);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("000000000000000249") == 249);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("000000000000000248") == 248);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("000000000000000247") == 247);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("000000000000000246") == 246);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("000000000000000245") == 245);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("000000000000000244") == 244);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("0000000000000000") == 0);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("0000000000000001") == 1);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("0000000000000002") == 2);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("0000000000000003") == 3);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("0000000000000004") == 4);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("0000000000000005") == 5);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("0000000000000006") == 6);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("0000000000000007") == 7);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("0000000000000008") == 8);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("0000000000000009") == 9);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("00000000000000010") == 10);
    BOOST_CHECK(from_decimal_string<std::uint8_t>("00000000000000011") == 11);

    // now cases that should fail
    bool overflow_detected = false;

    try {
        from_decimal_string<std::uint8_t>("266");
    } catch(std::overflow_error&) {
        overflow_detected = true;
    }
    BOOST_REQUIRE(overflow_detected);

    overflow_detected = false;
    try {
        from_decimal_string<std::uint8_t>("265");
    } catch(std::overflow_error&) {
        overflow_detected = true;
    }
    BOOST_REQUIRE(overflow_detected);

    overflow_detected = false;
    try {
        from_decimal_string<std::uint8_t>("264");
    } catch(std::overflow_error&) {
        overflow_detected = true;
    }
    BOOST_REQUIRE(overflow_detected);

    overflow_detected = false;
    try {
        from_decimal_string<std::uint8_t>("263");
    } catch(std::overflow_error&) {
        overflow_detected = true;
    }
    BOOST_REQUIRE(overflow_detected);

    overflow_detected = false;
    try {
        from_decimal_string<std::uint8_t>("262");
    } catch(std::overflow_error&) {
        overflow_detected = true;
    }
    BOOST_REQUIRE(overflow_detected);

    overflow_detected = false;
    try {
        from_decimal_string<std::uint8_t>("261");
    } catch(std::overflow_error&) {
        overflow_detected = true;
    }
    BOOST_REQUIRE(overflow_detected);

    overflow_detected = false;
    try {
        from_decimal_string<std::uint8_t>("260");
    } catch(std::overflow_error&) {
        overflow_detected = true;
    }
    BOOST_REQUIRE(overflow_detected);

    overflow_detected = false;
    try {
        from_decimal_string<std::uint8_t>("259");
    } catch(std::overflow_error&) {
        overflow_detected = true;
    }
    BOOST_REQUIRE(overflow_detected);

    overflow_detected = false;
    try {
        from_decimal_string<std::uint8_t>("258");
    } catch(std::overflow_error&) {
        overflow_detected = true;
    }
    BOOST_REQUIRE(overflow_detected);

    overflow_detected = false;
    try {
        from_decimal_string<std::uint8_t>("257");
    } catch(std::overflow_error&) {
        overflow_detected = true;
    }
    BOOST_REQUIRE(overflow_detected);

    overflow_detected = false;
    try {
        from_decimal_string<std::uint8_t>("256");
    } catch(std::overflow_error&) {
        overflow_detected = true;
    }
    BOOST_REQUIRE(overflow_detected);

    // Now with 000's prefix

    try {
        from_decimal_string<std::uint8_t>("0000000000000000000000000000000266");
    } catch(std::overflow_error&) {
        overflow_detected = true;
    }
    BOOST_REQUIRE(overflow_detected);

    overflow_detected = false;
    try {
        from_decimal_string<std::uint8_t>("0000000000000000000000000000000265");
    } catch(std::overflow_error&) {
        overflow_detected = true;
    }
    BOOST_REQUIRE(overflow_detected);

    overflow_detected = false;
    try {
        from_decimal_string<std::uint8_t>("0000000000000000000000000000000264");
    } catch(std::overflow_error&) {
        overflow_detected = true;
    }
    BOOST_REQUIRE(overflow_detected);

    overflow_detected = false;
    try {
        from_decimal_string<std::uint8_t>("0000000000000000000000000000000263");
    } catch(std::overflow_error&) {
        overflow_detected = true;
    }
    BOOST_REQUIRE(overflow_detected);

    overflow_detected = false;
    try {
        from_decimal_string<std::uint8_t>("0000000000000000000000000000000262");
    } catch(std::overflow_error&) {
        overflow_detected = true;
    }
    BOOST_REQUIRE(overflow_detected);

    overflow_detected = false;
    try {
        from_decimal_string<std::uint8_t>("0000000000000000000000000000000261");
    } catch(std::overflow_error&) {
        overflow_detected = true;
    }
    BOOST_REQUIRE(overflow_detected);

    overflow_detected = false;
    try {
        from_decimal_string<std::uint8_t>("0000000000000000000000000000000260");
    } catch(std::overflow_error&) {
        overflow_detected = true;
    }
    BOOST_REQUIRE(overflow_detected);

    overflow_detected = false;
    try {
        from_decimal_string<std::uint8_t>("0000000000000000000000000000000259");
    } catch(std::overflow_error&) {
        overflow_detected = true;
    }
    BOOST_REQUIRE(overflow_detected);

    overflow_detected = false;
    try {
        from_decimal_string<std::uint8_t>("0000000000000000000000000000000258");
    } catch(std::overflow_error&) {
        overflow_detected = true;
    }
    BOOST_REQUIRE(overflow_detected);

    overflow_detected = false;
    try {
        from_decimal_string<std::uint8_t>("0000000000000000000000000000000257");
    } catch(std::overflow_error&) {
        overflow_detected = true;
    }
    BOOST_REQUIRE(overflow_detected);

    overflow_detected = false;
    try {
        from_decimal_string<std::uint8_t>("0000000000000000000000000000000256");
    } catch(std::overflow_error&) {
        overflow_detected = true;
    }
    BOOST_REQUIRE(overflow_detected);
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
    BOOST_CHECK(!rfc1123(string_ref(" Sun, 06 Nov 1994 08:49:37 GMT"),
                         datetime));
    BOOST_CHECK(!rfc1123(string_ref("Sun, 06 Nov 1994 08:49:37 GMT "),
                         datetime));
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

    BOOST_CHECK(!rfc1036(string_ref(" Sunday, 06-Nov-94 08:49:37 GMT"),
                         datetime));
    BOOST_CHECK(!rfc1036(string_ref("Sunday, 06-Nov-94 08:49:37 GMT "),
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

    BOOST_CHECK(!asctime(string_ref(" Tue Dec  1 00:00:00 1903"),
                         datetime));
    BOOST_CHECK(!asctime(string_ref("Tue Dec  1 00:00:00 1903 "),
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

BOOST_AUTO_TEST_CASE(append_number_case) {
    using std::string;
    using boost::http::detail::append_number;

    string buffer;

    append_number<string, 0>(buffer, 1);
    BOOST_REQUIRE(buffer.size() == 0);

    append_number<string, 8>(buffer, 12345678);
    BOOST_REQUIRE(buffer == "12345678");

    append_number<string, 8>(buffer, 0);
    BOOST_REQUIRE(buffer == "12345678"
                  "00000000");

    append_number<string, 3>(buffer, 1002);
    BOOST_REQUIRE(buffer == "12345678"
                  "00000000"
                  "002");

    append_number<string, 3>(buffer, 1);
    BOOST_REQUIRE(buffer == "12345678"
                  "00000000"
                  "002"
                  "001");
}

BOOST_AUTO_TEST_CASE(to_http_date_case) {
    using std::string;
    using boost::http::to_http_date;

    BOOST_CHECK(to_http_date<string>(make_datetime(1994, 11, 6, 8, 49, 37))
                == "Sun, 06 Nov 1994 08:49:37 GMT");
    BOOST_CHECK(to_http_date<string>(make_datetime(1903, 12, 1, 0, 0, 0))
                == "Tue, 01 Dec 1903 00:00:00 GMT");

    const boost::posix_time::ptime invalid_ptime;
    bool exception_throw = false;

    try {
        to_http_date<string>(boost::posix_time::ptime{});
    } catch(std::out_of_range&) {
        exception_throw = true;
    }

    BOOST_CHECK(exception_throw);
}
