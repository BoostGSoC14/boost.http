/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_SERVER_SOCKET_ADAPTOR_HPP
#define BOOST_HTTP_SERVER_SOCKET_ADAPTOR_HPP

#include <boost/http/polymorphic_server_socket.hpp>

namespace boost {
namespace http {

/**
 * This design was chosen taking a number of shortcomings in consideration.
 *
 * If the user can control object construction, he might benefit by less levels
 * of indirection by constructing both (the HTTP socket and its runtime-based
 * polymorphic adaptor) at once and at a single memory space.
 *
 * Also, if the user needs to query for the specific type at runtime, the user
 * can do so with a single call to dynamic_cast to the specific polymorphic
 * wrapper (rather than calling a second function to query for the wrapped
 * object).
 *
 * The scenario where the user don't control the object construction was also
 * taken in consideration. In these cases, it's possible to use the
 * server_socket_reference_adaptor.
 *
 * Although very different, the name and inspiration were borrowed from N3525
 * (polymorphic allocators).
 */
template<class Socket>
class server_socket_adaptor: public polymorphic_server_socket, private Socket
{
public:
    typedef Socket next_layer_type;

    template<class... Args>
    server_socket_adaptor(Args&&... args);

    /**
     * Socket isn't exposed directly to avoid confusion over the duplication of
     * interfaces.
     *
     * The name socket is not used because both (the wrapped object and this
     * object itself) are sockets and it would be confusing.
     */
    next_layer_type &next_layer();

    const next_layer_type &next_layer() const;

    // ### polymorphic_socket INTERFACE IMPLEMENTATION ###
    http::read_state read_state() const override;
    http::write_state write_state() const override;
    bool write_response_native_stream() const override;
    void async_read_request(std::string &method, std::string &path,
                            message &message, callback_type handler) override;
    void async_read_some(message &message, callback_type handler) override;
    void async_read_trailers(message &message, callback_type handler) override;
    void async_write_response(std::uint_fast16_t status_code,
                              const boost::string_ref &reason_phrase,
                              const message &message, callback_type handler)
        override;
    void async_write_response_continue(callback_type handler) override;
    void async_write_response_metadata(std::uint_fast16_t status_code,
                                       const boost::string_ref &reason_phrase,
                                       const message &message,
                                       callback_type handler) override;
    void async_write(const message &message, callback_type handler) override;
    void async_write_trailers(const message &message, callback_type handler)
        override;
    void async_write_end_of_message(callback_type handler) override;
};

} // namespace http
} // namespace boost

#include "server_socket_adaptor-inl.hpp"

#endif // BOOST_HTTP_SERVER_SOCKET_ADAPTOR_HPP
