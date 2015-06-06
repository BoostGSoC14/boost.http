/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_HTTP_CATEGORY_HPP
#define BOOST_HTTP_HTTP_CATEGORY_HPP

#include <boost/http/detail/config.hpp>
#include <boost/system/error_code.hpp>

namespace boost {
namespace http {

BOOST_HTTP_DECL const boost::system::error_category& http_category();

} // namespace http
} // namespace boost

#endif // BOOST_HTTP_HTTP_CATEGORY_HPP
