/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_BASIC_SOCKET_ACCEPTOR_H
#define BOOST_HTTP_BASIC_SOCKET_ACCEPTOR_H

#include <functional>
#include "basic_socket.hpp"

namespace boost {
namespace http {

template<class Protocol>
class basic_socket_acceptor
{
public:
    typedef Protocol protocol_type;

    // TODO: this typedef should NOT be part of the acceptor concept. refine
    // into a name more unlikely to collide.
    typedef std::function<void(const boost::system::error_code&)> handler_type;

    void accept(basic_socket<protocol_type> &socket);

    void async_accept(basic_socket<protocol_type> &socket,
                      handler_type handler);
};

} // namespace http
} // namespace boost

#endif // BOOST_HTTP_BASIC_SOCKET_ACCEPTOR_H
