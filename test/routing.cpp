#include "unit_test.hpp"

#include <boost/http/regex_router.hpp>

BOOST_AUTO_TEST_CASE(regex_route_test)
{
    using std::regex;

    //*********** Set-up types for router

    #define ROUTER_FUNCTION_ARGUMENTS int

    typedef std::function<void(ROUTER_FUNCTION_ARGUMENTS)> RouteFunctionType;

    typedef ::boost::http::regex_router<RouteFunctionType, ROUTER_FUNCTION_ARGUMENTS>
                RouterType;

    //*********** Route destination
    /* This function will set route_flags bits for each call.  At the end of
     * each test, only one route should have been called, so only one bit
     * should have been set.
     */

    int route_flags = 0;

    auto setRouteFlag = [&route_flags](int index)
    {
        route_flags |= (1 << index);
    };

    //*********** Create router that matches regex to a call to setRouteFlag

    RouterType router = {
        {regex("/index.html"),  [&](int){ setRouteFlag(0); }},
        {regex("/.*"),          [&](int){ setRouteFlag(1); }},
    };

    //*********** Run tests

    BOOST_CHECK(router("/index.html", 0) == true);
    BOOST_CHECK(route_flags == 1);

    route_flags = 0;

    BOOST_CHECK(router("/fake.html", 0) == true);
    BOOST_CHECK(route_flags == 2);

    route_flags = 0;

    BOOST_CHECK(router("/", 0) == true);
    BOOST_CHECK(route_flags == 2);

    route_flags = 0;

    BOOST_CHECK(router("index.html", 0) == false); // no initial "/"
    BOOST_CHECK(route_flags == 0);

    route_flags = 0;

    BOOST_CHECK(router("", 0) == false); // no initial "/"
    BOOST_CHECK(route_flags == 0);
}
