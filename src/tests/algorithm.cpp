#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <boost/http/algorithm.hpp>
#include <boost/http/message.hpp>
#include <boost/http/socket.hpp>

BOOST_AUTO_TEST_CASE(header_value_any_of_word_splitting_and_iteration_count) {
    using boost::http::header_value_any_of;
    using boost::string_ref;
    using std::string;
    using std::vector;

    int counter = 0;
    string connection;
    bool ret;
    ret = header_value_any_of(connection, [&counter](string_ref) {
            ++counter;
            return true;
        });
    BOOST_REQUIRE(counter == 0);
    BOOST_REQUIRE(ret == false);

    connection = "upgrade";
    ret = header_value_any_of(connection, [&counter](string_ref value) {
                                  BOOST_REQUIRE(value == "upgrade");
                                  ++counter;
                                  return false;
        });
    BOOST_REQUIRE(counter == 1);
    BOOST_REQUIRE(ret == false);

    connection = "upgrade,";
    ret = header_value_any_of(connection, [&counter](string_ref value) {
                                  BOOST_REQUIRE(value == "upgrade");
                                  ++counter;
                                  return true;
        });
    BOOST_REQUIRE(counter == 2);
    BOOST_REQUIRE(ret == true);

    connection = ",upgrade";
    ret = header_value_any_of(connection, [&counter](string_ref value) {
                                  BOOST_REQUIRE(value == "upgrade");
                                  ++counter;
                                  return false;
        });
    BOOST_REQUIRE(counter == 3);
    BOOST_REQUIRE(ret == false);

    connection = "upgrade   ,";
    ret = header_value_any_of(connection, [&counter](string_ref value) {
                                  BOOST_REQUIRE(value == "upgrade");
                                  ++counter;
                                  return false;
        });
    BOOST_REQUIRE(counter == 4);
    BOOST_REQUIRE(ret == false);

    connection = ",    upgrade";
    ret = header_value_any_of(connection, [&counter](string_ref value) {
                                  BOOST_REQUIRE(value == "upgrade");
                                  ++counter;
                                  return false;
        });
    BOOST_REQUIRE(counter == 5);
    BOOST_REQUIRE(ret == false);

    connection = "upgrade,close";
    ret = header_value_any_of(connection, [&counter](string_ref value) {
                                  vector<string> v = {"upgrade", "close"};
                                  BOOST_REQUIRE(value == v[counter-5]);
                                  ++counter;
            return false;
        });
    BOOST_REQUIRE(counter == 7);
    BOOST_REQUIRE(ret == false);

    connection = "upgrade   ,close";
    ret = header_value_any_of(connection, [&counter](string_ref value) {
            vector<string> v = {"upgrade", "close"};
            BOOST_REQUIRE(value == v[counter-7]);
            ++counter;
            return false;
        });
    BOOST_REQUIRE(counter == 9);
    BOOST_REQUIRE(ret == false);

    connection = "upgrade,   close";
    ret = header_value_any_of(connection, [&counter](string_ref value) {
                                  vector<string> v = {"upgrade", "close"};
                                  BOOST_REQUIRE(value == v[counter-9]);
                                  ++counter;
                                  return false;
        });
    BOOST_REQUIRE(counter == 11);
    BOOST_REQUIRE(ret == false);

    connection = "upgrade    ,    close";
    ret = header_value_any_of(connection, [&counter](string_ref value) {
                                  vector<string> v = {"upgrade", "close"};
                                  BOOST_REQUIRE(value == v[counter-11]);
                                  ++counter;
                                  return false;
        });
    BOOST_REQUIRE(counter == 13);
    BOOST_REQUIRE(ret == false);
}

BOOST_AUTO_TEST_CASE(header_value_any_of_iteration_control) {
    using boost::http::header_value_any_of;
    using boost::string_ref;
    using std::string;
    using std::vector;

    int counter = 0;
    string connection;
    bool ret;

    ret = header_value_any_of(connection, [&counter](string_ref) {
            ++counter;
            return true;
        });
    BOOST_REQUIRE(counter == 0);
    BOOST_REQUIRE(ret == false);

    connection = ",,test, test2,,test4";
    ret = header_value_any_of(connection, [&counter](string_ref value) {
                                  ++counter;
                                  return value == "test";
        });
    BOOST_REQUIRE(counter == 1);
    BOOST_REQUIRE(ret == true);

    connection = "test ,test2  ,,test4 ,  ,";
    ret = header_value_any_of(connection, [&counter](string_ref value) {
                                  ++counter;
                                  return value == "test2";
        });
    BOOST_REQUIRE(counter == 3);
    BOOST_REQUIRE(ret == true);

    connection = ",   ,  test, test2 ,test4 ,,";
    ret = header_value_any_of(connection, [&counter](string_ref value) {
                                  ++counter;
                                  return value == "test4";
        });
    BOOST_REQUIRE(counter == 6);
    BOOST_REQUIRE(ret == true);
}

BOOST_AUTO_TEST_CASE(request_continue_required) {
    boost::http::message m;
    m.headers.emplace("expect", "100-continue");
    BOOST_CHECK(boost::http::request_continue_required(m) == true);
    m.headers.clear();
    BOOST_CHECK(boost::http::request_continue_required(m) == false);
    m.headers.emplace("expect", "100-CoNtInUe");
    BOOST_CHECK(boost::http::request_continue_required(m) == true);
    m.headers.clear();
    m.headers.emplace("expect", "element,100-continue");
    BOOST_CHECK(boost::http::request_continue_required(m) == false);
    m.headers.emplace("expect", "100-continue");
    BOOST_CHECK(boost::http::request_continue_required(m) == true);
}

BOOST_AUTO_TEST_CASE(request_upgrade_desired) {
    boost::http::message m;
    m.headers.emplace("upgrade", "websocket");
    BOOST_CHECK(boost::http::request_upgrade_desired(m) == false);
    m.headers.emplace("connection", "close");
    BOOST_CHECK(boost::http::request_upgrade_desired(m) == false);
    m.headers.emplace("connection", "uPgRaDe");
    BOOST_CHECK(boost::http::request_upgrade_desired(m) == true);
    m.headers.clear();
    m.headers.emplace("connection", "close,uPgRaDe");
    BOOST_CHECK(boost::http::request_upgrade_desired(m) == false);
    m.headers.emplace("upgrade", "websocket");
    BOOST_CHECK(boost::http::request_upgrade_desired(m) == true);
}

/* This test is pretty much just to ensure that these functions compile and
   don't try very hard to enforce the Socket concept. */
BOOST_AUTO_TEST_CASE(write_without_reason_phrase) {
    boost::asio::io_service ios;
    char buffer[1];
    boost::http::socket socket(ios, boost::asio::buffer(buffer));
    boost::http::message m;
    async_write_response(socket, boost::http::status_code::ok, m,
                         [](boost::system::error_code) {});
    async_write_response_metadata(socket, boost::http::status_code::ok, m,
                                  [](boost::system::error_code) {});
}
