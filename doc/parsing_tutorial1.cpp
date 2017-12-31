#include <boost/http/reader/request.hpp>
#include <string>
#include <map>

namespace http = boost::http;
namespace asio = boost::asio;

struct my_socket_consumer
{
private:
    http::reader::request request_reader;
    std::string buffer;

    std::string last_header;

public:
    std::string method;
    std::string request_target;
    int version;

    std::multimap<std::string, std::string> headers;

    void on_socket_callback(asio::buffer data)
    {
        using namespace http::token;
        using token::code;

        buffer.push_back(data);
        request_reader.set_buffer(buffer);

        while (request_reader.code() != code::end_of_message) {
            switch (request_reader.code()) {
            case code::skip:
                // do nothing
                break;
            case code::method:
                method = request_reader.value<token::method>();
                break;
            case code::request_target:
                request_target = request_reader.value<token::request_target>();
                break;
            case code::version:
                version = request_reader.value<token::version>();
                break;
            case code::field_name:
            case code::trailer_name:
                last_header = request_reader.value<token::field_name>();
            }
            request_reader.next();
        }
        request_reader.next();

        ready();
    }

protected:
    virtual void ready() = 0;
};
