#include <boost/asio.hpp>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

// Boost.Coroutine 1.72 workaround {{{
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
// }}}

#include <boost/asio/spawn.hpp>

#include <boost/http/socket.hpp>
#include <boost/http/request.hpp>
#include <boost/http/response.hpp>

#include "utils/mocksocket.hpp"

using namespace boost;

using std::vector;
using std::max;

template<class F>
void feed_with_buffer(std::size_t min_buf_size, F &&f)
{
    char buffer[2048];
    if (min_buf_size == 0) {
        min_buf_size = max({
            string_view("Host").size(),
            string_view("Transfer-Encoding").size(),
            string_view("Content-Length").size(),
        });
    }
    for (std::size_t i = min_buf_size ; i != 2048 ; ++i) {
        BOOST_TEST_MESSAGE("buffer_size = " << i);
        f(asio::buffer(buffer, i));
    }
}

template<unsigned N>
void fill_vector(vector<char> &v, const char (&s)[N])
{
    v.insert(v.end(), s, s + N - 1);
}

BOOST_AUTO_TEST_CASE(socket_basic) {
    asio::io_context ios;
    bool reached_the_end_of_the_test = false;
    auto work = [&ios,&reached_the_end_of_the_test](asio::yield_context yield) {
        feed_with_buffer(38, [&](asio::mutable_buffer inbuffer) {
            http::basic_socket<mock_socket> socket(inbuffer, ios);
            http::request request;
            http::response response;

            request.method() = "GET";
            request.target() = "/";
            request.headers().emplace("host", "localhost:8080");

            BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
            BOOST_REQUIRE(socket.write_state() == http::write_state::empty);

            socket.async_write_request(request, yield);

            {
                vector<char> v;
                fill_vector(v,
                            "GET / HTTP/1.1\r\n"
                            "host: localhost:8080\r\n"
                            "\r\n");
                BOOST_REQUIRE(socket.next_layer().output_buffer == v);
            }

            // ---

            BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
            BOOST_REQUIRE(socket.write_state() == http::write_state::empty);

            socket.next_layer().output_buffer.clear();

            request.body().push_back('z');
            socket.async_write_request(request, yield);

            {
                vector<char> v;
                fill_vector(v,
                            "GET / HTTP/1.1\r\n"
                            "content-length: 1\r\n"
                            "host: localhost:8080\r\n"
                            "\r\n"
                            "z");
                BOOST_REQUIRE(socket.next_layer().output_buffer == v);
            }

            // ---

            BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
            BOOST_REQUIRE(socket.write_state() == http::write_state::empty);

            socket.next_layer().output_buffer.clear();

            request.body().resize(10, 'z');
            socket.async_write_request(request, yield);

            {
                vector<char> v;
                fill_vector(v,
                            "GET / HTTP/1.1\r\n"
                            "content-length: 10\r\n"
                            "host: localhost:8080\r\n"
                            "\r\n"
                            "zzzzzzzzzz");
                BOOST_REQUIRE(socket.next_layer().output_buffer == v);
            }

            // ---

            BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
            BOOST_REQUIRE(socket.write_state() == http::write_state::empty);

            socket.next_layer().output_buffer.clear();

            request.body().push_back('z');
            socket.async_write_request(request, yield);

            {
                vector<char> v;
                fill_vector(v,
                            "GET / HTTP/1.1\r\n"
                            "content-length: 11\r\n"
                            "host: localhost:8080\r\n"
                            "\r\n"
                            "zzzzzzzzzzz");
                BOOST_REQUIRE(socket.next_layer().output_buffer == v);
            }

            // ---

            BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
            BOOST_REQUIRE(socket.write_state() == http::write_state::empty);

            socket.next_layer().output_buffer.clear();

            socket.async_write_request_metadata(request, yield);

            {
                vector<char> v;
                fill_vector(v,
                            "GET / HTTP/1.1\r\n"
                            "transfer-encoding: chunked\r\n"
                            "host: localhost:8080\r\n"
                            "\r\n");
                BOOST_REQUIRE(socket.next_layer().output_buffer == v);
            }

            BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
            BOOST_REQUIRE(socket.write_state()
                          == http::write_state::metadata_issued);

            socket.next_layer().output_buffer.clear();

            socket.async_write(request, yield);

            {
                vector<char> v;
                fill_vector(v,
                            "b\r\n"
                            "zzzzzzzzzzz\r\n");
                BOOST_REQUIRE(socket.next_layer().output_buffer == v);
            }

            BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
            BOOST_REQUIRE(socket.write_state()
                          == http::write_state::metadata_issued);

            socket.next_layer().output_buffer.clear();

            socket.async_write_end_of_message(yield);

            {
                vector<char> v;
                fill_vector(v,
                            "0\r\n"
                            "\r\n");
                BOOST_REQUIRE(socket.next_layer().output_buffer == v);
            }

            // ---

            BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
            BOOST_REQUIRE(socket.write_state() == http::write_state::empty);

            socket.next_layer().output_buffer.clear();

            socket.async_write_request_metadata(request, yield);

            {
                vector<char> v;
                fill_vector(v,
                            "GET / HTTP/1.1\r\n"
                            "transfer-encoding: chunked\r\n"
                            "host: localhost:8080\r\n"
                            "\r\n");
                BOOST_REQUIRE(socket.next_layer().output_buffer == v);
            }

            BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
            BOOST_REQUIRE(socket.write_state()
                          == http::write_state::metadata_issued);

            socket.next_layer().output_buffer.clear();

            socket.async_write(request, yield);

            {
                vector<char> v;
                fill_vector(v,
                            "b\r\n"
                            "zzzzzzzzzzz\r\n");
                BOOST_REQUIRE(socket.next_layer().output_buffer == v);
            }

            BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
            BOOST_REQUIRE(socket.write_state()
                          == http::write_state::metadata_issued);

            socket.next_layer().output_buffer.clear();

            request.trailers().emplace("x-ping", "true");

            socket.async_write_trailers(request, yield);

            {
                vector<char> v;
                fill_vector(v,
                            "0\r\n"
                            "x-ping: true\r\n"
                            "\r\n");
                BOOST_REQUIRE(socket.next_layer().output_buffer == v);
            }

            // ---

            BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
            BOOST_REQUIRE(socket.write_state() == http::write_state::empty);

            socket.next_layer().input_buffer.emplace_back();
            fill_vector(socket.next_layer().input_buffer.front(),
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Length: 0\r\n"
                        "\r\n");

            socket.async_read_response(response, yield);

            BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
            BOOST_REQUIRE(socket.write_state() == http::write_state::empty);

            BOOST_REQUIRE(response.status_code() == 200);
            BOOST_REQUIRE(response.reason_phrase() == "OK");

            // ---

            reached_the_end_of_the_test = true;
        });
    };

    spawn(ios, work);
    ios.run();
    BOOST_REQUIRE(reached_the_end_of_the_test);
}

