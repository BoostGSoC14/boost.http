#include "unit_test.hpp"

#include <boost/http/request_response_wrapper.hpp>
#include <boost/http/request.hpp>
#include <boost/http/response.hpp>

namespace http = boost::http;

BOOST_AUTO_TEST_CASE(mocksocket_read) {
    typedef http::request_response_wrapper<http::request, http::response> M1;
    typedef
        http::request_response_wrapper<const http::request,
                                       const http::response>
        M2;
    BOOST_CHECK(!std::is_const<M1::headers_type>::value);
    BOOST_CHECK(std::is_const<M2::headers_type>::value);
    BOOST_CHECK(!std::is_const<M1::body_type>::value);
    BOOST_CHECK(std::is_const<M2::body_type>::value);
}
