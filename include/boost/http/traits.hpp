/* Copyright (c) 2015 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_TRAITS_HPP
#define BOOST_HTTP_TRAITS_HPP

#include <type_traits>

namespace boost {
namespace http {

template<class T>
struct is_message: public std::false_type {};

template<class T>
struct is_server_socket: public std::false_type {};

/* Will be refined later. It'll be something like:

   is_server_socket<T> || is_client_socket<T> */
template<class T>
struct is_socket
    : public std::integral_constant<bool, is_server_socket<T>::value>
{};

} // namespace http
} // namespace boost

#endif /* BOOST_HTTP_TRAITS_HPP */
