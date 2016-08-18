/* Copyright (c) 2016 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

namespace boost {
namespace http {
namespace syntax {

template<class CharT>
std::size_t status_code<CharT>::match(view_type view)
{
    /* status‑code    = 3DIGIT

       Section 3.1.2 of RFC7230. */
    const std::size_t size = 3;

    if (view.size() < size)
        return 0;

    for (int i = 0 ; i != size ; ++i) {
        if (!detail::is_digit(view[i]))
            return 0;
    }

    return size;
}

template<class CharT>
uint_least16_t status_code<CharT>::decode(view_type view)
{
    assert(view.size() == 3);
    return (view[0] - '0') * 100 + (view[1] - '0') * 10 + view[2] - '0';
}

} // namespace syntax
} // namespace http
} // namespace boost
