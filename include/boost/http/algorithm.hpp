/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_ALGORITHM_H
#define BOOST_HTTP_ALGORITHM_H

#include <cctype>

#include <algorithm>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/range/iterator_range.hpp>

#include <boost/http/message.hpp>

namespace boost {
namespace http {

namespace detail {

/**
 * \p's signature MUST be:
 * bool(String::const_iterator begin, String::const_iterator end);
 *
 * This function should only be "updated" to the public interface after
 * string_view is available and the function is updated to make use of it.
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
            if (value_begin != value_end && p(value_begin, value_end))
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

} // namespace detail

/**
 * Check if received headers include `100-continue` in the _Expect_ header.
 *
 * The name _required_ is used instead _supported_, because a 100-continue
 * status require action from the server.
 */
template<class Message>
bool incoming_request_continue_required(const Message &message)
{
    typedef decltype(*message.headers.begin()) value_type;
    auto values = message.headers.equal_range("expect");

    return std::any_of(values.first, values.second, [](const value_type &v) {
        return iequals(v.second, "100-continue");
    });
}

/**
 * Check if the client desires to initiate a protocol upgrade.
 *
 * The desired protocols will be in the `upgrade` header as a comma-separated
 * list.
 *
 * The upgrade can always be safely ignored.
 */
template<class Message>
bool incoming_request_upgrade_desired(const Message &message)
{
    typedef typename Message::headers_type::value_type header_type;
    typedef typename header_type::second_type field_value_type; // a string
    typedef typename field_value_type::const_iterator field_value_iterator;
    using detail::header_value_any_of;

    auto connection_headers = message.headers.equal_range("connection");

    auto contains_upgrade = [](const field_value_iterator &begin,
                               const field_value_iterator &end) {
        return iequals(make_iterator_range(begin, end), "upgrade");
    };

    return std::any_of(connection_headers.first, connection_headers.second,
                       [contains_upgrade](const header_type &v) {
                           return header_value_any_of(v.second,
                                                      contains_upgrade);
                       })
        && message.headers.find("upgrade") != message.headers.end();
}

} // namespace http
} // namespace boost

#endif // BOOST_HTTP_ALGORITHM_H
