/* Copyright (c) 2017 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_RESPONSE_HPP
#define BOOST_HTTP_RESPONSE_HPP

#include <string>
#include <cstdint>
#include <boost/asio/buffer.hpp>
#include "headers.hpp"
#include <boost/http/traits.hpp>

namespace boost {
namespace http {

template<class String, class Headers, class Body>
struct basic_response
{
    typedef String string_type;
    typedef Headers headers_type;
    typedef Body body_type;

    std::uint_least16_t &status_code();

    const std::uint_least16_t &status_code() const;

    string_type &reason_phrase();

    const string_type &reason_phrase() const;

    headers_type &headers();

    const headers_type &headers() const;

    body_type &body();

    const body_type &body() const;

    headers_type &trailers();

    const headers_type &trailers() const;

private:
    std::uint_least16_t status_code_;
    string_type reason_phrase_;
    headers_type headers_;
    body_type body_;
    headers_type trailers_;
};

typedef basic_response<std::string, boost::http::headers,
                       std::vector<std::uint8_t>>
response;

template<class String, class Headers, class Body>
struct is_response_message<basic_response<String, Headers, Body>>
    : public std::true_type
{};

} // namespace http
} // namespace boost

#include "response.ipp"

#endif // BOOST_HTTP_RESPONSE_HPP
