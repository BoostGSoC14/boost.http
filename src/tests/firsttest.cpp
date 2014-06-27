#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <boost/http/embedded_server_socket_acceptor.hpp>

BOOST_AUTO_TEST_CASE(Simple_attributes) {
    boost::asio::io_service ios;
    char buffer[1];
    boost::http::embedded_server_socket
        socket(ios, boost::asio::buffer(buffer),
               boost::http::channel_type::server);
    BOOST_CHECK(boost::http::outgoing_state::empty
                == socket.outgoing_state());
}
