/* For some tests, I really really really want to use C++11 features, so here
   they are */

#ifdef NDEBUG
#undef NDEBUG
#endif

#define CATCH_CONFIG_MAIN
#include "common.hpp"
#include <boost/http/reader/request.hpp>
#include <boost/variant.hpp>

using boost::variant;

namespace asio = boost::asio;
namespace http = boost::http;

// Code from StackOverflow {{{

template <typename ReturnType, typename... Lambdas>
struct lambda_visitor;

template <typename ReturnType, typename Lambda1, typename... Lambdas>
struct lambda_visitor< ReturnType, Lambda1 , Lambdas...>
    : public lambda_visitor<ReturnType, Lambdas...>, public Lambda1 {

    using Lambda1::operator();
    using lambda_visitor< ReturnType , Lambdas...>::operator();
    lambda_visitor(Lambda1 l1, Lambdas... lambdas)
        : Lambda1(l1), lambda_visitor< ReturnType , Lambdas...> (lambdas...)
    {}
};


template <typename ReturnType, typename Lambda1>
struct lambda_visitor<ReturnType, Lambda1>
    : public boost::static_visitor<ReturnType>, public Lambda1 {

    using Lambda1::operator();
    lambda_visitor(Lambda1 l1)
        : boost::static_visitor<ReturnType>(), Lambda1(l1)
    {}
};


template <typename ReturnType>
struct lambda_visitor<ReturnType>
    : public boost::static_visitor<ReturnType> {

    lambda_visitor() : boost::static_visitor<ReturnType>() {}
};


template <typename ReturnT, typename... Lambdas>
lambda_visitor<ReturnT, Lambdas...> make_lambda_visitor(Lambdas... lambdas)
{
    return { lambdas... };
}

// }}}

namespace my_token {

struct end_of_message
{
    bool operator==(const end_of_message &o) const
    {
        return size == o.size;
    }

    std::size_t size;
};

struct skip
{
    bool operator==(const skip &o) const
    {
        return size == o.size;
    }

    std::size_t size;
};

struct error
{
    bool operator==(const error &o) const
    {
        return code == o.code;
    }

    http::token::code::value code;
};

struct field_name
{
    bool operator==(const field_name &o) const
    {
        return value == o.value;
    }

    std::string value;
};

struct field_value
{
    bool operator==(const field_value &o) const
    {
        return size == o.size && value == o.value;
    }

    std::string value;
    std::size_t size;
};

struct body_chunk
{
    bool operator==(const body_chunk &o) const
    {
        return value == o.value;
    }

    std::vector<boost::uint8_t> value;
};

struct end_of_headers
{
    bool operator==(const end_of_headers &o) const
    {
        return size == o.size;
    }

    std::size_t size;
};

struct end_of_body
{
    bool operator==(const end_of_body &o) const
    {
        return size == o.size;
    }

    std::size_t size;
};

struct method
{
    bool operator==(const method &o) const
    {
        return value == o.value;
    }

    std::string value;
};

struct request_target
{
    bool operator==(const request_target &o) const
    {
        return value == o.value;
    }

    std::string value;
};

struct version
{
    bool operator==(const version &o) const
    {
        return value == o.value;
    }

    int value;
};

struct status_code
{
    bool operator==(const status_code &o) const
    {
        return value == o.value;
    }

    int value;
};

struct reason_phrase
{
    bool operator==(const reason_phrase &o) const
    {
        return value == o.value;
    }

