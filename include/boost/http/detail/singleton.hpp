/* Copyright (c) 2016 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_HTTP_DETAIL_SINGLETON_HPP
#define BOOST_HTTP_HTTP_DETAIL_SINGLETON_HPP

namespace boost {
namespace http {
namespace detail {

template<class T, class Tag = void>
struct singleton
{
    static T instance;
};

template<class T, class Tag>
T singleton<T, Tag>::instance;

} // namespace detail
} // namespace http
} // namespace boost

#endif // BOOST_HTTP_HTTP_DETAIL_SINGLETON_HPP
