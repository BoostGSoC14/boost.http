/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_MESSAGE_HPP
#define BOOST_HTTP_MESSAGE_HPP

#include <cstdint>
#include <boost/asio/buffer.hpp>
#include "headers.hpp"
#include <boost/http/traits.hpp>

namespace boost {
namespace http {

template<class Headers, class Body>
struct basic_message
{
    typedef Headers headers_type;
    typedef Body body_type;

    headers_type &headers();

    const headers_type &headers() const;

    body_type &body();

    const body_type &body() const;

    headers_type &trailers();

    const headers_type &trailers() const;

private:
    headers_type headers_;
    body_type body_;
    headers_type trailers_;
};

typedef basic_message<boost::http::headers, std::vector<std::uint8_t>> message;

template<class Headers, class Body>
struct is_message<basic_message<Headers, Body>>: public std::true_type {};

} // namespace http
} // namespace boost

#include "message-inl.hpp"

#endif // BOOST_HTTP_MESSAGE_HPP
