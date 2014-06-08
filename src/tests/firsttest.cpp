#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <boost/http/embedded_server_socket_acceptor.hpp>

BOOST_AUTO_TEST_CASE(Simple_attributes) {
    boost::asio::io_service ios;
    boost::http::embedded_server_socket
        socket(ios, boost::http::embedded_server_mode_flags::server);
    BOOST_CHECK(boost::http::outgoing_state::empty
                == socket.outgoing_state());
}
