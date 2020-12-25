#include <boost/asio.hpp>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

// Boost.Coroutine 1.72 workaround {{{
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
// }}}

#include <boost/asio/spawn.hpp>

#include <boost/http/buffered_socket.hpp>
#include <boost/http/algorithm.hpp>
#include <boost/http/response.hpp>
#include <boost/http/request.hpp>

#include "utils/get_socket_pair.hpp"
#include "utils/mocksocket.hpp"

using namespace boost;

enum {
    SERVER,
    CLIENT
};

template<unsigned N>
void fill_vector(std::vector<char> &v, const char (&s)[N])
{
    v.insert(v.end(), s, s + N - 1);
}

BOOST_AUTO_TEST_CASE(chunkext)
{
    asio::io_context ioctx;

    asio::spawn(ioctx, [&ioctx](asio::yield_context yield) {
        fiber_baton baton{ioctx};
        auto pair = get_socket_pair(ioctx, yield);

        asio::spawn(yield, [&baton,&pair](asio::yield_context yield) {
            http::buffered_socket socket{std::move(std::get<CLIENT>(pair))};
            baton.post();

            http::request request;

            request.method() = "GET";
            request.target() = "/";
            request.headers().emplace("host", "localhost");
            socket.async_write_request(request, yield);

            http::response response;
            socket.async_read_response(response, yield);
            baton.post();

            auto &message = response;
            http::response::headers_type chunkext;

            auto n = socket.async_read_chunkext(message, chunkext, yield);
            BOOST_REQUIRE_EQUAL(n, 1);
            BOOST_REQUIRE_EQUAL(message.body().size(), 0);
            BOOST_REQUIRE_EQUAL(chunkext.size(), 1);
            BOOST_REQUIRE_EQUAL(chunkext.begin()->first, "pants");
            BOOST_REQUIRE_EQUAL(chunkext.find("pants")->second, "on");

            n = socket.async_read_chunkext(message, chunkext, yield);
            BOOST_REQUIRE_EQUAL(n, 0);
            BOOST_REQUIRE_EQUAL(message.body().size(), 1);
            BOOST_REQUIRE_EQUAL(chunkext.size(), 0);
            baton.post();

            n = socket.async_read_chunkext(message, chunkext, yield);
            BOOST_REQUIRE_EQUAL(n, 0);
            BOOST_REQUIRE_EQUAL(message.body().size(), 2);
            BOOST_REQUIRE_EQUAL(chunkext.size(), 0);
            baton.post();

            n = socket.async_read_chunkext(message, chunkext, yield);
            BOOST_REQUIRE_EQUAL(n, 1);
            BOOST_REQUIRE_EQUAL(message.body().size(), 2);
            BOOST_REQUIRE_EQUAL(chunkext.size(), 3);
            {
                http::response::headers_type expected;
                expected.emplace("pants", "on");
                expected.emplace("pants", "off");
                expected.emplace("empty", "");
                BOOST_REQUIRE(chunkext == expected);
            }

            n = socket.async_read_chunkext(message, chunkext, yield);
            BOOST_REQUIRE_EQUAL(n, 0);
            BOOST_REQUIRE_EQUAL(message.body().size(), 3);
            BOOST_REQUIRE_EQUAL(chunkext.size(), 0);

        });
        baton.wait(yield);

        http::buffered_socket socket{std::move(std::get<SERVER>(pair))};
        http::request request;

        socket.async_read_request(request, yield);
        BOOST_REQUIRE_EQUAL(request.method(), "GET");
        BOOST_REQUIRE_EQUAL(request.target(), "/");
        BOOST_REQUIRE_EQUAL(request.headers().size(), 1);
        BOOST_REQUIRE_EQUAL(request.headers().begin()->first, "host");
        BOOST_REQUIRE_EQUAL(request.headers().find("host")->second,
                            "localhost");

        http::response response;
        response.status_code() = 200;
        response.reason_phrase() = "OK";
        socket.async_write_response_metadata(response, yield);
        baton.wait(yield);

        auto &message = response;
        http::response::headers_type chunkext;

        chunkext.emplace("pants", "on");
        message.body().push_back(22);
        socket.async_write_chunkext(message, chunkext, yield);
        baton.wait(yield);

        socket.async_write(message, yield);
        baton.wait(yield);

        chunkext.emplace("pants", "off");
        chunkext.emplace("empty", "");
        socket.async_write_chunkext(message, chunkext, yield);
    });

    ioctx.run();
}

BOOST_AUTO_TEST_CASE(invalid_chunkext)
{
    asio::io_context ioctx;
    asio::spawn(ioctx, [&ioctx](asio::yield_context yield) {
        http::basic_buffered_socket<mock_socket> socket{ioctx};
        socket.next_layer().input_buffer.emplace_back();
        fill_vector(socket.next_layer().input_buffer.back(),
                    "HTTP/1.1 200 OK\r\n"
                    "Transfer-Encoding: chunked\r\n"
                    "\r\n");

        socket.next_layer().input_buffer.emplace_back();
        fill_vector(socket.next_layer().input_buffer.back(),
                    "4this-is-wrong;true=1;false=0\r\n"
                    "Wiki\r\n"

                    "5;;;=hey;;=you\r\n"
                    "pedia\r\n"

                    "3\r\n"
                    " in\r\n");

        http::request request;
        http::response response;
        request.method() = "GET";
        request.target() = "/";

        socket.async_write_request(request, yield);
        socket.async_read_response(response, yield);

        auto &message = response;
        http::response::headers_type chunkext;

        auto n = socket.async_read_chunkext(message, chunkext, yield);
        BOOST_REQUIRE_EQUAL(n, 4);
        BOOST_REQUIRE_EQUAL(message.body().size(), 0);
        BOOST_REQUIRE_EQUAL(chunkext.size(), 2);
        {
            http::response::headers_type expected;
            expected.emplace("true", "1");
            expected.emplace("false", "0");
            BOOST_REQUIRE(chunkext == expected);
        }

        n = socket.async_read_chunkext(message, chunkext, yield);
        BOOST_REQUIRE_EQUAL(n, 0);
        BOOST_REQUIRE_EQUAL(message.body().size(), 12);
        BOOST_REQUIRE_EQUAL(chunkext.size(), 0);
    });
    ioctx.run();
}
