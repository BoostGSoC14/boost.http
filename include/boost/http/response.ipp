/* Copyright (c) 2017 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

namespace boost {
namespace http {

template<class String, class Headers, class Body>
std::uint_least16_t &basic_response<String, Headers, Body>::status_code()
{
    return status_code_;
}

template<class String, class Headers, class Body>
const std::uint_least16_t&
basic_response<String, Headers, Body>::status_code() const
{
    return status_code_;
}

template<class String, class Headers, class Body>
String &basic_response<String, Headers, Body>::reason_phrase()
{
    return reason_phrase_;
}

template<class String, class Headers, class Body>
const String &basic_response<String, Headers, Body>::reason_phrase() const
{
    return reason_phrase_;
}

template<class String, class Headers, class Body>
Headers &basic_response<String, Headers, Body>::headers()
{
    return headers_;
}

template<class String, class Headers, class Body>
const Headers &basic_response<String, Headers, Body>::headers() const
{
    return headers_;
}

template<class String, class Headers, class Body>
Body &basic_response<String, Headers, Body>::body()
{
    return body_;
}

template<class String, class Headers, class Body>
const Body &basic_response<String, Headers, Body>::body() const
{
    return body_;
}

template<class String, class Headers, class Body>
Headers &basic_response<String, Headers, Body>::trailers()
{
    return trailers_;
}

template<class String, class Headers, class Body>
const Headers &basic_response<String, Headers, Body>::trailers() const
{
    return trailers_;
}

} // namespace http
} // namespace boost