    std::string value;
};

typedef variant<
    end_of_message,
    skip,
    error,
    field_name,
    field_value,
    body_chunk,
    end_of_headers,
    end_of_body,
    method,
    request_target,
    version,
    status_code,
    reason_phrase
    > value;

std::ostream& operator<<(std::ostream &os, const value &v)
{
    auto visitor
        = make_lambda_visitor<void>([&os](const end_of_message &v) {
                os << "end_of_message(size = " << v.size << ")";
            },
            [&os](const skip &v) {
                os << "skip(size = " << v.size << ")";
            },
            [&os](const error &v) {
                os << "error(" << Catch::toString(v.code) << ")";
            },
            [&os](const field_name &v) {
                os << "field_name(\"" << v.value << "\")";
            },
            [&os](const field_value &v) {
                os << "field_value(value = \"" << v.value << "\", size = "
                << v.size << ")";
            },
            [&os](const body_chunk &v) {
                os << "body_chunk(size = " << v.value.size() << ")";
            },
            [&os](const end_of_headers &v) {
                os << "end_of_headers(size = " << v.size << ")";
            },
            [&os](const end_of_body &v) {
                os << "end_of_body(size = " << v.size << ")";
            },
            [&os](const method &v) {
                os << "method(\"" << v.value << "\")";
            },
            [&os](const request_target &v) {
                os << "request_target(\"" << v.value << "\")";
            },
            [&os](const version &v) {
                os << "version(" << v.value << ")";
            },
            [&os](const status_code &v) {
                // TODO
            },
            [&os](const reason_phrase &v) {
                // TODO
            });
    boost::apply_visitor(visitor, v);
    return os;
}

} // namespace my_token

my_token::value make_end_of_message(std::size_t size)
{
    my_token::end_of_message t;
    t.size = size;
    return t;
}

my_token::value make_skip(std::size_t size)
{
    my_token::skip t;
    t.size = size;
    return t;
}

my_token::value make_error(http::token::code::value error)
{
    my_token::error t;
    t.code = error;
    return t;
}

my_token::value make_field_name(boost::string_ref value)
{
    my_token::field_name t;
    t.value = std::string(&value.front(), value.size());
    return t;
}

my_token::value make_field_value(boost::string_ref value, std::size_t size)
{
    my_token::field_value t;
    t.value = std::string(&value.front(), value.size());
    t.size = size;
    return t;
}

template<std::size_t N>
my_token::value make_field_value(const char (&value)[N], std::size_t size)
{
    assert(N - 1 <= size);
    my_token::field_value t;
    t.value = std::string(value, N - 1);
    t.size = size;
    return t;
}

template<std::size_t N>
my_token::value make_field_value(const char (&value)[N])
{
    my_token::field_value t;
    t.value = std::string(value, N - 1);
    t.size = N - 1;
    return t;
}

template<std::size_t N>
my_token::value make_body_chunk(const char (&value)[N])
{
    my_token::body_chunk t;
    t.value.resize(N - 1);
    for (std::size_t i = 0 ; i != N - 1 ; ++i) {
        t.value[i] = (boost::uint8_t)(value[i]);
    }
    return t;
}

my_token::value make_body_chunk(asio::const_buffer value)
{
    my_token::body_chunk t;
    t.value.resize(asio::buffer_size(value));
    for (std::size_t i = 0 ; i != asio::buffer_size(value) ; ++i) {
        t.value[i] = asio::buffer_cast<const boost::uint8_t*>(value)[i];
    }
    return t;
}

my_token::value make_end_of_headers(std::size_t size)
{
    my_token::end_of_headers t;
    t.size = size;
    return t;
}

my_token::value make_end_of_body(std::size_t size)
{
    my_token::end_of_body t;
    t.size = size;
    return t;
}

my_token::value make_method(boost::string_ref value)
{
    my_token::method t;
    t.value = std::string(&value.front(), value.size());
    return t;
}

my_token::value make_request_target(boost::string_ref value)
{
    my_token::request_target t;
    t.value = std::string(&value.front(), value.size());
    return t;
}

my_token::value make_version(int value)
{
    my_token::version t;
    t.value = value;
    return t;
}

my_token::value make_status_code(int value)
{
    my_token::status_code t;
    t.value = value;
    return t;
}

my_token::value make_reason_phrase(std::string value)
{
    my_token::reason_phrase t;
    t.value = value;
    return t;
}

