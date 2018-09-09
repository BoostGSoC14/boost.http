/* Copyright (c) 2014, 2017, 2018 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

namespace boost {
namespace http {

template<class Socket, class Request, class Response, class Message>
template<class... Args>
server_socket_adaptor<Socket, Request, Response, Message>
::server_socket_adaptor(Args&&... args)
    : Socket(std::forward<Args>(args)...)
{}

template<class Socket, class Request, class Response, class Message>
Socket &server_socket_adaptor<Socket, Request, Response, Message>
::next_layer()
{
    return *this;
}

template<class Socket, class Request, class Response, class Message>
const Socket&
server_socket_adaptor<Socket, Request, Response, Message>
::next_layer() const
{
    return *this;
}

template<class Socket, class Request, class Response, class Message>
typename server_socket_adaptor<
    Socket, Request, Response, Message
>::executor_type
server_socket_adaptor<Socket, Request, Response, Message>::get_executor()
{
    return Socket::get_executor();
}

template<class Socket, class Request, class Response, class Message>
bool server_socket_adaptor<Socket, Request, Response, Message>::is_open() const
{
    return Socket::is_open();
}

template<class Socket, class Request, class Response, class Message>
read_state
server_socket_adaptor<Socket, Request, Response, Message>::read_state() const
{
    return Socket::read_state();
}

template<class Socket, class Request, class Response, class Message>
write_state
server_socket_adaptor<Socket, Request, Response, Message>::write_state() const
{
    return Socket::write_state();
}

template<class Socket, class Request, class Response, class Message>
bool server_socket_adaptor<Socket, Request, Response, Message>
::write_response_native_stream() const
{
    return Socket::write_response_native_stream();
}

template<class Socket, class Request, class Response, class Message>
void server_socket_adaptor<Socket, Request, Response, Message>
::async_read_request(request_server_type &request, handler_type handler)
{
    Socket::async_read_request(request, handler);
}

template<class Socket, class Request, class Response, class Message>
void server_socket_adaptor<Socket, Request, Response, Message>
::async_read_some(message_type &message, handler_type handler)
{
    Socket::async_read_some(message, handler);
}

template<class Socket, class Request, class Response, class Message>
void server_socket_adaptor<Socket, Request, Response, Message>
::async_read_trailers(message_type &message, handler_type handler)
{
    Socket::async_read_trailers(message, handler);
}

template<class Socket, class Request, class Response, class Message>
void server_socket_adaptor<Socket, Request, Response, Message>
::async_write_response(const response_server_type &response,
                       handler_type handler)
{
    Socket::async_write_response(response, handler);
}

template<class Socket, class Request, class Response, class Message>
void server_socket_adaptor<Socket, Request, Response, Message>
::async_write_response_continue(handler_type handler)
{
    Socket::async_write_response_continue(handler);
}

template<class Socket, class Request, class Response, class Message>
void server_socket_adaptor<Socket, Request, Response, Message>
::async_write_response_metadata(const response_server_type &response,
                                handler_type handler)
{
    Socket::async_write_response_metadata(response, handler);
}

template<class Socket, class Request, class Response, class Message>
void server_socket_adaptor<Socket, Request, Response, Message>
::async_write(const message_type &message, handler_type handler)
{
    Socket::async_write(message, handler);
}

template<class Socket, class Request, class Response, class Message>
void server_socket_adaptor<Socket, Request, Response, Message>
::async_write_trailers(const message_type &message, handler_type handler)
{
    Socket::async_write_trailers(message, handler);
}

template<class Socket, class Request, class Response, class Message>
void server_socket_adaptor<Socket, Request, Response, Message>
::async_write_end_of_message(handler_type handler)
{
    Socket::async_write_end_of_message(handler);
}

template<class Socket, class Request, class Response, class Message>
server_socket_adaptor<std::reference_wrapper<Socket>, Request, Response,
                      Message>
::server_socket_adaptor(Socket &socket)
    : wrapped_socket(socket)
{}

template<class Socket, class Request, class Response, class Message>
Socket&
server_socket_adaptor<std::reference_wrapper<Socket>, Request, Response,
                      Message>
::next_layer()
{
    return wrapped_socket;
}

template<class Socket, class Request, class Response, class Message>
const Socket&
server_socket_adaptor<std::reference_wrapper<Socket>, Request, Response,
                      Message>
::next_layer() const
{
    return wrapped_socket;
}

template<class Socket, class Request, class Response, class Message>
typename server_socket_adaptor<
    std::reference_wrapper<Socket>, Request, Response, Message
>::executor_type
server_socket_adaptor<std::reference_wrapper<Socket>, Request, Response,
                      Message>
::get_executor()
{
    return wrapped_socket.get().get_executor();
}

template<class Socket, class Request, class Response, class Message>
bool
server_socket_adaptor<std::reference_wrapper<Socket>, Request, Response,
                      Message>
::is_open() const
{
    return wrapped_socket.get().is_open();
}

template<class Socket, class Request, class Response, class Message>
read_state
server_socket_adaptor<std::reference_wrapper<Socket>, Request, Response,
                      Message>
::read_state() const
{
    return wrapped_socket.get().read_state();
}

template<class Socket, class Request, class Response, class Message>
write_state
server_socket_adaptor<std::reference_wrapper<Socket>, Request, Response,
                      Message>
::write_state() const
{
    return wrapped_socket.get().write_state();
}

template<class Socket, class Request, class Response, class Message>
bool
server_socket_adaptor<std::reference_wrapper<Socket>, Request, Response,
                      Message>
::write_response_native_stream() const
{
    return wrapped_socket.get().write_response_native_stream();
}

template<class Socket, class Request, class Response, class Message>
void
server_socket_adaptor<std::reference_wrapper<Socket>, Request, Response,
                      Message>
::async_read_request(request_server_type &request, handler_type handler)
{
    wrapped_socket.get().async_read_request(request, handler);
}

template<class Socket, class Request, class Response, class Message>
void
server_socket_adaptor<std::reference_wrapper<Socket>, Request, Response,
                      Message>
::async_read_some(message_type &message, handler_type handler)
{
    wrapped_socket.get().async_read_some(message, handler);
}

template<class Socket, class Request, class Response, class Message>
void
server_socket_adaptor<std::reference_wrapper<Socket>, Request, Response,
                      Message>
::async_read_trailers(message_type &message, handler_type handler)
{
    wrapped_socket.get().async_read_trailers(message, handler);
}

template<class Socket, class Request, class Response, class Message>
void
server_socket_adaptor<std::reference_wrapper<Socket>, Request, Response,
                      Message>
::async_write_response(const response_server_type &response,
                       handler_type handler)
{
    wrapped_socket.get().async_write_response(response, handler);
}

template<class Socket, class Request, class Response, class Message>
void
server_socket_adaptor<std::reference_wrapper<Socket>, Request, Response,
                      Message>
::async_write_response_continue(handler_type handler)
{
    wrapped_socket.get().async_write_response_continue(handler);
}

template<class Socket, class Request, class Response, class Message>
void
server_socket_adaptor<std::reference_wrapper<Socket>, Request, Response,
                      Message>
::async_write_response_metadata(const response_server_type &response,
                                handler_type handler)
{
    wrapped_socket.get().async_write_response_metadata(response, handler);
}

template<class Socket, class Request, class Response, class Message>
void
server_socket_adaptor<std::reference_wrapper<Socket>, Request, Response,
                      Message>
::async_write(const message_type &message, handler_type handler)
{
    wrapped_socket.get().async_write(message, handler);
}

template<class Socket, class Request, class Response, class Message>
void
server_socket_adaptor<std::reference_wrapper<Socket>, Request, Response,
                      Message>
::async_write_trailers(const message_type &message, handler_type handler)
{
    wrapped_socket.get().async_write_trailers(message, handler);
}

template<class Socket, class Request, class Response, class Message>
void
server_socket_adaptor<std::reference_wrapper<Socket>, Request, Response,
                      Message>
::async_write_end_of_message(handler_type handler)
{
    wrapped_socket.get().async_write_end_of_message(handler);
}

} // namespace http
} // namespace boost
