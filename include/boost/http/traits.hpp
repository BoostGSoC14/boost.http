/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#include <type_traits>

namespace boost {
namespace http {

template<class T>
struct is_message: public std::true_type {}; //< TODO: refine concept

namespace detail {

template<class T, bool>
struct has_outgoing_state_helper
{
    static constexpr bool value = false;
};

template<class T>
struct has_outgoing_state_helper<T, true>
{
    static constexpr bool f(...) {return false;}

    static constexpr bool f(decltype((static_cast<T const*>(0))->outgoing_state())*)
    {
        return std::is_same<decltype((static_cast<T*>(0))->outgoing_state()),
                            boost::http::outgoing_state>::value;
    }

    static constexpr bool value = f(0);
};

template<class T, bool = is_class<T>::value /* TODO: make it work on is_final<T>::value */>
struct has_outgoing_state
{
    static bool constexpr value = false;
};

template<class T>
struct has_outgoing_state<T, true>
{
    struct Fallback {
        int outgoing_state;
    };
 
    struct Derived : T, Fallback { };
 
    template<typename C, C>
    struct ChT;
 
    template<typename C>
    static std::false_type f(ChT<int Fallback::*, &C::outgoing_state>*);

    template<typename C>
    static std::true_type f(...);

    static bool constexpr value2 = decltype(f<Derived>(0))::value;

    static bool constexpr value = value2 && has_outgoing_state_helper<T, value2>::value;
};

template<class T>
bool constexpr is_http_socket_helper()
{
    return is_message<T>::value
        && has_outgoing_state<T>::value;
}

} // namespace detail

template<class T>
struct is_socket
    : public std::conditional<detail::is_http_socket_helper<T>(),
                              std::true_type, std::false_type>::type
{};

} // namespace http
} // namespace boost