template<std::size_t N>
void my_tester(const char (&input)[N],
               std::vector<my_token::value> expected_output)
{
    INFO("# REQUEST:\n" << input);
    const auto size = N - 1;
    auto buffer = asio::buffer(input, size);
    for (std::size_t init_chunk_size = 1 ; init_chunk_size != size
             ; ++init_chunk_size) {
        CAPTURE(init_chunk_size);
        bool stop = false;
        std::size_t nparsed = 0;
        std::size_t chunk_size = init_chunk_size;
        http::reader::request parser;
        std::vector<my_token::value> output;

        REQUIRE(parser.code()
                == http::token::code::error_insufficient_data);
        REQUIRE(parser.token_size() == 0);
        REQUIRE(parser.expected_token() == http::token::code::method);

        while (true) {
            auto buffer_view = asio::buffer(buffer + nparsed, chunk_size);
            parser.set_buffer(buffer_view);
            parser.next();

            switch (parser.code()) {
            case http::token::code::end_of_message:
                output.push_back(make_end_of_message(parser.token_size()));
                break;
            case http::token::code::skip:
                {
                    my_token::skip *v = NULL;

                    if (output.size())
                        v = boost::get<my_token::skip>(&output.back());

                    if (v)
                        v->size += parser.token_size();
                    else
                        output.push_back(make_skip(parser.token_size()));
                }
                break;
            case http::token::code::error_insufficient_data:
                if (nparsed == size) {
                    stop = true;
                    break;
                }

                ++chunk_size;
                continue;
            case http::token::code::error_invalid_data:
            case http::token::code::error_no_host:
            case http::token::code::error_invalid_content_length:
            case http::token::code::error_content_length_overflow:
            case http::token::code::error_invalid_transfer_encoding:
            case http::token::code::error_chunk_size_overflow:
                output.push_back(make_error(parser.code()));
                stop = true;
                break;
            case http::token::code::field_name:
                {
                    auto value = parser.value<http::token::field_name>();
                    output.push_back(make_field_name(value));
                }
                break;
            case http::token::code::field_value:
                {
                    auto value = parser.value<http::token::field_value>();
                    output.push_back(make_field_value(value,
                                                      parser.token_size()));
                }
                break;
            case http::token::code::body_chunk:
                {
                    auto value = parser.value<http::token::body_chunk>();
                    my_token::body_chunk *v = NULL;

                    if (output.size())
                        v = boost::get<my_token::body_chunk>(&output.back());

                    if (v) {
                        auto body
                            = asio::buffer_cast<const boost::uint8_t*>(value);
                        for (std::size_t i = 0 ; i != asio::buffer_size(value)
                                 ; ++i) {
                            auto e = body[i];
                            v->value.push_back(e);
                        }
                    } else {
                        output.push_back(make_body_chunk(value));
                    }
                }
                break;
            case http::token::code::end_of_headers:
                output.push_back(make_end_of_headers(parser.token_size()));
                break;
            case http::token::code::end_of_body:
                output.push_back(make_end_of_body(parser.token_size()));
                break;
            case http::token::code::method:
                {
                    auto value = parser.value<http::token::method>();
                    output.push_back(make_method(value));
                }
                break;
            case http::token::code::request_target:
                {
                    auto value = parser.value<http::token::request_target>();
                    output.push_back(make_request_target(value));
                }
                break;
            case http::token::code::version:
                {
                    auto value = parser.value<http::token::version>();
                    output.push_back(make_version(value));
                }
                break;
            case http::token::code::error_set_method:
            case http::token::code::error_use_another_connection:
            case http::token::code::status_code:
            case http::token::code::reason_phrase:
                BOOST_HTTP_DETAIL_UNREACHABLE("SHOULDN'T HAPPEN");
                break;
            }

            if (output.size() >= expected_output.size() || stop)
                break;

            nparsed += parser.token_size();

            // consumes current buffer and stops at insufficient data
            parser.set_buffer(asio::buffer(buffer_view, parser.token_size()));
            parser.next();
            if (parser.code() != http::token::code::error_insufficient_data) {
                switch (parser.code()) {
                case http::token::code::end_of_body:
                    output.push_back(make_end_of_body(0));
                    break;
                case http::token::code::end_of_message:
                    output.push_back(make_end_of_message(0));
                    break;
                default:
                    FAIL("token \"" << Catch::toString(parser.code())
                         <<  "\" cannot be 0-sized in current implementation.");
                }
            }

            chunk_size = init_chunk_size;
        }

        INFO("chunk_size,nparsed := " << chunk_size << "," << nparsed);
        REQUIRE(output == expected_output);
    }
}

