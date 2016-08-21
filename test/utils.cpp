#ifdef NDEBUG
#undef NDEBUG
#endif

#define CATCH_CONFIG_MAIN
#include "common.hpp"
#include <boost/algorithm/string/find.hpp>
#include <boost/http/detail/macros.hpp>

TEST_CASE("Unreachable macro", "[detail]")
{
#define BOOST_HTTP_SPONSOR "[random string]: anarchy is coming"
    try {
        BOOST_HTTP_DETAIL_UNREACHABLE(BOOST_HTTP_SPONSOR);
    } catch(const char *ex) {
        REQUIRE(!boost::find_first(ex, BOOST_HTTP_SPONSOR).empty());
        return;
    }
    throw "shouldn't happen";
#undef BOOST_HTTP_SPONSOR
}
