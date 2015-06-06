/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_BUFFERED_SOCKET_HPP
#define BOOST_HTTP_BUFFERED_SOCKET_HPP

#include <boost/http/socket.hpp>

#ifndef BOOST_HTTP_SOCKET_DEFAULT_BUFFER_SIZE
#define BOOST_HTTP_SOCKET_DEFAULT_BUFFER_SIZE 256
#endif // BOOST_HTTP_SOCKET_DEFAULT_BUFFER_SIZE

namespace boost {
namespace http {

template<class Socket, std::size_t N = BOOST_HTTP_SOCKET_DEFAULT_BUFFER_SIZE>
class basic_buffered_socket: private ::boost::http::basic_socket<Socket>
{
public:
    static_assert(N > 0, "N must be greater than 0");

    typedef Socket next_layer_type;

    http::read_state read_state() const
    {
        return Parent::read_state();
    }

    http::write_state write_state() const
    {
        return Parent::write_state();
    }

    bool write_response_native_stream() const
    {
        return Parent::write_response_native_stream();
    }

    asio::io_service &get_io_service()
    {
        return Parent::get_io_service();
    }

    template<class String, class Message, class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_read_request(String &method, String &path, Message &message,
                       CompletionToken &&token)
    {
        return Parent::async_read_request(method, path, message,
                                          std::forward<CompletionToken>(token));
    }

    template<class Message, class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_read_some(Message &message, CompletionToken &&token)
    {
        return Parent::async_read_some(message,
                                       std::forward<CompletionToken>(token));
    }

    template<class Message, class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_read_trailers(Message &message, CompletionToken &&token)
    {
        typedef CompletionToken CT;
        return Parent::async_read_trailers(message, std::forward<CT>(token));
    }

    template<class StringRef, class Message, class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_write_response(std::uint_fast16_t status_code,
                         const StringRef &reason_phrase, const Message &message,
                         CompletionToken &&token)
    {
        typedef CompletionToken CT;
        return Parent::async_write_response(status_code, reason_phrase, message,
                                            std::forward<CT>(token));
    }

    template<class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_write_response_continue(CompletionToken &&token)
    {
        typedef CompletionToken CT;
        return Parent::async_write_response_continue(std::forward<CT>(token));
    }

    template<class StringRef, class Message, class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_write_response_metadata(std::uint_fast16_t status_code,
                                  const StringRef &reason_phrase,
                                  const Message &message,
                                  CompletionToken &&token)
    {
        typedef CompletionToken CT;
        return Parent::async_write_response_metadata(status_code, reason_phrase,
                                                     message,
                                                     std::forward<CT>(token));
    }

    template<class Message, class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_write(const Message &message, CompletionToken &&token)
    {
        typedef CompletionToken CT;
        return Parent::async_write(message, std::forward<CT>(token));
    }

    template<class Message, class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_write_trailers(const Message &message, CompletionToken &&token)
    {
        typedef CompletionToken CT;
        return Parent::async_write_trailers(message, std::forward<CT>(token));
    }

    template<class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_write_end_of_message(CompletionToken &&token)
    {
        typedef CompletionToken CT;
        return Parent::async_write_end_of_message(std::forward<CT>(token));
    }

    basic_buffered_socket(boost::asio::io_service &io_service)
        : Parent(io_service, boost::asio::buffer(buffer))
    {}

    template<class... Args>
    basic_buffered_socket(Args&&... args)
        : Parent(boost::asio::buffer(buffer), std::forward<Args>(args)...)
    {}

    next_layer_type &next_layer()
    {
        return Parent::next_layer();
    }

    const next_layer_type &next_layer() const
    {
        return Parent::next_layer();
    }

private:
    typedef ::boost::http::basic_socket<Socket> Parent;

    char buffer[N];
};

typedef basic_buffered_socket<boost::asio::ip::tcp::socket> buffered_socket;

} // namespace http
} // namespace boost

#endif // BOOST_HTTP_BUFFERED_SOCKET_HPP