TEST_CASE("Lots of messages described declaratively and tested with varying"
          " buffer sizes", "[parser,good]")
{
    my_tester("GET / HTTP/1.1\r\n"
              "host: localhost\r\n"
              "\r\n",
              {
                  make_method("GET"),
                  make_skip(1),
                  make_request_target("/"),
                  make_skip(8),
                  make_version(1),
                  make_skip(2),
                  make_field_name("host"),
                  make_skip(2),
                  make_field_value("localhost"),
                  make_skip(2),
                  make_end_of_headers(2),
                  make_end_of_body(0),
                  make_end_of_message(0)
              });
    my_tester("POST / HTTP/1.1\r\n"
              "host: localhost\r\n"
              "Content-length: 4\r\n"
              "\r\n"
              "ping",
              {
                  make_method("POST"),
                  make_skip(1),
                  make_request_target("/"),
                  make_skip(8),
                  make_version(1),
                  make_skip(2),
                  make_field_name("host"),
                  make_skip(2),
                  make_field_value("localhost"),
                  make_skip(2),
                  make_field_name("Content-length"),
                  make_skip(2),
                  make_field_value("4"),
                  make_skip(2),
                  make_end_of_headers(2),
                  make_body_chunk("ping"),
                  make_end_of_body(0),
                  make_end_of_message(0)
              });
    my_tester("POST /american_jesus HTTP/1.1\r\n"
              "Transfer-Encoding:chunked\t \t \t\r\n"
              "host:         \tlocalhost\t\t  \r\n"
              "\r\n"

              "4\r\n"
              "Wiki\r\n"

              "5\r\n"
              "pedia\r\n"

              "e\r\n"
              " in\r\n\r\nchunks.\r\n"

              "0\r\n"
              "\r\n",
              {
                  make_method("POST"),
                  make_skip(1),
                  make_request_target("/american_jesus"),
                  make_skip(8),
                  make_version(1),
                  make_skip(2),
                  make_field_name("Transfer-Encoding"),
                  make_skip(1),
                  make_field_value("chunked", 12),
                  make_skip(2),
                  make_field_name("host"),
                  make_skip(11),
                  make_field_value("localhost", 13),
                  make_skip(2),
                  make_end_of_headers(2),
                  make_skip(3),
                  make_body_chunk("Wiki"),
                  make_skip(5),
                  make_body_chunk("pedia"),
                  make_skip(5),
                  make_body_chunk(" in\r\n\r\nchunks."),
                  make_skip(3),
                  make_end_of_body(2),
                  make_end_of_message(2)
              });
    my_tester("POST /american_jesus HTTP/1.1\r\n"
              "Transfer-Encoding:chunked\t \t \t\r\n"
              "host:         \tlocalhost\t\t  \r\n"
              "\r\n"

              "4\r\n"
              "Wiki\r\n"

              "5\r\n"
              "pedia\r\n"

              "e\r\n"
              " in\r\n\r\nchunks.\r\n"

              "0\r\n"
              "X-Group: hack'n'cast\r\n"
              "\r\n",
              {
                  make_method("POST"),
                  make_skip(1),
                  make_request_target("/american_jesus"),
                  make_skip(8),
                  make_version(1),
                  make_skip(2),
                  make_field_name("Transfer-Encoding"),
                  make_skip(1),
                  make_field_value("chunked", 12),
                  make_skip(2),
                  make_field_name("host"),
                  make_skip(11),
                  make_field_value("localhost", 13),
                  make_skip(2),
                  make_end_of_headers(2),
                  make_skip(3),
                  make_body_chunk("Wiki"),
                  make_skip(5),
                  make_body_chunk("pedia"),
                  make_skip(5),
                  make_body_chunk(" in\r\n\r\nchunks."),
                  make_skip(3),
                  make_end_of_body(2),
                  make_field_name("X-Group"),
                  make_skip(2),
                  make_field_value("hack'n'cast"),
                  make_skip(2),
                  make_end_of_message(2)
              });
}

