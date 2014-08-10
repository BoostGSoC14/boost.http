/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_POLYMORPHIC_SOCKET_BASE_H
#define BOOST_HTTP_POLYMORPHIC_SOCKET_BASE_H

#include <functional>

#include <boost/system/error_code.hpp>

#include <boost/http/read_state.hpp>
#include <boost/http/write_state.hpp>
#include <boost/http/message.hpp>

namespace boost {
namespace http {

class polymorphic_socket_base
{
public:
    typedef std::function<void(system::error_code)> callback_type;

    // ### ABI-stable interface ###
    virtual http::read_state read_state() const = 0;
    virtual http::write_state write_state() const = 0;
    virtual void async_read_some(message &message, callback_type handler) = 0;
    virtual void async_read_trailers(message &message,
                                     callback_type handler) = 0;
    virtual void async_write(const message &message,
                             callback_type handler) = 0;
    virtual void async_write_trailers(const message &message,
                                      callback_type handler) = 0;
    virtual void async_write_end_of_message(callback_type handler) = 0;

    virtual ~polymorphic_socket_base() = 0;

    // ### wrappers for the ASIO's extensible model ###
    template<class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_read_some(message &message, CompletionToken &&token);

    template<class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_read_trailers(message &message, CompletionToken &&token);

    template<class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_write(message &message, CompletionToken &&token);

    template<class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_write_trailers(message &message, CompletionToken &&token);

    template<class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_write_end_of_message(CompletionToken &&token);
};

} // namespace http
} // namespace boost

#include "polymorphic_socket_base-inl.hpp"

#endif // BOOST_HTTP_POLYMORPHIC_SOCKET_BASE_H
