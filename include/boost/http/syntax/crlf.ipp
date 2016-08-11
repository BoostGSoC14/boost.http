/* Copyright (c) 2016 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

namespace boost {
namespace http {
namespace syntax {

template<class CharT>
std::size_t strict_crlf<CharT>::match(view_type view)
{
    /* CRLF =  %x0D %x0A

       Appendix B of RFC5234. */
    const std::size_t size = 2;

    if (view.size() < size)
        return 0;

    if (view[0] != 0x0D || view[1] != 0x0A)
        return 0;

    return 2;
}

template<class CharT>
typename liberal_crlf<CharT>::result liberal_crlf<CharT>::match(view_type view)
{
    /* Although the line terminator for the start-line and header fields is the
       sequence CRLF, a recipient MAY recognize a single LF as a line terminator
       and ignore any preceding CR (section 3.5 of RFC7230). */
    enum {
        CR = 0x0D,
        LF = 0x0A
    };

    if (view.size() >= 1) {
        switch (view[0]) {
        case LF:
            return result::lf;
        case CR:
            break;
        default:
            return result::invalid_data;
        }
    }

    if (view.size() >= 2) {
        switch (view[1]) {
        case LF:
            return result::crlf;
        default:
            return result::invalid_data;
        }
    }

    return result::insufficient_data;
}

} // namespace syntax
} // namespace http
} // namespace boost
