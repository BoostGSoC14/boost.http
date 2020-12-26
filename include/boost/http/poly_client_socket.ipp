/* Copyright (c) 2018, 2020 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

namespace boost {
namespace http {

template<class Request, class Response, class Message>
template<class Handler>
void basic_poly_client_socket<Request, Response, Message>
::async_write_request_initiation::operator()(
    Handler&& handler, std::reference_wrapper<const request_type> request)
{
    self.async_write_request(
        request.get(), handler_type{std::forward<Handler>(handler)});
}

template<class Request, class Response, class Message>
template<class CToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CToken, void(system::error_code))
basic_poly_client_socket<Request, Response, Message>
::async_write_request(const request_type& request, CToken&& token)
{
    return boost::asio::async_initiate<CToken, void(system::error_code)>(
        async_write_request_initiation{*this}, token, std::ref(request)
    );
}

template<class Request, class Response, class Message>
template<class Handler>
void basic_poly_client_socket<Request, Response, Message>
::async_write_request_metadata_initiation::operator()(
    Handler&& handler, std::reference_wrapper<const request_type> request)
{
    self.async_write_request_metadata(
        request.get(), handler_type{std::forward<Handler>(handler)});
}

template<class Request, class Response, class Message>
template<class CToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CToken, void(system::error_code))
basic_poly_client_socket<Request, Response, Message>
::async_write_request_metadata(const request_type& request, CToken&& token)
{
    return boost::asio::async_initiate<CToken, void(system::error_code)>(
        async_write_request_metadata_initiation{*this}, token, std::ref(request)
    );
}

template<class Request, class Response, class Message>
template<class Handler>
void basic_poly_client_socket<Request, Response, Message>
::async_read_response_initiation::operator()(
    Handler&& handler, std::reference_wrapper<response_type> response)
{
    self.async_read_response(
        response.get(), handler_type(std::forward<Handler>(handler)));
}

template<class Request, class Response, class Message>
template<class CToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CToken, void(system::error_code))
basic_poly_client_socket<Request, Response, Message>
::async_read_response(response_type& response, CToken&& token)
{
    return boost::asio::async_initiate<CToken, void(system::error_code)>(
        async_read_response_initiation{*this}, token, std::ref(response)
    );
}

} // namespace http
} // namespace boost
