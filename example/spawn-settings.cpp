// This example shows architectural ideas that are popular elsewhere:
//
// * https://www.ncameron.org/blog/macros-and-syntax-extensions-and-compiler-plugins-where-are-we-at/
// * https://gstreamer.freedesktop.org/documentation/application-development/introduction/basics.html#bins-and-pipelines
//
// Some ideas on how you could use them:
//
// * Transform some tokens into `error_invalid_data` to reject a few requests.
// ** Maybe `field_name` or `field_value` are too large.
// ** Maybe you got suspicious about the request.
// * Inject new headers into every request.
// * Transform some header values to lower case.
//
// You can customize all at once or chain each customization point. If you're
// building your own message framework and only wants the parser, there are even
// more wrappers that could be useful for you (e.g. ensuring header name and
// header value are always tied together or `error_insufficient_data` is
// returned).

#include <iostream>
#include <algorithm>

#include <boost/utility/string_view.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/http/buffered_socket.hpp>
#include <boost/http/algorithm.hpp>
#include <boost/http/request.hpp>
#include <boost/http/response.hpp>

using namespace std;
using namespace boost;

template<class T>
struct reader_discards_useless_headers: private T
{
    using typename T::size_type;
    using typename T::value_type;
    using typename T::pointer;
    using typename T::view_type;

    using T::reset;
    using T::token_size;
    using T::value;
    using T::expected_token;
    using T::set_buffer;
    using T::parsed_count;

    http::token::code::value code() const
    {
        const auto code_ = T::code();

        switch (code_) {
        case http::token::code::field_value:
            if (skip_next)
                return http::token::code::skip;
            else
                return code_;
        case http::token::code::field_name:
            {
                auto x = T::template value<http::token::field_name>();
                if (!algorithm::iequals(x, "Cookie")
                    && !algorithm::iequals(x, "X-Pants")) {
                    skip_next = true;
                    return http::token::code::skip;
                }
            }
        default:
            return code_;
        }
    }

    void next()
    {
        if (T::code() == http::token::code::field_value)
            skip_next = false;

        T::next();
    }

private:
    mutable bool skip_next = false;
};

template<class T>
struct reader_discards_trailers: private T
{
    using typename T::size_type;
    using typename T::value_type;
    using typename T::pointer;
    using typename T::view_type;

    using T::reset;
    using T::token_size;
    using T::value;
    using T::expected_token;
    using T::next;
    using T::set_buffer;
    using T::parsed_count;

    http::token::code::value code() const
    {
        const auto code_ = T::code();

        switch (code_) {
        case http::token::code::trailer_name:
        case http::token::code::trailer_value:
            return http::token::code::skip;
        default:
            return code_;
        }
    }
};

struct socket_settings
{
    typedef reader_discards_trailers<
        reader_discards_useless_headers<http::reader::request>
        > req_parser;
    typedef http::reader::response res_parser;
};

class connection: public std::enable_shared_from_this<connection>
{
public:
    void operator()(asio::yield_context yield)
    {
        auto self = shared_from_this();
        try {
            while (self->socket.is_open()) {
                cout << "--\n[" << self->counter
                     << "] About to receive a new message" << endl;
                self->socket.async_read_request(self->request, yield);
                //self->request.body().clear(); // freeing not used resources

                if (http::request_continue_required(self->request)) {
                    cout << '[' << self->counter
                         << "] Continue required. About to send"
                        " \"100-continue\""
                         << std::endl;
                    self->socket.async_write_response_continue(yield);
                }

                while (self->socket.read_state()
                       != http::read_state::finished) {
                    cout << '[' << self->counter
                         << "] Message not fully received" << endl;
                    switch (self->socket.read_state()) {
                    case http::read_state::message_ready:
                        cout << '[' << self->counter
                             << "] About to receive some body" << endl;
                        self->socket.async_read_some(self->request, yield);
                        break;
                    case http::read_state::body_ready:
                        cout << '[' << self->counter
                             << "] About to receive trailers" << endl;
                        self->socket.async_read_trailers(self->request, yield);
                        break;
                    default:;
                    }
                }

                //cout << "BODY:==";
                //for (const auto &e: self->request.body()) {
                //    cout << char(e);
                //}
                //cout << "==" << endl;

                cout << '[' << self->counter << "] Message received. State = "
                     << int(self->socket.read_state()) << endl;
                cout << '[' << self->counter << "] Method: "
                     << self->request.method() << endl;
                cout << '[' << self->counter << "] Path: "
                     << self->request.target() << endl;
                {
                    auto host = self->request.headers().find("host");
                    if (host != self->request.headers().end())
                        cout << '[' << self->counter << "] Host header: "
                             << host->second << endl;
                }

                std::cout << '[' << self->counter << "] Write state = "
                          << int(self->socket.write_state()) << std::endl;

                cout << '[' << self->counter << "] About to send a reply"
                     << endl;

                http::response reply;
                reply.status_code() = 200;
                reply.reason_phrase() = "OK";
                //reply.headers().emplace("connection", "close");
                const char body[] = "Hello World\n";
                std::copy(body, body + sizeof(body) - 1,
                          std::back_inserter(reply.body()));

                self->socket.async_write_response(reply, yield);
            }
        } catch (system::system_error &e) {
            if (e.code() != system::error_code{asio::error::connection_reset}
                && e.code() != system::error_code{asio::error::broken_pipe}
                && e.code() != system::error_code{asio::error::eof}) {
                cerr << '[' << self->counter << "] Aborting on exception: "
                     << e.what() << endl;
                std::exit(1);
            }

            cout << '[' << self->counter << "] Error: " << e.what() << endl;
        } catch (std::exception &e) {
            cerr << '[' << self->counter << "] Aborting on exception: "
                 << e.what() << endl;
            std::exit(1);
        }
    }

    asio::ip::tcp::socket &tcp_layer()
    {
        return socket.next_layer();
    }

    static std::shared_ptr<connection> make_connection(asio::io_context &ios,
                                                       int counter)
    {
        return std::shared_ptr<connection>{new connection{ios, counter}};
    }

private:
    connection(asio::io_context &ios, int counter)
        : socket(ios)
        , counter(counter)
    {}

    http::basic_buffered_socket<asio::ip::tcp::socket, socket_settings> socket;
    int counter;

    http::request request;
};

int main()
{
    asio::io_context ios;
    asio::ip::tcp::acceptor acceptor(ios,
                                     asio::ip::tcp
                                     ::endpoint(asio::ip::tcp::v6(), 8080));

    auto work = [&acceptor,&ios](asio::yield_context yield) {
        int counter = 0;
        for ( ; true ; ++counter ) {
            try {
                auto connection
                    = connection::make_connection(ios, counter);
                cout << "About to accept a new connection" << endl;
                acceptor.async_accept(connection->tcp_layer(), yield);

                auto handle_connection
                    = [connection](asio::yield_context yield) mutable {
                    (*connection)(yield);
                };
                spawn(acceptor.get_executor(), handle_connection);
            } catch (std::exception &e) {
                cerr << "Aborting on exception: " << e.what() << endl;
                std::exit(1);
            }
        }
    };

    cout << "About to spawn" << endl;
    spawn(ios, work);

    cout << "About to run" << endl;
    ios.run();

    return 0;
}
