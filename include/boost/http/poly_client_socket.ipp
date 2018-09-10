/* Copyright (c) 2018 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

namespace boost {
namespace http {

template<class Request, class Response, class Message>
template<class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(system::error_code))
basic_poly_client_socket<Request, Response, Message>
::async_write_request(const request_type &request, CompletionToken &&token)
{
    boost::asio::async_completion<CompletionToken, void(system::error_code)>
        init{token};
    async_write_request(request, handler_type{init.completion_handler});
    return init.result.get();
}

template<class Request, class Response, class Message>
template<class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(system::error_code))
basic_poly_client_socket<Request, Response, Message>
::async_write_request_metadata(const request_type &request,
                               CompletionToken &&token)
{
    boost::asio::async_completion<CompletionToken, void(system::error_code)>
        init{token};
    async_write_request_metadata(request,
                                 handler_type{init.completion_handler});
    return init.result.get();
}

template<class Request, class Response, class Message>
template<class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(system::error_code))
basic_poly_client_socket<Request, Response, Message>
::async_read_response(response_type &response, CompletionToken &&token)
{
    boost::asio::async_completion<CompletionToken, void(system::error_code)>
        init{token};
    async_read_response(response, handler_type(init.completion_handler));
    return init.result.get();
}

} // namespace http
} // namespace boost
