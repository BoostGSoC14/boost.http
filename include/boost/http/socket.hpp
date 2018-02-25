/* Copyright (c) 2014, 2016, 2017 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_SOCKET_HPP
#define BOOST_HTTP_SOCKET_HPP

#include <cstdint>
#include <cstddef>
#include <cstring>

#include <memory>
#include <algorithm>
#include <sstream>
#include <array>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <boost/none.hpp>
#include <boost/variant.hpp>
#include <boost/utility/string_view.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>

#include <boost/http/reader/request.hpp>
#include <boost/http/reader/response.hpp>
#include <boost/http/traits.hpp>
#include <boost/http/read_state.hpp>
#include <boost/http/write_state.hpp>
#include <boost/http/http_errc.hpp>
#include <boost/http/detail/writer_helper.hpp>
#include <boost/http/detail/count_decdigits.hpp>
#include <boost/http/detail/constchar_helper.hpp>
#include <boost/http/algorithm/header.hpp>

namespace boost {
namespace http {

struct default_socket_settings
{
    typedef reader::request req_parser;
    typedef reader::response res_parser;
};

template<class Socket, class Settings = default_socket_settings>
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

    template<class Request, class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_read_request(Request &request, CompletionToken &&token);

    template<class Response, class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_read_response(Response &response, CompletionToken &&token);

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

    template<class Response, class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_write_response(const Response &response, CompletionToken &&token);

    template<class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_write_response_continue(CompletionToken &&token);

    template<class Response, class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_write_response_metadata(const Response &response,
                                  CompletionToken &&token);

    template<class Request, class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_write_request(const Request &request, CompletionToken &&token);

    template<class Request, class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_write_request_metadata(const Request &request,
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

    asio::const_buffer upgrade_head() const;

    void lock_client_to_http10();

private:
    typedef typename Settings::req_parser req_parser;
    typedef typename Settings::res_parser res_parser;

    enum SentMethod {
        HEAD,
        CONNECT,
        OTHER_METHOD
    };

    template<class Message, class Handler>
    void schedule_on_async_read_message(Handler &handler, Message &message);

    template<bool server_mode, class Parser, class Message, class Handler>
    void on_async_read_message(Handler handler, Message &message,
                               const system::error_code &ec,
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

    asio::mutable_buffer buffer;
    std::size_t used_size = 0;

    boost::variant<none_t, req_parser, res_parser> parser = none;
    boost::container::small_vector<SentMethod, 1> sent_requests;

    bool modern_http = true; // is HTTP/1.1?
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

template<class Socket, class Settings>
struct is_server_socket<basic_socket<Socket, Settings>>: public std::true_type
{};

template<class Socket, class Settings>
struct is_client_socket<basic_socket<Socket, Settings>>: public std::true_type
{};

} // namespace http
} // namespace boost

#include "socket.ipp"

#endif // BOOST_HTTP_SOCKET_HPP
