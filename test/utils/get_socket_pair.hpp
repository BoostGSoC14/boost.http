#include <tuple>

#include <boost/asio/ip/tcp.hpp>

#include "fiber_baton.hpp"

std::tuple<boost::asio::ip::tcp::socket, boost::asio::ip::tcp::socket>
get_socket_pair(boost::asio::io_context &ioctx,
                boost::asio::yield_context &yield)
{
    fiber_baton baton{ioctx};
    boost::asio::ip::tcp::socket s1{ioctx}, s2{ioctx};

    boost::asio::ip::tcp::acceptor acceptor{ioctx};
    {
        boost::asio::ip::tcp::endpoint ep(
            boost::asio::ip::address_v4::loopback(), 0);
        acceptor.open(ep.protocol());
        acceptor.bind(ep);
    }
    auto ep = acceptor.local_endpoint();
    acceptor.listen();

    boost::asio::spawn(
        yield,
        [ep,&baton,&s2](boost::asio::yield_context yield) {
            s2.async_connect(ep, yield);
            baton.post();
        }
    );

    acceptor.async_accept(s1, yield);

    // A `fiber.join()` would be much better, but it has been problematic to use
    // boost::asio::spawn Handler-based overload to build this primitive. I gave
    // up. Only would do it if I were to build the coroutine completion token
    // from the ground up.
    baton.wait(yield);
    return {std::move(s1), std::move(s2)};
}
