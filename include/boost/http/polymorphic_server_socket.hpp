/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_POLYMORPHIC_SERVER_SOCKET_H
#define BOOST_HTTP_POLYMORPHIC_SERVER_SOCKET_H

#include <cstdint>

#include <string>

#include <boost/utility/string_ref.hpp>

#include <boost/http/polymorphic_socket_base.hpp>

namespace boost {
namespace http {

/**
 * The design of this hierarchy is inspired by C++ iostream.
 *
 * polymorphic_socket_base -> polymorphic_server_socket +-> polymorphic_socket
 *                         -> polymorphic_client_socket +
 *
 * Once the client socket concept is defined, a polymorphic_client_socket
 * abstract class should also be defined, alongside with a
 * polymorphic_socket. The polymorphic_socket should inherit from
 * polymorphic_client_socket and polymorphic_server_socket.
 *
 * Given there is no state in these classes, virtual inheritance isn't used.
 */
class polymorphic_server_socket: public polymorphic_socket_base
{
public:
    typedef std::function<void(system::error_code)> callback_type;

    // ### ABI-stable interface ###
    virtual bool write_response_native_stream() const = 0;
    virtual void async_read_request(std::string &method, std::string &path,
                                    message &message,
                                    callback_type handler) = 0;
    virtual void async_write_response(std::uint_fast16_t status_code,
                                      const boost::string_ref &reason_phrase,
                                      const message &message,
                                      callback_type handler) = 0;
    virtual void async_write_response_continue(callback_type handler) = 0;
    virtual void async_write_response_metadata(std::uint_fast16_t status_code,
                                               const boost::string_ref
                                               &reason_phrase,
                                               const message &message,
                                               callback_type handler) = 0;

    virtual ~polymorphic_server_socket() = default;

    // ### wrappers for the ASIO's extensible model ###
    template<class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_read_request(std::string &method, std::string &path,
                       message &message, CompletionToken &&token);

    template<class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_write_response(std::uint_fast16_t status_code,
                         const boost::string_ref &reason_phrase,
                         const message &message, CompletionToken &&token);

    template<class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_write_response_continue(CompletionToken &&token);

    template<class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_write_response_metadata(std::uint_fast16_t status_code,
                                  const boost::string_ref &reason_phrase,
                                  const message &message,
                                  CompletionToken &&token);
};

} // namespace http
} // namespace boost

#include "polymorphic_server_socket-inl.hpp"

#endif // BOOST_HTTP_POLYMORPHIC_SERVER_SOCKET_H
