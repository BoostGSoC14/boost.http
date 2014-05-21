/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#include <type_traits>

namespace boost {
namespace http {

template<class T>
struct is_message: public std::true_type {}; //< TODO: refine concept

namespace detail {

template <typename T, typename Enable = std::true_type>
struct has_outgoing_state : std::false_type
{};

template <typename T>
struct has_outgoing_state
<T, typename std::is_convertible<decltype(static_cast<T const*>(nullptr)
                                          ->outgoing_state()),
                                 boost::http::outgoing_state>::type>
  : std::true_type
{};

template<class T>
constexpr bool is_http_socket_helper()
{
    return is_message<T>::value && has_outgoing_state<T>::value;
}

} // namespace detail

template<class T>
struct is_socket
    : std::integral_constant<bool, detail::is_http_socket_helper<T>()>
{};

} // namespace http
} // namespace boost

