#include "unit_test.hpp"

#include <iostream>

#include <boost/asio/spawn.hpp>

#include <boost/http/socket.hpp>
#include <boost/http/algorithm.hpp>

#include "mocksocket.hpp"

using namespace boost;
using namespace std;

template<class F>
void feed_with_buffer(std::size_t min_buf_size, F &&f)
{
    char buffer[2048];
    if (min_buf_size == 0) {
        min_buf_size = max({
            string_ref("Host").size(),
            string_ref("Transfer-Encoding").size(),
            string_ref("Content-Length").size(),
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

template<class Message>
void clear_message(Message &m)
{
    m.headers().clear();
    m.body().clear();
    m.trailers().clear();
}

struct outer_storage
{
    typedef typename asio::handler_type<asio::yield_context,
                                        void(system::error_code)>::type
    coro_handler_t;
    typedef asio::async_result<coro_handler_t> coro_result_t;

    outer_storage(asio::yield_context &yield)
        : handler(yield)
        , result(handler)
    {}

    coro_handler_t handler;
    coro_result_t result;
};

struct use_yielded_future_t
{
    typedef typename asio::handler_type<asio::yield_context,
                                        void(system::error_code)>::type
    coro_handler_t;

    struct Handler
    {
        Handler(use_yielded_future_t use_yielded_future)
            : storage(use_yielded_future.storage)
            , coro_handler(std::move(use_yielded_future.storage.handler))
        {}

        void operator()(system::error_code ec)
        {
            coro_handler(ec);
        }

        outer_storage &storage;
        coro_handler_t coro_handler;
    };

    use_yielded_future_t(outer_storage &storage) : storage(storage) {}

    outer_storage &storage;
};

struct yielded_future
{
    yielded_future(outer_storage &storage) : storage(storage) {}

    void get()
    {
        storage.result.get();
    }

    outer_storage &storage;
};

namespace boost {
namespace asio {

template<>
struct handler_type<use_yielded_future_t, void(system::error_code)>
{
    typedef use_yielded_future_t::Handler type;
};

template<>
struct async_result<use_yielded_future_t::Handler>
{
    typedef yielded_future type;

    async_result(use_yielded_future_t::Handler &handler)
        : storage(handler.storage)
    {}

    yielded_future get()
    {
        return yielded_future(storage);
    }

    outer_storage &storage;
};

} } // namespace boost::asio

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
        feed_with_buffer(36, [&ios,&yield](asio::mutable_buffer inbuffer) {
                http::basic_socket<mock_socket> socket(ios, inbuffer);
                socket.next_layer().input_buffer.emplace_back();
                fill_vector(socket.next_layer().input_buffer.front(),
                            // first
                            "GET / HTTP/1.1\r\n"
                            "hOsT: \t \t  localhosT:8080  \t\r\n"
                            "X-men:  \t  the   beginning    \t   \r\n"
                            "x-Pants: On\r\n"
                            "x-code: meaw\r\n"
                            "X-ENEMY: y \t y,y\t\r\n"
                            "\r\n"
                            // second
                            "POST /file_upload HTTP/1.1\r\n"
                            "Content-length: 4\r\n"
                            "host: canyoushowmewhereithurts.org\r\n"
                            "\r\n"
                            "ping"
                            // third
                            "GET /whats_up HTTP/1.0\r\n"
                            "\r\n");

                std::string method;
                std::string path;
                http::message message;

                BOOST_REQUIRE(method.size() == 0);
                BOOST_REQUIRE(path.size() == 0);

                BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
                BOOST_REQUIRE(socket.write_state() == http::write_state::empty);
                {
                    outer_storage storage(yield);
                    auto fut = socket
                        .async_read_request(method, path, message,
                                            use_yielded_future_t(storage));
                    BOOST_REQUIRE(socket.write_state()
                                  == http::write_state::finished);
                    fut.get();
                }
                BOOST_REQUIRE(socket.write_state() == http::write_state::empty);

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
                BOOST_CHECK(message.trailers() == http::headers{});

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

                {
                    bool cond = (socket.read_state()
                                 == http::read_state::message_ready)
                        || (socket.read_state() == http::read_state::empty);
                    BOOST_REQUIRE(cond);
                }
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
                BOOST_CHECK(message.trailers() == http::headers{});
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
                BOOST_CHECK(message.trailers() == http::headers{});
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

                socket.async_write_response(200, string_ref("OK"), reply,
                                            yield);
                BOOST_REQUIRE(!socket.is_open());
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
        feed_with_buffer(40, [&ios,&yield](asio::mutable_buffer inbuffer) {
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
                BOOST_CHECK(message.trailers() == http::headers{});

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
                BOOST_CHECK(message.trailers() == http::headers{});
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

                {
                    bool cond = (socket.read_state()
                                 == http::read_state::message_ready)
                        || (socket.read_state() == http::read_state::empty);
                    BOOST_REQUIRE(cond);
                }
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
                BOOST_CHECK(message.trailers() == http::headers{});
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
                BOOST_CHECK(message.trailers() == http::headers{});
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

                socket.async_write_response(200, string_ref("OK"), reply,
                                            yield);
                BOOST_REQUIRE(!socket.is_open());
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

BOOST_AUTO_TEST_CASE(socket_chunked) {
    asio::io_service ios;
    auto work = [&ios](asio::yield_context yield) {
        feed_with_buffer(87, [&ios,&yield](asio::mutable_buffer inbuffer) {
                http::basic_socket<mock_socket> socket(ios, inbuffer);
                socket.next_layer().input_buffer.emplace_back();
                fill_vector(socket.next_layer().input_buffer.front(),
                            // first request
                            "POST /1 HTTP/1.1\r\n"
                            "Host: example.com\r\n"
                            "Transfer-Encoding: chunked\r\n"
                            "TE: trailers\r\n"
                            "Trailer: Content-MD5\r\n"
                            "\r\n"

                            "4\r\n"
                            "Wiki\r\n"

                            "5\r\n"
                            "pedia\r\n"

                            "e\r\n"
                            " in\r\n\r\nchunks.\r\n"

                            "0\r\n"
                            "Content-MD5: \t  \t\t   25b83662323c397c9944a8a7b3"
                            "fef7ab    \t\t\t\t   \t \r\n"
                            "\r\n"
                            // second request
                            "POST /2 HTTP/1.1\r\n"
                            "Expect: 100-continue\r\n"
                            "Host: example.com\r\n"
                            "Transfer-Encoding: chunked\r\n"
                            "TE: trailers\r\n"
                            "\r\n"

                            "4\r\n"
                            "Wiki\r\n"

                            "5\r\n"
                            "pedia\r\n"

                            "e\r\n"
                            " in\r\n\r\nchunks.\r\n"

                            "0\r\n"
                            "\r\n"
                            // third request
                            "POST /3 HTTP/1.1\r\n"
                            "Host: example.com\r\n"
                            "Transfer-Encoding: chunked\r\n"
                            "TE: trailers\r\n"
                            "Trailer: Content-MD5, X-Content-SHA1\r\n"
                            "\r\n"

                            "10\r\n"
                            "violets are blue\r\n"

                            "f\r\n"
                            ", roses are red\r\n"

                            "0\r\n"
                            "Content-MD5: \t\t  \t 27577ea5cf683e1b73bd3684998c"
                            "6510\t\t  \t \t  \r\n"
                            "X-Content-SHA1:\t   \t\t\t\t   \t  ac507bbd7ec6d86"
                            "8f2840dd8c6e283e2c96a5900     \t\t\t  \t    \t\r\n"
                            "\r\n");

                // ### First request
                std::string method;
                std::string path;
                http::message message;

                BOOST_REQUIRE(method.size() == 0);
                BOOST_REQUIRE(path.size() == 0);

                BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
                socket.async_read_request(method, path, message, yield);

                {
                    bool cond = (socket.read_state()
                                 == http::read_state::message_ready)
                        || (socket.read_state() == http::read_state::body_ready)
                        || (socket.read_state() == http::read_state::empty);
                    BOOST_REQUIRE(cond);
                }
                BOOST_CHECK(socket.write_response_native_stream());
                BOOST_CHECK(!http::request_continue_required(message));
                BOOST_CHECK(method == "POST");
                BOOST_CHECK(path == "/1");
                {
                    http::headers expected_headers{
                        {"host", "example.com"},
                        {"transfer-encoding", "chunked"},
                        {"te", "trailers"},
                        {"trailer", "Content-MD5"}
                    };
                    BOOST_CHECK(message.headers() == expected_headers);
                }

                while (socket.read_state() != http::read_state::empty) {
                    switch (socket.read_state()) {
                    case http::read_state::empty:
                    case http::read_state::finished:
                        BOOST_FAIL("this state should be unreachable");
                        break;
                    case http::read_state::message_ready:
                        BOOST_REQUIRE(message.trailers().size() == 0);
                        socket.async_read_some(message, yield);
                        break;
                    case http::read_state::body_ready:
                        socket.async_read_trailers(message, yield);
                        break;
                    }
                }
                BOOST_REQUIRE(socket.read_state() == http::read_state::empty);

                {
                    vector<uint8_t> v{'W', 'i', 'k', 'i', 'p', 'e', 'd', 'i',
                            'a', ' ', 'i', 'n', '\r', '\n', '\r', '\n', 'c',
                            'h', 'u', 'n', 'k', 's', '.'};
                    BOOST_CHECK(message.body() == v);
                }

                {
                    http::headers expected_trailers{
                        {"content-md5", "25b83662323c397c9944a8a7b3fef7ab"}
                    };
                    BOOST_CHECK(message.trailers() == expected_trailers);
                }

                BOOST_CHECK(socket.write_state() == http::write_state::empty);
                http::message reply;
                {
                    const char body[] = "Hello World\n";
                    copy(body, body + sizeof(body) - 1,
                         back_inserter(reply.body()));
                }
                socket.async_write_response_metadata(200, string_ref("OK"),
                                                     reply, yield);
                BOOST_CHECK(socket.write_state()
                            == http::write_state::metadata_issued);
                socket.async_write(reply, yield);
                BOOST_CHECK(socket.write_state()
                            == http::write_state::metadata_issued);
                socket.async_write_end_of_message(yield);
                BOOST_CHECK(socket.write_state()
                            == http::write_state::finished);
                {
                    vector<char> v;
                    fill_vector(v,
                                "HTTP/1.1 200 OK\r\n"
                                "transfer-encoding: chunked\r\n"
                                "\r\n"
                                "c\r\n"
                                "Hello World\n\r\n"
                                "0\r\n"
                                "\r\n");
                    BOOST_CHECK(socket.next_layer().output_buffer == v);
                }

                // ### Second request (on the same connection)
                socket.next_layer().output_buffer.clear();
                clear_message(reply);

                BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
                socket.async_read_request(method, path, message, yield);

                {
                    bool cond = (socket.read_state()
                                 == http::read_state::message_ready)
                        || (socket.read_state() == http::read_state::body_ready)
                        || (socket.read_state() == http::read_state::empty);
                    BOOST_REQUIRE(cond);
                }
                BOOST_CHECK(socket.write_response_native_stream());

                BOOST_CHECK(http::request_continue_required(message));
                socket.async_write_response_continue(yield);

                BOOST_CHECK(method == "POST");
                BOOST_CHECK(path == "/2");

                {
                    http::headers expected_headers{
                        {"expect", "100-continue"},
                        {"host", "example.com"},
                        {"transfer-encoding", "chunked"},
                        {"te", "trailers"}
                    };
                    BOOST_CHECK(message.headers() == expected_headers);
                }

                while (socket.read_state() != http::read_state::empty) {
                    switch (socket.read_state()) {
                    case http::read_state::empty:
                    case http::read_state::finished:
                        BOOST_FAIL("this state should be unreachable");
                        break;
                    case http::read_state::message_ready:
                        BOOST_REQUIRE(message.trailers().size() == 0);
                        socket.async_read_some(message, yield);
                        break;
                    case http::read_state::body_ready:
                        socket.async_read_trailers(message, yield);
                        break;
                    }
                }
                BOOST_REQUIRE(socket.read_state() == http::read_state::empty);

                {
                    vector<uint8_t> v{'W', 'i', 'k', 'i', 'p', 'e', 'd', 'i',
                            'a', ' ', 'i', 'n', '\r', '\n', '\r', '\n', 'c',
                            'h', 'u', 'n', 'k', 's', '.'};
                    BOOST_CHECK(message.body() == v);
                }
                BOOST_CHECK(message.trailers() == http::headers{});
                BOOST_REQUIRE(socket.write_state()
                              == http::write_state::continue_issued);
                reply.headers().emplace("trailer", "Content-MD5");
                {
                    const char body[] = "karate do";
                    copy(body, body + sizeof(body) - 1,
                         back_inserter(reply.body()));
                }
                reply.trailers().emplace("content-md5",
                                         "fa7d2a3fba7a239ef30f825827e613ef");
                socket.async_write_response_metadata(200, string_ref("OK"),
                                                     reply, yield);
                BOOST_CHECK(socket.write_state()
                            == http::write_state::metadata_issued);
                socket.async_write(reply, yield);
                BOOST_CHECK(socket.write_state()
                            == http::write_state::metadata_issued);
                socket.async_write_trailers(reply, yield);
                BOOST_CHECK(socket.write_state()
                            == http::write_state::finished);
                {
                    vector<char> v;
                    fill_vector(v,
                                "HTTP/1.1 100 Continue\r\n"
                                "\r\n"
                                "HTTP/1.1 200 OK\r\n"
                                "trailer: Content-MD5\r\n"
                                "transfer-encoding: chunked\r\n"
                                "\r\n"
                                "9\r\n"
                                "karate do\r\n"
                                "0\r\n"
                                "content-md5:"
                                " fa7d2a3fba7a239ef30f825827e613ef\r\n"
                                "\r\n");
                    BOOST_CHECK(socket.next_layer().output_buffer == v);
                }

                // ### Last request (on the very same connection)
                socket.next_layer().output_buffer.clear();
                clear_message(reply);

                BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
                socket.async_read_request(method, path, message, yield);

                {
                    bool cond = (socket.read_state()
                                 == http::read_state::message_ready)
                        || (socket.read_state() == http::read_state::body_ready)
                        || (socket.read_state() == http::read_state::empty);
                    BOOST_REQUIRE(cond);
                }
                BOOST_CHECK(socket.write_response_native_stream());
                BOOST_CHECK(!http::request_continue_required(message));
                BOOST_CHECK(method == "POST");
                BOOST_CHECK(path == "/3");

                {
                    http::headers expected_headers{
                        {"host", "example.com"},
                        {"transfer-encoding", "chunked"},
                        {"te", "trailers"},
                        {"trailer", "Content-MD5, X-Content-SHA1"}
                    };
                    BOOST_CHECK(message.headers() == expected_headers);
                }

                while (socket.read_state() != http::read_state::empty) {
                    switch (socket.read_state()) {
                    case http::read_state::empty:
                    case http::read_state::finished:
                        BOOST_FAIL("this state should be unreachable");
                        break;
                    case http::read_state::message_ready:
                        BOOST_REQUIRE(message.trailers().size() == 0);
                        socket.async_read_some(message, yield);
                        break;
                    case http::read_state::body_ready:
                        socket.async_read_trailers(message, yield);
                        break;
                    }
                }
                BOOST_REQUIRE(socket.read_state() == http::read_state::empty);

                {
                    vector<uint8_t> v{'v', 'i', 'o', 'l', 'e', 't', 's', ' ',
                            'a', 'r', 'e', ' ', 'b', 'l', 'u', 'e', ',', ' ',
                            'r', 'o', 's', 'e', 's', ' ', 'a', 'r', 'e', ' ',
                            'r', 'e', 'd'};
                    BOOST_CHECK(message.body() == v);
                }

                {
                    http::headers expected_trailers{
                        {"content-md5", "27577ea5cf683e1b73bd3684998c6510"},
                        {"x-content-sha1",
                         "ac507bbd7ec6d868f2840dd8c6e283e2c96a5900"}
                    };
                    BOOST_CHECK(message.trailers() == expected_trailers);
                }

                BOOST_REQUIRE(socket.write_state() == http::write_state::empty);
                {
                    const char body[] = "nothing special";
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

BOOST_AUTO_TEST_CASE(socket_connection_close) {
    asio::io_service ios;
    auto work = [&ios](asio::yield_context yield) {
        feed_with_buffer(19, [&ios,&yield](asio::mutable_buffer inbuffer) {
                http::basic_socket<mock_socket> socket(ios, inbuffer);
                socket.next_layer().input_buffer.emplace_back();
                fill_vector(socket.next_layer().input_buffer.front(),
                            "GET /1 HTTP/1.1\r\n"
                            "Host: example.com\r\n"
                            "Connection: close\r\n"
                            "\r\n"
                            "GET /2 HTTP/1.1\r\n"
                            "Host: example.com\r\n"
                            "\r\n"
                            "GET /3 HTTP/1.0\r\n"
                            "\r\n");

                // First request
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
                BOOST_CHECK(path == "/1");
                {
                    http::headers expected_headers{
                        {"host", "example.com"},
                        {"connection", "close"}
                    };
                    BOOST_CHECK(message.headers() == expected_headers);
                }
                BOOST_CHECK(message.body() == vector<uint8_t>{});
                BOOST_CHECK(message.trailers() == http::headers{});

                BOOST_CHECK(socket.write_state() == http::write_state::empty);
                http::message reply;
                {
                    const char body[] = "Hello World\n";
                    copy(body, body + sizeof(body) - 1,
                         back_inserter(reply.body()));
                }

                socket.async_write_response(200, string_ref("OK"), reply,
                                            yield);
                BOOST_REQUIRE(!socket.is_open());
                socket.open();
                BOOST_REQUIRE(socket.is_open());

                BOOST_CHECK(socket.write_state()
                            == http::write_state::finished);
                {
                    vector<char> v;
                    fill_vector(v,
                                "HTTP/1.1 200 OK\r\n"
                                "connection: close\r\n"
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

                BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
                BOOST_CHECK(socket.write_response_native_stream());
                BOOST_CHECK(!http::request_continue_required(message));
                BOOST_CHECK(method == "GET");
                BOOST_CHECK(path == "/2");
                {
                    http::headers expected_headers{
                        {"host", "example.com"}
                    };
                    BOOST_CHECK(message.headers() == expected_headers);
                }

                BOOST_CHECK(message.body() == vector<uint8_t>{});
                BOOST_CHECK(message.trailers() == http::headers{});
                BOOST_REQUIRE(socket.write_state() == http::write_state::empty);
                {
                    const char body[] = "Hello World";
                    copy(body, body + sizeof(body) - 1,
                         back_inserter(reply.body()));
                }

                reply.headers().emplace("connection", "close");

                socket.async_write_response(200, string_ref("OK"), reply,
                                            yield);
                BOOST_REQUIRE(!socket.is_open());
                socket.open();
                BOOST_REQUIRE(socket.is_open());

                BOOST_CHECK(socket.write_state()
                            == http::write_state::finished);
                {
                    vector<char> v;
                    fill_vector(v,
                                "HTTP/1.1 200 OK\r\n"
                                "connection: close\r\n"
                                "content-length: 11\r\n"
                                "\r\n"
                                "Hello World");
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
                BOOST_CHECK(path == "/3");
                BOOST_CHECK(message.headers() == http::headers{});
                BOOST_CHECK(message.body() == vector<uint8_t>{});
                BOOST_CHECK(message.trailers() == http::headers{});
                BOOST_REQUIRE(socket.write_state() == http::write_state::empty);
                {
                    const char body[] = "nothing special";
                    copy(body, body + sizeof(body) - 1,
                         back_inserter(reply.body()));
                }

                socket.async_write_response(200, string_ref("OK"), reply,
                                            yield);
                BOOST_REQUIRE(!socket.is_open());

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

BOOST_AUTO_TEST_CASE(socket_upgrade) {
    asio::io_service ios;
    auto work = [&ios](asio::yield_context yield) {
        feed_with_buffer(26, [&ios,&yield](asio::mutable_buffer inbuffer) {
                http::basic_socket<mock_socket> socket(ios, inbuffer);
                socket.next_layer().input_buffer.emplace_back();
                fill_vector(socket.next_layer().input_buffer.front(),
                            "GET /1 HTTP/1.1\r\n"
                            "Host: example.com\r\n"
                            "Connection: upgrade\r\n"
                            "Upgrade: vinipsmaker/0.1\r\n"
                            "\r\n");
                socket.next_layer().input_buffer.emplace_back();
                fill_vector(socket.next_layer().input_buffer.back(),
                            "OPTIONS /2 HTTP/1.1\r\n"
                            "host: example.com\r\n"
                            "expect: 100-continue\r\n"
                            "connection: upgrade\r\n"
                            "upgrade: vinipsmaker/0.1\r\n"
                            "\r\n");
                socket.next_layer().input_buffer.emplace_back();
                fill_vector(socket.next_layer().input_buffer.back(),
                            "GET /3 HTTP/1.0\r\n"
                            "connection: upgrade\r\n"
                            "upgrade: vinipsmaker/0.1\r\n"
                            "\r\n");

                // First request
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
                BOOST_CHECK(http::request_upgrade_desired(message));
                BOOST_CHECK(method == "GET");
                BOOST_CHECK(path == "/1");
                {
                    http::headers expected_headers{
                        {"host", "example.com"},
                        {"connection", "upgrade"},
                        {"upgrade", "vinipsmaker/0.1"}
                    };
                    BOOST_CHECK(message.headers() == expected_headers);
                }
                BOOST_CHECK(message.body() == vector<uint8_t>{});
                BOOST_CHECK(message.trailers() == http::headers{});

                BOOST_CHECK(socket.write_state() == http::write_state::empty);
                http::message reply;
                {
                    const char body[] = "Sing now!\n";
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
                                "content-length: 10\r\n"
                                "\r\n"
                                "Sing now!\n");
                    BOOST_CHECK(socket.next_layer().output_buffer == v);
                }

                // ### Second request (on the same connection)
                socket.next_layer().output_buffer.clear();
                clear_message(reply);

                BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
                socket.async_read_request(method, path, message, yield);

                BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
                BOOST_CHECK(socket.write_response_native_stream());

                BOOST_CHECK(http::request_continue_required(message));
                socket.async_write_response_continue(yield);

                BOOST_CHECK(http::request_upgrade_desired(message));
                BOOST_CHECK(method == "OPTIONS");
                BOOST_CHECK(path == "/2");
                {
                    http::headers expected_headers{
                        {"host", "example.com"},
                        {"expect", "100-continue"},
                        {"connection", "upgrade"},
                        {"upgrade", "vinipsmaker/0.1"}
                    };
                    BOOST_CHECK(message.headers() == expected_headers);
                }

                BOOST_CHECK(message.body() == vector<uint8_t>{});
                BOOST_CHECK(message.trailers() == http::headers{});
                BOOST_REQUIRE(socket.write_state()
                              == http::write_state::continue_issued);
                {
                    const char body[] = "I'm scatman!";
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
                                "I'm scatman!");
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
                BOOST_CHECK(!http::request_upgrade_desired(message));
                BOOST_CHECK(method == "GET");
                BOOST_CHECK(path == "/3");

                {
                    http::headers expected_headers{
                        {"connection", "upgrade"}
                    };
                    BOOST_CHECK(message.headers() == expected_headers);
                }

                BOOST_CHECK(message.body() == vector<uint8_t>{});
                BOOST_CHECK(message.trailers() == http::headers{});
                BOOST_REQUIRE(socket.write_state() == http::write_state::empty);
                {
                    const char body[] = "nothing special";
                    copy(body, body + sizeof(body) - 1,
                         back_inserter(reply.body()));
                }

                socket.async_write_response(200, string_ref("OK"), reply,
                                            yield);
                BOOST_REQUIRE(!socket.is_open());
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

    auto work2 = [&ios](asio::yield_context yield) {
        feed_with_buffer(25, [&ios,&yield](asio::mutable_buffer inbuffer) {
                http::basic_socket<mock_socket> socket(ios, inbuffer);
                socket.next_layer().input_buffer.emplace_back();
                fill_vector(socket.next_layer().input_buffer.front(),
                            "POST /pink%20floyd/the%20wall HTTP/1.1\r\n"
                            "Host: hyrule.org\r\n"
                            "Connection: upgrade\r\n"
                            "Content-length: 4\r\n"
                            "Upgrade: h2c\r\n"
                            "\r\n"
                            "!git");

                std::string method;
                std::string path;
                http::message message;

                BOOST_REQUIRE(method.size() == 0);
                BOOST_REQUIRE(path.size() == 0);

                BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
                socket.async_read_request(method, path, message, yield);

                {
                    bool cond = (socket.read_state()
                                 == http::read_state::message_ready)
                        || (socket.read_state() == http::read_state::empty);
                    BOOST_REQUIRE(cond);
                }
                BOOST_CHECK(socket.write_response_native_stream());
                BOOST_CHECK(!http::request_continue_required(message));
                BOOST_CHECK(http::request_upgrade_desired(message));
                BOOST_CHECK(method == "POST");
                BOOST_CHECK(path == "/pink%20floyd/the%20wall");
                {
                    http::headers expected_headers{
                        {"host", "hyrule.org"},
                        {"connection", "upgrade"},
                        {"content-length", "4"},
                        {"upgrade", "h2c"}
                    };
                    BOOST_CHECK(message.headers() == expected_headers);
                }

                while (socket.read_state() != http::read_state::empty)
                    socket.async_read_some(message, yield);
                BOOST_REQUIRE(socket.read_state() == http::read_state::empty);

                {
                    vector<uint8_t> v{'!', 'g', 'i', 't'};
                    BOOST_CHECK(message.body() == v);
                }
                BOOST_CHECK(message.trailers() == http::headers{});

                BOOST_CHECK(socket.write_state() == http::write_state::empty);
                http::message reply;
                {
                    const char body[] = "Sing now!\n";
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
                                "content-length: 10\r\n"
                                "\r\n"
                                "Sing now!\n");
                    BOOST_CHECK(socket.next_layer().output_buffer == v);
                }
            });
    };

    spawn(ios, work2);
    ios.run();
}
