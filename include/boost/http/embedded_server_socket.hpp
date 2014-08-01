/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_EMBEDDED_SERVER_SOCKET_H
#define BOOST_HTTP_EMBEDDED_SERVER_SOCKET_H

#include <cstdint>
#include <cstddef>

#include <algorithm>

#include <boost/lexical_cast.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>

#include <boost/http/incoming_state.hpp>
#include <boost/http/outgoing_state.hpp>
#include <boost/http/message.hpp>
#include <boost/http/http_errc.hpp>
#include <boost/http/detail/outgoing_writer_helper.hpp>
#include <boost/http/detail/constchar_helper.hpp>

namespace boost {
namespace http {

/** @TODO: design is going to be refactored later.
    Also, the vocabulary introduced here is interesting for general
    specializations, not only embedded_server. */
enum class channel_type
{
    server = 1//, client = 1 << 1
};

namespace detail {

extern "C" {

/** This **MUST** be a struct with the same properties from the http_parser
    present in the internal http_parser.h header from the Ryan Dahl's HTTP
    parser. */
struct http_parser
{
    unsigned int type : 2;
    unsigned int flags : 6;
    unsigned int state : 8;
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
};

} // extern "C"

} // namespace detail

/* TODO: templatize based on buffer and socket (TCP and SSL implementation are
   identical). There is no need to templatize based on message type here,
   because there is not a member of embedded_server_socket that depends on
   message type. */
class embedded_server_socket
{
public:
    // TODO: remove this typedef
    typedef message message_type;

    // ### QUERY FUNCTIONS ###

    /* Vocabulary might be confusing here. The word "incoming" could refer to
       request if in server-mode or to response if in client-mode. */

    http::incoming_state incoming_state() const;

    /* used to know if message is complete and what parts (body, trailers) were
       completed. */
    http::outgoing_state outgoing_state() const;

    /** If output support native stream or will buffer the body internally.
     *
     * This query is only available after the message is ready (e.g. on the
     * async_receive_message handler).
     *
     * outgoing_response prefix is used instead plain outgoing, because it's not
     * possible to query capabilities information w/o communication with the
     * other peer, then this query is only available in server-mode. clients can
     * just issue a request and try again later if not supported. design may be
     * refined later. */
    bool outgoing_response_native_stream() const;

    // The following two algorithms should be moved to reusable free functions:

    // ### END OF QUERY FUNCTIONS ###

    // ### READ FUNCTIONS ###

    // only warns you when the message is ready (start-line and headers).
    void receive_message(message_type &message);

    /* \warning async_receive_message is a composed operation. It is implemented
       in terms of zero or more calls to underlying low-level operations. The
       program must ensure that no other operation is performed on the
       embedded_server_socket until this operation completes (the user handler
       is called, either with or without an error code set). */
    template<class Message, class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_receive_message(Message &message, CompletionToken &&token);

    // body might very well not fit into memory and user might very well want to
    // save it to disk or immediately stream to somewhere else (proxy?)
    void receive_some_body(message_type &type);

    template<class Message, class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_receive_some_body(Message &message, CompletionToken &&token);

    // it doesn't make sense to expose an interface that only feed one trailer
    // at a time, because headers and trailers are metadata about the body and
    // the user need the metadata to correctly decode/interpret the body anyway.
    void receive_trailers(message_type &type);

    template<class Message, class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_receive_trailers(Message &message, CompletionToken &&token);

    // ### END OF READ FUNCTIONS ###

    // ### WRITE FUNCTIONS ###

    // write the whole message in "one phase"
    void write_message(const message_type &message);

    /* TODO: refactor it to allow concurrent responses being issued (no
       "iterator invalidation"). */
    template<class Message, class CompletionToken>
    typename asio::async_result<
        typename asio::handler_type<CompletionToken,
                                    void(system::error_code)>::type>::type
    async_write_message(Message &message, CompletionToken &&token);

    void write_continue();

    // write start-line and headers
    // TODO: rename to write_header (?)
    void write_metadata(const message_type &message);

    /** write a body part
     *
     * this function already requires "polymorphic" behaviour to take HTTP/1.0
     * buffering into account */
    void write(boost::asio::const_buffer &part);

    void write_trailers(http::headers headers);

    void end();

    /**
     * Write the 100-continue status that must be written before the client
     * proceed to feed body of the request.
     *
     * \warning If `incoming_request_continue_required` returns true, you
     * **MUST** call this function (before open) to allow the remote client to
     * send the body. If your handler can give an appropriate answer without the
     * body, just reply as usual.
     */
    bool outgoing_response_write_continue();

    // ### END OF WRITE FUNCTIONS ###

    // ### START OF embedded_server SPECIFIC FUNCTIONS ###

    embedded_server_socket(boost::asio::io_service &io_service,
                           boost::asio::mutable_buffer inbuffer,
                           channel_type /*mode*/);

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
        KEEP_ALIVE = 1 << 4
    };

    enum class parser_error
    {
        cb_headers_complete = 5, // HPE_CB_headers_complete
        cb_message_complete = 7  // HPE_CB_message_complete
    };

    template<int target, class Message, class Handler>
    void on_async_receive_message(Handler handler, Message &message,
                                  const system::error_code &ec,
                                  std::size_t bytes_transferred);

    static void init(http_parser &parser);
    static std::size_t execute(http_parser &parser,
                               const http_parser_settings &settings,
                               const std::uint8_t *data, std::size_t len);
    static bool should_keep_alive(const http_parser &parser);
    static bool body_is_final(const http_parser &parser);

    template</*class Buffer, */class Message>
    static http_parser_settings settings();

    // templatize callbacks (plural) below based on Buffer type too
    template<class Message>
    static int on_message_begin(http_parser *parser);

    template<class Message>
    static int on_url(http_parser *parser, const char *at, std::size_t size);

    template<class Message>
    static int on_header_field(http_parser *parser, const char *at,
                               std::size_t size);

    template<class Message>
    static int on_header_value(http_parser *parser, const char *at,
                               std::size_t size);

    template<class Message>
    static int on_headers_complete(http_parser *parser);

    template<class Message>
    static int on_body(http_parser *parser, const char *data, std::size_t size);

    template<class Message>
    static int on_message_complete(http_parser *parser);

    void clear_buffer();

    template<class Message>
    static void clear_message(Message &message);

    boost::asio::ip::tcp::socket channel;
    http::incoming_state istate;
    //channel_type mode;

    // TODO: maybe replace by buffersequence to allow scatter-gather operations
    asio::mutable_buffer buffer;
    std::size_t used_size = 0;

    http_parser parser;
    int flags = 0;

    /* Thanks to current HTTP parser, I need to resort to this technique of
       storing the current message as a "global"-like pointer as opposed to keep
       it within the handler. */
    void *current_message;

    std::pair<std::string, std::string> last_header;
    bool use_trailers = false;

    // Output state
    detail::outgoing_writer_helper writer_helper;
    std::string content_length_buffer;

    friend class embedded_server_socket_acceptor;
};

} // namespace http
} // namespace boost

#include "embedded_server_socket-inl.hpp"

#endif // BOOST_HTTP_EMBEDDED_SERVER_SOCKET_H
