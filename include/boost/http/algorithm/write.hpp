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

template<class String = string_ref, class Socket, class Message,
         class CompletionToken>
typename asio::async_result<
    typename asio::handler_type<CompletionToken,
                                void(system::error_code)>::type>::type
async_write_response(Socket &socket, status_code sc, const Message &message,
                     CompletionToken &&token)
{
    return socket.async_write_response(static_cast<std::uint_fast16_t>(sc),
                                       to_string<String>(sc), message,
                                       std::forward<CompletionToken>(token));
}

template<class String = string_ref, class Socket, class Message,
         class CompletionToken>
typename asio::async_result<
    typename asio::handler_type<CompletionToken,
                                void(system::error_code)>::type>::type
async_write_response_metadata(Socket &socket, status_code sc,
                              const Message &message, CompletionToken &&token)
{
    return socket
        .async_write_response_metadata(static_cast<std::uint_fast16_t>(sc),
                                       to_string<String>(sc), message,
                                       std::forward<CompletionToken>(token));
}

} // namespace http
} // namespace boost

#endif // BOOST_HTTP_ALGORITHM_WRITE_HPP
