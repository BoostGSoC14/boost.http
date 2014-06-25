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
enum class outgoing_state
{
    /**
     * This is the initial state.
     *
     * It means that the response object wasn't used yet.
     *
     * At this state, you can only issue the start line or issue a continue
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
     * Only makes sense in server mode, when issuing a outgoing response.
     */
    continue_issued,
    /**
     * This state can be reached either from `empty` or `continue_issued`.
     *
     * It happens when the start line is reached (through `open` or
     * `write_head`).
     */
    start_line_issued,
    /**
     * This state is reached once the first chunk of body is issued.
     *
     * \warning
     * Once this state is reached, it is no longer safe to access the
     * `boost::http::server::response::headers` attribute, which is left in an
     * unspecified state and might or might nor be used again by the backend.
     * You're safe to access and modify this attribute again once the `FINISHED`
     * state is reached, but the attribute will be at an unspecified state and
     * is recommended to _clear_ it.
     */
    headers_issued,
    /**
     * This state is reached once the first trailer is issued to the backend.
     *
     * After this state is reached, it's not allowed to write the body again.
     * You can either proceed to issue more trailers or `end` the response.
     */
    body_issued,
    /**
     * The response is considered complete once this state is reached. You
     * should no longer access this response or the associated request objects,
     * because the backend has the freedom to recycle or destroy the objects.
     */
    finished
};

} // namespace http
} // namespace boost

#endif // BOOST_HTTP_OUTGOING_STATE_H
