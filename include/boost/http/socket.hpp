/* Copyright (c) 2014 Vinícius dos Santos Oliveira

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

#include <boost/http/read_state.hpp>
#include <boost/http/write_state.hpp>
#include <boost/http/message.hpp>
#include <boost/http/http_errc.hpp>
#include <boost/http/detail/writer_helper.hpp>
#include <boost/http/detail/constchar_helper.hpp>
#include <boost/http/algorithm/header.hpp>

namespace boost {
namespace http {

namespace detail {

extern "C" {

/** This **MUST** be a struct with the same properties from the http_parser
    present in the internal http_parser.h header from the Ryan Dahl's HTTP
    parser. */
struct http_parser
{
    unsigned int type : 2;
    unsigned int flags : 7;
    unsigned int state : 7;
    unsigned int header_state : 8;
    unsigned int index : 8;
    std::uint32_t nread;
    std::uint64_t content_length;
    unsigned short http_major;
    unsigned short http_minor;
    unsigned int status_code : 16;
    unsigned int method : 8;
    unsigned int http_errno : 7;
    unsigned int upgrade : 1;
    void *data;
};

typedef int (*http_data_cb)(http_parser*, const char *at, std::size_t length);
typedef int (*http_cb)(http_parser*);

/** This **MUST** be a struct with the same properties from the
    http_parser_settings present in the internal http_parser.h header from the
    Ryan Dahl's HTTP parser. */
struct http_parser_settings
{
    http_cb      on_message_begin;
    http_data_cb on_url;
    http_data_cb on_status;
    http_data_cb on_header_field;
    http_data_cb on_header_value;
    http_cb      on_headers_complete;
    http_data_cb on_body;
    http_cb      on_message_complete;
    http_cb      on_chunk_header;
    http_cb      on_chunk_complete;
};

} // extern "C"

enum class parser_error
{
    cb_headers_complete = 5, // HPE_CB_headers_complete
    cb_message_complete = 7  // HPE_CB_message_complete
};

BOOST_HTTP_DECL void init(http_parser &parser);
BOOST_HTTP_DECL void init(http_parser_settings &settings);
BOOST_HTTP_DECL std::size_t execute(http_parser &parser,
                                    const http_parser_settings &settings,
                                    const std::uint8_t *data, std::size_t len);
BOOST_HTTP_DECL bool should_keep_alive(const http_parser &parser);
BOOST_HTTP_DECL bool body_is_final(const http_parser &parser);

} // namespace detail

template<class Socket>
class basic_socket
{
public:
    typedef Socket next_layer_type;

    // ### QUERY FUNCTIONS ###

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

private:
    typedef detail::http_parser http_parser;
    typedef detail::http_parser_settings http_parser_settings;

    enum Flags
    {
        NONE,
        READY,
        DATA       = 1 << 1,
        END        = 1 << 2,
        HTTP_1_1   = 1 << 3,
        KEEP_ALIVE = 1 << 4,
        UPGRADE    = 1 << 5
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

    template<class Message, class String>
    static http_parser_settings settings();

    template<class Message>
    static int on_message_begin(http_parser *parser);

    template<class Message, class String>
    static int on_url(http_parser *parser, const char *at, std::size_t size);

    template<class Message>
    static int on_header_field(http_parser *parser, const char *at,
                               std::size_t size);

    template<class Message>
    static int on_header_value(http_parser *parser, const char *at,
                               std::size_t size);

    template<class Message, class String>
    static int on_headers_complete(http_parser *parser);

    template<class Message>
    static int on_body(http_parser *parser, const char *data, std::size_t size);

    template<class Message>
    static int on_message_complete(http_parser *parser);

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
    http::read_state istate;

    // TODO: maybe replace by buffersequence to allow scatter-gather operations
    asio::mutable_buffer buffer;
    std::size_t used_size = 0;

    /* pimpl is not used to avoid the extra level of indirection and the extra
       allocation. Also, related objects that are closer together are more cache
       friendly. It is not ideal, but Ryan Dahl's HTTP parser don't support
       arbitrary HTTP verbs and it will have to be replaced anyway later, then
       I'm prioritizing header/interface isolation and a reason for people to
       use this wrapper (performance). */
    http_parser parser;
    int flags;

    /* Thanks to current HTTP parser, I need to resort to this technique of
       storing the current message as a "global"-like pointer as opposed to keep
       it within the handler. */
    void *current_method;
    void *current_path;
    void *current_message;

    std::pair<std::string, std::string> last_header;
    bool use_trailers;

    // Output state
    detail::writer_helper writer_helper;
    std::string content_length_buffer;
    bool connect_request;
};

typedef basic_socket<boost::asio::ip::tcp::socket> socket;

} // namespace http
} // namespace boost

#include "socket-inl.hpp"

#endif // BOOST_HTTP_SOCKET_HPP
