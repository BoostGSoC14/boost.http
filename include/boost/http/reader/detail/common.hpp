/* Copyright (c) 2016 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */


#ifndef BOOST_HTTP_READER_DETAIL_COMMON_HPP
#define BOOST_HTTP_READER_DETAIL_COMMON_HPP

#include <boost/utility/string_ref.hpp>
#include <boost/http/reader/detail/abnf.hpp>

namespace boost {
namespace http {
namespace reader {
namespace detail {

inline string_ref decode_field_value(string_ref in)
{
    /* The field value does not include any leading or trailing whitespace: OWS
       occurring before the first non-whitespace octet of the field value or
       after the last non-whitespace octet of the field value ought to be
       excluded by parsers when extracting the field value from a header field
       (section 3.2.4 of RFC7230).

       OWS can happen in the middle of the field value too. Therefore, the only
       OWS we can detect "ahead of time" is leading OWS (i.e. when only part of
       the field has been received) and ending OWS must be removed once the
       whole field has been received (i.e. a job to this layer of
       abstraction).

       We don't check in[0] because every header field has at least one
       non-whitespace character. */
    std::size_t ending_ows = 0;
    const unsigned char *v = reinterpret_cast<const unsigned char*>(in.data());
    for (std::size_t i = in.size() - 1 ; i > 0 ; --i) {
        if (!is_ows(v[i]))
            break;
        else
            ++ending_ows;
    }

    in.remove_suffix(ending_ows);

    return in;
}

} // namespace detail
} // namespace reader
} // namespace http
} // namespace boost

#endif // BOOST_HTTP_READER_DETAIL_COMMON_HPP
