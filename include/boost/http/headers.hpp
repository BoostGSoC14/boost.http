/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_HEADERS_H
#define BOOST_HTTP_HEADERS_H

#include <boost/container/flat_map.hpp>
#include <string>

namespace boost {
namespace http {

// TODO: Define the `headers` concept and evaluate which container is more
// appropriate

// I think the container shouldn't be sorted. you only pay for what you use (?)
typedef boost::container::flat_multimap<std::string, std::string> headers;

} // namespace http
} // namespace boost

#endif // BOOST_HTTP_HEADERS_H
