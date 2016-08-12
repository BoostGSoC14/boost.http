/* Copyright (c) 2016 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

namespace boost {
namespace http {
namespace syntax {

namespace detail {

template<class CharT>
bool is_tchar(CharT c)
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

} // namespace detail

template<class CharT>
std::size_t field_name<CharT>::match(view_type view)
{
    std::size_t res = 0;

    for (std::size_t i = 0 ; i != view.size() ; ++i) {
        if (!detail::is_tchar(view[i]))
            break;

        ++res;
    }

    return res;
}

} // namespace syntax
} // namespace http
} // namespace boost
