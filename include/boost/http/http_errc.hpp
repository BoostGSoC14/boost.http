/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_SERVER_HTTP_ERRC_H
#define BOOST_HTTP_SERVER_HTTP_ERRC_H

#include "http_category.hpp"

namespace boost {
namespace http {

/**
 * The standard error codes reported by HTTP message producers and consumers.
 */
enum class http_errc {
    out_of_order = 1
};

/**
 * Constructs a http error_code.
 */
inline std::error_code make_error_code(http_errc e)
{
    return std::error_code(static_cast<int>(e), http_category());
}
/**
 * Constructs a http error_condition.
 */
inline std::error_condition make_error_condition(http_errc e)
{
    return std::error_condition(static_cast<int>(e), http_category());
}

} // namespace http
} // namespace boost

namespace std {

/**
 * Extends the type trait std::is_error_code_enum to identify http error codes.
 */
template<>
struct is_error_code_enum<boost::http::http_errc>: public std::true_type {};

} // namespace std

#endif // BOOST_HTTP_SERVER_HTTP_ERRC_H
