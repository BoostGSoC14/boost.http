/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_ALGORITHM_HEADER_HPP
#define BOOST_HTTP_ALGORITHM_HEADER_HPP

#include <cctype>

#include <algorithm>

#include <boost/utility/string_ref.hpp>

namespace boost {
namespace http {

/**
 * \p's signature MUST be:
 * bool(boost::string_ref value);
 */
template<class String, class Predicate>
bool header_value_any_of(const String &header_value, const Predicate &p)
{
    typedef typename String::value_type char_type;
    typedef typename String::const_reverse_iterator reverse_iterator;

    auto comma = header_value.begin();
    decltype(comma) next_comma;
    do {
        next_comma = std::find(comma, header_value.end(), ',');

        auto value_begin = std::find_if_not(comma, next_comma,
                                            [](const char_type &c) {
                                                return std::isspace(c);
                                            });

        if (value_begin != next_comma) {
            auto value_end = std::find_if_not(reverse_iterator(next_comma),
                                              reverse_iterator(value_begin),
                                              [](const char_type &c){
                                                  return std::isspace(c);
                                              }).base();
            if (value_begin != value_end
                && p(string_ref(&*value_begin, value_end - value_begin)))
                return true;
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

#endif // BOOST_HTTP_ALGORITHM_HEADER_HPP
