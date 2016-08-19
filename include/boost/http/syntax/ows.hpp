/* Copyright (c) 2016 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */


#ifndef BOOST_HTTP_SYNTAX_OWS_HPP
#define BOOST_HTTP_SYNTAX_OWS_HPP

#include <boost/utility/string_ref.hpp>
#include <boost/http/syntax/detail/is_ows.hpp>

namespace boost {
namespace http {
namespace syntax {

template<class CharT>
struct ows {
    typedef basic_string_ref<CharT> view_type;

    static std::size_t match(view_type view);
};

} // namespace syntax
} // namespace http
} // namespace boost

#include "ows.ipp"

#endif // BOOST_HTTP_SYNTAX_OWS_HPP
