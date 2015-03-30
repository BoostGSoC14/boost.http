/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_DETAIL_OUTGOING_WRITER_HELPER_H
#define BOOST_HTTP_DETAIL_OUTGOING_WRITER_HELPER_H

#include <boost/http/write_state.hpp>

namespace boost {
namespace http {
namespace detail {

struct writer_helper
{
    writer_helper() = default;

    writer_helper(write_state state)
        : state(state)
    {}

    writer_helper& operator=(write_state state)
    {
        this->state = state;
        return *this;
    }

    bool write_message();
    bool write_continue();
    bool write_metadata();
    bool write();
    bool write_trailers();
    bool end();

    write_state state;
};

inline bool writer_helper::write_message()
{
    if (state != write_state::empty
        && state != write_state::continue_issued) {
        return false;
    }

    state = write_state::finished;
    return true;
}

inline bool writer_helper::write_continue()
{
    if (state != write_state::empty)
        return false;

    state = write_state::continue_issued;
    return true;
}

inline bool writer_helper::write_metadata()
{
    if (state != write_state::empty
        && state != write_state::continue_issued) {
        return false;
    }

    state = write_state::metadata_issued;
    return true;
}

inline bool writer_helper::write()
{
    if (state != write_state::metadata_issued)
        return false;

    return true;
}

inline bool writer_helper::write_trailers()
{
    if (state != write_state::metadata_issued)
        return false;

    state = write_state::finished;
    return true;
}

inline bool writer_helper::end()
{
    if (state != write_state::metadata_issued)
        return false;

    state = write_state::finished;
    return true;
}

} // namespace detail
} // namespace http
} // namespace boost

#endif // BOOST_HTTP_DETAIL_OUTGOING_WRITER_HELPER_H
