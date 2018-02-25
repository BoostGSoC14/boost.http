/* Copyright (c) 2014, 2017 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_ALGORITHM_QUERY_HPP
#define BOOST_HTTP_ALGORITHM_QUERY_HPP

#include <boost/utility/string_view.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <boost/http/traits.hpp>
#include <boost/http/algorithm/header.hpp>

namespace boost {
namespace http {

template<class Request>
bool request_continue_required(const Request &request)
{
    static_assert(is_request_message<Request>::value,
                  "Request must fulfill the Request concept");

    auto values = request.headers().equal_range("expect");

    return std::distance(values.first, values.second) == 1
        && iequals(values.first->second, "100-continue");
}

template<class Request,
         class StringView = boost::basic_string_view<
             typename Request::headers_type::mapped_type::value_type>>
bool request_upgrade_desired(const Request &request)
{
    static_assert(is_request_message<Request>::value,
                  "Request must fulfill the Request concept");

    typedef typename Request::headers_type::value_type header_type;

    auto connection_headers = request.headers().equal_range("connection");

    auto contains_upgrade = [](const StringView &value) {
        return iequals(value, "upgrade");
    };

    return request.headers().find("upgrade") != request.headers().end()
        && std::any_of(connection_headers.first, connection_headers.second,
                       [contains_upgrade](const header_type &v) {
                           return header_value_any_of(v.second,
                                                      contains_upgrade);
                       });
}

} // namespace http
} // namespace boost

#endif // BOOST_HTTP_ALGORITHM_QUERY_HPP
