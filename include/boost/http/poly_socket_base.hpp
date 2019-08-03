/* Copyright (c) 2014, 2017, 2018 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_POLY_SOCKET_BASE_HPP
#define BOOST_HTTP_POLY_SOCKET_BASE_HPP

#include <boost/system/error_code.hpp>

#include <boost/asio/async_result.hpp>

#include <boost/http/asio/experimental/poly_handler.hpp>
#include <boost/http/request_response_wrapper.hpp>
#include <boost/http/write_state.hpp>
#include <boost/http/read_state.hpp>
#include <boost/http/response.hpp>
#include <boost/http/request.hpp>

namespace boost {
namespace http {

template<class Message>
class basic_poly_socket_base
{
public:
    static_assert(is_message<Message>::value,
                  "Message must fulfill the Message concept");

    using executor_type = boost::asio::executor;
    using message_type = Message;
    using handler_type = asio::experimental::poly_handler<
        void(system::error_code)
    >;

    // ### ABI-stable interface ###
    virtual executor_type get_executor() = 0;
    virtual bool is_open() const = 0;
    virtual http::read_state read_state() const = 0;
    virtual http::write_state write_state() const = 0;
    virtual void async_read_some(message_type &message,
                                 handler_type handler) = 0;
    virtual void async_write(const message_type &message,
                             handler_type handler) = 0;
    virtual void async_write_trailers(const message_type &message,
                                      handler_type handler) = 0;
    virtual void async_write_end_of_message(handler_type handler) = 0;

    virtual ~basic_poly_socket_base() = 0;

    // ### wrappers for the ASIO's extensible model ###
    template<class CompletionToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(system::error_code))
    async_read_some(message_type &message, CompletionToken &&token);

    template<class CompletionToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(system::error_code))
    async_write(const message_type &message, CompletionToken &&token);

    template<class CompletionToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(system::error_code))
    async_write_trailers(const message_type &message, CompletionToken &&token);

    template<class CompletionToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, void(system::error_code))
    async_write_end_of_message(CompletionToken &&token);
};

using poly_socket_base = basic_poly_socket_base<
    request_response_wrapper<request, response>
>;

template<class Message>
struct is_socket<basic_poly_socket_base<Message>> : public std::true_type
{};

} // namespace http
} // namespace boost

#include "poly_socket_base.ipp"

#endif // BOOST_HTTP_POLY_SOCKET_BASE_HPP
