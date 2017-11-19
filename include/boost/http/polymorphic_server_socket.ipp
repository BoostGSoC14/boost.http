/* Copyright (c) 2014, 2017 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

namespace boost {
namespace http {

template<class Request, class Response, class Message>
template<class CompletionToken>
typename asio::async_result<
    typename asio::handler_type<CompletionToken,
                                void(system::error_code)>::type>::type
basic_polymorphic_server_socket<Request, Response, Message>
::async_read_request(request_server_type &request, CompletionToken &&token)
{
    typedef typename asio::handler_type<
        CompletionToken, void(system::error_code)>::type Handler;

    Handler handler(std::forward<CompletionToken>(token));

    asio::async_result<Handler> result(handler);

    async_read_request(request, callback_type(handler));

    return result.get();
}

template<class Request, class Response, class Message>
template<class CompletionToken>
typename asio::async_result<
    typename asio::handler_type<CompletionToken,
                                void(system::error_code)>::type>::type
basic_polymorphic_server_socket<Request, Response, Message>
::async_write_response(const response_server_type &response,
                       CompletionToken &&token)
{
    typedef typename asio::handler_type<
        CompletionToken, void(system::error_code)>::type Handler;

    Handler handler(std::forward<CompletionToken>(token));

    asio::async_result<Handler> result(handler);

    async_write_response(response, callback_type(handler));

    return result.get();
}

template<class Request, class Response, class Message>
template<class CompletionToken>
typename asio::async_result<
    typename asio::handler_type<CompletionToken,
                                void(system::error_code)>::type>::type
basic_polymorphic_server_socket<Request, Response, Message>
::async_write_response_continue(CompletionToken &&token)
{
    typedef typename asio::handler_type<
        CompletionToken, void(system::error_code)>::type Handler;

    Handler handler(std::forward<CompletionToken>(token));

    asio::async_result<Handler> result(handler);

    async_write_response_continue(callback_type(handler));

    return result.get();
}

template<class Request, class Response, class Message>
template<class CompletionToken>
typename asio::async_result<
    typename asio::handler_type<CompletionToken,
                                void(system::error_code)>::type>::type
basic_polymorphic_server_socket<Request, Response, Message>
::async_write_response_metadata(const response_server_type &response,
                                CompletionToken &&token)
{
    typedef typename asio::handler_type<
        CompletionToken, void(system::error_code)>::type Handler;

    Handler handler(std::forward<CompletionToken>(token));

    asio::async_result<Handler> result(handler);

    async_write_response_metadata(response, callback_type(handler));

    return result.get();
}

} // namespace http
} // namespace boost
