/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

namespace boost {
namespace http {

template<class Socket>
template<class... Args>
server_socket_adaptor<Socket>::server_socket_adaptor(Args&&... args)
    : Socket(std::forward<Args>(args)...)
{}

template<class Socket>
Socket &server_socket_adaptor<Socket>::next_layer()
{
    return *this;
}

template<class Socket>
const Socket &server_socket_adaptor<Socket>::next_layer() const
{
    return *this;
}

template<class Socket>
asio::io_service& server_socket_adaptor<Socket>::get_io_service()
{
    return Socket::get_io_service();
}

template<class Socket>
read_state server_socket_adaptor<Socket>::read_state() const
{
    return Socket::read_state();
}

template<class Socket>
write_state server_socket_adaptor<Socket>::write_state() const
{
    return Socket::write_state();
}

template<class Socket>
bool server_socket_adaptor<Socket>::write_response_native_stream() const
{
    return Socket::write_response_native_stream();
}

template<class Socket>
void server_socket_adaptor<Socket>::async_read_request(std::string &method,
                                                       std::string &path,
                                                       message &message,
                                                       callback_type handler)
{
    Socket::async_read_request(method, path, message, handler);
}

template<class Socket>
void server_socket_adaptor<Socket>::async_read_some(message &message,
                                                    callback_type handler)
{
    Socket::async_read_some(message, handler);
}

template<class Socket>
void server_socket_adaptor<Socket>::async_read_trailers(message &message,
                                                        callback_type handler)
{
    Socket::async_read_trailers(message, handler);
}

template<class Socket>
void server_socket_adaptor<Socket>
::async_write_response(std::uint_fast16_t status_code,
                       const boost::string_ref &reason_phrase,
                       const message &message, callback_type handler)
{
    Socket::async_write_response(status_code, reason_phrase, message, handler);
}

template<class Socket>
void server_socket_adaptor<Socket>
::async_write_response_continue(callback_type handler)
{
    Socket::async_write_response_continue(handler);
}

template<class Socket>
void server_socket_adaptor<Socket>
::async_write_response_metadata(std::uint_fast16_t status_code,
                                const boost::string_ref &reason_phrase,
                                const message &message, callback_type handler)
{
    Socket::async_write_response_metadata(status_code, reason_phrase, message,
                                          handler);
}

template<class Socket>
void server_socket_adaptor<Socket>
::async_write(const message &message, callback_type handler)
{
    Socket::async_write(message, handler);
}

template<class Socket>
void server_socket_adaptor<Socket>
::async_write_trailers(const message &message, callback_type handler)
{
    Socket::async_write_trailers(message, handler);
}

template<class Socket>
void server_socket_adaptor<Socket>
::async_write_end_of_message(callback_type handler)
{
    Socket::async_write_end_of_message(handler);
}

template<class Socket>
server_socket_adaptor<std::reference_wrapper<Socket>>
::server_socket_adaptor(Socket &socket)
    : wrapped_socket(socket)
{}

template<class Socket>
Socket &server_socket_adaptor<std::reference_wrapper<Socket>>::next_layer()
{
    return wrapped_socket;
}

template<class Socket>
const Socket &server_socket_adaptor<std::reference_wrapper<Socket>>
::next_layer() const
{
    return wrapped_socket;
}

template<class Socket>
asio::io_service& server_socket_adaptor<std::reference_wrapper<Socket>>
::get_io_service()
{
    return wrapped_socket.get().get_io_service();
}

template<class Socket>
read_state server_socket_adaptor<std::reference_wrapper<Socket>>
::read_state() const
{
    return wrapped_socket.get().read_state();
}

template<class Socket>
write_state server_socket_adaptor<std::reference_wrapper<Socket>>
::write_state() const
{
    return wrapped_socket.get().write_state();
}

template<class Socket>
bool server_socket_adaptor<std::reference_wrapper<Socket>>
::write_response_native_stream() const
{
    return wrapped_socket.get().write_response_native_stream();
}

template<class Socket>
void server_socket_adaptor<std::reference_wrapper<Socket>>
::async_read_request(std::string &method, std::string &path, message &message,
                     callback_type handler)
{
    wrapped_socket.get().async_read_request(method, path, message, handler);
}

template<class Socket>
void server_socket_adaptor<std::reference_wrapper<Socket>>
::async_read_some(message &message, callback_type handler)
{
    wrapped_socket.get().async_read_some(message, handler);
}

template<class Socket>
void server_socket_adaptor<std::reference_wrapper<Socket>>
::async_read_trailers(message &message, callback_type handler)
{
    wrapped_socket.get().async_read_trailers(message, handler);
}

template<class Socket>
void server_socket_adaptor<std::reference_wrapper<Socket>>
::async_write_response(std::uint_fast16_t status_code,
                       const boost::string_ref &reason_phrase,
                       const message &message, callback_type handler)
{
    wrapped_socket.get().async_write_response(status_code, reason_phrase,
                                              message, handler);
}

template<class Socket>
void server_socket_adaptor<std::reference_wrapper<Socket>>
::async_write_response_continue(callback_type handler)
{
    wrapped_socket.get().async_write_response_continue(handler);
}

template<class Socket>
void server_socket_adaptor<std::reference_wrapper<Socket>>
::async_write_response_metadata(std::uint_fast16_t status_code,
                                const boost::string_ref &reason_phrase,
                                const message &message, callback_type handler)
{
    wrapped_socket.get().async_write_response_metadata(status_code,
                                                       reason_phrase, message,
                                                       handler);
}

template<class Socket>
void server_socket_adaptor<std::reference_wrapper<Socket>>
::async_write(const message &message, callback_type handler)
{
    wrapped_socket.get().async_write(message, handler);
}

template<class Socket>
void server_socket_adaptor<std::reference_wrapper<Socket>>
::async_write_trailers(const message &message, callback_type handler)
{
    wrapped_socket.get().async_write_trailers(message, handler);
}

template<class Socket>
void server_socket_adaptor<std::reference_wrapper<Socket>>
::async_write_end_of_message(callback_type handler)
{
    wrapped_socket.get().async_write_end_of_message(handler);
}

} // namespace http
} // namespace boost
