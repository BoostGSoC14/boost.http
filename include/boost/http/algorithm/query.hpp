/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_ALGORITHM_QUERY_HPP
#define BOOST_HTTP_ALGORITHM_QUERY_HPP

#include <boost/utility/string_ref.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <boost/http/message.hpp>
#include <boost/http/algorithm/header.hpp>

namespace boost {
namespace http {

template<class Message>
bool request_continue_required(const Message &message)
{
    static_assert(is_message<Message>::value,
                  "Message must fulfill the Message concept");

    auto values = message.headers().equal_range("expect");

    return std::distance(values.first, values.second) == 1
        && iequals(values.first->second, "100-continue");
}

template<class Message,
         class StringRef = boost::basic_string_ref<
             typename Message::headers_type::mapped_type::value_type>>
bool request_upgrade_desired(const Message &message)
{
    static_assert(is_message<Message>::value,
                  "Message must fulfill the Message concept");

    typedef typename Message::headers_type::value_type header_type;

    auto connection_headers = message.headers().equal_range("connection");

    auto contains_upgrade = [](const StringRef &value) {
        return iequals(value, "upgrade");
    };

    return message.headers().find("upgrade") != message.headers().end()
        && std::any_of(connection_headers.first, connection_headers.second,
                       [contains_upgrade](const header_type &v) {
                           return header_value_any_of(v.second,
                                                      contains_upgrade);
                       });
}

} // namespace http
} // namespace boost

#endif // BOOST_HTTP_ALGORITHM_QUERY_HPP
