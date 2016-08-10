/* Copyright (c) 2016 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

namespace boost {
namespace http {
namespace syntax {

namespace detail {

template<class CharT>
bool is_hexdigit(CharT c)
{
    switch (c) {
    case '0': case '1': case '2': case '3': case '4': case '5': case '6':
    case '7': case '8': case '9': case 'A': case 'B': case 'C': case 'D':
    case 'E': case 'F': case 'a': case 'b': case 'c': case 'd': case 'e':
    case 'f':
        return true;
    default:
        return false;
    }
}

} // namespace detail

template<class CharT>
std::size_t chunk_size<CharT>::match(view_type view)
{
    std::size_t res = 0;

    for (std::size_t i = 0 ; i != view.size() ; ++i) {
        if (!detail::is_hexdigit(view[i]))
            break;

        ++res;
    }

    return res;
}

template<class CharT>
template<class Target>
typename chunk_size<CharT>::result
chunk_size<CharT>::decode(view_type in, Target &out)
{
    if (in.size() == 0)
        return result::invalid;

    out = 0;

    while (in.size() && in[0] == '0')
        in.remove_prefix(1);

    if (in.size() == 0)
        return result::ok;

    Target digit = 1;

    for ( std::size_t i = in.size() - 1 ; ; ) {
        Target value;

        switch (in[i]) {
        case '0': case '1': case '2': case '3': case '4': case '5': case '6':
        case '7': case '8': case '9':
            value = in[i] - '0';
            break;
        case 'a': case 'A': case 'b': case 'B': case 'c': case 'C': case 'd':
        case 'D': case 'e': case 'E': case 'f': case 'F':
            {
                /* "lower case bit" = 0x20 */
                char c = in[i] | 0x20;
                value = 10 + c - 'a';
            }
            break;
        default:
            return result::invalid;
        }

        if (std::numeric_limits<Target>::max() / digit < value)
            return result::overflow;

        value *= digit;

        if (std::numeric_limits<Target>::max() - value < out)
            return result::overflow;

        out += value;

        if (i == 0) {
            break;
        } else {
            if (std::numeric_limits<Target>::max() / 16 < digit)
                return result::overflow;
            else
                digit *= 16;

            --i;
        }
    }
    return result::ok;
}

} // namespace syntax
} // namespace http
} // namespace boost
