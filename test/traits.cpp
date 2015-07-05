#include <boost/http/message.hpp>
#include <boost/http/socket.hpp>

using namespace boost;
using namespace std;

class dumb {};

class dumb2 {};

namespace boost {
namespace http {
template<>
struct is_server_socket<dumb2>: public std::true_type {};
} // namespace http {
} // namespace boost {

static_assert(!http::is_message<dumb>::value, "dumb is a message?!");
static_assert(http::is_message<http::message>::value,
              "http::message is not a message?!");
static_assert(http::is_message<http::basic_message<dumb, dumb>>::value,
              "http::basic_message<dumb, dumb> is not a message?!");

static_assert(!http::is_server_socket<dumb>::value,
              "dumb is a server_socket?!");
static_assert(http::is_server_socket<http::socket>::value,
              "http::socket is not a server_socket?!");
static_assert(http::is_server_socket<http::basic_socket<dumb>>::value,
              "http::basic_socket<dumb> is not a server_socket?!");
static_assert(http::is_server_socket<dumb2>::value,
              "dumb2 is not a server_socket?!");

static_assert(!http::is_socket<dumb>::value, "dumb is a socket?!");
static_assert(http::is_socket<http::socket>::value,
              "http::socket is not a socket?!");
static_assert(http::is_socket<http::basic_socket<dumb>>::value,
              "http::basic_socket<dumb> is not a socket?!");
static_assert(http::is_socket<dumb2>::value, "dumb2 is not a socket?!");

int main() {}