TEST_CASE("A big buffer of messages described declaratively and tested with"
          " varying buffer sizes", "[parser,good]")
{
    my_tester("GET / HTTP/1.1\r\n"
              "host: localhost\r\n"
              "\r\n"

              // second request
              "POST / HTTP/1.1\r\n"
              "host: localhost\r\n"
              "Content-length: 4\r\n"
              "\r\n"
              "ping"

              // third request
              "POST /american_jesus HTTP/1.1\r\n"
              "Transfer-Encoding:chunked\t \t \t\r\n"
              "host:         \tlocalhost\t\t  \r\n"
              "\r\n"

              "4\r\n"
              "Wiki\r\n"

              "5\r\n"
              "pedia\r\n"

              "e\r\n"
              " in\r\n\r\nchunks.\r\n"

              "0\r\n"
              "\r\n"

              // fourth request
              "POST /american_jesus HTTP/1.1\r\n"
              "Transfer-Encoding:chunked\t \t \t\r\n"
              "host:         \tlocalhost\t\t  \r\n"
              "\r\n"

              "4\r\n"
              "Wiki\r\n"

              "5\r\n"
              "pedia\r\n"

              "e\r\n"
              " in\r\n\r\nchunks.\r\n"

              "0\r\n"
              "X-Group: hack'n'cast\r\n"
              "\r\n",
              {
                  make_method("GET"),
                  make_skip(1),
                  make_request_target("/"),
                  make_skip(8),
                  make_version(1),
                  make_skip(2),
                  make_field_name("host"),
                  make_skip(2),
                  make_field_value("localhost"),
                  make_skip(2),
                  make_end_of_headers(2),
                  make_end_of_body(0),
                  make_end_of_message(0),

                  // second request
                  make_method("POST"),
                  make_skip(1),
                  make_request_target("/"),
                  make_skip(8),
                  make_version(1),
                  make_skip(2),
                  make_field_name("host"),
                  make_skip(2),
                  make_field_value("localhost"),
                  make_skip(2),
                  make_field_name("Content-length"),
                  make_skip(2),
                  make_field_value("4"),
                  make_skip(2),
                  make_end_of_headers(2),
                  make_body_chunk("ping"),
                  make_end_of_body(0),
                  make_end_of_message(0),

                  // third request
                  make_method("POST"),
                  make_skip(1),
                  make_request_target("/american_jesus"),
                  make_skip(8),
                  make_version(1),
                  make_skip(2),
                  make_field_name("Transfer-Encoding"),
                  make_skip(1),
                  make_field_value("chunked", 12),
                  make_skip(2),
                  make_field_name("host"),
                  make_skip(11),
                  make_field_value("localhost", 13),
                  make_skip(2),
                  make_end_of_headers(2),
                  make_skip(3),
                  make_body_chunk("Wiki"),
                  make_skip(5),
                  make_body_chunk("pedia"),
                  make_skip(5),
                  make_body_chunk(" in\r\n\r\nchunks."),
                  make_skip(3),
                  make_end_of_body(2),
                  make_end_of_message(2),

                  // fourth request
                  make_method("POST"),
                  make_skip(1),
                  make_request_target("/american_jesus"),
                  make_skip(8),
                  make_version(1),
                  make_skip(2),
                  make_field_name("Transfer-Encoding"),
                  make_skip(1),
                  make_field_value("chunked", 12),
                  make_skip(2),
                  make_field_name("host"),
                  make_skip(11),
                  make_field_value("localhost", 13),
                  make_skip(2),
                  make_end_of_headers(2),
                  make_skip(3),
                  make_body_chunk("Wiki"),
                  make_skip(5),
                  make_body_chunk("pedia"),
                  make_skip(5),
                  make_body_chunk(" in\r\n\r\nchunks."),
                  make_skip(3),
                  make_end_of_body(2),
                  make_field_name("X-Group"),
                  make_skip(2),
                  make_field_value("hack'n'cast"),
                  make_skip(2),
                  make_end_of_message(2)
              });
}

