/* Copyright (c) 2014, 2016 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_SOCKET_HPP
#define BOOST_HTTP_SOCKET_HPP

#include <cstdint>
#include <cstddef>

#include <algorithm>
#include <sstream>
#include <array>
#include <type_traits>
#include <utility>

#include <boost/utility/string_ref.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>

#include <boost/http/reader/request.hpp>
#include <boost/http/traits.hpp>
#include <boost/http/read_state.hpp>
#include <boost/http/write_state.hpp>
#include <boost/http/message.hpp>
#include <boost/http/http_errc.hpp>
#include <boost/http/detail/writer_helper.hpp>
#include <boost/http/detail/constchar_helper.hpp>
#include <boost/http/algorithm/header.hpp>

namespace boost {
namespace http {

template<class Socket>
class basic_socket
{
public:
    typedef Socket next_layer_type;

    // ### QUERY FUNCTIONS ###

    bool is_open() const;
    http::read_state read_state() const;
    http::write_state write_state() const;
    bool write_response_native_stream() const;

    asio::io_service &get_io_service();

    // ### END OF QUERY FUNCTIONS ###

    // ### READ FUNCTIONS ###

    template<class String, class Message, class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_read_request(String &method, String &path, Message &message,
                       CompletionToken &&token);

    template<class Message, class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_read_some(Message &message, CompletionToken &&token);

    template<class Message, class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_read_trailers(Message &message, CompletionToken &&token);

    // ### END OF READ FUNCTIONS ###

    // ### WRITE FUNCTIONS ###

    template<class StringRef, class Message, class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_write_response(std::uint_fast16_t status_code,
                         const StringRef &reason_phrase, const Message &message,
                         CompletionToken &&token);

    template<class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_write_response_continue(CompletionToken &&token);

    template<class StringRef, class Message, class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_write_response_metadata(std::uint_fast16_t status_code,
                                  const StringRef &reason_phrase,
                                  const Message &message,
                                  CompletionToken &&token);

    template<class Message, class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_write(const Message &message, CompletionToken &&token);

    template<class Message, class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_write_trailers(const Message &message, CompletionToken &&token);

    template<class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_write_end_of_message(CompletionToken &&token);

    // ### END OF WRITE FUNCTIONS ###

    // ### START OF basic_server SPECIFIC FUNCTIONS ###

    basic_socket(boost::asio::io_service &io_service,
                 boost::asio::mutable_buffer inbuffer);

    template<class... Args>
    basic_socket(boost::asio::mutable_buffer inbuffer, Args&&... args);

    next_layer_type &next_layer();
    const next_layer_type &next_layer() const;

    void open();

private:
    enum Target {
        READY = 1,
        DATA  = 1 << 1,
        END   = 1 << 2,
    };

    template<int target, class Message, class Handler,
             class String = std::string>
    void schedule_on_async_read_message(Handler &handler, Message &message,
                                        String *method = NULL,
                                        String *path = NULL);

    template<int target, class Message, class Handler,
             class String = std::string>
    void on_async_read_message(Handler handler, String *method, String *path,
                               Message &message, const system::error_code &ec,
                               std::size_t bytes_transferred);

    void clear_buffer();

    template<class Message>
    static void clear_message(Message &message);

    template <typename Handler,
              typename ErrorCode>
    void invoke_handler(Handler&& handler,
                        ErrorCode error);

    template<class Handler>
    void invoke_handler(Handler &&handler);

    Socket channel;
    bool is_open_ = true;
    http::read_state istate;

    // TODO: maybe replace by buffersequence to allow scatter-gather operations
    asio::mutable_buffer buffer;
    std::size_t used_size = 0;

    reader::request parser;

    /* `field_name` value is stored in `[buffer[0], field_name_size)`.
       `expecting_field` means don't touch the buffer or madness will come.
       `use_trailers` indicate where we insert the field. */
    std::size_t field_name_size;
    bool expecting_field = false;
    bool use_trailers;
    bool modern_http; // at least HTTP/1.1
    enum {
        KEEP_ALIVE_UNKNOWN,
        KEEP_ALIVE_CLOSE_READ,
        KEEP_ALIVE_KEEP_ALIVE_READ
    } keep_alive;

    // Output state
    detail::writer_helper writer_helper;
    std::string content_length_buffer;
    bool connect_request;
};

typedef basic_socket<boost::asio::ip::tcp::socket> socket;

template<class Socket>
struct is_server_socket<basic_socket<Socket>>: public std::true_type {};

} // namespace http
} // namespace boost

#include "socket-inl.hpp"

#endif // BOOST_HTTP_SOCKET_HPP
