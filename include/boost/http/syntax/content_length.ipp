/* Copyright (c) 2016 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

namespace boost {
namespace http {
namespace syntax {

template<class CharT>
template<class Target>
typename content_length<CharT>::result
content_length<CharT>::decode(view_type in, Target &out)
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
            if (std::numeric_limits<Target>::max() / 10 < digit)
                return result::overflow;
            else
                digit *= 10;

            --i;
        }
    }
    return result::ok;
}

} // namespace syntax
} // namespace http
} // namespace boost
