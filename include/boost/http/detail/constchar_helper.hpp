/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_DETAIL_CONSTCHAR_HELPER_H
#define BOOST_HTTP_DETAIL_CONSTCHAR_HELPER_H

namespace boost {
namespace http {
namespace detail {

struct constchar_helper
{
    template<unsigned N>
    constchar_helper(const char (&input)[N]) :
        data(input),
        size(N - 1)
    {}

    const char *data;
    const unsigned size;
};

} // namespace detail
} // namespace http
} // namespace boost

#endif // BOOST_HTTP_DETAIL_CONSTCHAR_HELPER_H
