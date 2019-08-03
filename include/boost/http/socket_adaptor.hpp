/* Copyright (c) 2018 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_SOCKET_ADAPTOR_HPP
#define BOOST_HTTP_SOCKET_ADAPTOR_HPP

#include <boost/http/poly_socket.hpp>
#include <boost/http/response.hpp>
#include <boost/http/request.hpp>

namespace boost {
namespace http {

template<class Socket, class Request = request, class Response = response,
         class Message = request_response_wrapper<Request, Response>>
class socket_adaptor
    : public basic_poly_socket<Request, Response, Message>
    , private Socket
{
public:
    static_assert(is_server_socket<Socket>::value,
                  "Socket must fulfill the ServerSocket concept");
    static_assert(is_client_socket<Socket>::value,
                  "Socket must fulfill the ClientSocket concept");

    using typename basic_poly_socket<Request, Response, Message>::executor_type;
    using typename basic_poly_socket<
        Request, Response, Message
    >::request_type;
    using typename basic_poly_socket<
        Request, Response, Message
    >::response_type;
    using typename basic_poly_socket<
        Request, Response, Message
    >::message_type;
    using typename basic_poly_socket<
        Request, Response, Message
    >::handler_type;
    using next_layer_type = Socket;

    template<class... Args>
    socket_adaptor(Args&&... args);

    next_layer_type &next_layer();

    const next_layer_type &next_layer() const;

    // ### poly_socket INTERFACE IMPLEMENTATION ###
    executor_type get_executor() override;
    bool is_open() const override;
    http::read_state read_state() const override;
    http::write_state write_state() const override;
    bool write_response_native_stream() const override;
    void async_read_request(request_type &request,
                            handler_type handler) override;
    void async_read_response(response_type &response,
                             handler_type handler) override;
    void async_read_some(message_type &message, handler_type handler) override;
    void async_write_response(const response_type &response,
                              handler_type handler) override;
    void async_write_response_continue(handler_type handler) override;
    void async_write_response_metadata(const response_type &response,
                                       handler_type handler) override;
    void async_write_request(const request_type &request,
                             handler_type handler) override;
    void async_write_request_metadata(
        const request_type &request, handler_type handler
    ) override;
    void async_write(const message_type &message,
                     handler_type handler) override;
    void async_write_trailers(const message_type &message,
                              handler_type handler) override;
    void async_write_end_of_message(handler_type handler) override;
};

template<class Socket, class Request, class Response, class Message>
class socket_adaptor<std::reference_wrapper<Socket>, Request, Response, Message>
    : public basic_poly_socket<Request, Response, Message>
{
public:
    static_assert(is_server_socket<Socket>::value,
                  "Socket must fulfill the ServerSocket concept");
    static_assert(is_client_socket<Socket>::value,
                  "Socket must fulfill the ClientSocket concept");

    using typename basic_poly_socket<
        Request, Response, Message
    >::executor_type;
    using typename basic_poly_socket<
        Request, Response, Message
    >::request_type;
    using typename basic_poly_socket<
        Request, Response, Message
    >::response_type;
    using typename basic_poly_socket<
        Request, Response, Message
    >::message_type;
    using typename basic_poly_socket<
        Request, Response, Message
    >::handler_type;
    using next_layer_type = Socket;

    socket_adaptor(Socket &socket);

    socket_adaptor(Socket &&socket) = delete;

    next_layer_type &next_layer();

    const next_layer_type &next_layer() const;

    // ### poly_socket INTERFACE IMPLEMENTATION ###
    executor_type get_executor() override;
    bool is_open() const override;
    http::read_state read_state() const override;
    http::write_state write_state() const override;
    bool write_response_native_stream() const override;
    void async_read_request(request_type &request,
                            handler_type handler) override;
    void async_read_response(response_type &response,
                             handler_type handler) override;
    void async_read_some(message_type &message, handler_type handler) override;
    void async_write_response(const response_type &response,
                              handler_type handler) override;
    void async_write_response_continue(handler_type handler) override;
    void async_write_response_metadata(const response_type &response,
                                       handler_type handler) override;
    void async_write_request(const request_type &request,
                             handler_type handler) override;
    void async_write_request_metadata(
        const request_type &request, handler_type handler
    ) override;
    void async_write(const message_type &message,
                     handler_type handler) override;
    void async_write_trailers(const message_type &message,
                              handler_type handler) override;
    void async_write_end_of_message(handler_type handler) override;

private:
    std::reference_wrapper<Socket> wrapped_socket;
};

template<class Socket, class Request, class Response, class Message>
struct is_server_socket<
    socket_adaptor<Socket, Request, Response, Message>
>
    : public std::true_type
{};

template<class Socket, class Request, class Response, class Message>
struct is_client_socket<
    socket_adaptor<Socket, Request, Response, Message>
>
    : public std::true_type
{};

} // namespace http
} // namespace boost

#include "socket_adaptor.ipp"

#endif // BOOST_HTTP_SOCKET_ADAPTOR_HPP
