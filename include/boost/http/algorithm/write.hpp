/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_ALGORITHM_WRITE_HPP
#define BOOST_HTTP_ALGORITHM_WRITE_HPP

#include <cstdint>

#include <boost/utility/string_ref.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/async_result.hpp>

#include <boost/http/message.hpp>
#include <boost/http/status_code.hpp>

namespace boost {
namespace http {

template<class StringRef = string_ref, class ServerSocket, class Message,
         class CompletionToken>
typename asio::async_result<
    typename asio::handler_type<CompletionToken,
                                void(system::error_code)>::type>::type
async_write_response(ServerSocket &socket, status_code sc,
                     const Message &message, CompletionToken &&token)
{
    static_assert(is_server_socket<ServerSocket>::value,
                  "ServerSocket must fulfill the ServerSocket concept");
    static_assert(is_message<Message>::value,
                  "Message must fulfill the Message concept");

    return socket.async_write_response(static_cast<std::uint_fast16_t>(sc),
                                       to_string<StringRef>(sc), message,
                                       std::forward<CompletionToken>(token));
}

template<class StringRef = string_ref, class ServerSocket, class Message,
         class CompletionToken>
typename asio::async_result<
    typename asio::handler_type<CompletionToken,
                                void(system::error_code)>::type>::type
async_write_response_metadata(ServerSocket &socket, status_code sc,
                              const Message &message, CompletionToken &&token)
{
    static_assert(is_server_socket<ServerSocket>::value,
                  "ServerSocket must fulfill the ServerSocket concept");
    static_assert(is_message<Message>::value,
                  "Message must fulfill the Message concept");

    return socket
        .async_write_response_metadata(static_cast<std::uint_fast16_t>(sc),
                                       to_string<StringRef>(sc), message,
                                       std::forward<CompletionToken>(token));
}

} // namespace http
} // namespace boost

#endif // BOOST_HTTP_ALGORITHM_WRITE_HPP
