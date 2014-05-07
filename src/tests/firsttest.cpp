#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <boost/http/embedded_server/embedded_server.hpp>

BOOST_AUTO_TEST_CASE(Simple_attributes) {
    boost::asio::io_service ios;
    boost::http::basic_socket<boost::http::embedded_server>
        socket(ios, boost::http::embedded_server_mode_flags::server);
    boost::http::outgoing_state::finished == socket.outgoing_state();
    BOOST_CHECK(boost::http::outgoing_state::empty
                == socket.outgoing_state());
}
