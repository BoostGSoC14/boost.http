/* Copyright (c) 2017 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

namespace boost {
namespace http {

template<class Request, class Response>
request_response_wrapper<Request, Response>
::request_response_wrapper(Request &request)
    : headers_(request.headers())
    , body_(request.body())
    , trailers_(request.trailers())
{}

template<class Request, class Response>
request_response_wrapper<Request, Response>
::request_response_wrapper(Response &response)
    : headers_(response.headers())
    , body_(response.body())
    , trailers_(response.trailers())
{}

template<class Request, class Response>
template<class Request2, class Response2>
request_response_wrapper<Request, Response>
::request_response_wrapper(request_response_wrapper<Request2, Response2> &other)
    : headers_(other.headers())
    , body_(other.body())
    , trailers_(other.trailers())
{}

template<class Request, class Response>
auto request_response_wrapper<Request, Response>::headers() -> headers_type&
{
    return headers_;
}

template<class Request, class Response>
auto request_response_wrapper<Request, Response>::headers() const
    -> const headers_type&
{
    return headers_;
}

template<class Request, class Response>
auto request_response_wrapper<Request, Response>::body() -> body_type&
{
    return body_;
}

template<class Request, class Response>
auto request_response_wrapper<Request, Response>::body() const
    -> const body_type&
{
    return body_;
}

template<class Request, class Response>
auto request_response_wrapper<Request, Response>::trailers()
    -> headers_type&
{
    return trailers_;
}

template<class Request, class Response>
auto request_response_wrapper<Request, Response>::trailers() const
    -> const headers_type&
{
    return trailers_;
}

} // namespace http
} // namespace boost
