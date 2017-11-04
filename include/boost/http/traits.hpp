/* Copyright (c) 2015 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_TRAITS_HPP
#define BOOST_HTTP_TRAITS_HPP

#include <type_traits>

namespace boost {
namespace http {

template<class T>
struct is_request_message: public std::false_type {};

template<class T>
struct is_response_message: public std::false_type {};

template<class T>
struct is_message
    : public std::integral_constant<bool,
                                    is_request_message<T>::value
                                    || is_response_message<T>::value>
{};

template<class T>
struct is_server_socket: public std::false_type {};

template<class T>
struct is_client_socket: public std::false_type {};

template<class T>
struct is_socket
    : public std::integral_constant<bool,
                                    is_server_socket<T>::value
                                    || is_client_socket<T>::value>
{};

} // namespace http
} // namespace boost

#endif /* BOOST_HTTP_TRAITS_HPP */
