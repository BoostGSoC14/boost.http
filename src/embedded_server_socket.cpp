/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#include <boost/http/embedded_server_socket.hpp>

namespace boost {
namespace http {

outgoing_state embedded_server_socket::outgoing_state() const
{
    return ostate;
}

} // namespace http
} // namespace boost
