#include <iostream>
#include <algorithm>

#include <boost/utility/string_ref.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/http/buffered_socket.hpp>
#include <boost/http/algorithm.hpp>

using namespace std;
using namespace boost;

class connection: public std::enable_shared_from_this<connection>
{
public:
    void operator()(asio::yield_context yield)
    {
        auto guard = shared_from_this();
        try {
            while (true) {
                cout << '[' << counter << "] About to receive a new message"
                     << endl;
                socket.async_read_request(method, path, message, yield);
                //message.body().clear(); // freeing not used resources

                if (http::request_continue_required(message)) {
                    cout << '[' << counter
                         << "] Continue required. About to send"
                        " \"100-continue\""
                         << std::endl;
                    socket.async_write_response_continue(yield);
                }

                while (socket.read_state() != http::read_state::empty) {
                    cout << '[' << counter << "] Message not fully received"
                         << endl;
                    switch (socket.read_state()) {
                    case http::read_state::message_ready:
                        cout << '[' << counter << "] About to receive some body"
                             << endl;
                        socket.async_read_some(message, yield);
                        break;
                    case http::read_state::body_ready:
                        cout << '[' << counter << "] About to receive trailers"
                             << endl;
                        socket.async_read_trailers(message, yield);
                        break;
                    default:;
                    }
                }

                //cout << "BODY:==";
                //for (const auto &e: message.body()) {
                //    cout << char(e);
                //}
                //cout << "==" << endl;

                cout << '[' << counter << "] Message received. State = "
                     << int(socket.read_state()) << endl;
                cout << '[' << counter << "] Method: " << method << endl;
                cout << '[' << counter << "] Path: " << path << endl;
                {
                    auto host = message.headers().find("host");
                    if (host != message.headers().end())
                        cout << '[' << counter << "] Host header: "
                             << host->second << endl;
                }

                std::cout << '[' << counter << "] Write state = "
                          << int(socket.write_state()) << std::endl;

                cout << '[' << counter << "] About to send a reply" << endl;

                http::message reply;
                //reply.headers().emplace("connection", "close");
                const char body[] = "Hello World\n";
                std::copy(body, body + sizeof(body) - 1,
                          std::back_inserter(reply.body()));

                socket.async_write_response(200, string_ref("OK"), reply,
                                            yield);
            }
        } catch (system::system_error &e) {
            if (e.code()
                != system::error_code{http::http_errc::stream_finished}) {
                cerr << '[' << counter << "] Aborting on exception: "
                     << e.what() << endl;
                std::exit(1);
            } else {
                cout << '[' << counter << "] Closing connection" << endl;
            }

            socket.next_layer()
            .shutdown(asio::ip::tcp::socket::shutdown_both);
            socket.next_layer().close();
        } catch (std::exception &e) {
            cerr << '[' << counter << "] Aborting on exception: " << e.what()
                 << endl;
            std::exit(1);
        }
    }

    asio::ip::tcp::socket &tcp_layer()
    {
        return socket.next_layer();
    }

    static std::shared_ptr<connection> make_connection(asio::io_service &ios,
                                                       int counter)
    {
        return std::shared_ptr<connection>{new connection{ios, counter}};
    }

private:
    connection(asio::io_service &ios, int counter)
        : socket(ios)
        , counter(counter)
    {}

    http::buffered_socket socket;
    int counter;

    std::string method;
    std::string path;
    http::message message;
};

int main()
{
    asio::io_service ios;
    asio::ip::tcp::acceptor acceptor(ios,
                                     asio::ip::tcp
                                     ::endpoint(asio::ip::tcp::v6(), 8080));

    auto work = [&acceptor](asio::yield_context yield) {
        int counter = 0;
        for ( ; true ; ++counter ) {
            try {
                auto connection
                    = connection::make_connection(acceptor.get_io_service(),
                                                  counter);
                cout << "About to accept a new connection" << endl;
                acceptor.async_accept(connection->tcp_layer(), yield);

                auto handle_connection
                    = [connection](asio::yield_context yield) mutable {
                    (*connection)(yield);
                };
                spawn(acceptor.get_io_service(), handle_connection);
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
