#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <boost/http/embedded_server_socket.hpp>
#include <boost/http/traits.hpp>

class A
{};

struct B
{
    boost::http::outgoing_state outgoing_state() const;
    bool outgoing_response_native_stream() const;
    bool incoming_request_continue_required() const;
    bool incoming_request_upgrade_required() const;
    bool outgoing_response_write_continue();
};

struct C: B
{
};

struct D
{
    int outgoing_state() const;
    void outgoing_response_native_stream() const;
    void incoming_request_continue_required() const;
    void incoming_request_upgrade_required() const;
    void outgoing_response_write_continue();
};

struct E
{
    boost::http::outgoing_state outgoing_state();
    bool outgoing_response_native_stream();
    bool incoming_request_continue_required();
    bool incoming_request_upgrade_required();
    bool outgoing_response_write_continue() const; //< kind of useless
};

struct F
{
    struct X
    {
        operator boost::http::outgoing_state();
    };

    struct Y
    {
        operator bool();
    };

    X outgoing_state() const;
    Y outgoing_response_native_stream() const;
    Y incoming_request_continue_required() const;
    Y incoming_request_upgrade_required() const;
    Y outgoing_response_write_continue();
};

BOOST_AUTO_TEST_CASE(Simple_attributes) {
    using namespace boost::http;

    BOOST_CHECK(!is_socket<int>::value);
    BOOST_CHECK(!is_socket<A>::value);
    BOOST_CHECK(is_socket<B>::value);
    BOOST_CHECK(is_socket<C>::value);
    BOOST_CHECK(!is_socket<D>::value);
    BOOST_CHECK(!is_socket<E>::value);
    BOOST_CHECK(is_socket<F>::value);
    BOOST_CHECK(is_socket<embedded_server_socket<>>::value);
}
