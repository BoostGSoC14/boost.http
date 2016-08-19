/* Copyright (c) 2016 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */


#ifndef BOOST_HTTP_SYNTAX_FIELD_VALUE_HPP
#define BOOST_HTTP_SYNTAX_FIELD_VALUE_HPP

#include <boost/utility/string_ref.hpp>
#include <boost/http/syntax/detail/is_ows.hpp>
#include <boost/http/syntax/detail/is_vchar.hpp>
#include <boost/http/syntax/detail/is_obs_text.hpp>

namespace boost {
namespace http {
namespace syntax {

template<class CharT>
struct left_trimmed_field_value {
    typedef basic_string_ref<CharT> view_type;

    static std::size_t match(view_type view);
};

} // namespace syntax
} // namespace http
} // namespace boost

#include "field_value.ipp"

#endif // BOOST_HTTP_SYNTAX_FIELD_VALUE_HPP
