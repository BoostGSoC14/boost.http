#ifdef NDEBUG
#undef NDEBUG
#endif

#define CATCH_CONFIG_MAIN
#include "common.hpp"

namespace asio = boost::asio;

TEST_CASE("Utility test functions", "[misc]")
{
    CHECK(asio::buffer_size(my_buffer("")) == 0);
    CHECK(asio::buffer_size(my_buffer("Nu")) == 2);

    {
        std::vector<char> buf(10, 'c');
        my_copy(buf, 0, "a");
        REQUIRE(buf[0] == 'a');
        REQUIRE(buf[1] == 'c');
        my_copy(buf, 2, "bb");
        REQUIRE(buf[0] == 'a');
        REQUIRE(buf[1] == 'c');
        REQUIRE(buf[2] == 'b');
        REQUIRE(buf[3] == 'b');
        REQUIRE(buf[4] == 'c');
        my_copy(buf, 0, "");
        REQUIRE(buf[0] == 'a');
    }
}
