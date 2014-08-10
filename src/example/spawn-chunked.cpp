#include <cstdlib>
#include <iostream>
#include <algorithm>

#include <boost/asio/io_service.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/http/socket.hpp>
#include <boost/http/algorithm.hpp>

using namespace std;
using namespace boost;

int main()
{
    asio::io_service ios;

    asio::ip::tcp::acceptor acceptor(ios, asio::ip::tcp
                                     ::endpoint(asio::ip::tcp::v6(), 8080));

    auto work = [&acceptor](asio::yield_context yield) {
        while (true) {
            try {
                char buffer[4];
                http::socket socket(acceptor.get_io_service(),
                                    asio::buffer(buffer));
                std::string method;
                std::string path;
                http::message message;

                cout << "About to accept a new connection" << endl;
                acceptor.async_accept(socket.next_layer(), yield);

                cout << "About to receive a new message" << endl;
                socket.async_read_request(method, path, message, yield);
                //message.body.clear(); // freeing not used resources

                if (http::request_continue_required(message)) {
                    cout << "Continue required. About to send \"100-continue\""
                         << std::endl;
                    socket.async_write_response_continue(yield);
                }

                while (socket.read_state() != http::read_state::empty) {
                    cout << "Message not fully received" << endl;
                    switch (socket.read_state()) {
                    case http::read_state::message_ready:
                        cout << "About to receive some body" << endl;
                        socket.async_read_some(message, yield);
                        break;
                    case http::read_state::body_ready:
                        cout << "About to receive trailers" << endl;
                        socket.async_read_trailers(message, yield);
                        break;
                    default:;
                    }
                }

                //cout << "BODY:==";
                //for (const auto &e: message.body) {
                //    cout << char(e);
                //}
                //cout << "==" << endl;

                cout << "Message received. State = "
                     << int(socket.read_state()) << endl;
                cout << "Method: " << method << endl;
                cout << "Path: " << path << endl;
                cout << "Host header: "
                     << message.headers.find("host")->second << endl;

                std::cout << "Outgoing state = " << int(socket.write_state())
                << std::endl;

                cout << "About to send the reply's metadata" << endl;

                http::message reply;
                socket.async_write_response_metadata(200, "OK", reply, yield);

                std::cout << "Outgoing state = " << int(socket.write_state())
                << std::endl;

                std::cout << "About to send the reply's body first part"
                << std::endl;

                const char body[] = "Foo";
                std::copy(body, body + sizeof(body) - 1,
                          std::back_inserter(reply.body));
                socket.async_write(reply, yield);

                std::cout << "Outgoing state = " << int(socket.write_state())
                << std::endl;

                std::cout << "About to send the reply's body second part"
                << std::endl;

                reply.body[0] = 'b';
                reply.body[1] = 'a';
                reply.body[2] = 'r';
                socket.async_write(reply, yield);

                std::cout << "Outgoing state = " << int(socket.write_state())
                << std::endl;

                std::cout << "About to send the trailers" << std::endl;

                reply.trailers.emplace("content-md5",
                                       "89d5739baabbbe65be35cbe61c88e06d");
                socket.async_write_trailers(reply, yield);

                std::cout << "Outgoing state = " << int(socket.write_state())
                << std::endl;
            } catch (std::exception &e) {
                cerr << "Aborting on exception: " << e.what() << endl;
                std::exit(1);
            }
        };
    };

    cout << "About to spawn" << endl;
    spawn(ios, work);

    cout << "About to run" << endl;
    ios.run();

    return 0;
}
