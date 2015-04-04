#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <iostream>

#include <boost/asio/spawn.hpp>

#include <boost/http/socket.hpp>
#include <boost/http/algorithm.hpp>

#include "mocksocket.hpp"

using namespace boost;
using namespace std;

template<class F>
void feed_with_buffer(F &&f)
{
    char buffer[2048];
    for (int i = 1;i != 2048;++i) {
        cout << "buffer_size = " << i << endl;
        f(asio::buffer(buffer, /*i*/3));
    }
}

template<unsigned N>
void fill_vector(vector<char> &v, const char (&s)[N])
{
    v.insert(v.end(), s, s + N - 1);
}

template<class Message>
void clear_message(Message &m)
{
    m.headers().clear();
    m.body().clear();
    m.trailers().clear();
}

BOOST_AUTO_TEST_CASE(socket_ctor) {
    bool captured = false;
    try {
        asio::io_service ios;
        http::socket s(ios, asio::mutable_buffer{});
    } catch(invalid_argument &) {
        captured = true;
    }
    BOOST_REQUIRE(captured);
}

BOOST_AUTO_TEST_CASE(socket_simple) {
    asio::io_service ios;
    auto work = [&ios](asio::yield_context yield) {
        feed_with_buffer([&ios,&yield](asio::mutable_buffer inbuffer) {
                http::basic_socket<mock_socket> socket(ios, inbuffer);
                socket.next_layer().input_buffer.emplace_back();
                fill_vector(socket.next_layer().input_buffer.front(),
                            "GET / HTTP/1.1\r\n"
                            "hOsT: \t \t  localhosT:8080  \t\r\n"
                            "X-men:  \t  the   beginning    \t   \r\n"
                            "x-Pants: On\r\n"
                            "x-code: meaw\r\n"
                            "X-ENEMY: y \t y,y\t\r\n"
                            "\r\n"
                            "POST /file_upload HTTP/1.1\r\n"
                            "Content-length: 4\r\n"
                            "host: canyoushowmewhereithurts.org\r\n"
                            "\r\n"
                            "ping"
                            "GET /whats_up HTTP/1.0\r\n"
                            "\r\n");

                std::string method;
                std::string path;
                http::message message;

                BOOST_REQUIRE(method.size() == 0);
                BOOST_REQUIRE(path.size() == 0);

                BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
                socket.async_read_request(method, path, message, yield);

                BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
                BOOST_CHECK(socket.write_response_native_stream());
                BOOST_CHECK(!http::request_continue_required(message));
                BOOST_CHECK(method == "GET");
                BOOST_CHECK(path == "/");
                {
                    http::headers expected_headers{
                        {"host", "localhosT:8080"},
                        {"x-men", "the   beginning"},
                        {"x-pants", "On"},
                        {"x-code", "meaw"},
                        {"x-enemy", "y \t y,y"}
                    };
                    BOOST_CHECK(message.headers() == expected_headers);
                }
                BOOST_CHECK(message.body() == vector<uint8_t>{});

                BOOST_CHECK(socket.write_state() == http::write_state::empty);
                http::message reply;
                {
                    const char body[] = "Hello World\n";
                    copy(body, body + sizeof(body) - 1,
                         back_inserter(reply.body()));
                }
                socket.async_write_response(200, string_ref("OK"), reply,
                                            yield);
                BOOST_CHECK(socket.write_state()
                            == http::write_state::finished);
                {
                    vector<char> v;
                    fill_vector(v,
                                "HTTP/1.1 200 OK\r\n"
                                "content-length: 12\r\n"
                                "\r\n"
                                "Hello World\n");
                    BOOST_CHECK(socket.next_layer().output_buffer == v);
                }

                // ### Second request (on the same connection)
                socket.next_layer().output_buffer.clear();
                clear_message(reply);

                BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
                socket.async_read_request(method, path, message, yield);

                BOOST_REQUIRE((socket.read_state()
                               == http::read_state::message_ready)
                              || (socket.read_state()
                                  == http::read_state::empty));
                BOOST_CHECK(socket.write_response_native_stream());
                BOOST_CHECK(!http::request_continue_required(message));
                BOOST_CHECK(method == "POST");
                BOOST_CHECK(path == "/file_upload");
                {
                    http::headers expected_headers{
                        {"content-length", "4"},
                        {"host", "canyoushowmewhereithurts.org"}
                    };
                    BOOST_CHECK(message.headers() == expected_headers);
                }

                while (socket.read_state() != http::read_state::empty)
                    socket.async_read_some(message, yield);
                BOOST_REQUIRE(socket.read_state() == http::read_state::empty);

                {
                    vector<uint8_t> v{'p', 'i', 'n', 'g'};
                    BOOST_CHECK(message.body() == v);
                }
                BOOST_REQUIRE(socket.write_state() == http::write_state::empty);
                {
                    const char body[] = "pong";
                    copy(body, body + sizeof(body) - 1,
                         back_inserter(reply.body()));
                }
                socket.async_write_response(200, string_ref("OK"), reply,
                                            yield);
                BOOST_CHECK(socket.write_state()
                            == http::write_state::finished);
                {
                    vector<char> v;
                    fill_vector(v,
                                "HTTP/1.1 200 OK\r\n"
                                "content-length: 4\r\n"
                                "\r\n"
                                "pong");
                    BOOST_CHECK(socket.next_layer().output_buffer == v);
                }

                // ### Last request (on the very same connection)
                socket.next_layer().output_buffer.clear();
                clear_message(reply);

                BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
                socket.async_read_request(method, path, message, yield);

                BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
                BOOST_CHECK(!socket.write_response_native_stream());
                BOOST_CHECK(!http::request_continue_required(message));
                BOOST_CHECK(method == "GET");
                BOOST_CHECK(path == "/whats_up");
                BOOST_CHECK(message.headers() == http::headers{});
                BOOST_CHECK(message.body() == vector<uint8_t>{});
                BOOST_REQUIRE(socket.write_state() == http::write_state::empty);
                {
                    const char body[] = "nothing special";
                    copy(body, body + sizeof(body) - 1,
                         back_inserter(reply.body()));
                }

                bool captured = false;
                try {
                    socket.async_write_response_metadata(200, string_ref("OK"),
                                                         reply, yield);
                } catch(system::system_error &e) {
                    BOOST_REQUIRE(e.code()
                                  == system::error_code{http::http_errc
                                          ::native_stream_unsupported});
                    captured = true;
                }
                BOOST_REQUIRE(captured);

                captured = false;
                try {
                    socket.async_write_response(200, string_ref("OK"), reply,
                                                yield);
                } catch(system::system_error &e) {
                    BOOST_REQUIRE(e.code()
                                  == system::error_code{http::http_errc
                                                        ::stream_finished});
                    captured = true;
                }
                BOOST_REQUIRE(captured);
                BOOST_CHECK(socket.write_state()
                            == http::write_state::finished);
                {
                    vector<char> v;
                    fill_vector(v,
                                "HTTP/1.0 200 OK\r\n"
                                "connection: close\r\n"
                                "content-length: 15\r\n"
                                "\r\n"
                                "nothing special");
                    BOOST_CHECK(socket.next_layer().output_buffer == v);
                }
            });
    };

    spawn(ios, work);
    ios.run();
}

