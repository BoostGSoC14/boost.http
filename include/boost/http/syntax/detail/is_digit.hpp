/* Copyright (c) 2016 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */


#ifndef BOOST_HTTP_SYNTAX_DETAIL_IS_DIGIT_HPP
#define BOOST_HTTP_SYNTAX_DETAIL_IS_DIGIT_HPP

namespace boost {
namespace http {
namespace syntax {
namespace detail {

template<class CharT>
bool is_digit(CharT c)
{
    /* DIGIT          =  %x30-39   ; 0-9

       from Appendix B of RFC5234. */
    return c >= 0x30 && c <= 0x39;
}

} // namespace detail
} // namespace syntax
} // namespace http
} // namespace boost

#endif // BOOST_HTTP_SYNTAX_DETAIL_IS_DIGIT_HPP
