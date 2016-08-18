/* Copyright (c) 2016 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

namespace boost {
namespace http {
namespace syntax {

namespace detail {

/* all valid field value characters except OWS */
template<class CharT>
bool is_nonnull_field_value_char(CharT c)
{
    return is_vchar(c) || is_obs_text(c);
}

template<class CharT>
bool is_field_value_char(CharT c)
{
    return is_nonnull_field_value_char(c) || is_ows(c);
}

} // namespace detail

template<class CharT>
std::size_t left_trimmed_field_value<CharT>::match(view_type view)
{
    std::size_t res = 0;

    for (std::size_t i = 0 ; i != view.size() ; ++i) {
        if (!detail::is_field_value_char(view[i]))
            break;

        ++res;
    }

    return res;
}

} // namespace syntax
} // namespace http
} // namespace boost
