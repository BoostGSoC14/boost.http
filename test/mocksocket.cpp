#include "unit_test.hpp"

#include "mocksocket.hpp"

#include <boost/asio/spawn.hpp>

using namespace boost;
using namespace std;

BOOST_AUTO_TEST_CASE(mocksocket_read) {
    asio::io_service ios;

    auto work = [&ios](asio::yield_context yield) {
        mock_socket s(ios);

        {
            vector<char> v = {'\x00', '\x01', '\x02'};

            s.input_buffer.emplace_back(std::move(v));

            v.clear();
            v.push_back('\x03');
            v.push_back('\x04');

            s.input_buffer.emplace_back(std::move(v));

            v.clear();
            v.push_back('\x05');
            v.push_back('\x06');

            s.input_buffer.emplace_back(std::move(v));
        }

        char buffer[2];
        auto transfered = s.async_read_some(asio::buffer(buffer), yield);
        BOOST_REQUIRE(transfered == 2);
        BOOST_REQUIRE(buffer[0] == '\x00');
        BOOST_REQUIRE(buffer[1] == '\x01');

        transfered = s.async_read_some(asio::buffer(buffer), yield);
        BOOST_REQUIRE(transfered == 1);
        BOOST_REQUIRE(buffer[0] == '\x02');

        transfered = s.async_read_some(asio::buffer(buffer), yield);
        BOOST_REQUIRE(transfered == 2);
        BOOST_REQUIRE(buffer[0] == '\x03');
        BOOST_REQUIRE(buffer[1] == '\x04');

        transfered = s.async_read_some(asio::buffer(buffer), yield);
        BOOST_REQUIRE(transfered == 2);
        BOOST_REQUIRE(buffer[0] == '\x05');
        BOOST_REQUIRE(buffer[1] == '\x06');

        bool captured = false;
        try {
            s.async_read_some(asio::buffer(buffer), yield);
        } catch(system::system_error &e) {
            BOOST_REQUIRE(e.code() == system::error_code(asio::error::eof));
            captured = true;
        }
        BOOST_REQUIRE(captured);
   };

    spawn(ios, work);
    ios.run();
}

BOOST_AUTO_TEST_CASE(mocksocket_write) {
    asio::io_service ios;

    auto work = [&ios](asio::yield_context yield) {
        mock_socket s(ios);

        std::vector<asio::const_buffer> buffers;
        buffers.push_back(asio::buffer("hey"));
        buffers.push_back(asio::buffer("listen"));

        auto transfered = s.async_write_some(buffers, yield);
        BOOST_REQUIRE(transfered == 11);
        BOOST_REQUIRE(s.output_buffer[0] == 'h');
        BOOST_REQUIRE(s.output_buffer[1] == 'e');
        BOOST_REQUIRE(s.output_buffer[2] == 'y');
        BOOST_REQUIRE(s.output_buffer[3] == '\0');
        BOOST_REQUIRE(s.output_buffer[4] == 'l');
        BOOST_REQUIRE(s.output_buffer[5] == 'i');
        BOOST_REQUIRE(s.output_buffer[6] == 's');
        BOOST_REQUIRE(s.output_buffer[7] == 't');
        BOOST_REQUIRE(s.output_buffer[8] == 'e');
        BOOST_REQUIRE(s.output_buffer[9] == 'n');
        BOOST_REQUIRE(s.output_buffer[10] == '\0');
   };

    spawn(ios, work);
    ios.run();
}
