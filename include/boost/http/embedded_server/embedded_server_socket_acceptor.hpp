/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_EMBEDDED_SERVER_SOCKET_ACCEPTOR_H
#define BOOST_HTTP_EMBEDDED_SERVER_SOCKET_ACCEPTOR_H

#include "embedded_server_socket.hpp"
#include "../basic_socket_acceptor.hpp"

namespace boost {
namespace http {

template<>
class basic_socket_acceptor<embedded_server>
{
public:
    typedef embedded_server protocol_type;
    typedef std::function<void(const boost::system::error_code& error)>
    handler_type;

    basic_socket_acceptor(boost::asio::ip::tcp::acceptor &&acceptor);
    basic_socket_acceptor(boost::asio::io_service &io_service,
                          boost::asio::ip::tcp::endpoint
                          = boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),
                                                           80));

    // TODO: maybe just expose a public attribute?
    const boost::asio::ip::tcp::acceptor &acceptor() const;
    boost::asio::ip::tcp::acceptor &acceptor();

    void accept(basic_socket<protocol_type> &socket);

    void async_accept(basic_socket<protocol_type> &socket,
                      handler_type handler);

private:
    boost::asio::ip::tcp::acceptor acceptor_;
};

extern template class basic_socket_acceptor<embedded_server>;

} // namespace http
} // namespace boost

#endif // BOOST_HTTP_EMBEDDED_SERVER_SOCKET_ACCEPTOR_H
