/* Copyright (c) 2016 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */


#ifndef BOOST_HTTP_READER_DETAIL_ABNF_HPP
#define BOOST_HTTP_READER_DETAIL_ABNF_HPP

namespace boost {
namespace http {
namespace reader {
namespace detail {

inline bool isalpha(unsigned char c)
{
    /* ALPHA          =  %x41-5A / %x61-7A   ; A-Z / a-z

       from Appendix B of RFC5234. */
    return (c >= 0x41 && c <= 0x5A) || (c >= 0x61 && c <= 0x7A);
}

inline bool isdigit(unsigned char c)
{
    /* DIGIT          =  %x30-39   ; 0-9

       from Appendix B of RFC5234. */
    return c >= 0x30 && c <= 0x39;
}

inline bool isalnum(unsigned char c)
{
    return isalpha(c) || isdigit(c);
}

inline bool is_tchar(unsigned char c)
{
    switch (c) {
    case '!': case '#': case '$': case '%': case '&': case '\'': case '*':
    case '+': case '-': case '.': case '^': case '_': case '`': case '|':
    case '~':
        return true;
    default:
        return isalnum(c);
    }
}

inline bool is_sp(unsigned char c)
{
    return c == '\x20';
}

inline bool is_vchar(unsigned char c)
{
    return c >= '\x21' && c <= '\x7E';
}

inline bool is_obs_text(unsigned char c)
{
    return c >= 0x80 && c <= 0xFF;
}

inline bool is_ows(unsigned char c)
{
    switch (c) {
    case '\x20':
    case '\x09':
        return true;
    default:
        return false;
    }
}

inline bool is_chunk_ext_char(unsigned char c)
{
    switch (c) {
    case ';':
    case '=':
    case '"':
    case '\t':
    case ' ':
    case '!':
    case '\\':
        return true;
    default:
        return is_tchar(c) || (c >= 0x23 && c <= 0x5B)
            || (c >= 0x5D && c <= 0x7E) || is_obs_text(c) || is_vchar(c);
    }
}

} // namespace detail
} // namespace reader
} // namespace http
} // namespace boost

#endif // BOOST_HTTP_READER_DETAIL_ABNF_HPP
