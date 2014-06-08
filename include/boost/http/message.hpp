/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_SERVER_MESSAGE_H
#define BOOST_HTTP_SERVER_MESSAGE_H

#include <cstdint>
#include <boost/asio/buffer.hpp>
#include "headers.hpp"

namespace boost {
namespace http {

/**
 * A container to hold generic HTTP messages.
 *
 * Common operations between request and response that the underlying channel
 * should provide:
 *
 * - Send and receive the start line.
 * - Populate the headers.
 * - Signalize about a new body part.
 * - Populate or signalize about trailers.
 * - Signalize about end of message.
 *
 * @TODO: define the basic_message concept
 */
struct message
{
    /**
     * It's context dependant and have a different structure for requests and
     * responses. The object should store the start line (excluding "\r\n")
     * without trying to decode its meaning, leaving this task for a specialized
     * abstraction of the appropriate HTTP message type (request or response).
     *
     * @TODO: write rationable why UTF-8 and why the null terminator overhead
     * won't matter much here. Maybe arguing using SSO as example and some
     * benchmark too.
     */
    std::string start_line;

    /**
     * The metadata about the message. The user will need this info to handle
     * the message appropriately.
     */
    boost::http::headers headers;

    /**
     * Its size may not fit into memory. Also, the user might be able to handle
     * parts of its contents (in-place, processing while receiving). Given these
     * facts, events for each piece of body gathered should be generated, but
     * this container still could be used to buffer the data.
     *
     * std::vector<uint8_t> is used over string, because fits the purpose better
     * (no '\0' character terminator, well-defined behaviours of capacity, size
     * and iterator invalidation, ...)
     */
    std::vector<uint8_t> body;
};

} // namespace http
} // namespace boost

#endif // BOOST_HTTP_SERVER_MESSAGE_H
