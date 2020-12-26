/* Copyright (c) 2014, 2017, 2018, 2020 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

namespace boost {
namespace http {

template<class Request, class Response, class Message>
template<class Handler>
void basic_poly_server_socket<Request, Response, Message>
::async_read_request_initiation::operator()(
    Handler&& handler, std::reference_wrapper<request_type> request)
{
    self.async_read_request(
        request.get(), handler_type(std::forward<Handler>(handler)));
}

template<class Request, class Response, class Message>
template<class CToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CToken, void(system::error_code))
basic_poly_server_socket<Request, Response, Message>
::async_read_request(request_type& request, CToken&& token)
{
    return boost::asio::async_initiate<CToken, void(system::error_code)>(
        async_read_request_initiation{*this}, token, std::ref(request)
    );
}

template<class Request, class Response, class Message>
template<class Handler>
void basic_poly_server_socket<Request, Response, Message>
::async_write_response_initiation::operator()(
    Handler&& handler, std::reference_wrapper<const response_type> response)
{
    self.async_write_response(
        response.get(), handler_type(std::forward<Handler>(handler)));
}

template<class Request, class Response, class Message>
template<class CToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CToken, void(system::error_code))
basic_poly_server_socket<Request, Response, Message>
::async_write_response(const response_type& response, CToken&& token)
{
    return boost::asio::async_initiate<CToken, void(system::error_code)>(
        async_write_response_initiation{*this}, token, std::ref(response)
    );
}

template<class Request, class Response, class Message>
template<class Handler>
void basic_poly_server_socket<Request, Response, Message>
::async_write_response_continue_initiation::operator()(Handler&& handler)
{
    self.async_write_response_continue(
        handler_type(std::forward<Handler>(handler)));
}

template<class Request, class Response, class Message>
template<class CToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CToken, void(system::error_code))
basic_poly_server_socket<Request, Response, Message>
::async_write_response_continue(CToken&& token)
{
    return boost::asio::async_initiate<CToken, void(system::error_code)>(
        async_write_response_continue_initiation{*this}, token
    );
}

template<class Request, class Response, class Message>
template<class Handler>
void basic_poly_server_socket<Request, Response, Message>
::async_write_response_metadata_initiation::operator()(
    Handler&& handler, std::reference_wrapper<const response_type> response)
{
    self.async_write_response_metadata(
        response.get(), handler_type(std::forward<Handler>(handler))
    );
}

template<class Request, class Response, class Message>
template<class CToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CToken, void(system::error_code))
basic_poly_server_socket<Request, Response, Message>
::async_write_response_metadata(const response_type& response,
                                CToken&& token)
{
    return boost::asio::async_initiate<CToken, void(system::error_code)>(
        async_write_response_metadata_initiation{*this}, token,
        std::ref(response)
    );
}

} // namespace http
} // namespace boost
