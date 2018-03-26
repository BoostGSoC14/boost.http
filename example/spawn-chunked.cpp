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
                    if (host != self->request.headers().end()) {
                        cout << '[' << self->counter << "] Host header: "
                             << host->second << endl;
                    }
                }

                std::cout << '[' << self->counter << "] Write state = "
                          << int(self->socket.write_state()) << std::endl;

                cout << '[' << self->counter
                     << "] About to send the reply's metadata" << endl;

                http::response reply;
                reply.status_code() = 200;
                reply.reason_phrase() = "OK";
                //reply.headers().emplace("connection", "close");

                self->socket.async_write_response_metadata(reply, yield);

                std::cout << '[' << self->counter << "] Write state = "
                          << int(self->socket.write_state()) << std::endl;

                std::cout << '[' << self->counter
                          << "] About to send the reply's body first part"
                          << std::endl;

                const char body[] = "Foo";
                std::copy(body, body + sizeof(body) - 1,
                          std::back_inserter(reply.body()));
                socket.async_write(reply, yield);

                std::cout << '[' << self->counter << "] Write state = "
                          << int(self->socket.write_state()) << std::endl;

                std::cout << '[' << self->counter
                          << "] About to send the reply's body second part"
                          << std::endl;

                reply.body()[0] = 'b';
                reply.body()[1] = 'a';
                reply.body()[2] = 'r';
                socket.async_write(reply, yield);

                std::cout << '[' << self->counter << "] Write state = "
                          << int(self->socket.write_state()) << std::endl;

                std::cout << '[' << self->counter
                          << "] About to send the trailers" << std::endl;

                reply.trailers().emplace("content-md5",
                                         "89d5739baabbbe65be35cbe61c88e06d");
                socket.async_write_trailers(reply, yield);

                std::cout << '[' << self->counter << "] Write state = "
                          << int(self->socket.write_state()) << std::endl;
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

    http::buffered_socket socket;
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
