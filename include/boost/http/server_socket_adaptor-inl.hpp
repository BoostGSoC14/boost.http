/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

namespace boost {
namespace http {

template<class Socket, class Message>
template<class... Args>
server_socket_adaptor<Socket, Message>::server_socket_adaptor(Args&&... args)
    : Socket(std::forward<Args>(args)...)
{}

template<class Socket, class Message>
Socket &server_socket_adaptor<Socket, Message>::next_layer()
{
    return *this;
}

template<class Socket, class Message>
const Socket &server_socket_adaptor<Socket, Message>::next_layer() const
{
    return *this;
}

template<class Socket, class Message>
asio::io_service& server_socket_adaptor<Socket, Message>::get_io_service()
{
    return Socket::get_io_service();
}

template<class Socket, class Message>
bool server_socket_adaptor<Socket, Message>::is_open() const
{
    return Socket::is_open();
}

template<class Socket, class Message>
read_state server_socket_adaptor<Socket, Message>::read_state() const
{
    return Socket::read_state();
}

template<class Socket, class Message>
write_state server_socket_adaptor<Socket, Message>::write_state() const
{
    return Socket::write_state();
}

template<class Socket, class Message>
bool server_socket_adaptor<Socket, Message>
::write_response_native_stream() const
{
    return Socket::write_response_native_stream();
}

template<class Socket, class Message>
void server_socket_adaptor<Socket, Message>
::async_read_request(std::string &method, std::string &path,
                     message_type &message, callback_type handler)
{
    Socket::async_read_request(method, path, message, handler);
}

template<class Socket, class Message>
void server_socket_adaptor<Socket, Message>
::async_read_some(message_type &message, callback_type handler)
{
    Socket::async_read_some(message, handler);
}

template<class Socket, class Message>
void server_socket_adaptor<Socket, Message>
::async_read_trailers(message_type &message, callback_type handler)
{
    Socket::async_read_trailers(message, handler);
}

template<class Socket, class Message>
void server_socket_adaptor<Socket, Message>
::async_write_response(std::uint_fast16_t status_code,
                       const boost::string_ref &reason_phrase,
                       const message_type &message, callback_type handler)
{
    Socket::async_write_response(status_code, reason_phrase, message, handler);
}

template<class Socket, class Message>
void server_socket_adaptor<Socket, Message>
::async_write_response_continue(callback_type handler)
{
    Socket::async_write_response_continue(handler);
}

template<class Socket, class Message>
void server_socket_adaptor<Socket, Message>
::async_write_response_metadata(std::uint_fast16_t status_code,
                                const boost::string_ref &reason_phrase,
                                const message_type &message,
                                callback_type handler)
{
    Socket::async_write_response_metadata(status_code, reason_phrase, message,
                                          handler);
}

template<class Socket, class Message>
void server_socket_adaptor<Socket, Message>
::async_write(const message_type &message, callback_type handler)
{
    Socket::async_write(message, handler);
}

template<class Socket, class Message>
void server_socket_adaptor<Socket, Message>
::async_write_trailers(const message_type &message, callback_type handler)
{
    Socket::async_write_trailers(message, handler);
}

template<class Socket, class Message>
void server_socket_adaptor<Socket, Message>
::async_write_end_of_message(callback_type handler)
{
    Socket::async_write_end_of_message(handler);
}

template<class Socket, class Message>
server_socket_adaptor<std::reference_wrapper<Socket>, Message>
::server_socket_adaptor(Socket &socket)
    : wrapped_socket(socket)
{}

template<class Socket, class Message>
Socket& server_socket_adaptor<std::reference_wrapper<Socket>, Message>
::next_layer()
{
    return wrapped_socket;
}

template<class Socket, class Message>
const Socket &server_socket_adaptor<std::reference_wrapper<Socket>, Message>
::next_layer() const
{
    return wrapped_socket;
}

template<class Socket, class Message>
asio::io_service& server_socket_adaptor<std::reference_wrapper<Socket>, Message>
::get_io_service()
{
    return wrapped_socket.get().get_io_service();
}

template<class Socket, class Message>
bool server_socket_adaptor<std::reference_wrapper<Socket>, Message>
::is_open() const
{
    return wrapped_socket.get().is_open();
}

template<class Socket, class Message>
read_state server_socket_adaptor<std::reference_wrapper<Socket>, Message>
::read_state() const
{
    return wrapped_socket.get().read_state();
}

template<class Socket, class Message>
write_state server_socket_adaptor<std::reference_wrapper<Socket>, Message>
::write_state() const
{
    return wrapped_socket.get().write_state();
}

template<class Socket, class Message>
bool server_socket_adaptor<std::reference_wrapper<Socket>, Message>
::write_response_native_stream() const
{
    return wrapped_socket.get().write_response_native_stream();
}

template<class Socket, class Message>
void server_socket_adaptor<std::reference_wrapper<Socket>, Message>
::async_read_request(std::string &method, std::string &path,
                     message_type &message, callback_type handler)
{
    wrapped_socket.get().async_read_request(method, path, message, handler);
}

template<class Socket, class Message>
void server_socket_adaptor<std::reference_wrapper<Socket>, Message>
::async_read_some(message_type &message, callback_type handler)
{
    wrapped_socket.get().async_read_some(message, handler);
}

template<class Socket, class Message>
void server_socket_adaptor<std::reference_wrapper<Socket>, Message>
::async_read_trailers(message_type &message, callback_type handler)
{
    wrapped_socket.get().async_read_trailers(message, handler);
}

template<class Socket, class Message>
void server_socket_adaptor<std::reference_wrapper<Socket>, Message>
::async_write_response(std::uint_fast16_t status_code,
                       const boost::string_ref &reason_phrase,
                       const message_type &message, callback_type handler)
{
    wrapped_socket.get().async_write_response(status_code, reason_phrase,
                                              message, handler);
}

template<class Socket, class Message>
void server_socket_adaptor<std::reference_wrapper<Socket>, Message>
::async_write_response_continue(callback_type handler)
{
    wrapped_socket.get().async_write_response_continue(handler);
}

template<class Socket, class Message>
void server_socket_adaptor<std::reference_wrapper<Socket>, Message>
::async_write_response_metadata(std::uint_fast16_t status_code,
                                const boost::string_ref &reason_phrase,
                                const message_type &message,
                                callback_type handler)
{
    wrapped_socket.get().async_write_response_metadata(status_code,
                                                       reason_phrase, message,
                                                       handler);
}

template<class Socket, class Message>
void server_socket_adaptor<std::reference_wrapper<Socket>, Message>
::async_write(const message_type &message, callback_type handler)
{
    wrapped_socket.get().async_write(message, handler);
}

template<class Socket, class Message>
void server_socket_adaptor<std::reference_wrapper<Socket>, Message>
::async_write_trailers(const message_type &message, callback_type handler)
{
    wrapped_socket.get().async_write_trailers(message, handler);
}

template<class Socket, class Message>
void server_socket_adaptor<std::reference_wrapper<Socket>, Message>
::async_write_end_of_message(callback_type handler)
{
    wrapped_socket.get().async_write_end_of_message(handler);
}

} // namespace http
} // namespace boost
