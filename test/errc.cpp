#include "unit_test.hpp"

#include <boost/http/http_errc.hpp>

BOOST_AUTO_TEST_CASE(errc) {
    BOOST_CHECK(boost::system::is_error_condition_enum<boost::http::http_errc>::value);
    BOOST_CHECK(static_cast<int>(boost::http::http_errc::out_of_order) == 1);
    BOOST_CHECK(boost::system::error_code(boost::http::http_errc::out_of_order)
                .value() == 1);
    BOOST_CHECK(&boost::http::http_category() == &boost::http::http_category());
    BOOST_CHECK(std::string(boost::http::http_category().name()) == "http");
    BOOST_CHECK(&boost::system::error_code(boost::http::http_errc::out_of_order)
                .category() == &boost::http::http_category());
    BOOST_CHECK(boost::system::error_code(boost::http::http_errc::out_of_order)
                .message() == "HTTP actions issued on the wrong order for some"
                " object");
    BOOST_CHECK(&boost::system::error_condition(boost::http::http_errc::out_of_order)
                .category() == &boost::http::http_category());
    BOOST_CHECK(boost::system::error_condition(boost::http::http_errc::out_of_order)
                .message() == "HTTP actions issued on the wrong order for some"
                " object");
}
