/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_POLYMORPHIC_SERVER_SOCKET_HPP
#define BOOST_HTTP_POLYMORPHIC_SERVER_SOCKET_HPP

#include <cstdint>

#include <string>

#include <boost/utility/string_ref.hpp>

#include <boost/http/polymorphic_socket_base.hpp>

namespace boost {
namespace http {

template<class Message>
class basic_polymorphic_server_socket
    : public basic_polymorphic_socket_base<Message>
{
public:
    using typename basic_polymorphic_socket_base<Message>::message_type;
    using typename basic_polymorphic_socket_base<Message>::callback_type;
    typedef std::string method_server_type;
    typedef std::string path_server_type;
    typedef boost::string_ref reason_phrase_server_type;

    // ### ABI-stable interface ###
    virtual bool write_response_native_stream() const = 0;
    virtual void async_read_request(std::string &method, std::string &path,
                                    message_type &message,
                                    callback_type handler) = 0;
    virtual void async_write_response(std::uint_fast16_t status_code,
                                      const boost::string_ref &reason_phrase,
                                      const message_type &message,
                                      callback_type handler) = 0;
    virtual void async_write_response_continue(callback_type handler) = 0;
    virtual void async_write_response_metadata(std::uint_fast16_t status_code,
                                               const boost::string_ref
                                               &reason_phrase,
                                               const message_type &message,
                                               callback_type handler) = 0;

    virtual ~basic_polymorphic_server_socket() = default;

    // ### wrappers for the ASIO's extensible model ###
    template<class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_read_request(std::string &method, std::string &path,
                       message_type &message, CompletionToken &&token);

    template<class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_write_response(std::uint_fast16_t status_code,
                         const boost::string_ref &reason_phrase,
                         const message_type &message, CompletionToken &&token);

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
                                  const message_type &message,
                                  CompletionToken &&token);
};

typedef basic_polymorphic_server_socket<message> polymorphic_server_socket;

template<class Message>
struct is_server_socket<basic_polymorphic_server_socket<Message>>
    : public std::true_type
{};

} // namespace http
} // namespace boost

#include "polymorphic_server_socket-inl.hpp"

#endif // BOOST_HTTP_POLYMORPHIC_SERVER_SOCKET_HPP
