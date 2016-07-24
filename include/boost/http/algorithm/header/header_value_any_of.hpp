/* Copyright (c) 2014, 2016 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_ALGORITHM_HEADER_VALUE_ANY_OF_HPP
#define BOOST_HTTP_ALGORITHM_HEADER_VALUE_ANY_OF_HPP

#include <algorithm>
#include <boost/algorithm/cxx11/find_if_not.hpp>

namespace boost {
namespace http {

namespace detail {

template<class char_type>
struct isspace
{
    static bool p(const char_type &c) {
        return c == ' ' || c == '\t';
    }
};

} // namespace detail

template<class StringRef, class Predicate>
bool header_value_any_of(const StringRef &header_value, Predicate p)
{
    using detail::isspace;

    typedef typename StringRef::value_type char_type;
    typedef typename StringRef::const_reverse_iterator reverse_iterator;
    typedef typename StringRef::const_iterator iterator;

    iterator comma = header_value.begin();
    iterator next_comma;
    do {
        next_comma = std::find(comma, header_value.end(), ',');

        iterator value_begin = algorithm::find_if_not(comma, next_comma,
                                                      isspace<char_type>::p);

        if (value_begin != next_comma) {
            iterator value_end
                = algorithm::find_if_not(reverse_iterator(next_comma),
                                         reverse_iterator(value_begin),
                                         isspace<char_type>::p).base();
            if (value_begin != value_end
                && p(header_value.substr(value_begin - header_value.begin(),
                                         value_end - value_begin))) {
                return true;
            }
        }

        comma = next_comma;

        /* skip comma, so won't process an empty string in the next iteration
           and enter within an infinite loop afterwards. */
        if (next_comma != header_value.end())
            ++comma;
    } while (comma != header_value.end());
    return false;
}

} // namespace http
} // namespace boost

#endif // BOOST_HTTP_ALGORITHM_HEADER_VALUE_ANY_OF_HPP