BOOST_AUTO_TEST_CASE(socket_http10_native_stream_unsupported) {
    // Tests:
    //
    // * `native_stream_unsupported`.
    // * `stream_finished` if a second request is tried within HTTP/1.0.
    asio::io_context ios;
    bool reached_the_end_of_the_test = false;
    auto work = [&ios,&reached_the_end_of_the_test](asio::yield_context yield) {
        feed_with_buffer(38, [&](asio::mutable_buffer inbuffer) {
            http::basic_socket<mock_socket> socket(inbuffer, ios);
            http::request request;
            http::response response;
            boost::system::error_code ec;

            request.method() = "GET";
            request.target() = "/";
            request.headers().emplace("host", "localhost:8080");

            BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
            BOOST_REQUIRE(socket.write_state() == http::write_state::empty);

            socket.lock_client_to_http10();
            socket.async_write_request_metadata(request, yield[ec]);

            BOOST_REQUIRE(make_error_code(http::http_errc
                                          ::native_stream_unsupported)
                          == ec);

            socket.async_write_request(request, yield);

            {
                vector<char> v;
                fill_vector(v,
                            "GET / HTTP/1.0\r\n"
                            "host: localhost:8080\r\n"
                            "\r\n");
                BOOST_REQUIRE(socket.next_layer().output_buffer == v);
            }

            // ---

            BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
            BOOST_REQUIRE(socket.write_state() == http::write_state::empty);

            socket.next_layer().input_buffer.emplace_back();
            fill_vector(socket.next_layer().input_buffer.front(),
                        "HTTP/1.1 200 OK\r\n"
                        "content-length: 0\r\n"
                        "\r\n");
            socket.next_layer().close_on_empty_input = true;

            socket.async_read_response(response, yield);

            BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
            BOOST_REQUIRE(socket.write_state() == http::write_state::empty);

            BOOST_REQUIRE(response.status_code() == 200);
            BOOST_REQUIRE(response.reason_phrase() == "OK");

            BOOST_REQUIRE(!socket.is_open());

            // ---

            reached_the_end_of_the_test = true;
        });
    };

    spawn(ios, work);
    ios.run();
    BOOST_REQUIRE(reached_the_end_of_the_test);
}

