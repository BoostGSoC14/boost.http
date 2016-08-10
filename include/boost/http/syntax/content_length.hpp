/* Copyright (c) 2016 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */


#ifndef BOOST_HTTP_SYNTAX_CONTENT_LENGTH_HPP
#define BOOST_HTTP_SYNTAX_CONTENT_LENGTH_HPP

#include <cassert>

#include <boost/utility/string_ref.hpp>
#include <boost/core/scoped_enum.hpp>

namespace boost {
namespace http {
namespace syntax {

template<class CharT>
struct content_length {
    typedef basic_string_ref<CharT> view_type;

    BOOST_SCOPED_ENUM_DECLARE_BEGIN(result)
    {
        invalid,
        ok,
        overflow
    }
    BOOST_SCOPED_ENUM_DECLARE_END(result)

    template<class Target>
    static result decode(view_type in, Target &out);
};

} // namespace syntax
} // namespace http
} // namespace boost

#include "content_length.ipp"

#endif // BOOST_HTTP_SYNTAX_CONTENT_LENGTH_HPP
