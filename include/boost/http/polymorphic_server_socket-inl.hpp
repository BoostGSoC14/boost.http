/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

namespace boost {
namespace http {

template<class Message>
template<class CompletionToken>
typename asio::async_result<
    typename asio::handler_type<CompletionToken,
                                void(system::error_code)>::type>::type
basic_polymorphic_server_socket<Message>
::async_read_request(std::string &method, std::string &path,
                     message_type &message, CompletionToken &&token)
{
    typedef typename asio::handler_type<
        CompletionToken, void(system::error_code)>::type Handler;

    Handler handler(std::forward<CompletionToken>(token));

    asio::async_result<Handler> result(handler);

    async_read_request(method, path, message, callback_type(handler));

    return result.get();
}

template<class Message>
template<class CompletionToken>
typename asio::async_result<
    typename asio::handler_type<CompletionToken,
                                void(system::error_code)>::type>::type
basic_polymorphic_server_socket<Message>
::async_write_response(std::uint_fast16_t status_code,
                       const boost::string_ref &reason_phrase,
                       const message_type &message, CompletionToken &&token)
{
    typedef typename asio::handler_type<
        CompletionToken, void(system::error_code)>::type Handler;

    Handler handler(std::forward<CompletionToken>(token));

    asio::async_result<Handler> result(handler);

    async_write_response(status_code, reason_phrase, message,
                         callback_type(handler));

    return result.get();
}

template<class Message>
template<class CompletionToken>
typename asio::async_result<
    typename asio::handler_type<CompletionToken,
                                void(system::error_code)>::type>::type
basic_polymorphic_server_socket<Message>
::async_write_response_continue(CompletionToken &&token)
{
    typedef typename asio::handler_type<
        CompletionToken, void(system::error_code)>::type Handler;

    Handler handler(std::forward<CompletionToken>(token));

    asio::async_result<Handler> result(handler);

    async_write_response_continue(callback_type(handler));

    return result.get();
}

template<class Message>
template<class CompletionToken>
typename asio::async_result<
    typename asio::handler_type<CompletionToken,
                                void(system::error_code)>::type>::type
basic_polymorphic_server_socket<Message>
::async_write_response_metadata(std::uint_fast16_t status_code,
                                const boost::string_ref &reason_phrase,
                                const message_type &message,
                                CompletionToken &&token)
{
    typedef typename asio::handler_type<
        CompletionToken, void(system::error_code)>::type Handler;

    Handler handler(std::forward<CompletionToken>(token));

    asio::async_result<Handler> result(handler);

    async_write_response_metadata(status_code, reason_phrase, message,
                                  callback_type(handler));

    return result.get();
}

} // namespace http
} // namespace boost
