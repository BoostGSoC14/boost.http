/* Copyright (c) 2017 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_REQUEST_HPP
#define BOOST_HTTP_REQUEST_HPP

#include <string>
#include <cstdint>
#include <boost/asio/buffer.hpp>
#include "headers.hpp"
#include <boost/http/traits.hpp>

namespace boost {
namespace http {

template<class String, class Headers, class Body>
struct basic_request
{
    typedef String string_type;
    typedef Headers headers_type;
    typedef Body body_type;

    string_type &method();

    const string_type &method() const;

    string_type &target();

    const string_type &target() const;

    headers_type &headers();

    const headers_type &headers() const;

    body_type &body();

    const body_type &body() const;

    headers_type &trailers();

    const headers_type &trailers() const;

private:
    string_type method_;
    string_type target_;
    headers_type headers_;
    body_type body_;
    headers_type trailers_;
};

typedef basic_request<std::string, boost::http::headers,
                      std::vector<std::uint8_t>>
request;

template<class String, class Headers, class Body>
struct is_request_message<basic_request<String, Headers, Body>>
    : public std::true_type
{};

} // namespace http
} // namespace boost

#include "request.ipp"

#endif // BOOST_HTTP_REQUEST_HPP
