/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#include <stdexcept>
#include <boost/http/embedded_server_socket.hpp>

using boost::http::detail::http_parser;
using boost::http::detail::http_parser_settings;

// ### BEGINNING OF DECLARATIONS FROM http_parser.h ###

extern "C" {

enum http_parser_type { HTTP_REQUEST, HTTP_RESPONSE, HTTP_BOTH };

void http_parser_init(http_parser *parser, enum http_parser_type type);
std::size_t http_parser_execute(http_parser *parser,
                                const http_parser_settings *settings,
                                const char *data,
                                std::size_t len);
int http_should_keep_alive(const http_parser *parser);
int http_body_is_final(const http_parser *parser);

#define HTTP_ERRNO_MAP(XX)                                           \
  /* No error */                                                     \
  XX(OK, "success")                                                  \
                                                                     \
  /* Callback-related errors */                                      \
  XX(CB_message_begin, "the on_message_begin callback failed")       \
  XX(CB_url, "the on_url callback failed")                           \
  XX(CB_header_field, "the on_header_field callback failed")         \
  XX(CB_header_value, "the on_header_value callback failed")         \
  XX(CB_headers_complete, "the on_headers_complete callback failed") \
  XX(CB_body, "the on_body callback failed")                         \
  XX(CB_message_complete, "the on_message_complete callback failed") \
  XX(CB_status, "the on_status callback failed")                     \
                                                                     \
  /* Parsing-related errors */                                       \
  XX(INVALID_EOF_STATE, "stream ended at an unexpected time")        \
  XX(HEADER_OVERFLOW,                                                \
     "too many header bytes seen; overflow detected")                \
  XX(CLOSED_CONNECTION,                                              \
     "data received after completed connection: close message")      \
  XX(INVALID_VERSION, "invalid HTTP version")                        \
  XX(INVALID_STATUS, "invalid HTTP status code")                     \
  XX(INVALID_METHOD, "invalid HTTP method")                          \
  XX(INVALID_URL, "invalid URL")                                     \
  XX(INVALID_HOST, "invalid host")                                   \
  XX(INVALID_PORT, "invalid port")                                   \
  XX(INVALID_PATH, "invalid path")                                   \
  XX(INVALID_QUERY_STRING, "invalid query string")                   \
  XX(INVALID_FRAGMENT, "invalid fragment")                           \
  XX(LF_EXPECTED, "LF character expected")                           \
  XX(INVALID_HEADER_TOKEN, "invalid character in header")            \
  XX(INVALID_CONTENT_LENGTH,                                         \
     "invalid character in content-length header")                   \
  XX(INVALID_CHUNK_SIZE,                                             \
     "invalid character in chunk size header")                       \
  XX(INVALID_CONSTANT, "invalid constant string")                    \
  XX(INVALID_INTERNAL_STATE, "encountered unexpected internal state")\
  XX(STRICT, "strict mode assertion failed")                         \
  XX(PAUSED, "parser is paused")                                     \
  XX(UNKNOWN, "an unknown error occurred")

#define HTTP_ERRNO_GEN(n, s) HPE_##n,
enum http_errno {
  HTTP_ERRNO_MAP(HTTP_ERRNO_GEN)
};
#undef HTTP_ERRNO_GEN

} // extern "C"

// ### END OF DECLARATIONS FROM http_parser.h ###

namespace boost {
namespace http {

void embedded_server_socket::init(http_parser &parser)
{
    http_parser_init(&parser, HTTP_REQUEST);

    static_assert((int(parser_error::cb_headers_complete)
                   == HPE_CB_headers_complete)
                  && (int(parser_error::cb_message_complete)
                      == HPE_CB_message_complete),
                  "parser_error enum is out-of-sync with current version of"
                  " Ryan Dahl's HTTP parser");
}

std::size_t embedded_server_socket::execute(http_parser &parser,
                                            const http_parser_settings &settings,
                                            const std::uint8_t *data,
                                            std::size_t len)
{
    return http_parser_execute(&parser, &settings,
                               reinterpret_cast<const char*>(data), len);
}

bool embedded_server_socket::should_keep_alive(const http_parser &parser)
{
    return http_should_keep_alive(&parser);
}

bool embedded_server_socket::body_is_final(const http_parser &parser)
{
    return http_body_is_final(&parser);
}

} // namespace http
} // namespace boost
