/* Copyright (c) 2017 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_REQUEST_RESPONSE_WRAPPER_HPP
#define BOOST_HTTP_REQUEST_RESPONSE_WRAPPER_HPP

#include <type_traits>
#include <boost/http/traits.hpp>

namespace boost {
namespace http {

template<class Request, class Response>
struct request_response_wrapper
{
    static_assert(std::is_same<typename Request::headers_type,
                               typename Response::headers_type>
                  ::value,
                  "headers are incompatible");
    static_assert(std::is_same<typename Request::body_type,
                               typename Response::body_type>
                  ::value,
                  "bodies are incompatible");
    static_assert(std::is_const<Request>::value
                  == std::is_const<Response>::value,
                  "constness between Request and Response are incompatible");

    typedef
    typename std::conditional<
        std::is_const<Request>::value,
        const typename Request::headers_type,
        typename Request::headers_type
        >::type
    headers_type;

    typedef
    typename std::conditional<
        std::is_const<Request>::value,
        const typename Request::body_type,
        typename Request::body_type
        >::type
    body_type;

    request_response_wrapper(Request &request);
    request_response_wrapper(Response &response);

    template<class Request2, class Response2>
    request_response_wrapper(request_response_wrapper<Request2, Response2>
                             &other);

    headers_type &headers();

    const headers_type &headers() const;

    body_type &body();

    const body_type &body() const;

    headers_type &trailers();

    const headers_type &trailers() const;

private:
    headers_type &headers_;
    body_type &body_;
    headers_type &trailers_;
};

template<class Request, class Response>
struct is_message<request_response_wrapper<Request, Response>>
    : public std::true_type
{};

} // namespace http
} // namespace boost

#include "request_response_wrapper.ipp"

#endif // BOOST_HTTP_REQUEST_RESPONSE_WRAPPER_HPP
