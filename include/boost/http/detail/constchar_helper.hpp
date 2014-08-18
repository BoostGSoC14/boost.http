/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_DETAIL_CONSTCHAR_HELPER_HPP
#define BOOST_HTTP_DETAIL_CONSTCHAR_HELPER_HPP

#include <boost/asio/buffer.hpp>

namespace boost {
namespace http {
namespace detail {

struct constchar_helper
{
    template<std::size_t N>
    constexpr constchar_helper(const char (&input)[N]) :
        data(input),
        size(N - 1)
    {}

    template<class T>
    const T *begin() const
    {
        return reinterpret_cast<const T*>(data);
    }

    template<class T>
    const T *end() const
    {
        return reinterpret_cast<const T*>(data+size);
    }

    const char *data;
    const std::size_t size;
};

template<std::size_t N>
asio::const_buffers_1 string_literal_buffer(const char (&input)[N])
{
    return asio::buffer(input, N - 1);
}

} // namespace detail
} // namespace http
} // namespace boost

#endif // BOOST_HTTP_DETAIL_CONSTCHAR_HELPER_HPP
