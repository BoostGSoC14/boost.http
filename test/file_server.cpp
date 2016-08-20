#include "unit_test.hpp"

#include <boost/http/file_server.hpp>
#include "mocksocket.hpp"

using namespace boost;
using namespace std;

bool check_not_found(const system::system_error &e)
{
    return system::error_code(http::file_server_errc::file_not_found)
        == e.code();
}

BOOST_AUTO_TEST_CASE(resolve_dots_or_throw_not_found) {
    using http::detail::resolve_dots_or_throw_not_found;
    using filesystem::path;

    // string-based tests (the URL read from the HTTP request)
    BOOST_CHECK_EQUAL(resolve_dots_or_throw_not_found("/"), path{});
    BOOST_CHECK_EQUAL(resolve_dots_or_throw_not_found("/."), path{});
    BOOST_CHECK_EQUAL(resolve_dots_or_throw_not_found("/./"), path{});
    BOOST_CHECK_EQUAL(resolve_dots_or_throw_not_found("/abc"),
                      path{} / "abc");
    BOOST_CHECK_EQUAL(resolve_dots_or_throw_not_found("/abc/def"),
                      path{} / "abc" / "def");
    BOOST_CHECK_EQUAL(resolve_dots_or_throw_not_found("/abc/../def"),
                      path{} / "def");
    BOOST_CHECK_EXCEPTION(resolve_dots_or_throw_not_found("/../abc"),
                          system::system_error, check_not_found);
    BOOST_CHECK_EXCEPTION(resolve_dots_or_throw_not_found("/.."),
                          system::system_error, check_not_found);
    BOOST_CHECK_EQUAL(resolve_dots_or_throw_not_found("/abc/.."), path{});

    // user-provided path
    BOOST_CHECK_EQUAL(resolve_dots_or_throw_not_found(path{}), path{});
    BOOST_CHECK_EQUAL(resolve_dots_or_throw_not_found(path{} / "abc"),
                      path{} / "abc");
    BOOST_CHECK_EQUAL(resolve_dots_or_throw_not_found(path{} / "abc" / "def"),
                      path{} / "abc" / "def");
    BOOST_CHECK_EQUAL(resolve_dots_or_throw_not_found(path{} / "abc" / ".."
                                                      / "def"),
                      path{} / "def");
    BOOST_CHECK_EXCEPTION(resolve_dots_or_throw_not_found(path{} / ".."
                                                          / "abc"),
                          system::system_error, check_not_found);
    BOOST_CHECK_EXCEPTION(resolve_dots_or_throw_not_found(path{} / ".."),
                          system::system_error, check_not_found);
    BOOST_CHECK_EQUAL(resolve_dots_or_throw_not_found(path{} / "abc" / ".."),
                      path{});
}
