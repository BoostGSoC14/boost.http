/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

namespace boost {
namespace http {

template<class Headers, class Body>
Headers &basic_message<Headers, Body>::headers()
{
    return headers_;
}

template<class Headers, class Body>
const Headers &basic_message<Headers, Body>::headers() const
{
    return headers_;
}

template<class Headers, class Body>
Body &basic_message<Headers, Body>::body()
{
    return body_;
}

template<class Headers, class Body>
const Body &basic_message<Headers, Body>::body() const
{
    return body_;
}

template<class Headers, class Body>
Headers &basic_message<Headers, Body>::trailers()
{
    return trailers_;
}

template<class Headers, class Body>
const Headers &basic_message<Headers, Body>::trailers() const
{
    return trailers_;
}

} // namespace http
} // namespace boost
