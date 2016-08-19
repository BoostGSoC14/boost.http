/* Copyright (c) 2016 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */


#ifndef BOOST_HTTP_SYNTAX_DETAIL_IS_OWS_HPP
#define BOOST_HTTP_SYNTAX_DETAIL_IS_OWS_HPP

namespace boost {
namespace http {
namespace syntax {
namespace detail {

template<class CharT>
bool is_ows(CharT c)
{
    switch (c) {
    case '\x20':
    case '\x09':
        return true;
    default:
        return false;
    }
}

} // namespace detail
} // namespace syntax
} // namespace http
} // namespace boost

#endif // BOOST_HTTP_SYNTAX_DETAIL_IS_OWS_HPP
