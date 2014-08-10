/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_ALGORITHM_QUERY_HPP
#define BOOST_HTTP_ALGORITHM_QUERY_HPP

#include <boost/algorithm/string/predicate.hpp>

#include <boost/http/message.hpp>
#include <boost/http/algorithm/header.hpp>

namespace boost {
namespace http {

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

#endif // BOOST_HTTP_ALGORITHM_QUERY_HPP
