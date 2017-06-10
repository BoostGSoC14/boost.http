/* Copyright (c) 2014, 2017 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_POLYMORPHIC_SERVER_SOCKET_HPP
#define BOOST_HTTP_POLYMORPHIC_SERVER_SOCKET_HPP

#include <boost/http/polymorphic_socket_base.hpp>

namespace boost {
namespace http {

template<class Request, class Response,
         class Message = request_response_wrapper<Request, Response>>
class basic_polymorphic_server_socket
    : public basic_polymorphic_socket_base<Message>
{
public:
    static_assert(is_request_message<Request>::value,
                  "Request must fulfill the Request concept");
    static_assert(is_response_message<Response>::value,
                  "Response must fulfill the Response concept");

    using typename basic_polymorphic_socket_base<Message>::message_type;
    using typename basic_polymorphic_socket_base<Message>::callback_type;
    typedef Request request_server_type;
    typedef Response response_server_type;

    // ### ABI-stable interface ###
    virtual bool write_response_native_stream() const = 0;
    virtual void async_read_request(request_server_type &request,
                                    callback_type handler) = 0;
    virtual void async_write_response(const response_server_type &response,
                                      callback_type handler) = 0;
    virtual void async_write_response_continue(callback_type handler) = 0;
    virtual void async_write_response_metadata(const response_server_type
                                               &response,
                                               callback_type handler) = 0;

    virtual ~basic_polymorphic_server_socket() = default;

    // ### wrappers for the ASIO's extensible model ###
    template<class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_read_request(request_server_type &request, CompletionToken &&token);

    template<class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_write_response(const response_server_type &response,
                         CompletionToken &&token);

    template<class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_write_response_continue(CompletionToken &&token);

    template<class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_write_response_metadata(const response_server_type &response,
                                  CompletionToken &&token);
};

typedef basic_polymorphic_server_socket<request, response>
    polymorphic_server_socket;

template<class Request, class Response, class Message>
struct is_server_socket<basic_polymorphic_server_socket<Request, Response,
                                                        Message>>
    : public std::true_type
{};

} // namespace http
} // namespace boost

#include "polymorphic_server_socket-inl.hpp"

#endif // BOOST_HTTP_POLYMORPHIC_SERVER_SOCKET_HPP
