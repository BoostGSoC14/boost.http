#include <boost/asio.hpp>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <boost/asio/spawn.hpp>

#include <boost/http/socket.hpp>
#include <boost/http/algorithm.hpp>
#include <boost/http/request.hpp>
#include <boost/http/response.hpp>

#include "mocksocket.hpp"

using namespace boost;
using namespace std;

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

template<class Message>
void clear_message(Message &m)
{
    m.headers().clear();
    m.body().clear();
    m.trailers().clear();
}

struct outer_storage
{
    outer_storage(asio::yield_context &yield)
        : init(yield)
    {}

    asio::async_completion<asio::yield_context, void(system::error_code)> init;
};

struct use_yielded_future_t
{
    typedef BOOST_ASIO_HANDLER_TYPE(asio::yield_context,
                                    void(system::error_code))
        coro_handler_t;

    struct Handler
    {
        Handler(use_yielded_future_t use_yielded_future)
            : storage(use_yielded_future.storage)
            , coro_handler(std::move(use_yielded_future.storage.init
                                     .completion_handler))
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
        storage.init.result.get();
    }

    outer_storage &storage;
};

namespace boost {
namespace asio {

template<>
struct async_result<use_yielded_future_t, void(system::error_code)>
{
    typedef use_yielded_future_t::Handler completion_handler_type;
    typedef yielded_future return_type;

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

bool check_out_of_order(const system::system_error &e)
{
    return system::error_code(http::http_errc::out_of_order) == e.code();
}

BOOST_AUTO_TEST_CASE(double_read_request_call) {
    asio::io_context ios;
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
                            "x-moRe:\r\n"
                            "x-mOre2:  \r\n"
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

                http::request request;

                BOOST_REQUIRE(request.method().size() == 0);
                BOOST_REQUIRE(request.target().size() == 0);

                BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
                BOOST_REQUIRE(socket.write_state() == http::write_state::empty);
                {
                    outer_storage storage(yield);
                    auto fut = socket
                        .async_read_request(request,
                                            use_yielded_future_t(storage));
                    BOOST_REQUIRE(socket.write_state()
                                  == http::write_state::finished);
                    fut.get();
                }
                BOOST_REQUIRE(socket.write_state() == http::write_state::empty);

                BOOST_REQUIRE(socket.read_state()
                              == http::read_state::finished);
                BOOST_CHECK_EXCEPTION(socket.async_read_request(request, yield),
                                      system::system_error, check_out_of_order);
                BOOST_CHECK(socket.write_response_native_stream());
                BOOST_CHECK(!http::request_continue_required(request));
                BOOST_CHECK(request.method() == "GET");
                BOOST_CHECK(request.target() == "/");
                {
                    http::headers expected_headers{
                        {"host", "localhosT:8080"},
                        {"x-men", "the   beginning"},
                        {"x-pants", "On"},
                        {"x-code", "meaw"},
                        {"x-enemy", "y \t y,y"},
                        {"x-more", ""},
                        {"x-more2", ""}
                    };
                    BOOST_CHECK(request.headers() == expected_headers);
                }
                BOOST_CHECK(request.body() == vector<uint8_t>{});
                BOOST_CHECK(request.trailers() == http::headers{});

                BOOST_CHECK(socket.write_state() == http::write_state::empty);
                http::response reply;
                reply.status_code() = 200;
                reply.reason_phrase() = "OK";
                {
                    const char body[] = "Hello World\n";
                    copy(body, body + sizeof(body) - 1,
                         back_inserter(reply.body()));
                }
                socket.async_write_response(reply, yield);
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
                socket.async_read_request(request, yield);

                BOOST_REQUIRE((socket.read_state()
                               == http::read_state::message_ready)
                              || (socket.read_state()
                                  == http::read_state::finished));
                BOOST_CHECK(socket.write_response_native_stream());
                BOOST_CHECK(!http::request_continue_required(request));
                BOOST_CHECK(request.method() == "POST");
                BOOST_CHECK(request.target() == "/file_upload");
                {
                    http::headers expected_headers{
                        {"content-length", "4"},
                        {"host", "canyoushowmewhereithurts.org"}
                    };
                    BOOST_CHECK(request.headers() == expected_headers);
                }

                while (socket.read_state() != http::read_state::finished)
                    socket.async_read_some(request, yield);
                BOOST_REQUIRE(socket.read_state()
                              == http::read_state::finished);
                BOOST_CHECK_EXCEPTION(socket.async_read_request(request, yield),
                                      system::system_error, check_out_of_order);

                {
                    vector<uint8_t> v{'p', 'i', 'n', 'g'};
                    BOOST_CHECK(request.body() == v);
                }
                BOOST_CHECK(request.trailers() == http::headers{});
                BOOST_REQUIRE(socket.write_state() == http::write_state::empty);
                reply.status_code() = 200;
                reply.reason_phrase() = "OK";
                {
                    const char body[] = "pong";
                    copy(body, body + sizeof(body) - 1,
                         back_inserter(reply.body()));
                }
                socket.async_write_response(reply, yield);
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
                socket.async_read_request(request, yield);

                BOOST_REQUIRE(socket.read_state()
                              == http::read_state::finished);
                BOOST_CHECK(!socket.write_response_native_stream());
                BOOST_CHECK(!http::request_continue_required(request));
                BOOST_CHECK(request.method() == "GET");
                BOOST_CHECK(request.target() == "/whats_up");
                BOOST_CHECK(request.headers() == http::headers{});
                BOOST_CHECK(request.body() == vector<uint8_t>{});
                BOOST_CHECK(request.trailers() == http::headers{});
                BOOST_REQUIRE(socket.write_state() == http::write_state::empty);
                reply.status_code() = 200;
                reply.reason_phrase() = "OK";
                {
                    const char body[] = "nothing special";
                    copy(body, body + sizeof(body) - 1,
                         back_inserter(reply.body()));
                }

                bool captured = false;
                try {
                    socket.async_write_response_metadata(reply, yield);
                } catch(system::system_error &e) {
                    BOOST_REQUIRE(e.code()
                                  == system::error_code{http::http_errc
                                          ::native_stream_unsupported});
                    captured = true;
                }
                BOOST_REQUIRE(captured);

                socket.async_write_response(reply, yield);
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
                BOOST_REQUIRE(socket.read_state() == http::read_state::empty);
            });
    };

    spawn(ios, work);
    ios.run();
}
