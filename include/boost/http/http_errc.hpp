/* Copyright (c) 2014, 2016 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_HTTP_ERRC_HPP
#define BOOST_HTTP_HTTP_ERRC_HPP

#include <boost/system/error_code.hpp>
#include <boost/http/detail/singleton.hpp>

namespace boost {

namespace http {

enum class http_errc {
    out_of_order = 1,
    native_stream_unsupported,
    parsing_error,
    buffer_exhausted,
    wrong_direction
};

namespace detail {

class http_category_impl: public boost::system::error_category
{
public:
    const char* name() const noexcept override;
    std::string message(int condition) const noexcept override;
};

inline const char* http_category_impl::name() const noexcept
{
    return "http";
}

inline std::string http_category_impl::message(int condition) const noexcept
{
    switch (condition) {
    case static_cast<int>(http_errc::out_of_order):
        return "HTTP actions issued on the wrong order for some object";
    case static_cast<int>(http_errc::native_stream_unsupported):
        return "The underlying communication channel doesn't support native"
            " streaming. Use write_message instead.";
    case static_cast<int>(http_errc::parsing_error):
        return "The parser encountered an error";
    case static_cast<int>(http_errc::buffer_exhausted):
        return "Need more buffer space to complete an operation";
    case static_cast<int>(http_errc::wrong_direction):
        return "You're trying to use a server channel in client mode or vice"
            " versa!";
    default:
        return "undefined";
    }
}

} // namespace detail

inline const boost::system::error_category& http_category()
{
    return detail::singleton<detail::http_category_impl>::instance;
}

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
