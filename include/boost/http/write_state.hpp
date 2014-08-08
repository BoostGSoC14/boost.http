/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_OUTGOING_STATE_H
#define BOOST_HTTP_OUTGOING_STATE_H

namespace boost {
namespace http {

/**
 * Represents the current state in the HTTP outgoing response or HTTP outgoing
 * request.
 *
 * It has extra values that won't be used in "outgoing-request" mode.
 * Explanation focuses in "outgoing-response" mode.
 *
 * The picture response_state.png can help you understand this file.
 */
enum class write_state
{
    /**
     * This is the initial state.
     *
     * It means that the response object wasn't used yet.
     *
     * At this state, you can only issue the metadata or issue a continue
     * action, if continue is supported/used in this HTTP session. Even if
     * continue was requested, issue a continue action is optional and only
     * required if you need the request's body.
     */
    empty,
    /**
     * This state is reached from the `empty` state, once you issue a continue
     * action.
     *
     * No more continue actions can be issued from this state.
     *
     * Only makes sense in server mode, when issuing an outgoing response.
     */
    continue_issued,
    /**
     * This state can be reached either from `empty` or `continue_issued`.
     *
     * It happens when the metadata (start line + header section) is reached
     * (through `write_metadata`).
     *
     * From this state, you can only issue the body, the trailers or the end of
     * the message.
     */
    metadata_issued,
    /**
     * The message is considered complete once this state is reached.
     *
     * You can no longer issue anything once this state is reached. The
     * underlying channel will change the outgoing_state once some unspecified
     * event occurs. This event is usually a new request in server mode or the
     * response fully received in client mode.
     */
    finished
};

} // namespace http
} // namespace boost

#endif // BOOST_HTTP_OUTGOING_STATE_H
