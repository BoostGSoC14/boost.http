/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_HTTP_ERRC_HPP
#define BOOST_HTTP_HTTP_ERRC_HPP

#include <boost/http/http_category.hpp>

namespace boost {

namespace http {

enum class http_errc {
    out_of_order = 1,
    native_stream_unsupported,
    parsing_error,
    buffer_exhausted,
    wrong_direction
};

inline boost::system::error_code make_error_code(http_errc e)
{
    return boost::system::error_code(static_cast<int>(e), http_category());
}

inline boost::system::error_condition make_error_condition(http_errc e)
{
    return boost::system::error_condition(static_cast<int>(e), http_category());
}

} // namespace http

namespace system {

template<>
struct is_error_code_enum<boost::http::http_errc>: public std::true_type {};

template<>
struct is_error_condition_enum<boost::http::http_errc>: public std::true_type {};

} // namespace system

} // namespace boost

#endif // BOOST_HTTP_HTTP_ERRC_HPP
