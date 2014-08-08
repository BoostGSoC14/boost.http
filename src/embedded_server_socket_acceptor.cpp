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

embedded_server_socket_acceptor
::embedded_server_socket_acceptor(boost::asio::io_service &io_service,
                                  boost::asio::ip::tcp::endpoint endpoint) :
    acceptor_(io_service, endpoint)
{}

void embedded_server_socket_acceptor
::accept(endpoint_type &socket)
{
    acceptor_.accept(socket.next_layer());
}

} // namespace http
} // namespace boost
