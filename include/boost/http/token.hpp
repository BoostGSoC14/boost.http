/* Copyright (c) 2016 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */


#ifndef BOOST_HTTP_TOKEN_HPP
#define BOOST_HTTP_TOKEN_HPP

#include <boost/utility/string_view.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/cstdint.hpp>

namespace boost {
namespace http {
namespace token {

struct code
{
    enum value
    {
        error_insufficient_data,
        error_set_method,
        error_use_another_connection,
        error_invalid_data,
        /* It's only an error on versions more recent than HTTP/1.0 */
        error_no_host,
        error_invalid_content_length,
        error_content_length_overflow,
        error_invalid_transfer_encoding,
        error_chunk_size_overflow,
        // used to skip unneeded bytes so user can keep buffer small when asking
        // for more data
        skip,
        method,
        request_target,
        version,
        status_code,
        reason_phrase,
        field_name,
        field_value,
        end_of_headers,
        chunk_ext,
        body_chunk,
        end_of_body,
        trailer_name,
        trailer_value,
        end_of_message
    };
};

struct symbol
{
    enum value
    {
        error,

        skip,

        method,
        request_target,
        version,
        status_code,
        reason_phrase,
        field_name,
        field_value,

        end_of_headers,

        chunk_ext,
        body_chunk,

        end_of_body,

        trailer_name,
        trailer_value,

        end_of_message
    };

    static value convert(code::value);
};

struct category
{
    enum value
    {
        status,
        data,
        structural
    };

    static value convert(code::value);
    static value convert(symbol::value);
};

//-----------------------------------------------------------------------------
// Token tags
//-----------------------------------------------------------------------------

struct skip
{
    static const token::code::value code = token::code::skip;
};

struct field_name
{
    typedef boost::string_view type;
    static const token::code::value code = token::code::field_name;
};

struct field_value
{
    typedef boost::string_view type;
    static const token::code::value code = token::code::field_value;
};

struct chunk_ext
{
    struct type
    {
        uint_least64_t chunk_size;
        boost::string_view ext;
    };
    static const token::code::value code = token::code::chunk_ext;
};

struct body_chunk
{
    typedef boost::asio::const_buffer type;
    static const token::code::value code = token::code::body_chunk;
};

struct trailer_name
{
    typedef boost::string_view type;
    static const token::code::value code = token::code::trailer_name;
};

struct trailer_value
{
    typedef boost::string_view type;
    static const token::code::value code = token::code::trailer_value;
};

struct end_of_headers
{
    static const token::code::value code = token::code::end_of_headers;
};

struct end_of_body
{
    static const token::code::value code = token::code::end_of_body;
};

struct end_of_message
{
    static const token::code::value code = token::code::end_of_message;
};

struct method
{
    typedef boost::string_view type;
    static const token::code::value code = token::code::method;
};

struct request_target
{
    typedef boost::string_view type;
    static const token::code::value code = token::code::request_target;
};

struct version
{
    typedef int type;
    static const token::code::value code = token::code::version;
};

struct status_code
{
    typedef uint_least16_t type;
    static const token::code::value code = token::code::status_code;
};

struct reason_phrase
{
    typedef boost::string_view type;
    static const token::code::value code = token::code::reason_phrase;
};

} // namespace token
} // namespace http
} // namespace boost

#include "token.ipp"

#endif // BOOST_HTTP_TOKEN_HPP
