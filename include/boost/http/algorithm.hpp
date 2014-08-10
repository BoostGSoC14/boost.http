/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_ALGORITHM_H
#define BOOST_HTTP_ALGORITHM_H

#include <cctype>

#include <algorithm>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/utility/string_ref.hpp>

#include <boost/http/message.hpp>

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

/**
 * Check if received headers include `100-continue` in the _Expect_ header.
 *
 * The name _required_ is used instead _supported_, because a 100-continue
 * status require action from the server.
 */
template<class Message>
bool request_continue_required(const Message &message)
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
bool request_upgrade_desired(const Message &message)
{
    typedef typename Message::headers_type::value_type header_type;

    auto connection_headers = message.headers.equal_range("connection");

    auto contains_upgrade = [](const string_ref &value) {
        return iequals(value, "upgrade");
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
