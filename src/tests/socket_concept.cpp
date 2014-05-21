#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <boost/http/embedded_server/embedded_server.hpp>
#include <boost/http/traits.hpp>

class A
{};

struct B
{
    boost::http::outgoing_state outgoing_state() const;
};

struct C: B
{
};

struct D
{
    int outgoing_state() const;
};

struct E
{
    boost::http::outgoing_state outgoing_state();
};

struct F
{
    struct X
    {
        operator boost::http::outgoing_state();
    };

    X outgoing_state() const;
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
    BOOST_CHECK(is_socket<basic_socket<embedded_server>>::value);
}
