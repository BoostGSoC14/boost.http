/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#include <boost/http/embedded_server/embedded_server.hpp>

namespace boost {
namespace http {

outgoing_state basic_socket<embedded_server>::outgoing_state() const
{
    return ostate;
}

template class basic_socket<embedded_server>;

} // namespace http
} // namespace boost
