/* Copyright (c) 2014, 2018, 2020 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

namespace boost {
namespace http {

template<class Message>
basic_poly_socket_base<Message>::~basic_poly_socket_base() = default;

template<class Message>
template<class Handler>
void basic_poly_socket_base<Message>::async_read_some_initiation::operator()(
    Handler&& handler, std::reference_wrapper<message_type> message)
{
    self.async_read_some(
        message.get(), handler_type(std::forward<Handler>(handler)));
}

template<class Message>
template<class CToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CToken, void(system::error_code))
basic_poly_socket_base<Message>
::async_read_some(message_type& message, CToken&& token)
{
    return boost::asio::async_initiate<CToken, void(system::error_code)>(
        async_read_some_initiation{*this}, token, std::ref(message)
    );
}

template<class Message>
template<class Handler>
void basic_poly_socket_base<Message>::async_write_initiation::operator()(
    Handler&& handler, std::reference_wrapper<const message_type> message)
{
    self.async_write(
        message.get(), handler_type(std::forward<Handler>(handler)));
}

template<class Message>
template<class CToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CToken, void(system::error_code))
basic_poly_socket_base<Message>
::async_write(const message_type& message, CToken&& token)
{
    return boost::asio::async_initiate<CToken, void(system::error_code)>(
        async_write_initiation{*this}, token, std::ref(message)
    );
}

template<class Message>
template<class Handler>
void
basic_poly_socket_base<Message>::async_write_trailers_initiation::operator()(
    Handler&& handler, std::reference_wrapper<const message_type> message)
{
    self.async_write_trailers(
        message.get(), handler_type(std::forward<Handler>(handler)));
}

template<class Message>
template<class CToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CToken, void(system::error_code))
basic_poly_socket_base<Message>
::async_write_trailers(const message_type& message, CToken&& token)
{
    return boost::asio::async_initiate<CToken, void(system::error_code)>(
        async_write_trailers_initiation{*this}, token, std::ref(message)
    );
}

template<class Message>
template<class Handler>
void basic_poly_socket_base<Message>::async_write_end_of_message_initiation
::operator()(Handler&& handler)
{
    self.async_write_end_of_message(
        handler_type(std::forward<Handler>(handler)));
}

template<class Message>
template<class CToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CToken, void(system::error_code))
basic_poly_socket_base<Message>
::async_write_end_of_message(CToken&& token)
{
    return boost::asio::async_initiate<CToken, void(system::error_code)>(
        async_write_end_of_message_initiation{*this}, token
    );
}

} // namespace http
} // namespace boost
