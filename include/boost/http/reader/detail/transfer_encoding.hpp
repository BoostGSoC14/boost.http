/* Copyright (c) 2016 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */


#ifndef BOOST_HTTP_READER_DETAIL_TRANSFER_ENCODING_HPP
#define BOOST_HTTP_READER_DETAIL_TRANSFER_ENCODING_HPP

#include <boost/algorithm/string/predicate.hpp>
#include <boost/http/algorithm/header/header_value_any_of.hpp>

namespace boost {
namespace http {
namespace reader {
namespace detail {

enum DecodeTransferEncodingResult {
    CHUNKED_NOT_FOUND,
    CHUNKED_AT_END,
    CHUNKED_INVALID
};

struct decode_transfer_encoding_p
{
    struct state
    {
        state()
            : count(0)
            , res(CHUNKED_NOT_FOUND)
        {}

        unsigned count;
        DecodeTransferEncodingResult res;
    };

    static const bool STOP_ITER = true;
    static const bool PROC_ITER = false;

    decode_transfer_encoding_p(state &s)
        : count(s.count)
        , res(s.res)
    {}

    bool operator()(string_ref v) const
    {
        using boost::algorithm::iequals;

        // All transfer-coding names are case-insensitive (section 4 of RFC7230)
        if (!iequals(v, "chunked")) {
            if (count == 1) {
                /* If any transfer coding other than chunked is applied to a
                   request payload body, the sender MUST apply chunked as the
                   final transfer coding (section 3.3.1 of RFC7230) */
                res = CHUNKED_INVALID;
                return STOP_ITER;
            }

            return PROC_ITER;
        }

        ++count;

        if (count == 2) {
            /* A sender MUST NOT apply chunked more than once to a message body
               (section 3.3.1 of RFC7230) */
            res = CHUNKED_INVALID;
            return STOP_ITER;
        }

        res = CHUNKED_AT_END;
        return PROC_ITER;
    }

    unsigned &count;
    DecodeTransferEncodingResult &res;
};

inline DecodeTransferEncodingResult decode_transfer_encoding(string_ref field)
{
    decode_transfer_encoding_p::state p_state;
    decode_transfer_encoding_p p(p_state);
    header_value_any_of(field, p);
    return p.res;
}

} // namespace detail
} // namespace reader
} // namespace http
} // namespace boost

#endif // BOOST_HTTP_READER_DETAIL_TRANSFER_ENCODING_HPP
