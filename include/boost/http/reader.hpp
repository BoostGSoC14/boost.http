/* Copyright (c) 2016 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */


#ifndef BOOST_HTTP_READER_HPP
#define BOOST_HTTP_READER_HPP

// private

#include <cctype>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/type_traits/common_type.hpp>
#include <boost/cstdint.hpp>

// public

#include <boost/system/error_code.hpp>

#include <boost/http/token.hpp>
#include <boost/http/algorithm/header/header_value_any_of.hpp>

namespace boost {
namespace http {

class request_reader
{
public:
    // types
    typedef std::size_t size_type;
    typedef const char value_type;
    typedef value_type *pointer;
    typedef boost::string_ref view_type;

    request_reader();

    // Inspect current token
    token::code::value code() const;
    system::error_code error() const BOOST_NOEXCEPT;
    size_type token_size() const;
    template<class T>
    typename T::type value() const;

    /* You can use this function if you're getting `error_insufficient_data` but
       cannot allocate more data into the buffer. You'll know which token would
       come next and report an appropriate answer back to the client (e.g. 431
       Request Header Fields Too Large). The `error_insufficient_data` error
       should never happens to deliver body chunks as they're notified in
       chunks. */
    token::code::value expected_token() const;

    // Error handling

    // Consumes current element and goes to the next one
    void next();

    /**
     * It's expected that unread bytes from previous buffer will be present at
     * the beginning of \p inbuffer (i.e. you MUST NOT discard unread bytes from
     * previous buffer).
     */
    void set_buffer(asio::const_buffer inbuffer);

private:
    enum State {
        ERRORED,
        EXPECT_METHOD,
        EXPECT_SP_AFTER_METHOD,
        EXPECT_REQUEST_TARGET,
        EXPECT_STATIC_STR_AFTER_TARGET,
        EXPECT_VERSION,
        EXPECT_CRLF_AFTER_VERSION,
        EXPECT_FIELD_NAME,
        EXPECT_COLON,
        EXPECT_OWS_AFTER_COLON,
        EXPECT_FIELD_VALUE,
        EXPECT_CRLF_AFTER_FIELD_VALUE,
        EXPECT_CRLF_AFTER_HEADERS,
        EXPECT_BODY,
        EXPECT_EOM,
        EXPECT_CHUNK_SIZE,
        EXPECT_CHUNK_EXT,
        EXPEXT_CRLF_AFTER_CHUNK_EXT,
        EXPECT_CHUNK_DATA,
        EXPECT_CRLF_AFTER_CHUNK_DATA,
        EXPECT_TRAILER_NAME,
        EXPECT_TRAILER_COLON,
        EXPECT_OWS_AFTER_TRAILER_COLON,
        EXPECT_TRAILER_VALUE,
        EXPECT_CRLF_AFTER_TRAILER_VALUE,
        EXPECT_CRLF_AFTER_TRAILERS
    };

    enum {
        HTTP_1_0,
        NOT_HTTP_1_0_AND_HOST_NOT_READ,
        NOT_HTTP_1_0_AND_HOST_READ
    } version;

    // State that needs to be reset at every new request {{{

    enum {
        // Initial state
        NO_BODY,
        // Set after decoding the field value {{{
        CONTENT_LENGTH_READ,
        CHUNKED_ENCODING_READ,
        RANDOM_ENCODING_READ,
        // }}}
        // Set after decoding the field name {{{
        READING_CONTENT_LENGTH,
        READING_ENCODING
        // }}}
    } body_type;

    /* This state is special as it is allowed to keep garbage while
       `body_type == NO_BODY`. */
    uint_least64_t body_size;

    // }}}

    State state;

    /* Once `next()` is called to start reading a new token, `code_` must
       immediately change to `error_insufficient_data` and only change once the
       new token has been completely read (or erroed).*/
    token::code::value code_;

    /* `idx` always point to the beginning of the currently being parsed token
       in the buffer. */
    size_type idx;

    /* if `code_ == error_insufficient_data`, token_size_ has the amount of data
       already parsed from current token. Otherwise, it contains the token
       size. */
    size_type token_size_;
    boost::asio::const_buffer ibuffer;
};

} // namespace http
} // namespace boost

#include "reader-inl.hpp"

#endif // BOOST_HTTP_READER_HPP
