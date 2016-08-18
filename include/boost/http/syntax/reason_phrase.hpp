/* Copyright (c) 2016 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */


#ifndef BOOST_HTTP_SYNTAX_REASON_PHRASE_HPP
#define BOOST_HTTP_SYNTAX_REASON_PHRASE_HPP

#include <boost/utility/string_ref.hpp>
#include <boost/http/syntax/detail/is_ows.hpp>
#include <boost/http/syntax/detail/is_vchar.hpp>
#include <boost/http/syntax/detail/is_obs_text.hpp>

namespace boost {
namespace http {
namespace syntax {

template<class CharT>
struct reason_phrase {
    typedef basic_string_ref<CharT> view_type;

    static std::size_t match(view_type view);
};

} // namespace syntax
} // namespace http
} // namespace boost

#include "reason_phrase.ipp"

#endif // BOOST_HTTP_SYNTAX_REASON_PHRASE_HPP