TEST_CASE("Lots of messages described declaratively and tested with varying"
          " buffer sizes (bad input version)", "[parser,bad]")
{
    my_tester("GET / HTTP/1.1\r\n"
              "host: localhost\r\n"
              "\r \n",
              {
                  make_method("GET"),
                  make_skip(1),
                  make_request_target("/"),
                  make_skip(8),
                  make_version(1),
                  make_skip(2),
                  make_field_name("host"),
                  make_skip(2),
                  make_field_value("localhost"),
                  make_skip(2),
                  make_error(http::token::code::error_invalid_data)
              });
    my_tester("GET / HTTP/1.1\r\n"
              "host: localhost\r \n"
              "\r\n",
              {
                  make_method("GET"),
                  make_skip(1),
                  make_request_target("/"),
                  make_skip(8),
                  make_version(1),
                  make_skip(2),
                  make_field_name("host"),
                  make_skip(2),
                  make_field_value("localhost"),
                  make_error(http::token::code::error_invalid_data)
              });
    my_tester("GET / HTTP/1.1\r\n"
              "host: ""\x7F""localhost\r \n"
              "\r\n",
              {
                  make_method("GET"),
                  make_skip(1),
                  make_request_target("/"),
                  make_skip(8),
                  make_version(1),
                  make_skip(2),
                  make_field_name("host"),
                  make_skip(2),
                  make_field_value(""),
                  make_error(http::token::code::error_invalid_data)
              });
    my_tester("GET / HTTP/1.1\r\n"
              "host : localhost\r \n"
              "\r\n",
              {
                  make_method("GET"),
                  make_skip(1),
                  make_request_target("/"),
                  make_skip(8),
                  make_version(1),
                  make_skip(2),
                  make_field_name("host"),
                  make_error(http::token::code::error_invalid_data)
              });
    my_tester("GET / HTTP/1.1\r\n"
              "@host: localhost\r \n"
              "\r\n",
              {
                  make_method("GET"),
                  make_skip(1),
                  make_request_target("/"),
                  make_skip(8),
                  make_version(1),
                  make_skip(2),
                  make_error(http::token::code::error_invalid_data)
              });
    my_tester("GET / HTTP/1.1\r \n"
              "host: localhost\r \n"
              "\r\n",
              {
                  make_method("GET"),
                  make_skip(1),
                  make_request_target("/"),
                  make_skip(8),
                  make_version(1),
                  make_error(http::token::code::error_invalid_data)
              });
    my_tester("GET / HTTP/1.x\r\n"
              "host: localhost\r\n"
              "\r\n",
              {
                  make_method("GET"),
                  make_skip(1),
                  make_request_target("/"),
                  make_skip(8),
                  make_error(http::token::code::error_invalid_data)
              });
    my_tester("GET / HTTP|1.1\r\n"
              "host: localhost\r\n"
              "\r\n",
              {
                  make_method("GET"),
                  make_skip(1),
                  make_request_target("/"),
                  make_error(http::token::code::error_invalid_data)
              });
    my_tester("POST / HTTP/1.1\r\n"
              "host: localhost\r\n"
              "Content-length: 999999999999999999999999999999999999999999\r\n"
              "\r\n"
              "ping",
              {
                  make_method("POST"),
                  make_skip(1),
                  make_request_target("/"),
                  make_skip(8),
                  make_version(1),
                  make_skip(2),
                  make_field_name("host"),
                  make_skip(2),
                  make_field_value("localhost"),
                  make_skip(2),
                  make_field_name("Content-length"),
                  make_skip(2),
                  make_error(http::token::code::error_content_length_overflow)
              });
    my_tester("GET /happy HTTP/1.1\r\n"
              "\r\n",
              {
                  make_method("GET"),
                  make_skip(1),
                  make_request_target("/happy"),
                  make_skip(8),
                  make_version(1),
                  make_skip(2),
                  make_error(http::token::code::error_no_host)
              });
}
