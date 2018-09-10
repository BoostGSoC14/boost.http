/* Copyright (c) 2018 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_POLY_SOCKET_HPP
#define BOOST_HTTP_POLY_SOCKET_HPP

#include <boost/http/poly_server_socket.hpp>
#include <boost/http/poly_client_socket.hpp>

namespace boost {
namespace http {

template<class Request, class Response,
         class Message = request_response_wrapper<Request, Response>>
class basic_poly_socket
    : public basic_poly_server_socket<Request, Response, Message>
    , public basic_poly_client_socket<Request, Response, Message>
{
public:
    using typename basic_poly_socket_base<Message>::executor_type;
    using typename basic_poly_socket_base<Message>::message_type;
    using typename basic_poly_socket_base<Message>::handler_type;
    using request_type = Request;
    using response_type = Response;
};

using poly_socket = basic_poly_socket<request, response>;

template<class Request, class Response, class Message>
struct is_server_socket<basic_poly_socket<Request, Response, Message>>
    : public std::true_type
{};

template<class Request, class Response, class Message>
struct is_client_socket<basic_poly_socket<Request, Response, Message>>
    : public std::true_type
{};

} // namespace http
} // namespace boost

#endif // BOOST_HTTP_POLY_SOCKET_HPP
