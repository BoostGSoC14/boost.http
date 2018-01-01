/* Copyright (c) 2017 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_DETAIL_ATOMIC_FIELD_PARSER_HPP
#define BOOST_HTTP_DETAIL_ATOMIC_FIELD_PARSER_HPP

#include <boost/http/reader/request.hpp>
#include <boost/http/reader/response.hpp>

// Here, we have parsers where field is atomic. In other words, if field (be it
// trailer or non-trailer) is present, it is complete (i.e. name + value). If it
// is absent or half-complete, it is translated to `error_insufficient_data` and
// no value is exposed until it is complete.

namespace boost {
namespace http {
namespace detail {

template<typename... Ts> struct make_void { typedef void type;};
template<typename... Ts> using void_t = typename make_void<Ts...>::type;

template<class T, class = std::void_t<>>
struct guarantee_response_fns
{
    static void set_method(T&, string_ref) {}
    static void puteof(T&) {}
};

template<class T>
struct guarantee_response_fns<T, void_t<decltype(std::declval<T>().puteof())>>
{
    static void set_method(T &p, string_ref m)
    {
        p.set_method(m);
    }

    static void puteof(T &p)
    {
        p.puteof();
    }
};

template<class T>
struct reader_with_atomic_field: private T
{
    using typename T::size_type;
    using typename T::value_type;
    using typename T::pointer;
    using typename T::view_type;

    using T::reset;
    using T::token_size;
    using T::value;
    using T::expected_token;
    using T::next;
    using T::set_buffer;
    using T::parsed_count;

    token::code::value code() const
    {
        const auto code_ = T::code();
        auto copy = T(static_cast<const T&>(*this));

        switch (code_) {
        case token::code::field_name:
        case token::code::trailer_name:
            copy.next();

            if (copy.code() == token::code::skip)
                copy.next();

            switch (copy.code()) {
            case token::code::error_insufficient_data:
                return token::code::error_insufficient_data;
            case token::code::field_value:
            case token::code::trailer_value:
                return code_;
            default:
                return copy.code();
            }
        default:
            return code_;
        }
    }

    void puteof()
    {
        guarantee_response_fns<T>::puteof(*this);
    }

    void set_method(string_ref m)
    {
        guarantee_response_fns<T>::set_method(*this, m);
    }
};

} // namespace detail
} // namespace http
} // namespace boost

#endif // BOOST_HTTP_DETAIL_ATOMIC_FIELD_PARSER_HPP
