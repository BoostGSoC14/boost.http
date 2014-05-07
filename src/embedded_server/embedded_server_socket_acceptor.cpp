/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#include <boost/http/embedded_server/embedded_server.hpp>

namespace boost {
namespace http {

basic_socket_acceptor<embedded_server>
::basic_socket_acceptor(boost::asio::ip::tcp::acceptor &&acceptor) :
    acceptor(std::move(acceptor))
{}

void basic_socket_acceptor<embedded_server>
::accept(basic_socket<protocol_type> &socket)
{
    acceptor.accept(socket.channel);
}

void basic_socket_acceptor<embedded_server>
::async_accept(basic_socket<protocol_type> &socket, handler_type handler)
{
    // handler could be perfect-forwarded, because it has the same signature
    // get rid of std::function<> (?) TODO
    acceptor.async_accept(socket.channel, handler);
}

template class basic_socket_acceptor<embedded_server>;

} // namespace http
} // namespace boost
