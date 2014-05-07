/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_SERVER_HTTP_CATEGORY_H
#define BOOST_HTTP_SERVER_HTTP_CATEGORY_H

#include <system_error>

namespace boost {
namespace http {

const std::error_category& http_category();

} // namespace http
} // namespace boost

#endif // BOOST_HTTP_SERVER_HTTP_CATEGORY_H
