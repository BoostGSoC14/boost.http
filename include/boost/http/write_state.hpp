/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_WRITE_STATE_HPP
#define BOOST_HTTP_WRITE_STATE_HPP

namespace boost {
namespace http {

enum class write_state
{
    empty,
    continue_issued,
    metadata_issued,
    finished
};

} // namespace http
} // namespace boost

#endif // BOOST_HTTP_WRITE_STATE_HPP
