namespace boost {
namespace http {

template<class String>
String to_string(status_code sc)
{
    switch (sc) {
    case status_code::continue_request:
        return "Continue";
    case status_code::switching_protocols:
        return "Switching Protocols";
    case status_code::processing:
        return "Processing";
    case status_code::ok:
        return "OK";
    case status_code::created:
        return "Created";
    case status_code::accepted:
        return "Accepted";
    case status_code::non_authoritative_information:
        return "Non-Authoritative Information";
    case status_code::no_content:
        return "No Content";
    case status_code::reset_content:
        return "Reset Content";
    case status_code::partial_content:
        return "Partial Content";
    case status_code::multi_status:
        return "Multi-Status";
    case status_code::already_reported:
        return "Already Reported";
    case status_code::im_used:
        return "IM Used";
    case status_code::multiple_choices:
        return "Multiple Choices";
    case status_code::moved_permanently:
        return "Moved Permanently";
    case status_code::found:
        return "Found";
    case status_code::see_other:
        return "See Other";
    case status_code::not_modified:
        return "Not Modified";
    case status_code::use_proxy:
        return "Use Proxy";
    case status_code::switch_proxy:
        return "Switch Proxy";
    case status_code::temporary_redirect:
        return "Temporary Redirect";
    case status_code::permanent_redirect:
        return "Permanent Redirect";
    case status_code::bad_request:
        return "Bad Request";
    case status_code::unauthorized:
        return "Unauthorized";
    case status_code::payment_required:
        return "Payment Required";
    case status_code::forbidden:
        return "Forbidden";
    case status_code::not_found:
        return "Not Found";
    case status_code::method_not_allowed:
        return "Method Not Allowed";
    case status_code::not_acceptable:
        return "Not Acceptable";
    case status_code::proxy_authentication_required:
        return "Proxy Authentication Required";
    case status_code::request_timeout:
        return "Request Timeout";
    case status_code::conflict:
        return "Conflict";
    case status_code::gone:
        return "Gone";
    case status_code::length_required:
        return "Length Required";
    case status_code::precondition_failed:
        return "Precondition Failed";
    case status_code::payload_too_large:
        return "Payload Too Large";
    case status_code::uri_too_long:
        return "URI Too Long";
    case status_code::unsupported_media_type:
        return "Unsupported Media Type";
    case status_code::requested_range_not_satisfiable:
        return "Requested Range Not Satisfiable";
    case status_code::expectation_failed:
        return "Expectation Failed";
    case status_code::unprocessable_entity:
        return "Unprocessable Entity";
    case status_code::locked:
        return "Locked";
    case status_code::failed_dependency:
        return "Failed Dependency";
    case status_code::upgrade_required:
        return "Upgrade Required";
    case status_code::precondition_required:
        return "Precondition Required";
    case status_code::too_many_requests:
        return "Too Many Requests";
    case status_code::request_header_fields_too_large:
        return "Request Header Fields Too Large";
    case status_code::internal_server_error:
        return "Internal Server Error";
    case status_code::not_implemented:
        return "Not Implemented";
    case status_code::bad_gateway:
        return "Bad Gateway";
    case status_code::service_unavailable:
        return "Service Unavailable";
    case status_code::gateway_timeout:
        return "Gateway Timeout";
    case status_code::http_version_not_supported:
        return "HTTP Version Not Supported";
    case status_code::variant_also_negotiates:
        return "Variant Also Negotiates";
    case status_code::insufficient_storage:
        return "Insufficient Storage";
    case status_code::loop_detected:
        return "Loop Detected";
    case status_code::not_extended:
        return "Not Extended";
    case status_code::network_authentication_required:
        return "Network Authentication Required";
    }
    return "";
}

} // namespace http
} // namespace boost
