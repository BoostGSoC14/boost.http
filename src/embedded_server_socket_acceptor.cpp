/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#include <boost/http/embedded_server_socket_acceptor.hpp>

namespace boost {
namespace http {

embedded_server_socket_acceptor
::embedded_server_socket_acceptor(boost::asio::ip::tcp::acceptor &&acceptor) :
    acceptor_(std::move(acceptor))
{}

void embedded_server_socket_acceptor
::accept(endpoint_type &socket)
{
    acceptor_.accept(socket.channel);
}

void embedded_server_socket_acceptor
::async_accept(endpoint_type &socket, handler_type handler)
{
    // handler could be perfect-forwarded, because it has the same signature
    // get rid of std::function<> (?) TODO
    acceptor_.async_accept(socket.channel, handler);
}

} // namespace http
} // namespace boost
