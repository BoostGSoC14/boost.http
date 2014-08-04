/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_EMBEDDED_SERVER_SOCKET_ACCEPTOR_H
#define BOOST_HTTP_EMBEDDED_SERVER_SOCKET_ACCEPTOR_H

#include <utility>
#include <boost/http/embedded_server_socket.hpp>

namespace boost {
namespace http {

/** @TODO: templatize based on the callback type (currently std::function). */
class embedded_server_socket_acceptor
{
public:
    typedef embedded_server_socket<> endpoint_type;

    embedded_server_socket_acceptor(boost::asio::ip::tcp::acceptor &&acceptor);
    embedded_server_socket_acceptor(boost::asio::io_service &io_service,
                                    boost::asio::ip::tcp::endpoint
                                    = boost::asio::ip::tcp
                                    ::endpoint(boost::asio::ip::tcp::v4(), 80));

    // TODO: maybe just expose a public attribute?
    const boost::asio::ip::tcp::acceptor &acceptor() const;
    boost::asio::ip::tcp::acceptor &acceptor();

    void accept(endpoint_type &socket);

    template<class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_accept(endpoint_type &socket, CompletionToken &&token)
    {
        return acceptor_.async_accept(socket.channel,
                                      std::forward<CompletionToken>(token));
    }

    // ### USUAL FUNCTIONS
    asio::io_service & get_io_service()
    {
        return acceptor_.get_io_service();
    }

private:
    boost::asio::ip::tcp::acceptor acceptor_;
};

} // namespace http
} // namespace boost

#endif // BOOST_HTTP_EMBEDDED_SERVER_SOCKET_ACCEPTOR_H
