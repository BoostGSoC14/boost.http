/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_FILE_SERVER_HPP
#define BOOST_HTTP_FILE_SERVER_HPP

// TODO (Boost 1.5X): replace by AFIO?
#include <boost/filesystem/fstream.hpp>
#include <memory>

#include <boost/system/error_code.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/utility/string_ref.hpp>

#include <boost/http/algorithm/header.hpp>
#include <boost/http/write_state.hpp>
#include <boost/http/detail/constchar_helper.hpp>

#ifndef BOOST_HTTP_FILE_SERVER_BOUNDARY
/* MUST NOT be empty and SHOULD have the smallest size possible (1).

   Also, MUST fulfill the HTTP token rule:

       token = 1*tchar
       tchar = "!" / "#" / "$" / "%" / "&" / "’" / "*" / "+" / "-" / "." / "^" /
               "_" / "‘" / "|" / "~" / DIGIT / ALPHA
       DIGIT = ? decimal 0-9 ?
       ALPHA = ? letters ?

   Boundaries MUST NOT be longer than 66 characters.
*/
#define BOOST_HTTP_FILE_SERVER_BOUNDARY "-"
#endif // BOOST_HTTP_FILE_SERVER_BOUNDARY

namespace boost {
namespace http {

enum class file_server_errc {
    io_error = 1,
    irrecoverable_io_error,
    write_state_not_supported,
    file_not_found,
    file_type_not_supported,
    filter_set
};

} // namespace http
namespace system {

/**
 * Extends the type trait boost::system::is_error_code_enum to identify
 * file_server error codes.
 */
template<>
struct is_error_code_enum<boost::http::file_server_errc>: public std::true_type
{};

/**
 * Extends the type trait boost::system::is_error_code_enum to identify
 * file_server error conditions.
 */
template<>
struct is_error_condition_enum<boost::http::file_server_errc>
    : public std::true_type
{};

} // namespace system
namespace http {

const system::error_category& file_server_category();

/**
 * Constructs a file_server error_code.
 */
inline system::error_code make_error_code(file_server_errc e)
{
    return system::error_code(static_cast<int>(e), file_server_category());
}

/**
 * Constructs a file_server error_condition.
 */
inline system::error_condition make_error_condition(file_server_errc e)
{
    return system::error_condition(static_cast<int>(e), file_server_category());
}

namespace detail {

// HTTP-date precision goes until seconds. discard any extra precision.
inline
posix_time::ptime last_modified_http_date(const filesystem::path &file)
{
    auto last_modified = posix_time::from_time_t(last_write_time(file));
    auto d = last_modified.time_of_day();
    return posix_time::ptime(last_modified.date(),
                             posix_time::time_duration(d.hours(), d.minutes(),
                                                       d.seconds()));
}

inline
void insert_new_range(std::vector<std::pair<std::uintmax_t, std::uintmax_t>>
                      &range_set,
                      std::pair<std::uintmax_t, std::uintmax_t> range)
{
    /* TODO (improvement):

       Coalesce any of the ranges that overlap, or that are separated by a gap
       that is smaller than the overhead of sending multiple parts.

       Prefer to keep the already existing order rather than bringing a previous
       chunk to the soon-to-be-inserted new chunk

       If the new range is merged with a previous chunk, it won't be
       (re)inserted (again). */
    range_set.push_back(range);
}

/**
 * WARNING: even if the function returns false, \p range_set may be changed.
 *
 * WARNING: this function cannot handle file_size == 0
 */
template<class String>
bool is_valid_range(const String &value, std::uintmax_t file_size,
                    std::vector<std::pair<std::uintmax_t, std::uintmax_t>>
                    &range_set)
{
    typedef typename String::value_type CharT;
    typedef basic_string_ref<CharT> string_ref_type;
    typedef std::basic_regex<typename string_ref_type::value_type> regex_type;

    assert(file_size);

    const char prefix[] = "bytes=";
    constexpr auto prefix_size = 6;

    if (value.size() <= prefix_size)
        return false;

    if (!std::equal(prefix, prefix + prefix_size, value.begin()))
        return false;

    string_ref_type range_set_value(value);
    range_set_value.remove_prefix(prefix_size);

    /* accumulates ranges into range_set

       returns true if range is invalid */
    auto fail = [&range_set,file_size](const string_ref_type &value) {
        typedef std::match_results<typename string_ref_type::const_iterator>
        match_results_type;
        typedef typename match_results_type::value_type sub_match_type;

        static const regex_type regex("(\\d*)-(\\d*)");
        match_results_type matches;

        if (!std::regex_match(value.begin(), value.end(), matches, regex))
            return true;

        auto first_byte_pos = matches[1];
        auto last_byte_pos = matches[2];

        if (first_byte_pos.length() == 0 && last_byte_pos.length() == 0)
            return true;

        auto from_decimal = [](const sub_match_type &submatch) {
            /* note: detail::from_decimal_submatch is defined in
               algorithm/header.hpp */
            return detail::from_decimal_submatch<std::uintmax_t>(submatch);
        };

        std::pair<std::uintmax_t, std::uintmax_t> range;

        try {
            /* The order in which all the operations are processed were very
               carefully crafted to avoid integer overflow. Don't screw it!
               Although all the tests should be implemented already, this is UB
               land and errors cannot always be detected automatically. */
            if (first_byte_pos.length()) {
                // byte-range-spec
                range.first = from_decimal(first_byte_pos);

                if (range.first >= file_size)
                    return false;

                if (last_byte_pos.length()) {
                    try {
                        range.second = from_decimal(last_byte_pos);

                        if (range.second < range.first)
                            return false;

                        if (range.second > file_size - 1)
                            range.second = file_size - 1;
                    } catch(std::overflow_error&) {
                        range.second = file_size - 1;
                    }
                } else {
                    range.second = file_size - 1;
                }
                insert_new_range(range_set, range);
            } else {
                // suffix-byte-range-spec
                try {
                    // range.second initially means size
                    range.second = from_decimal(last_byte_pos);

                    if (range.second == 0)
                        return false;

                    if (range.second > file_size)
                        range.second = file_size;
                    range.first = file_size - range.second;
                } catch(std::overflow_error&) {
                    range.first = 0;
                }
                range.second = file_size - 1;
                insert_new_range(range_set, range);
            }

            return false;
        } catch(std::overflow_error&) {
            /* the current byte-range-spec/suffix-byte-range-spec isn't
               satisfiable, but it's still possible that remaining specs will
               build a byte-range-set valid and satisfiable.

               the other errors weren't treated as warnings because it could be
               an attack attempt making use of an invalid parsing routine
               implemented in some cache along the way. */
            return false;
        }
    };

    if (header_value_any_of(range_set_value, fail))
        return false;

    return range_set.size();
}

/**
 * Converts to the usual [begin,end) C++ convention.
 *
 * WARNING: range MUST be valid (parsed and checked) BEFORE this function is
 * called.
 */
inline
void to_cpp_range(std::pair<std::uintmax_t, std::uintmax_t> &range)
{
    /* The order in which all the operations are processed were very carefully
       crafted to avoid integer overflow. Don't screw it!  Although all the
       tests should be implemented already, this is UB land and errors cannot
       always be detected automatically. */
    range.second -= range.first;
    ++range.second;
}

/* a and b MUST NOT be signed */
template<class A, class B>
auto safe_add(const A &a, const B &b) -> decltype(a+b)
{
    constexpr auto max = std::numeric_limits<decltype(a+b)>::max();
    if (max - a < b)
        throw std::overflow_error("a + b > max");
    return a + b;
}

template<class Socket, class Message, class Handler>
struct on_async_response_transmit_file
    : public std::enable_shared_from_this<
        on_async_response_transmit_file<Socket,Message,Handler>>
{
    on_async_response_transmit_file(Socket &socket, Message &omessage,
                                    Handler &&handler,
                                    const filesystem::path &file,
                                    std::uintmax_t remaining)
        : socket(socket)
        , message(omessage)
        , handler(handler)
        , stream(file)
        , remaining(remaining)
    {
        stream.exceptions(filesystem::ifstream::badbit
                          | filesystem::ifstream::failbit
                          | filesystem::ifstream::eofbit);
    }

    void process(const system::error_code &ec)
    {
        if (ec) {
            handler(ec);
            return;
        }

        if (!remaining) {
            auto self = this->shared_from_this();
            socket.async_write_end_of_message([self](const system::error_code
                                                     &ec) {
                                                  self->handler(ec);
                                              });
            return;
        }

        try {
            auto next_read
                = std::min<std::uintmax_t>(remaining, message.body().size());
            stream.read(reinterpret_cast<filesystem::ifstream::char_type*>
                        (message.body().data()),
                        next_read);
            remaining -= next_read;

            if (!remaining)
                message.body().resize(next_read);

            auto self = this->shared_from_this();
            socket.async_write(message, [self](const system::error_code &ec) {
                    self->process(ec);
                });
        } catch(std::ios_base::failure&) {
            handler(file_server_errc::irrecoverable_io_error);
        }
    }

    Socket &socket;
    Message &message;
    Handler handler;
    filesystem::ifstream stream;
    std::uintmax_t remaining;
};

template<class Socket, class Message, class String, class Handler>
struct on_async_response_transmit_file_multi
    : public std::enable_shared_from_this<
        on_async_response_transmit_file_multi<Socket, Message, String, Handler>>
{
    enum State
    {
        STATE_BOUNDARY_LINE,
        STATE_CONTENT_TYPE_KEY,
        STATE_CONTENT_TYPE_VALUE,
        STATE_CONTENT_TYPE_CRLF,
        STATE_CONTENT_RANGE_KEY,
        STATE_CONTENT_RANGE_VALUE_FIRST,
        STATE_CONTENT_RANGE_VALUE_DASH,
        STATE_CONTENT_RANGE_VALUE_SECOND,
        STATE_CONTENT_RANGE_VALUE_SLASH,
        STATE_CONTENT_RANGE_VALUE_SIZE,
        STATE_CONTENT_RANGE_CRLF,
        STATE_BODY,
        STATE_FINAL_BOUNDARY
    };

    on_async_response_transmit_file_multi
        (Socket &socket, Message &omessage, String &&content_type,
         Handler &&handler, const filesystem::path &file,
         std::vector<std::pair<std::uintmax_t, std::uintmax_t>> &&remaining,
         const uintmax_t &file_size)
        : socket(socket)
        , message(omessage)
        , content_type(std::move(content_type))
        , handler(handler)
        , stream(file)
        , range_set(remaining)
        , file_size(std::to_string(file_size))
    {
        stream.exceptions(filesystem::ifstream::badbit
                          | filesystem::ifstream::failbit
                          | filesystem::ifstream::eofbit);
        this->remaining
            = constchar_helper("\r\n--" BOOST_HTTP_FILE_SERVER_BOUNDARY "\r\n")
            .size;
    }

    void process(const system::error_code &ec)
    {
        if (ec) {
            handler(ec);
            return;
        }

        if (index == range_set.size()) {
            auto self = this->shared_from_this();
            socket.async_write_end_of_message([self](const system::error_code
                                                     &ec) {
                                                  self->handler(ec);
                                              });
            return;
        }

        using std::uintmax_t;
        using std::min;
        using std::copy;
        using std::back_inserter;
        using std::to_string;

        typedef typename Message::body_type::value_type body_value_type;

        constchar_helper crlf("\r\n");
        constchar_helper double_crlf("\r\n\r\n");
        constchar_helper
            boundary_line{"\r\n--" BOOST_HTTP_FILE_SERVER_BOUNDARY "\r\n"};
        constchar_helper
            final_boundary{"\r\n--" BOOST_HTTP_FILE_SERVER_BOUNDARY "--\r\n"};
        constchar_helper content_type_key("content-type: ");
        constchar_helper content_range_key("content-range: bytes ");

        auto schedule_more = [this]() {
            auto self = this->shared_from_this();
            socket.async_write(message, [self](const system::error_code &ec) {
                    self->process(ec);
                });
        };

        try {
            auto next_read = min<uintmax_t>(remaining, message.body().size());
            auto body_it = message.body().begin();

            while (true) {
                auto &range = range_set[index];

                switch (state) {
                case STATE_BOUNDARY_LINE: {
                    auto it = boundary_line.begin<body_value_type>()
                        + (boundary_line.size - remaining);
                    copy(it, it + next_read, body_it);
                    body_it += next_read;
                    remaining -= next_read;

                    if (remaining) {
                        schedule_more();
                        return;
                    } else {
                        state = content_type.size()
                            ? STATE_CONTENT_TYPE_KEY : STATE_CONTENT_RANGE_KEY;
                        remaining = content_type.size()
                            ? content_type_key.size : content_range_key.size;
                        next_read = min<uintmax_t>(remaining,
                                                   message.body().end()
                                                   - body_it);

                        if (!next_read) {
                            schedule_more();
                            return;
                        }

                        break;
                    }
                }
                case STATE_CONTENT_TYPE_KEY: {
                    auto it = content_type_key.begin<body_value_type>()
                        + (content_type_key.size - remaining);
                    copy(it, it + next_read, body_it);
                    body_it += next_read;
                    remaining -= next_read;

                    if (remaining) {
                        schedule_more();
                        return;
                    } else {
                        state = STATE_CONTENT_TYPE_VALUE;
                        remaining = content_type.size();
                        next_read = min<uintmax_t>(remaining,
                                                   message.body().end()
                                                   - body_it);

                        if (!next_read) {
                            schedule_more();
                            return;
                        }
                    }
                }
                case STATE_CONTENT_TYPE_VALUE: {
                    auto it = content_type.begin()
                        + (content_type.size() - remaining);
                    copy(it, it + next_read, body_it);
                    body_it += next_read;
                    remaining -= next_read;

                    if (remaining) {
                        schedule_more();
                        return;
                    } else {
                        state = STATE_CONTENT_TYPE_CRLF;
                        remaining = crlf.size;
                        next_read = min<uintmax_t>(remaining,
                                                   message.body().end()
                                                   - body_it);

                        if (!next_read) {
                            schedule_more();
                            return;
                        }
                    }
                }
                case STATE_CONTENT_TYPE_CRLF: {
                    auto it = crlf.begin<body_value_type>()
                        + (crlf.size - remaining);
                    copy(it, it + next_read, body_it);
                    body_it += next_read;
                    remaining -= next_read;

                    if (remaining) {
                        schedule_more();
                        return;
                    } else {
                        state = STATE_CONTENT_RANGE_KEY;
                        remaining = content_range_key.size;
                        next_read = min<uintmax_t>(remaining,
                                                   message.body().end()
                                                   - body_it);

                        if (!next_read) {
                            schedule_more();
                            return;
                        }
                    }
                }
                case STATE_CONTENT_RANGE_KEY: {
                    auto it = content_range_key.begin<body_value_type>()
                        + (content_range_key.size - remaining);
                    copy(it, it + next_read, body_it);
                    body_it += next_read;
                    remaining -= next_read;

                    if (remaining) {
                        schedule_more();
                        return;
                    } else {
                        state = STATE_CONTENT_RANGE_VALUE_FIRST;
                        remaining = to_string(range.first).size();
                        next_read = min<uintmax_t>(remaining,
                                                   message.body().end()
                                                   - body_it);

                        if (!next_read) {
                            schedule_more();
                            return;
                        }
                    }
                }
                case STATE_CONTENT_RANGE_VALUE_FIRST: {
                    auto value = to_string(range.first);
                    auto it = value.begin() + (value.size() - remaining);
                    copy(it, it + next_read, body_it);
                    body_it += next_read;
                    remaining -= next_read;

                    if (remaining) {
                        schedule_more();
                        return;
                    } else {
                        state = STATE_CONTENT_RANGE_VALUE_DASH;
                        remaining = 1;
                        next_read = min<uintmax_t>(remaining,
                                                   message.body().end()
                                                   - body_it);

                        if (!next_read) {
                            schedule_more();
                            return;
                        }
                    }
                }
                case STATE_CONTENT_RANGE_VALUE_DASH: {
                    *body_it = '-';
                    body_it += next_read;
                    remaining -= next_read;

                    if (remaining) {
                        schedule_more();
                        return;
                    } else {
                        state = STATE_CONTENT_RANGE_VALUE_SECOND;
                        remaining = to_string(range.second).size();
                        next_read = min<uintmax_t>(remaining,
                                                   message.body().end()
                                                   - body_it);

                        if (!next_read) {
                            schedule_more();
                            return;
                        }
                    }
                }
                case STATE_CONTENT_RANGE_VALUE_SECOND: {
                    auto value = to_string(range.second);
                    auto it = value.begin() + (value.size() - remaining);
                    copy(it, it + next_read, body_it);
                    body_it += next_read;
                    remaining -= next_read;

                    if (remaining) {
                        schedule_more();
                        return;
                    } else {
                        to_cpp_range(range);

                        state = STATE_CONTENT_RANGE_VALUE_SLASH;
                        remaining = 1;
                        next_read = min<uintmax_t>(remaining,
                                                   message.body().end()
                                                   - body_it);

                        if (!next_read) {
                            schedule_more();
                            return;
                        }
                    }
                }
                case STATE_CONTENT_RANGE_VALUE_SLASH: {
                    *body_it = '/';
                    body_it += next_read;
                    remaining -= next_read;

                    if (remaining) {
                        schedule_more();
                        return;
                    } else {
                        state = STATE_CONTENT_RANGE_VALUE_SIZE;
                        remaining = file_size.size();
                        next_read = min<uintmax_t>(remaining,
                                                   message.body().end()
                                                   - body_it);

                        if (!next_read) {
                            schedule_more();
                            return;
                        }
                    }
                }
                case STATE_CONTENT_RANGE_VALUE_SIZE: {
                    auto it = file_size.begin()
                        + (file_size.size() - remaining);
                    copy(it, it + next_read, body_it);
                    body_it += next_read;
                    remaining -= next_read;

                    if (remaining) {
                        schedule_more();
                        return;
                    } else {
                        state = STATE_CONTENT_RANGE_CRLF;
                        remaining = double_crlf.size;
                        next_read = min<uintmax_t>(remaining,
                                                   message.body().end()
                                                   - body_it);

                        if (!next_read) {
                            schedule_more();
                            return;
                        }
                    }
                }
                case STATE_CONTENT_RANGE_CRLF: {
                    auto it = double_crlf.begin<body_value_type>()
                        + (double_crlf.size - remaining);
                    copy(it, it + next_read, body_it);
                    body_it += next_read;
                    remaining -= next_read;

                    if (remaining) {
                        schedule_more();
                        return;
                    } else {
                        state = STATE_BODY;
                        remaining = range.second;
                        next_read = min<uintmax_t>(remaining,
                                                   message.body().end()
                                                   - body_it);

                        if (!next_read) {
                            schedule_more();
                            return;
                        }
                    }
                }
                case STATE_BODY: {
                    stream.seekg(range.first);
                    stream.read(reinterpret_cast<filesystem::ifstream
                                    ::char_type*>
                                (&*body_it),
                                next_read);
                    body_it += next_read;
                    remaining -= next_read;

                    if (remaining) {
                        schedule_more();
                        return;
                    } else {
                        ++index;

                        state = (index == range_set.size())
                            ? STATE_FINAL_BOUNDARY : STATE_BOUNDARY_LINE;
                        remaining = (state == STATE_FINAL_BOUNDARY)
                            ? final_boundary.size : boundary_line.size;
                        next_read = min<uintmax_t>(remaining,
                                                   message.body().end()
                                                   - body_it);

                        if (state == STATE_FINAL_BOUNDARY) {
                            /* Required to avoid UB within the line:

                               auto &range = range_set[index]; */
                            --index;
                        }

                        if (!next_read) {
                            schedule_more();
                            return;
                        }

                        break;
                    }
                }
                case STATE_FINAL_BOUNDARY: {
                    auto it = final_boundary.begin<body_value_type>()
                        + (final_boundary.size - remaining);
                    copy(it, it + next_read, body_it);
                    body_it += next_read;
                    remaining -= next_read;

                    if (remaining) {
                        schedule_more();
                        return;
                    } else {
                        message.body().resize(body_it - message.body().begin());
                        auto self = this->shared_from_this();

                        auto on_finished
                            = [self](const system::error_code &ec) {
                            if (ec) {
                                self->handler(ec);
                                return;
                            }

                            self->socket
                            .async_write_end_of_message([self]
                                                        (const system::error_code
                                                         &ec) {
                                                            self->handler(ec);
                                                        });
                        };

                        socket.async_write(message, on_finished);
                        return;
                    }
                }
                }
            }
        } catch(std::ios_base::failure&) {
            handler(file_server_errc::irrecoverable_io_error);
        }
    }

    Socket &socket;
    Message &message;
    String content_type;
    Handler handler;
    filesystem::ifstream stream;
    State state = STATE_BOUNDARY_LINE;
    typename Message::body_type::size_type remaining;
    std::vector<std::pair<std::uintmax_t, std::uintmax_t>> range_set;
    std::vector<std::pair<std::uintmax_t, std::uintmax_t>>::size_type index = 0;
    const std::string file_size;
};

} // namespace detail

/**
 * This function might be similar to the previous one, but actually does a lot
 * more:
 *
 * - It'll interpret and process the request headers. However, it doesn't
 *   process the request method nor the the input path. This means you still
 *   have to guarantee the method is applicable and to resolve the input path to
 *   a valid local file path.
 * - It'll fill all the applicable response headers appropriate to the request.
 * - It'll interpret the file attributes to process conditional requests and
 *   partial download. However, MIME detection (the `content-type` optional
 *   header) is still left for the user to handler.
 *
 * The response headers filled by this function MUST NOT be sent through
 * trailers. Therefore, this function will not do any operation and it will call
 * the handler with an error_code (file_server_errc::write_state_not_supported)
 * set if you pass a socket for which the headers were already sent.
 *
 * If you cannot guarantee the \p file did not change twice during the second
 * covered by the last write time, you should remove all "range" and "if-range"
 * headers from \p imessage. It's possible to construct a more robust file
 * server by making use of system-level APIs that can provide unique identifiers
 * for file revisions.
 */
template<class ServerSocket, class Message, class CompletionToken>
typename asio::async_result<
    typename asio::handler_type<CompletionToken,
                                void(system::error_code)>::type>::type
async_response_transmit_file(ServerSocket &socket, const Message &imessage,
                             Message &omessage, const filesystem::path &file,
                             CompletionToken &&token);

/**
 * Same as before, but takes extra head argument to indicate if it is a head
 * request. If is_head_request is false, then it'll assume request method is
 * "GET" and will make use of range headers, which are only allowed within "GET"
 * requests. If this ain't a "GET" request, but isn't "HEAD" either, you MAY
 * remove all "range" and "if-range" headers and pass the value `false` to the
 * \p is_head_request argument.
 *
 * \p omessage.body() will be used as output buffer. If
 * omessage.body().capacity() == 0, an unspecified buffer size will be used and
 * is very likely it'll be highly inefficient.
 */
template<class ServerSocket, class Message, class CompletionToken>
typename asio::async_result<
    typename asio::handler_type<CompletionToken,
                                void(system::error_code)>::type>::type
async_response_transmit_file(ServerSocket &socket, const Message &imessage,
                             Message &omessage, const filesystem::path &file,
                             bool is_head_request, CompletionToken &&token)
{
    typedef typename Message::headers_type::mapped_type String;
    typedef typename Message::body_type::value_type body_value_type;
    typedef typename asio::handler_type<
        CompletionToken, void(system::error_code)>::type Handler;

    Handler handler(std::forward<CompletionToken>(token));
    asio::async_result<Handler> result(handler);

    {
        auto state = socket.write_state();
        if (state != write_state::empty
            && state != write_state::continue_issued) {
            handler(system::error_code(file_server_errc
                                       ::write_state_not_supported));
            return result.get();
        }
    }

    detail::constchar_helper crlf("\r\n");

    try {
        /* BEWARE: std::time_t is not TZ aware and some old filesystems report
           time as local (i.e. non-UTC). We don't try to detect filesystem
           behaviour to avoid races and because all modern filesystems adopted
           UTC. */
        auto last_modified = detail::last_modified_http_date(file);
        const auto size = file_size(file);
        const auto buffer_size = [&omessage]() {
            auto ret = omessage.body().capacity();
            // can be any number > 0
            decltype(ret) magic_constant = 64;
            return (ret != 0) ? ret : magic_constant;
        }();

        if (size == 0) {
            socket.async_write_response(200, string_ref("OK"), omessage,
                                        handler);
            return result.get();
        }

        omessage.body().clear();

        /* only fill response headers that need no more than the target file
           to be computed (i.e. ignore all input headers) */
        {
            omessage.headers().emplace("accept-ranges", "bytes");

            auto now = posix_time::second_clock::universal_time();

            /* MUST NOT send a "last-modified" date that is later than the
               server’s time of message origination ("date") */
            if (last_modified > now)
                last_modified = now;

            omessage.headers().emplace("date", to_http_date<String>(now));
            omessage.headers()
            .emplace("last-modified", to_http_date<String>(last_modified));
        };

        /* The order to check the conditional request headers is defined in
           RFC7232 */

        { // Check "if-unmodified-since" header
            auto query = imessage.headers().equal_range("if-unmodified-since");
            if (std::distance(query.first, query.second) == 1) {
                auto query_datetime = header_to_ptime(query.first->second);

                // invalid HTTP-date, must ignore it
                if (!query_datetime.is_not_a_date_time()) {
                    if (last_modified > query_datetime) {
                        socket
                            .async_write_response(412, string_ref("Precondition"
                                                                  " Failed"),
                                                  omessage, handler);
                        return result.get();
                    }
                }
            }
        }

        { // Check "if-modified-since" header
            auto query = imessage.headers().equal_range("if-modified-since");
            if (std::distance(query.first, query.second) == 1) {
                auto query_datetime = header_to_ptime(query.first->second);

                // invalid HTTP-date, must ignore it
                if (!query_datetime.is_not_a_date_time()) {
                    if (last_modified <= query_datetime) {
                        socket.async_write_response(304, string_ref("Not "
                                                                    "Modified"),
                                                    omessage, handler);
                        return result.get();
                    }
                }
            }
        }

        auto range_header = imessage.headers().equal_range("range");
        bool process_range = !is_head_request
            && (std::distance(range_header.first, range_header.second) == 1);

        // Check "if-range" header
        if (process_range) {
            auto query = imessage.headers().equal_range("if-range");
            if (std::distance(query.first, query.second) == 1) {
                auto query_datetime = header_to_ptime(query.first->second);

                /* valid HTTP-date (otherwise must ignore it) */
                if (!query_datetime.is_not_a_date_time()) {
                    if (last_modified != query_datetime)
                        process_range = false;
                }
            }
        }

        if (process_range) {
            std::vector<std::pair<std::uintmax_t, std::uintmax_t>> range_set;
            if (!detail::is_valid_range(range_header.first->second, size,
                                        range_set)) {
                omessage.headers().emplace("content-range",
                                           "bytes */" + std::to_string(size));
                socket.async_write_response(416,
                                            string_ref("Range Not Satisfiable"),
                                            omessage, handler);
                return result.get();
            }

            assert(range_set.size() > 0);

            if (range_set.size() == 1) {
                auto &range = range_set.front();
                omessage.headers()
                    .emplace("content-range",
                             "bytes " + std::to_string(range.first) + '-'
                             + std::to_string(range.second) + '/'
                             + std::to_string(size));

                detail::to_cpp_range(range);

                if (socket.write_response_native_stream()) {
                    omessage.body().resize(buffer_size);

                    typedef detail
                        ::on_async_response_transmit_file<ServerSocket, Message,
                                                          Handler> pointee;

                    auto loop = std::make_shared<pointee>
                        (socket, omessage, std::move(handler), file,
                         range.second);
                    loop->stream.seekg(range.first);

                    auto callback = [loop](const system::error_code &ec) {
                        loop->process(ec);
                    };

                    socket.async_write_response_metadata(206,
                                                         string_ref("Partial"
                                                                    " Content"),
                                                         omessage, callback);
                } else {
                    if (range.second > omessage.body().max_size()) {
                        handler(system::error_code{file_server_errc::io_error});
                        return result.get();
                    }

                    omessage.body().resize(range.second);

                    filesystem::ifstream stream(file);
                    stream.exceptions(filesystem::ifstream::badbit
                                      | filesystem::ifstream::failbit
                                      | filesystem::ifstream::eofbit);
                    stream.seekg(range.first);
                    stream.read(reinterpret_cast<filesystem::ifstream
                                    ::char_type*>(omessage.body().data()),
                                range.second);

                    socket.async_write_response(206,
                                                string_ref("Partial Content"),
                                                omessage, handler);
                }
                return result.get();
            } else {
                // range_set.size() > 1
                // TODO: use string_ref?
                String content_type;
                {
                    auto h = omessage.headers().equal_range("content-type");
                    if (std::distance(h.first, h.second) == 1)
                        content_type = h.first->second;
                    omessage.headers().erase(h.first, h.second);
                }

                omessage.headers().emplace("content-type",
                                           "multipart/byteranges;boundary="
                                           BOOST_HTTP_FILE_SERVER_BOUNDARY);

                if (socket.write_response_native_stream()) {
                    omessage.body().resize(buffer_size);

                    typedef detail
                        ::on_async_response_transmit_file_multi<ServerSocket,
                                                                Message, String,
                                                                Handler>
                        pointee;

                    auto loop = std::make_shared<pointee>
                        (socket, omessage, std::move(content_type),
                         std::move(handler), file, std::move(range_set), size);

                    auto callback = [loop](const system::error_code &ec) {
                        loop->process(ec);
                    };

                    socket.async_write_response_metadata(206,
                                                         string_ref("Partial"
                                                                    " Content"),
                                                         omessage, callback);
                } else {
                    using std::copy;
                    using std::back_inserter;
                    using std::to_string;

                    detail::constchar_helper
                        boundary_line("\r\n--" BOOST_HTTP_FILE_SERVER_BOUNDARY
                                      "\r\n");
                    detail::constchar_helper
                        final_boundary("\r\n--" BOOST_HTTP_FILE_SERVER_BOUNDARY
                                       "--\r\n");

                    filesystem::ifstream stream(file);
                    stream.exceptions(filesystem::ifstream::badbit
                                      | filesystem::ifstream::failbit
                                      | filesystem::ifstream::eofbit);

                    //omessage.body().resize(/*total size*/); // TODO
                    for (auto &range: range_set) {
                        // e.g. "\r\n--THIS_STRING_SEPARATES"
                        copy(boundary_line.begin<body_value_type>(),
                             boundary_line.end<body_value_type>(),
                             back_inserter(omessage.body()));

                        // e.g. "content-type: application/pdf\r\n"
                        if (content_type.size()) {
                            detail::constchar_helper h("content-type: ");
                            copy(h.begin<body_value_type>(),
                                 h.end<body_value_type>(),
                                 back_inserter(omessage.body()));
                            copy(content_type.begin(), content_type.end(),
                                 back_inserter(omessage.body()));
                            copy(crlf.begin<body_value_type>(),
                                 crlf.end<body_value_type>(),
                                 back_inserter(omessage.body()));
                        }

                        // e.g. "content-range: bytes 500-999/8000\r\n"
                        {
                            detail::constchar_helper h("content-range: bytes ");
                            copy(h.begin<body_value_type>(),
                                 h.end<body_value_type>(),
                                 back_inserter(omessage.body()));
                            auto value = to_string(range.first);
                            copy(value.begin(), value.end(),
                                 back_inserter(omessage.body()));
                            omessage.body().push_back('-');
                            value = to_string(range.second);
                            copy(value.begin(), value.end(),
                                 back_inserter(omessage.body()));
                            omessage.body().push_back('/');
                            value = to_string(size);
                            copy(value.begin(), value.end(),
                                 back_inserter(omessage.body()));
                        }
                        copy(crlf.begin<body_value_type>(),
                             crlf.end<body_value_type>(),
                             back_inserter(omessage.body()));
                        detail::to_cpp_range(range);
                        // "\r\n"
                        copy(crlf.begin<body_value_type>(),
                             crlf.end<body_value_type>(),
                             back_inserter(omessage.body()));

                        // body part
                        auto index = omessage.body().size();
                        try {
                            auto newsiz = detail::safe_add(index, range.second);
                            omessage.body().resize(newsiz);
                        } catch(std::overflow_error&) {
                            handler(system::error_code{file_server_errc
                                        ::io_error});
                            return result.get();
                        }
                        stream.seekg(range.first);
                        stream.read(reinterpret_cast<filesystem::ifstream
                                        ::char_type*>
                                    (omessage.body().data() + index),
                                    range.second);
                    }
                    copy(final_boundary.begin<body_value_type>(),
                         final_boundary.end<body_value_type>(),
                         back_inserter(omessage.body()));
                    socket.async_write_response(206,
                                                string_ref("Partial Content"),
                                                omessage, handler);
                }
                return result.get();
            }
        }

        if (is_head_request) {
            omessage.headers().emplace("content-length", std::to_string(size));
            socket.async_write_response(200, string_ref("OK"), omessage,
                                        handler);
            return result.get();
        }

        // non-byte-range-request

        if (socket.write_response_native_stream()) {
            omessage.body().resize(buffer_size);

            typedef detail
                ::on_async_response_transmit_file<ServerSocket, Message,
                                                  Handler> pointee;

            auto loop = std::make_shared<pointee>
                (socket, omessage, std::move(handler), file, size);

            auto callback = [loop](const system::error_code &ec) {
                loop->process(ec);
            };

            socket.async_write_response_metadata(200, string_ref("OK"),
                                                 omessage, callback);
        } else {
            if (size > omessage.body().max_size()) {
                handler(system::error_code{file_server_errc::io_error});
                return result.get();
            }

            omessage.body().resize(size);

            filesystem::ifstream stream(file);
            stream.exceptions(filesystem::ifstream::badbit
                              | filesystem::ifstream::failbit
                              | filesystem::ifstream::eofbit);
            stream.read(reinterpret_cast<filesystem::ifstream
                            ::char_type*>(omessage.body().data()),
                        size);

            socket.async_write_response(200, string_ref("OK"), omessage,
                                        handler);
        }
    } catch (std::ios_base::failure&) {
        handler(system::error_code{file_server_errc::io_error});
        return result.get();
    }

    return result.get();
}

template<class ServerSocket, class Message, class CompletionToken>
typename asio::async_result<
    typename asio::handler_type<CompletionToken,
                                void(system::error_code)>::type>::type
async_response_transmit_file(ServerSocket &socket, const Message &imessage,
                             Message &omessage, const filesystem::path &file,
                             CompletionToken &&token)
{
    return async_response_transmit_file(socket, imessage, omessage, file,
                                        /*is_head_request=*/false,
                                        std::forward<CompletionToken>(token));
}

/**
 * This function does a lot more than just sending bytes. It carries the
 * responsibilities from the previous function, but add a few more of its own:
 *
 * - It'll also handle the method.
 * - It'll also handle the file resolution. It does so with respect to the given
 *   root_dir argument.
 *
 * All urls are absolute, but are absolute with respect to the given \p
 * root_dir. This interface provides no means to disable this security check. If
 * the user needs that much complex logic, then it should write its own path
 * resolving solution and use async_response_transmit_file.
 *
 * \p ipath (standing for input path) is the **parsed path** from the requested
 * url and it is guaranteed that it'll only be used to construct a
 * filesystem::path object. Thus, the user can fake the requested path to force
 * an "internal redirect". Extracting the path is an extra responsibility for
 * the user, but it is an useful abstraction for scenarios where the user
 * doesn't control the served root dir. Thanks to the security check, this
 * "internal redirect" trick doesn't work for files outside the \p root_dir.
 *
 * This function can emit two additional error codes. The first is
 * `file_server_category::file_not_found`. This error arises when the requested
 * file, with respect to the given \p root_dir, cannot be found. The channel
 * remains untouched in this case, giving the user the opportunity to send
 * custom 404 messages.
 *
 * The second additional error code is
 * `file_server_category::file_type_not_supported` and happens when resolution
 * finishes but this function cannot process the result because the file is not
 * regular (directories, block devices, broken links...). The channel is also
 * left untouched, giving the user the opportunity to use another HTTP consumer.
 *
 * The only feature missing is mime support (content-type header). It cannot be
 * done reliably within this abstraction.
 *
 * This function will handle even the start line and the only acceptable
 * write_state is empty. It'll fail on any other write_state.
 *
 * If the file is found and processable, but HTTP method is different than `GET`
 * and `HEAD`, this function will reply with method not implemented.
 */
template<class ServerSocket, class String, class ConvertibleToPath,
         class Message, class CompletionToken>
typename asio::async_result<
    typename asio::handler_type<CompletionToken,
                                void(system::error_code)>::type>::type
async_response_transmit_dir(ServerSocket &socket, const String &method,
                            const ConvertibleToPath &ipath,
                            const Message &imessage, Message &omessage,
                            const filesystem::path &root_dir,
                            CompletionToken &&token);

/**
 * Same as above, but the predicate \p filter is applied to the resolved path as
 * the last step before proceeding to file and network operations. \p filter
 * signature must be as follows:
 *
 *     bool(filesystem::path &resolved_path)
 *
 * If filter returns falses, the functions finishes before touching the channel,
 * with the error_code `file_server_category::filter_set`.
 *
 * It's possible to use a stateful non-pure \p filter to add response headers
 * and properly process mime types (content-type header).
 *
 * \p filter can also be used to redirect files by modifying the input arg.
 *
 * This function might throw if \p filter throws. In this case, we provide the
 * basic exception guarantee.
 */
template<class ServerSocket, class String, class ConvertibleToPath,
         class Message, class Predicate, class CompletionToken>
typename asio::async_result<
    typename asio::handler_type<CompletionToken,
                                void(system::error_code)>::type>::type
async_response_transmit_dir(ServerSocket &socket, const String &method,
                            const ConvertibleToPath &ipath,
                            const Message &imessage, Message &omessage,
                            const filesystem::path &root_dir, Predicate filter,
                            CompletionToken &&token)
{
    // TODO
}

template<class ServerSocket, class String, class ConvertibleToPath,
         class Message, class CompletionToken>
typename asio::async_result<
    typename asio::handler_type<CompletionToken,
                                void(system::error_code)>::type>::type
async_response_transmit_dir(ServerSocket &socket, const String &method,
                            const ConvertibleToPath &ipath,
                            const Message &imessage, Message &omessage,
                            const filesystem::path &root_dir,
                            CompletionToken &&token)
{
    auto filter = [](const filesystem::path&){ return true; };
    return async_response_transmit_dir(socket, method, ipath, imessage,
                                       omessage, root_dir, filter, token);
}

} // namespace http
} // namespace boost

#endif // BOOST_HTTP_FILE_SERVER_HPP
