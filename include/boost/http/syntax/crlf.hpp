/* Copyright (c) 2016 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */


#ifndef BOOST_HTTP_SYNTAX_CRLF_HPP
#define BOOST_HTTP_SYNTAX_CRLF_HPP

#include <boost/utility/string_ref.hpp>
#include <boost/core/scoped_enum.hpp>

namespace boost {
namespace http {
namespace syntax {

template<class CharT>
struct strict_crlf {
    typedef basic_string_ref<CharT> view_type;

    static std::size_t match(view_type view);
};

template<class CharT>
struct liberal_crlf {
    typedef basic_string_ref<CharT> view_type;

    BOOST_SCOPED_ENUM_DECLARE_BEGIN(result)
    {
        crlf,
        lf,
        insufficient_data,
        invalid_data,
    }
    BOOST_SCOPED_ENUM_DECLARE_END(result)

    static result match(view_type view);
};

} // namespace syntax
} // namespace http
} // namespace boost

#include "crlf.ipp"

#endif // BOOST_HTTP_SYNTAX_CRLF_HPP
