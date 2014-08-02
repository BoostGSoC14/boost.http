#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <boost/http/algorithm.hpp>
#include <boost/http/message.hpp>

BOOST_AUTO_TEST_CASE(header_value_any_of_word_splitting_and_iteration_count) {
    using boost::http::detail::header_value_any_of;
    using std::string;
    typedef string::const_iterator iterator;
    using std::vector;

    int counter = 0;
    string connection;
    bool ret;
    ret = header_value_any_of(connection, [&counter](iterator, iterator) {
            ++counter;
            return true;
        });
    BOOST_REQUIRE(counter == 0);
    BOOST_REQUIRE(ret == false);

    connection = "upgrade";
    ret = header_value_any_of(connection, [&counter](iterator begin,
                                                     iterator end) {
                                  BOOST_REQUIRE(string(begin, end) == "upgrade");
                                  ++counter;
                                  return false;
        });
    BOOST_REQUIRE(counter == 1);
    BOOST_REQUIRE(ret == false);

    connection = "upgrade,";
    ret = header_value_any_of(connection, [&counter](iterator begin,
                                                     iterator end) {
                                  BOOST_REQUIRE(string(begin, end) == "upgrade");
                                  ++counter;
                                  return true;
        });
    BOOST_REQUIRE(counter == 2);
    BOOST_REQUIRE(ret == true);

    connection = ",upgrade";
    ret = header_value_any_of(connection, [&counter](iterator begin,
                                                     iterator end) {
                                  BOOST_REQUIRE(string(begin, end) == "upgrade");
                                  ++counter;
                                  return false;
        });
    BOOST_REQUIRE(counter == 3);
    BOOST_REQUIRE(ret == false);

    connection = "upgrade   ,";
    ret = header_value_any_of(connection, [&counter](iterator begin,
                                                     iterator end) {
                                  BOOST_REQUIRE(string(begin, end) == "upgrade");
                                  ++counter;
                                  return false;
        });
    BOOST_REQUIRE(counter == 4);
    BOOST_REQUIRE(ret == false);

    connection = ",    upgrade";
    ret = header_value_any_of(connection, [&counter](iterator begin,
                                                     iterator end) {
                                  BOOST_REQUIRE(string(begin, end) == "upgrade");
                                  ++counter;
                                  return false;
        });
    BOOST_REQUIRE(counter == 5);
    BOOST_REQUIRE(ret == false);

    connection = "upgrade,close";
    ret = header_value_any_of(connection, [&counter](iterator begin,
                                                     iterator end) {
                                  vector<string> v = {"upgrade", "close"};
                                  BOOST_REQUIRE(string(begin, end) == v[counter-5]);
                                  ++counter;
            return false;
        });
    BOOST_REQUIRE(counter == 7);
    BOOST_REQUIRE(ret == false);

    connection = "upgrade   ,close";
    ret = header_value_any_of(connection, [&counter](iterator begin,
                                                     iterator end) {
            vector<string> v = {"upgrade", "close"};
            BOOST_REQUIRE(string(begin, end) == v[counter-7]);
            ++counter;
            return false;
        });
    BOOST_REQUIRE(counter == 9);
    BOOST_REQUIRE(ret == false);

    connection = "upgrade,   close";
    ret = header_value_any_of(connection, [&counter](iterator begin,
                                                     iterator end) {
                                  vector<string> v = {"upgrade", "close"};
                                  BOOST_REQUIRE(string(begin, end) == v[counter-9]);
                                  ++counter;
                                  return false;
        });
    BOOST_REQUIRE(counter == 11);
    BOOST_REQUIRE(ret == false);

    connection = "upgrade    ,    close";
    ret = header_value_any_of(connection, [&counter](iterator begin,
                                                     iterator end) {
                                  vector<string> v = {"upgrade", "close"};
                                  BOOST_REQUIRE(string(begin, end) == v[counter-11]);
                                  ++counter;
                                  return false;
        });
    BOOST_REQUIRE(counter == 13);
    BOOST_REQUIRE(ret == false);
}

BOOST_AUTO_TEST_CASE(header_value_any_of_iteration_control) {
    using boost::http::detail::header_value_any_of;
    using std::string;
    typedef string::const_iterator iterator;
    using std::vector;

    int counter = 0;
    string connection;
    bool ret;

    ret = header_value_any_of(connection, [&counter](iterator, iterator) {
            ++counter;
            return true;
        });
    BOOST_REQUIRE(counter == 0);
    BOOST_REQUIRE(ret == false);

    connection = ",,test, test2,,test4";
    ret = header_value_any_of(connection, [&counter](iterator begin,
                                                     iterator end) {
                                  ++counter;
                                  return string(begin, end) == "test";
        });
    BOOST_REQUIRE(counter == 1);
    BOOST_REQUIRE(ret == true);

    connection = "test ,test2  ,,test4 ,  ,";
    ret = header_value_any_of(connection, [&counter](iterator begin,
                                                     iterator end) {
                                  ++counter;
                                  return string(begin, end) == "test2";
        });
    BOOST_REQUIRE(counter == 3);
    BOOST_REQUIRE(ret == true);

    connection = ",   ,  test, test2 ,test4 ,,";
    ret = header_value_any_of(connection, [&counter](iterator begin,
                                                     iterator end) {
                                  ++counter;
                                  return string(begin, end) == "test4";
        });
    BOOST_REQUIRE(counter == 6);
    BOOST_REQUIRE(ret == true);
}

BOOST_AUTO_TEST_CASE(incoming_request_continue_required) {
    boost::http::message m;
    m.headers.emplace("expect", "100-continue");
    BOOST_CHECK(boost::http::incoming_request_continue_required(m) == true);
    m.headers.clear();
    BOOST_CHECK(boost::http::incoming_request_continue_required(m) == false);
    m.headers.emplace("expect", "100-CoNtInUe");
    BOOST_CHECK(boost::http::incoming_request_continue_required(m) == true);
    m.headers.clear();
    m.headers.emplace("expect", "element,100-continue");
    BOOST_CHECK(boost::http::incoming_request_continue_required(m) == false);
    m.headers.emplace("expect", "100-continue");
    BOOST_CHECK(boost::http::incoming_request_continue_required(m) == true);
}

BOOST_AUTO_TEST_CASE(incoming_request_upgrade_desired) {
    boost::http::message m;
    m.headers.emplace("upgrade", "websocket");
    BOOST_CHECK(boost::http::incoming_request_upgrade_desired(m) == false);
    m.headers.emplace("connection", "close");
    BOOST_CHECK(boost::http::incoming_request_upgrade_desired(m) == false);
    m.headers.emplace("connection", "uPgRaDe");
    BOOST_CHECK(boost::http::incoming_request_upgrade_desired(m) == true);
    m.headers.clear();
    m.headers.emplace("connection", "close,uPgRaDe");
    BOOST_CHECK(boost::http::incoming_request_upgrade_desired(m) == false);
    m.headers.emplace("upgrade", "websocket");
    BOOST_CHECK(boost::http::incoming_request_upgrade_desired(m) == true);
}
