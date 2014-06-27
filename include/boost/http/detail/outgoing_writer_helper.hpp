/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_DETAIL_OUTGOING_WRITER_HELPER_H
#define BOOST_HTTP_DETAIL_OUTGOING_WRITER_HELPER_H

#include <boost/http/outgoing_state.hpp>

namespace boost {
namespace http {
namespace detail {

struct outgoing_writer_helper
{
    outgoing_writer_helper() = default;

    outgoing_writer_helper(outgoing_state state)
        : state(state)
    {}

    bool write_message();
    bool write_continue();
    bool write_metadata();
    bool write();
    bool write_trailers();
    bool end();

    outgoing_state state;
};

inline bool outgoing_writer_helper::write_message()
{
    if (state != outgoing_state::empty
        && state != outgoing_state::continue_issued) {
        return false;
    }

    state = outgoing_state::finished;
    return true;
}

inline bool outgoing_writer_helper::write_continue()
{
    if (state != outgoing_state::empty)
        return false;

    state = outgoing_state::continue_issued;
    return true;
}

inline bool outgoing_writer_helper::write_metadata()
{
    if (state != outgoing_state::empty
        && state != outgoing_state::continue_issued) {
        return false;
    }

    state = outgoing_state::headers_issued;
    return true;
}

inline bool outgoing_writer_helper::write()
{
    if (state != outgoing_state::start_line_issued
        && state != outgoing_state::headers_issued) {
        return false;
    }

    state = outgoing_state::headers_issued;
    return true;
}

inline bool outgoing_writer_helper::write_trailers()
{
    if (state != outgoing_state::headers_issued)
        return false;

    state = outgoing_state::body_issued;
    return true;
}

inline bool outgoing_writer_helper::end()
{
    if (state != outgoing_state::start_line_issued
        && state != outgoing_state::headers_issued
        && state != outgoing_state::body_issued) {
        return false;
    }

    state = outgoing_state::finished;
    return true;
}

} // namespace detail
} // namespace http
} // namespace boost

#endif // BOOST_HTTP_DETAIL_OUTGOING_WRITER_HELPER_H
