/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_SERVER_HTTP_ERRC_H
#define BOOST_HTTP_SERVER_HTTP_ERRC_H

#include <boost/http/http_category.hpp>

namespace boost {

namespace http {

/**
 * The standard error codes reported by HTTP message producers and consumers.
 */
enum class http_errc {
    out_of_order = 1,
    native_stream_unsupported,
    parsing_error,
    // This error should only happen if a poor parser is used
    buffer_exhausted
};

/**
 * Constructs a http error_code.
 */
inline boost::system::error_code make_error_code(http_errc e)
{
    return boost::system::error_code(static_cast<int>(e), http_category());
}

/**
 * Constructs a http error_condition.
 */
inline boost::system::error_condition make_error_condition(http_errc e)
{
    return boost::system::error_condition(static_cast<int>(e), http_category());
}

} // namespace http

namespace system {

/**
 * Extends the type trait boost::system::is_error_code_enum to identify http
 * error codes.
 */
template<>
struct is_error_code_enum<boost::http::http_errc>: public std::true_type {};

/**
 * Extends the type trait boost::system::is_error_code_enum to identify http
 * error conditions.
 */
template<>
struct is_error_condition_enum<boost::http::http_errc>: public std::true_type {};

} // namespace system

} // namespace boost

#endif // BOOST_HTTP_SERVER_HTTP_ERRC_H
