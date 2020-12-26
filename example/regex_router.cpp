#include <iostream>
#include <algorithm>

#include <boost/http/regex_router.hpp>

// Boost.Coroutine 1.72 workaround {{{
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
// }}}

#include <boost/utility/string_view.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/http/buffered_socket.hpp>
#include <boost/http/algorithm.hpp>
#include <boost/http/request.hpp>
#include <boost/http/response.hpp>

using namespace std;
using namespace boost;

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
                         << "] Message not fully received" << endl
                         << '[' << self->counter
                         << "] About to receive some data" << endl;
                    self->socket.async_read_some(self->request, yield);
                }

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

                if (!router(self->request.target(), this, yield))
                {
                    http::response reply;
                    reply.status_code() = 500;
                    reply.reason_phrase() = "Internal Server Error";
                    const char body[] = "500 Internal Server Error \n";
                    std::copy(body, body + sizeof(body) - 1,
                                std::back_inserter(reply.body()));

                    self->socket.async_write_response(reply, yield);
                }

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

    void index_route(asio::yield_context yield)
    {
        auto self = shared_from_this();

        http::response reply;
        reply.status_code() = 200;
        reply.reason_phrase() = "OK";
        const char body[] = "Hello World\n";
        std::copy(body, body + sizeof(body) - 1,
                    std::back_inserter(reply.body()));

        self->socket.async_write_response(reply, yield);
    }

    void redirect_route(asio::yield_context yield)
    {
        auto self = shared_from_this();

        http::response reply;

        reply.status_code() = 303;
        reply.reason_phrase() = "Moved temporarily";

        reply.headers().emplace("Location", "index.html");

        const char body[] = "303 Moved temporarily\n";
        std::copy(body, body + sizeof(body) - 1,
                    std::back_inserter(reply.body()));

        self->socket.async_write_response(reply, yield);
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

    http::buffered_socket socket;
    int counter;

    http::request request;

#define ROUTER_FUNCTION_ARGUMENTS connection*, \
                                  asio::yield_context

    static
    http::regex_router<std::function<void(ROUTER_FUNCTION_ARGUMENTS)>,
                       ROUTER_FUNCTION_ARGUMENTS
                       > router;
};

std::regex operator ""_r(const char* str, size_t len)
{
    return std::regex(std::string(str, len));
}

using namespace std::placeholders; // for _1, _2

http::regex_router<std::function<void(ROUTER_FUNCTION_ARGUMENTS)>,
                    ROUTER_FUNCTION_ARGUMENTS
                  > connection::router =
{
    { "/index.htm."_r, std::bind(&connection::index_route, _1, _2) },
    { "/.*"_r, std::bind(&connection::redirect_route, _1, _2) },
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
