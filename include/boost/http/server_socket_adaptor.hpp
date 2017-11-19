/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_SERVER_SOCKET_ADAPTOR_HPP
#define BOOST_HTTP_SERVER_SOCKET_ADAPTOR_HPP

#include <boost/http/polymorphic_server_socket.hpp>
#include <boost/http/request.hpp>
#include <boost/http/response.hpp>

namespace boost {
namespace http {

template<class Socket, class Request = request, class Response = response,
         class Message = request_response_wrapper<Request, Response>>
class server_socket_adaptor
    : public basic_polymorphic_server_socket<Request, Response, Message>
    , private Socket
{
public:
    static_assert(is_server_socket<Socket>::value,
                  "Socket must fulfill the ServerSocket concept");

    using typename basic_polymorphic_server_socket<Request, Response, Message>
        ::request_server_type;
    using typename basic_polymorphic_server_socket<Request, Response, Message>
        ::response_server_type;
    using typename basic_polymorphic_server_socket<Request, Response, Message>
        ::message_type;
    using typename basic_polymorphic_server_socket<Request, Response, Message>
        ::callback_type;
    typedef Socket next_layer_type;

    template<class... Args>
    server_socket_adaptor(Args&&... args);

    next_layer_type &next_layer();

    const next_layer_type &next_layer() const;

    // ### polymorphic_socket INTERFACE IMPLEMENTATION ###
    asio::io_service& get_io_service() override;
    bool is_open() const override;
    http::read_state read_state() const override;
    http::write_state write_state() const override;
    bool write_response_native_stream() const override;
    void async_read_request(request_server_type &request,
                            callback_type handler) override;
    void async_read_some(message_type &message, callback_type handler) override;
    void async_read_trailers(message_type &message,
                             callback_type handler) override;
    void async_write_response(const response_server_type &response,
                              callback_type handler) override;
    void async_write_response_continue(callback_type handler) override;
    void async_write_response_metadata(const response_server_type &response,
                                       callback_type handler) override;
    void async_write(const message_type &message,
                     callback_type handler) override;
    void async_write_trailers(const message_type &message,
                              callback_type handler) override;
    void async_write_end_of_message(callback_type handler) override;
};

template<class Socket, class Request, class Response, class Message>
class server_socket_adaptor<std::reference_wrapper<Socket>, Request, Response,
                            Message>
    : public basic_polymorphic_server_socket<Request, Response, Message>
{
public:
    static_assert(is_server_socket<Socket>::value,
                  "Socket must fulfill the ServerSocket concept");

    using typename basic_polymorphic_server_socket<Request, Response, Message>
        ::request_server_type;
    using typename basic_polymorphic_server_socket<Request, Response, Message>
        ::response_server_type;
    using typename basic_polymorphic_server_socket<Request, Response, Message>
        ::message_type;
    using typename basic_polymorphic_server_socket<Request, Response, Message>
        ::callback_type;
    typedef Socket next_layer_type;

    server_socket_adaptor(Socket &socket);

    server_socket_adaptor(Socket &&socket) = delete;

    next_layer_type &next_layer();

    const next_layer_type &next_layer() const;

    // ### polymorphic_socket INTERFACE IMPLEMENTATION ###
    asio::io_service& get_io_service() override;
    bool is_open() const override;
    http::read_state read_state() const override;
    http::write_state write_state() const override;
    bool write_response_native_stream() const override;
    void async_read_request(request_server_type &request,
                            callback_type handler) override;
    void async_read_some(message_type &message, callback_type handler) override;
    void async_read_trailers(message_type &message,
                             callback_type handler) override;
    void async_write_response(const response_server_type &response,
                              callback_type handler) override;
    void async_write_response_continue(callback_type handler) override;
    void async_write_response_metadata(const response_server_type &response,
                                       callback_type handler) override;
    void async_write(const message_type &message,
                     callback_type handler) override;
    void async_write_trailers(const message_type &message,
                              callback_type handler) override;
    void async_write_end_of_message(callback_type handler) override;

private:
    std::reference_wrapper<Socket> wrapped_socket;
};

template<class Socket, class Message>
struct is_server_socket<server_socket_adaptor<Socket, Message>>
    : public std::true_type
{};

} // namespace http
} // namespace boost

#include "server_socket_adaptor.ipp"

#endif // BOOST_HTTP_SERVER_SOCKET_ADAPTOR_HPP
