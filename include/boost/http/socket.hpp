/* Copyright (c) 2014, 2016, 2017 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_SOCKET_HPP
#define BOOST_HTTP_SOCKET_HPP

#include <cstdint>
#include <cstddef>
#include <cstring>

#include <memory>
#include <algorithm>
#include <sstream>
#include <array>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <limits>

#include <boost/none.hpp>
#include <boost/variant.hpp>
#include <boost/utility/string_view.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <boost/asio/async_result.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>

#include <boost/http/reader/request.hpp>
#include <boost/http/reader/response.hpp>
#include <boost/http/traits.hpp>
#include <boost/http/read_state.hpp>
#include <boost/http/write_state.hpp>
#include <boost/http/http_errc.hpp>
#include <boost/http/detail/writer_helper.hpp>
#include <boost/http/detail/count_decdigits.hpp>
#include <boost/http/detail/count_hexdigits.hpp>
#include <boost/http/detail/constchar_helper.hpp>
#include <boost/http/algorithm/header.hpp>

#ifndef BOOST_HTTP_SOCKET_DEFAULT_BODY_COPY_THRESHOLD
#define BOOST_HTTP_SOCKET_DEFAULT_BODY_COPY_THRESHOLD 1024
#endif // BOOST_HTTP_SOCKET_DEFAULT_BODY_COPY_THRESHOLD

namespace boost {
namespace http {

struct default_socket_settings
{
    typedef reader::request req_parser;
    typedef reader::response res_parser;
    static constexpr std::size_t body_copy_threshold
        = BOOST_HTTP_SOCKET_DEFAULT_BODY_COPY_THRESHOLD;
};

template<class Socket, class Settings = default_socket_settings>
class basic_socket
{
public:
    typedef typename Socket::executor_type executor_type;
    typedef Socket next_layer_type;

    // ### QUERY FUNCTIONS ###

    bool is_open() const;
    http::read_state read_state() const;
    http::write_state write_state() const;
    bool write_response_native_stream() const;

    executor_type get_executor();

    // ### END OF QUERY FUNCTIONS ###

    // ### READ FUNCTIONS ###

    template<class Request, class CToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CToken, void(system::error_code))
    async_read_request(Request& request, CToken&& token);

    template<class Response, class CToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CToken, void(system::error_code))
    async_read_response(Response& response, CToken&& token);

    template<class Message, class CToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CToken, void(system::error_code))
    async_read_some(Message& message, CToken&& token);

    template<class Message, class CToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CToken, void(system::error_code, std::size_t))
    async_read_chunkext(
        Message& message, typename Message::headers_type& chunkext,
        CToken&& token);

    // ### END OF READ FUNCTIONS ###

    // ### WRITE FUNCTIONS ###

    template<class Response, class CToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CToken, void(system::error_code))
    async_write_response(const Response& response, CToken&& token);

    template<class CToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CToken, void(system::error_code))
    async_write_response_continue(CToken&& token);

    template<class Response, class CToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CToken, void(system::error_code))
    async_write_response_metadata(const Response& response, CToken&& token);

    template<class Request, class CToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CToken, void(system::error_code))
    async_write_request(const Request& request, CToken&& token);

    template<class Request, class CToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CToken, void(system::error_code))
    async_write_request_metadata(const Request& request, CToken&& token);

    template<class Message, class CToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CToken, void(system::error_code))
    async_write(const Message& message, CToken&& token);

    template<class Message, class CToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CToken, void(system::error_code))
    async_write_chunkext(
        const Message& message, const typename Message::headers_type& chunkext,
        CToken&& token);

    template<class Message, class CToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CToken, void(system::error_code))
    async_write_trailers(const Message& message, CToken&& token);

    template<class CToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CToken, void(system::error_code))
    async_write_end_of_message(CToken&& token);

    // ### END OF WRITE FUNCTIONS ###

    // ### START OF basic_server SPECIFIC FUNCTIONS ###

    template<class... Args>
    basic_socket(boost::asio::mutable_buffer inbuffer, Args&&... args);

    basic_socket(const basic_socket&) = delete;
    basic_socket& operator=(const basic_socket&) = delete;

    next_layer_type &next_layer();
    const next_layer_type &next_layer() const;

    void open();

    boost::asio::const_buffer upgrade_head() const;

    void lock_client_to_http10();

private:
    typedef typename Settings::req_parser req_parser;
    typedef typename Settings::res_parser res_parser;

    enum SentMethod {
        HEAD,
        CONNECT,
        OTHER_METHOD
    };

    friend struct async_read_request_initiation;
    struct async_read_request_initiation
    {
        async_read_request_initiation(basic_socket& self)
            : self(self)
        {}

        template<class Handler, class Request>
        void operator()(
            Handler&& handler, std::reference_wrapper<Request> request);

        basic_socket& self;
    };

    friend struct async_read_response_initiation;
    struct async_read_response_initiation
    {
        async_read_response_initiation(basic_socket& self)
            : self(self)
        {}

        template<class Handler, class Response>
        void operator()(
            Handler&& handler, std::reference_wrapper<Response> response);

        basic_socket& self;
    };

    friend struct async_read_some_initiation;
    struct async_read_some_initiation
    {
        async_read_some_initiation(basic_socket& self)
            : self(self)
        {}

        template<class Handler, class Message>
        void operator()(
            Handler&& handler, std::reference_wrapper<Message> message);

        basic_socket& self;
    };

    friend struct async_read_chunkext_initiation;
    struct async_read_chunkext_initiation
    {
        async_read_chunkext_initiation(basic_socket& self)
            : self(self)
        {}

        template<class Handler, class Message>
        void operator()(
            Handler&& handler, std::reference_wrapper<Message> message,
            std::reference_wrapper<typename Message::headers_type> chunkext);

        basic_socket& self;
    };

    friend struct async_write_response_initiation;
    struct async_write_response_initiation
    {
        async_write_response_initiation(basic_socket& self)
            : self(self)
        {}

        template<class Handler, class Response>
        void operator()(
            Handler&& handler, std::reference_wrapper<const Response> response);

        basic_socket& self;
    };

    friend struct async_write_response_continue_initiation;
    struct async_write_response_continue_initiation
    {
        async_write_response_continue_initiation(basic_socket& self)
            : self(self)
        {}

        template<class Handler>
        void operator()(Handler&& handler);

        basic_socket& self;
    };

    friend struct async_write_response_metadata_initiation;
    struct async_write_response_metadata_initiation
    {
        async_write_response_metadata_initiation(basic_socket& self)
            : self(self)
        {}

        template<class Handler, class Response>
        void operator()(
            Handler&& handler, std::reference_wrapper<const Response> response);

        basic_socket& self;
    };

    friend struct async_write_request_initiation;
    struct async_write_request_initiation
    {
        async_write_request_initiation(basic_socket& self)
            : self(self)
        {}

        template<class Handler, class Request>
        void operator()(
            Handler&& handler, std::reference_wrapper<const Request> request);

        basic_socket& self;
    };

    friend struct async_write_request_metadata_initiation;
    struct async_write_request_metadata_initiation
    {
        async_write_request_metadata_initiation(basic_socket& self)
            : self(self)
        {}

        template<class Handler, class Request>
        void operator()(
            Handler&& handler, std::reference_wrapper<const Request> request);

        basic_socket& self;
    };

    friend struct async_write_initiation;
    struct async_write_initiation
    {
        async_write_initiation(basic_socket& self)
            : self(self)
        {}

        template<class Handler, class Message>
        void operator()(
            Handler&& handler, std::reference_wrapper<const Message> message);

        basic_socket& self;
    };

    friend struct async_write_chunkext_initiation;
    struct async_write_chunkext_initiation
    {
        async_write_chunkext_initiation(basic_socket& self)
            : self(self)
        {}

        template<class Handler, class Message>
        void operator()(
            Handler&& handler, std::reference_wrapper<const Message> message,
            std::reference_wrapper<
                const typename Message::headers_type
            > chunkext);

        basic_socket& self;
    };

    friend struct async_write_trailers_initiation;
    struct async_write_trailers_initiation
    {
        async_write_trailers_initiation(basic_socket& self)
            : self(self)
        {}

        template<class Handler, class Message>
        void operator()(
            Handler&& handler, std::reference_wrapper<const Message> message);

        basic_socket& self;
    };

    friend struct async_write_end_of_message_initiation;
    struct async_write_end_of_message_initiation
    {
        async_write_end_of_message_initiation(basic_socket& self)
            : self(self)
        {}

        template<class Handler>
        void operator()(Handler&& handler);

        basic_socket& self;
    };

    template<bool enable_chunkext, class Message, class Handler>
    void schedule_on_async_read_message(
        Handler &&handler, Message &message,
        typename Message::headers_type *chunkext = NULL
    );

    // `enable_chunkext` is not really required as we can just test for
    // `chunkext != NULL`, but on my tests the extra code to handle this piece
    // of data added a slight (but noticeable — ~1%) slowdown (I guess it
    // affected code cache) even to code not requiring such data. So I use this
    // template argument and rely on dead code optimization to not impose this
    // cost to everybody.
    template<bool server_mode, bool enable_chunkext, class Parser,
             class Message, class Handler>
    void on_async_read_message(Handler &&handler, Message &message,
                               typename Message::headers_type *chunkext,
                               const system::error_code &ec,
                               std::size_t bytes_transferred);

    void clear_buffer();

    template<class Message>
    static void clear_message(Message &message);

    template <typename Handler,
              typename ErrorCode>
    void invoke_handler(Handler&& handler,
                        ErrorCode error);

    template<class Handler>
    void invoke_handler(Handler &&handler);

    template<class Handler, class ErrorCode, class Value>
    void invoke_handler(Handler &&handler, ErrorCode error, Value value);

    Socket channel;
    bool is_open_ = true;
    http::read_state istate;

    boost::asio::mutable_buffer buffer;
    std::size_t used_size = 0;

    boost::variant<none_t, req_parser, res_parser> parser = none;
    boost::container::small_vector<SentMethod, 1> sent_requests;

    bool modern_http = true; // is HTTP/1.1?
    enum {
        KEEP_ALIVE_UNKNOWN,
        KEEP_ALIVE_CLOSE_READ,
        KEEP_ALIVE_KEEP_ALIVE_READ
    } keep_alive;

    // Output state
    detail::writer_helper writer_helper;
    bool connect_request;
};

typedef basic_socket<boost::asio::ip::tcp::socket> socket;

template<class Socket, class Settings>
struct is_server_socket<basic_socket<Socket, Settings>>: public std::true_type
{};

template<class Socket, class Settings>
struct is_client_socket<basic_socket<Socket, Settings>>: public std::true_type
{};

} // namespace http
} // namespace boost

#include "socket.ipp"

#endif // BOOST_HTTP_SOCKET_HPP
