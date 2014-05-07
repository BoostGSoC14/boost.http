/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_EMBEDDED_SERVER_SOCKET_H
#define BOOST_HTTP_EMBEDDED_SERVER_SOCKET_H

#include "../basic_socket.hpp"
#include <boost/asio/ip/tcp.hpp>

namespace boost {
namespace http {

struct embedded_server {};

/** @TODO: design is going to be refactored later.
    Also, the vocabulary introduced here is interesting for general
    specializations, not only embedded_server. */
enum embedded_server_mode_flags
{
    server//, client = 1 << 1
};

template<>
class basic_socket<embedded_server>
{
public:
    /* this is awkard. the message type doesn't suffer specialization and is
       exactly the same. it is only making difficult for different server
       backends to share some code. the problem is even worse than the non-SCARY
       iterators problem. TODO: solve it */
    typedef basic_message<embedded_server> message_type;

    // ### QUERY FUNCTIONS ###

    /* Vocabulary might be confusing here. The word "incoming" could refer to
       request if in server-mode or to response if in client-mode. */

    // used to know if message is complete and what parts (body, trailers) were
    // completed.
    http::outgoing_state outgoing_state() const;

    /** if output support native stream or will buffer the body internally
     *
     * outgoing_response prefix is used instead plain outgoing, because it's not
     * possible to query capabilities information w/o communication with the
     * other peer, then this query is only available in server-mode. clients can
     * just issue a request and try again later if not supported. design may be
     * refined later. */
    bool outgoing_response_native_stream() const;

    /**
     * Check if received headers include `100-continue` in the _Expect_ header.
     * It's just a convenient method that doesn't imply backend access overhead.
     *
     * The name _required_ is used instead _supported_, because a 100-continue
     * status require action from the server.
     */
    bool incoming_request_continue_required() const;

    bool incoming_request_upgrade_required() const;

    // ### END OF QUERY FUNCTIONS ###

    // ### READ FUNCTIONS ###

    // only warns you when the message is ready (start-line and headers).
    void receive_message(message_type &message);

    // body might very well not fit into memory and user might very well want to
    // save it to disk or immediately stream to somewhere else (proxy?)
    void receive_body_part(message_type &type);

    // it doesn't make sense to expose an interface that only feed one trailer
    // at a time, because headers and trailers are metadata about the body and
    // the user need the metadata to correctly decode/interpret the body anyway.
    void receive_trailers(message_type &type);

    // ### END OF READ FUNCTIONS ###

    // ### WRITE FUNCTIONS ###

    // write the whole message in "one phase"
    void write_message(const message_type &message);

    // write start-line and headers
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

    basic_socket(boost::asio::io_service & io_service,
                 embedded_server_mode_flags /*mode*/) :
        channel(io_service),
        ostate(http::outgoing_state::empty)//, mode(mode)
    {}

    boost::asio::ip::tcp::socket channel;

private:
    http::outgoing_state ostate;
    //embedded_server_mode_flags mode;
};

extern template class basic_socket<embedded_server>;

} // namespace http
} // namespace boost

#endif // BOOST_HTTP_EMBEDDED_SERVER_SOCKET_H
