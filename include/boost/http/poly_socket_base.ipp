/* Copyright (c) 2014, 2018 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

namespace boost {
namespace http {

template<class Message>
basic_poly_socket_base<Message>::~basic_poly_socket_base() = default;

template<class Message>
template<class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(system::error_code))
basic_poly_socket_base<Message>
::async_read_some(message_type &message, CompletionToken &&token)
{
    boost::asio::async_completion<CompletionToken, void(system::error_code)>
        init{token};
    async_read_some(message, handler_type(init.completion_handler));
    return init.result.get();
}

template<class Message>
template<class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(system::error_code))
basic_poly_socket_base<Message>
::async_write(const message_type &message, CompletionToken &&token)
{
    boost::asio::async_completion<CompletionToken, void(system::error_code)>
        init{token};
    async_write(message, handler_type(init.completion_handler));
    return init.result.get();
}

template<class Message>
template<class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(system::error_code))
basic_poly_socket_base<Message>
::async_write_trailers(const message_type &message, CompletionToken &&token)
{
    boost::asio::async_completion<CompletionToken, void(system::error_code)>
        init{token};
    async_write_trailers(message, handler_type(init.completion_handler));
    return init.result.get();
}

template<class Message>
template<class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(system::error_code))
basic_poly_socket_base<Message>
::async_write_end_of_message(CompletionToken &&token)
{
    boost::asio::async_completion<CompletionToken, void(system::error_code)>
        init{token};
    async_write_end_of_message(handler_type(init.completion_handler));
    return init.result.get();
}

} // namespace http
} // namespace boost
