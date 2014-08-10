/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_STATUS_CODE_HPP
#define BOOST_HTTP_STATUS_CODE_HPP

namespace boost {
namespace http {

/**
 * Contains useful status codes defined in RFC2616.
 *
 * A client library can receive integers not enumarated in this abstraction,
 * then an integer would be chosen instead of this enum in such client library,
 * but this abstraction is still useful for comparassions maintaining readable
 * code.
 */
enum class status_code
{
    // Informational (1xx)
    continue_request = 100,
    switching_protocols = 101,
    processing = 102,
    // Successful (2xx)
    ok = 200,
    created = 201,
    accepted = 202,
    non_authoritative_information = 203,
    no_content = 204,
    reset_content = 205,
    partial_content = 206,
    multi_status = 207,
    already_reported = 208,
    im_used = 226,
    // Redirection (3xx)
    multiple_choices = 300,
    moved_permanently = 301,
    found = 302,
    see_other = 303,
    not_modified = 304,
    use_proxy = 305,
    switch_proxy = 306, //< No longer used
    temporary_redirect = 307,
    permanent_redirect = 308,
    // Client Error (4xx)
    bad_request = 400,
    unauthorized = 401,
    payment_required = 402, //< Reserved for future usage
    forbidden = 403,
    not_found = 404,
    method_not_allowed = 405,
    not_acceptable = 406,
    proxy_authentication_required = 407,
    request_timeout = 408,
    conflict = 409,
    gone = 410,
    length_required = 411,
    precondition_failed = 412,
    payload_too_large = 413,
    uri_too_long = 414,
    unsupported_media_type = 415,
    requested_range_not_satisfiable = 416,
    expectation_failed = 417,
    unprocessable_entity = 422,
    locked = 423,
    failed_dependencey = 424,
    upgrade_required = 426,
    precondition_required = 428,
    too_many_requests = 429,
    request_header_fields_too_large = 431,
    // Server Error (5xx)
    internal_server_error = 500,
    not_implemented = 501,
    bad_gateway = 502,
    service_unavailable = 503,
    gateway_timeout = 504,
    http_version_not_supported = 505,
    variant_also_negotiates = 506,
    insufficient_storage = 507,
    loop_detected = 508,
    not_extended = 510,
    network_authentication_required = 511
};

inline bool operator==(status_code lhs, int rhs)
{
    return static_cast<int>(lhs) == rhs;
}

inline bool operator==(int lhs, status_code rhs)
{
    return lhs == static_cast<int>(rhs);
}

} // namespace http
} // namespace boost

#endif // BOOST_HTTP_STATUS_CODE_HPP
