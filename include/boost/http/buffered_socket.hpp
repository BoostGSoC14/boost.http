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

namespace detail {
template<std::size_t N>
struct buffered_socket_wrapping_buffer {
    char buffer[N];
};
} // namespace detail

template<class Socket, std::size_t N = BOOST_HTTP_SOCKET_DEFAULT_BUFFER_SIZE>
class basic_buffered_socket
    : private detail::buffered_socket_wrapping_buffer<N>
    , private ::boost::http::basic_socket<Socket>
{
    typedef ::boost::http::basic_socket<Socket> Parent;

public:
    static_assert(N > 0, "N must be greater than 0");

    typedef Socket next_layer_type;

    using Parent::is_open;
    using Parent::read_state;
    using Parent::write_state;
    using Parent::write_response_native_stream;
    using Parent::get_io_service;
    using Parent::async_read_request;
    using Parent::async_read_some;
    using Parent::async_read_trailers;
    using Parent::async_write_response;
    using Parent::async_write_response_continue;
    using Parent::async_write_response_metadata;
    using Parent::async_write;
    using Parent::async_write_trailers;
    using Parent::async_write_end_of_message;

    basic_buffered_socket(boost::asio::io_service &io_service)
        : Parent(io_service, boost::asio::buffer(BufferParent::buffer))
    {}

    template<class... Args>
    basic_buffered_socket(Args&&... args)
        : Parent(boost::asio::buffer(BufferParent::buffer),
                 std::forward<Args>(args)...)
    {}

    using Parent::next_layer;

private:
    typedef detail::buffered_socket_wrapping_buffer<N> BufferParent;
};

typedef basic_buffered_socket<boost::asio::ip::tcp::socket> buffered_socket;

template<class Socket>
struct is_server_socket<basic_buffered_socket<Socket>>: public std::true_type
{};

} // namespace http
} // namespace boost

#endif // BOOST_HTTP_BUFFERED_SOCKET_HPP