BOOST_AUTO_TEST_CASE(socket_http10_native_stream_unsupported2) {
    // Tests:
    //
    // * `native_stream_unsupported`.
    // * `stream_finished` if a second request is tried within HTTP/1.0.
    asio::io_context ios;
    bool reached_the_end_of_the_test = false;
    auto work = [&ios,&reached_the_end_of_the_test](asio::yield_context yield) {
        feed_with_buffer(38, [&](asio::mutable_buffer inbuffer) {
            http::basic_socket<mock_socket> socket(inbuffer, ios);
            http::request request;
            http::response response;
            boost::system::error_code ec;

            request.method() = "GET";
            request.target() = "/";
            request.headers().emplace("host", "localhost:8080");

            BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
            BOOST_REQUIRE(socket.write_state() == http::write_state::empty);

            socket.lock_client_to_http10();
            socket.async_write_request_metadata(request, yield[ec]);

            BOOST_REQUIRE(make_error_code(http::http_errc
                                          ::native_stream_unsupported)
                          == ec);

            socket.async_write_request(request, yield);

            {
                vector<char> v;
                fill_vector(v,
                            "GET / HTTP/1.0\r\n"
                            "host: localhost:8080\r\n"
                            "\r\n");
                BOOST_REQUIRE(socket.next_layer().output_buffer == v);
            }

            // ---

            BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
            BOOST_REQUIRE(socket.write_state() == http::write_state::empty);

            socket.next_layer().input_buffer.emplace_back();
            fill_vector(socket.next_layer().input_buffer.front(),
                        "HTTP/1.1 200 OK\r\n"
                        "\r\n");
            socket.next_layer().close_on_empty_input = true;

            socket.async_read_response(response, yield);
            BOOST_REQUIRE(socket.read_state()
                          == http::read_state::message_ready);
            socket.async_read_some(response, yield);

            BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
            BOOST_REQUIRE(socket.write_state() == http::write_state::empty);

            BOOST_REQUIRE(response.status_code() == 200);
            BOOST_REQUIRE(response.reason_phrase() == "OK");

            BOOST_REQUIRE(!socket.is_open());

            // ---

            reached_the_end_of_the_test = true;
        });
    };

    spawn(ios, work);
    ios.run();
    BOOST_REQUIRE(reached_the_end_of_the_test);
}

