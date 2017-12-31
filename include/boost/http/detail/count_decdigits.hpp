/* Copyright (c) 2016 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_HTTP_DETAIL_COUNT_DECDIGITS_HPP
#define BOOST_HTTP_HTTP_DETAIL_COUNT_DECDIGITS_HPP

#include <cstddef>

namespace boost {
namespace http {
namespace detail {

template<class T>
std::size_t count_decdigits(T n)
{
    if (n == 0)
        return 1;

    std::size_t ret = 0;
    while (n > 0) {
        n /= 10;
        ++ret;
    }
    return ret;
}

} // namespace detail
} // namespace http
} // namespace boost

#endif // BOOST_HTTP_HTTP_DETAIL_COUNT_DECDIGITS_HPP