BOOST_AUTO_TEST_CASE(socket_expect_continue) {
    asio::io_service ios;
    auto work = [&ios](asio::yield_context yield) {
        feed_with_buffer([&ios,&yield](asio::mutable_buffer inbuffer) {
                http::basic_socket<mock_socket> socket(ios, inbuffer);
                socket.next_layer().input_buffer.emplace_back();
                fill_vector(socket.next_layer().input_buffer.front(),
                            // first request
                            "GET / HTTP/1.1\r\n"
                            "Expect: 100-continue\r\n"
                            "hOsT: \t \t  localhosT:8080  \t\r\n"
                            "X-men:  \t  the   beginning    \t   \r\n"
                            "x-Pants: On\r\n"
                            "x-code: meaw\r\n"
                            "X-ENEMY: y \t y,y\t\r\n"
                            "\r\n"
                            // second request
                            "GET / HTTP/1.1\r\n"
                            "Expect: all your base are belong to us\r\n"
                            "hOsT: github.com\r\n"
                            "Expect: 100-continue\r\n"
                            "eXPECT: 100-continue\r\n"
                            "Expect: hells' bells\r\n"
                            "\r\n"
                            // third request
                            "POST /file_upload HTTP/1.1\r\n"
                            "Content-length: 4\r\n"
                            "host: canyoushowmewhereithurts.org\r\n"
                            "Expect: 100-continue\r\n"
                            "\r\n"
                            // last request
                            "ping"
                            "GET /whats_up HTTP/1.0\r\n"
                            "Expect: 100-continue\r\n"
                            "\r\n");

                std::string method;
                std::string path;
                http::message message;

                BOOST_REQUIRE(method.size() == 0);
                BOOST_REQUIRE(path.size() == 0);

                BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
                socket.async_read_request(method, path, message, yield);

                BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
                BOOST_CHECK(socket.write_response_native_stream());

                BOOST_ASSERT(http::request_continue_required(message));
                socket.async_write_response_continue(yield);

                BOOST_CHECK(method == "GET");
                BOOST_CHECK(path == "/");
                {
                    http::headers expected_headers{
                        {"expect", "100-continue"},
                        {"host", "localhosT:8080"},
                        {"x-men", "the   beginning"},
                        {"x-pants", "On"},
                        {"x-code", "meaw"},
                        {"x-enemy", "y \t y,y"}
                    };
                    BOOST_CHECK(message.headers() == expected_headers);
                }
                BOOST_CHECK(message.body() == vector<uint8_t>{});

                BOOST_CHECK(socket.write_state()
                            == http::write_state::continue_issued);
                http::message reply;
                {
                    const char body[] = "Hello World\n";
                    copy(body, body + sizeof(body) - 1,
                         back_inserter(reply.body()));
                }
                socket.async_write_response(200, string_ref("OK"), reply,
                                            yield);
                BOOST_CHECK(socket.write_state()
                            == http::write_state::finished);
                {
                    vector<char> v;
                    fill_vector(v,
                                "HTTP/1.1 100 Continue\r\n"
                                "\r\n"
                                "HTTP/1.1 200 OK\r\n"
                                "content-length: 12\r\n"
                                "\r\n"
                                "Hello World\n");
                    BOOST_CHECK(socket.next_layer().output_buffer == v);
                }

                // ### Second request (on the same connection)
                socket.next_layer().output_buffer.clear();
                clear_message(reply);

                BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
                socket.async_read_request(method, path, message, yield);

                BOOST_ASSERT(socket.read_state() == http::read_state::empty);
                BOOST_CHECK(socket.write_response_native_stream());
                BOOST_CHECK(!http::request_continue_required(message));
                BOOST_CHECK(method == "GET");
                BOOST_CHECK(path == "/");
                {
                    http::headers expected_headers{
                        {"host", "github.com"}
                    };
                    BOOST_CHECK(message.headers() == expected_headers);
                }

                BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
                BOOST_CHECK(message.body().size() == 0);
                BOOST_REQUIRE(socket.write_state() == http::write_state::empty);
                {
                    const char body[] = "pong";
                    copy(body, body + sizeof(body) - 1,
                         back_inserter(reply.body()));
                }
                socket.async_write_response(200, string_ref("OK"), reply,
                                            yield);
                BOOST_CHECK(socket.write_state()
                            == http::write_state::finished);
                {
                    vector<char> v;
                    fill_vector(v,
                                "HTTP/1.1 200 OK\r\n"
                                "content-length: 4\r\n"
                                "\r\n"
                                "pong");
                    BOOST_CHECK(socket.next_layer().output_buffer == v);
                }

                // ### Third request (on the same connection)
                socket.next_layer().output_buffer.clear();
                clear_message(reply);

                BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
                socket.async_read_request(method, path, message, yield);

                BOOST_REQUIRE((socket.read_state()
                               == http::read_state::message_ready)
                              || (socket.read_state()
                                  == http::read_state::empty));
                BOOST_CHECK(socket.write_response_native_stream());

                BOOST_CHECK(http::request_continue_required(message));
                socket.async_write_response_continue(yield);

                BOOST_CHECK(method == "POST");
                BOOST_CHECK(path == "/file_upload");
                {
                    http::headers expected_headers{
                        {"content-length", "4"},
                        {"host", "canyoushowmewhereithurts.org"},
                        {"expect", "100-continue"}
                    };
                    BOOST_CHECK(message.headers() == expected_headers);
                }

                while (socket.read_state() != http::read_state::empty)
                    socket.async_read_some(message, yield);
                BOOST_REQUIRE(socket.read_state() == http::read_state::empty);

                {
                    vector<uint8_t> v{'p', 'i', 'n', 'g'};
                    BOOST_CHECK(message.body() == v);
                }
                BOOST_REQUIRE(socket.write_state()
                              == http::write_state::continue_issued);
                {
                    const char body[] = "pong";
                    copy(body, body + sizeof(body) - 1,
                         back_inserter(reply.body()));
                }
                socket.async_write_response(200, string_ref("OK"), reply,
                                            yield);
                BOOST_CHECK(socket.write_state()
                            == http::write_state::finished);
                {
                    vector<char> v;
                    fill_vector(v,
                                "HTTP/1.1 100 Continue\r\n"
                                "\r\n"
                                "HTTP/1.1 200 OK\r\n"
                                "content-length: 4\r\n"
                                "\r\n"
                                "pong");
                    BOOST_CHECK(socket.next_layer().output_buffer == v);
                }

                // ### Last request (on the very same connection)
                socket.next_layer().output_buffer.clear();
                clear_message(reply);

                BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
                socket.async_read_request(method, path, message, yield);

                BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
                BOOST_CHECK(!socket.write_response_native_stream());
                BOOST_CHECK(!http::request_continue_required(message));
                BOOST_CHECK(method == "GET");
                BOOST_CHECK(path == "/whats_up");
                BOOST_CHECK(message.headers() == http::headers{});
                BOOST_CHECK(message.body() == vector<uint8_t>{});
                BOOST_REQUIRE(socket.write_state() == http::write_state::empty);
                {
                    const char body[] = "nothing special";
                    copy(body, body + sizeof(body) - 1,
                         back_inserter(reply.body()));
                }

                bool captured = false;
                try {
                    socket.async_write_response_metadata(200, string_ref("OK"),
                                                         reply, yield);
                } catch(system::system_error &e) {
                    BOOST_REQUIRE(e.code()
                                  == system::error_code{http::http_errc
                                          ::native_stream_unsupported});
                    captured = true;
                }
                BOOST_REQUIRE(captured);

                captured = false;
                try {
                    socket.async_write_response(200, string_ref("OK"), reply,
                                                yield);
                } catch(system::system_error &e) {
                    BOOST_REQUIRE(e.code()
                                  == system::error_code{http::http_errc
                                                        ::stream_finished});
                    captured = true;
                }
                BOOST_REQUIRE(captured);
                BOOST_CHECK(socket.write_state()
                            == http::write_state::finished);
                {
                    vector<char> v;
                    fill_vector(v,
                                "HTTP/1.0 200 OK\r\n"
                                "connection: close\r\n"
                                "content-length: 15\r\n"
                                "\r\n"
                                "nothing special");
                    BOOST_CHECK(socket.next_layer().output_buffer == v);
                }
            });
    };

    spawn(ios, work);
    ios.run();
}