BOOST_AUTO_TEST_CASE(socket_http10_native_stream_unsupported3)
{
    // Tests:
    //
    // * `native_stream_unsupported`.
    // * `stream_finished` if a second request is tried within HTTP/1.0.
    asio::io_context ios;
    bool reached_the_end_of_the_test = false;
    auto work = [&ios,&reached_the_end_of_the_test](asio::yield_context yield) {
        feed_with_buffer(38, [&](asio::mutable_buffer inbuffer) {
            http::basic_socket<mock_socket> socket(inbuffer, ios);
            http::request request;
            http::response response;
            boost::system::error_code ec;

            request.method() = "GET";
            request.target() = "/";
            request.headers().emplace("host", "localhost:8080");

            BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
            BOOST_REQUIRE(socket.write_state() == http::write_state::empty);

            socket.async_write_request(request, yield);

            {
                vector<char> v;
                fill_vector(v,
                            "GET / HTTP/1.1\r\n"
                            "host: localhost:8080\r\n"
                            "\r\n");
                BOOST_REQUIRE(socket.next_layer().output_buffer == v);
            }

            // ---

            BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
            BOOST_REQUIRE(socket.write_state() == http::write_state::empty);

            socket.next_layer().input_buffer.emplace_back();
            fill_vector(socket.next_layer().input_buffer.front(),
                        "HTTP/1.0 200 OK\r\n"
                        "content-length: 0\r\n"
                        "\r\n");
            socket.next_layer().close_on_empty_input = true;

            socket.async_read_response(response, yield);

            BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
            BOOST_REQUIRE(socket.write_state() == http::write_state::empty);

            BOOST_REQUIRE(response.status_code() == 200);
            BOOST_REQUIRE(response.reason_phrase() == "OK");

            BOOST_REQUIRE(!socket.is_open());

            // ---

            reached_the_end_of_the_test = true;
        });
    };

    spawn(ios, work);
    ios.run();
    BOOST_REQUIRE(reached_the_end_of_the_test);
}

BOOST_AUTO_TEST_CASE(socket_upgrade_head) {
    asio::io_context ios;
    bool reached_the_end_of_the_test = false;
    auto work = [&ios,&reached_the_end_of_the_test](asio::yield_context yield) {
        feed_with_buffer(131, [&](asio::mutable_buffer inbuffer) {
            http::basic_socket<mock_socket> socket(inbuffer, ios);
            http::request request;
            http::response response;
            boost::system::error_code ec;

            request.method() = "GET";
            request.target() = "/chat";
            request.headers().emplace("host", "localhost:8080");
            request.headers().emplace("upgrade", "websocket");
            request.headers().emplace("connection", "upgrade");
            request.headers().emplace("sec-websocket-key",
                                      "x3JJHMbDL1EzLkh9GBhXDw==");
            request.headers().emplace("sec-websocket-version", "13");
            request.headers().emplace("origin", "http://localhost:8080/");

            BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
            BOOST_REQUIRE(socket.write_state() == http::write_state::empty);

            socket.async_write_request(request, yield);

            {
                vector<char> v;
                fill_vector(v,
                            "GET /chat HTTP/1.1\r\n"
                            "connection: upgrade\r\n"
                            "host: localhost:8080\r\n"
                            "origin: http://localhost:8080/\r\n"
                            "sec-websocket-key: x3JJHMbDL1EzLkh9GBhXDw==\r\n"
                            "sec-websocket-version: 13\r\n"
                            "upgrade: websocket\r\n"
                            "\r\n");
                BOOST_REQUIRE(socket.next_layer().output_buffer == v);
            }

            // ---

            BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
            BOOST_REQUIRE(socket.write_state() == http::write_state::empty);

            socket.next_layer().input_buffer.emplace_back();
            fill_vector(socket.next_layer().input_buffer.front(),
                        "HTTP/1.1 101 Switching Protocols\r\n"
                        "upgrade: websocket\r\n"
                        "connection: Upgrade\r\n"
                        "sec-websocket-accept: HSmrc0sMlYUkAGmm5OPpG2HaGWk=\r\n"
                        "\r\n"
                        "\x59\x2F");

            socket.async_read_response(response, yield);

            BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
            BOOST_REQUIRE(socket.write_state() == http::write_state::empty);

            BOOST_REQUIRE(response.status_code() == 101);
            BOOST_REQUIRE(response.reason_phrase() == "Switching Protocols");

            {
                auto x = socket.upgrade_head();
                BOOST_REQUIRE(x.size() == 2);
                BOOST_REQUIRE(static_cast<const char*>(x.data())[0] == '\x59');
                BOOST_REQUIRE(static_cast<const char*>(x.data())[1] == '\x2F');
            }

            // ---

            reached_the_end_of_the_test = true;
        });
    };

    spawn(ios, work);
    ios.run();
    BOOST_REQUIRE(reached_the_end_of_the_test);
}
