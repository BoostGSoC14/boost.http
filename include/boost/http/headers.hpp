/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_HEADERS_HPP
#define BOOST_HTTP_HEADERS_HPP

#include <boost/container/flat_map.hpp>
#include <string>

namespace boost {
namespace http {

typedef boost::container::flat_multimap<std::string, std::string> headers;

} // namespace http
} // namespace boost

#endif // BOOST_HTTP_HEADERS_HPP
