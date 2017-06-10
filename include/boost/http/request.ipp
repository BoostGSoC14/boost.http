/* Copyright (c) 2017 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

namespace boost {
namespace http {

template<class String, class Headers, class Body>
String &basic_request<String, Headers, Body>::method()
{
    return method_;
}

template<class String, class Headers, class Body>
const String &basic_request<String, Headers, Body>::method() const
{
    return method_;
}

template<class String, class Headers, class Body>
String &basic_request<String, Headers, Body>::target()
{
    return target_;
}

template<class String, class Headers, class Body>
const String &basic_request<String, Headers, Body>::target() const
{
    return target_;
}

template<class String, class Headers, class Body>
Headers &basic_request<String, Headers, Body>::headers()
{
    return headers_;
}

template<class String, class Headers, class Body>
const Headers &basic_request<String, Headers, Body>::headers() const
{
    return headers_;
}

template<class String, class Headers, class Body>
Body &basic_request<String, Headers, Body>::body()
{
    return body_;
}

template<class String, class Headers, class Body>
const Body &basic_request<String, Headers, Body>::body() const
{
    return body_;
}

template<class String, class Headers, class Body>
Headers &basic_request<String, Headers, Body>::trailers()
{
    return trailers_;
}

template<class String, class Headers, class Body>
const Headers &basic_request<String, Headers, Body>::trailers() const
{
    return trailers_;
}

} // namespace http
} // namespace boost
