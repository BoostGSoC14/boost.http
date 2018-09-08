#include <iostream>

#include <boost/http/asio/experimental/poly_handler.hpp>

struct MyHandler
{
    using allocator_type = std::experimental::pmr::polymorphic_allocator<void>;

    MyHandler(std::experimental::pmr::memory_resource *r)
        : r(r)
    {}

    void operator()() {}

    allocator_type get_allocator() const
    {
        return {r};
    }

    std::experimental::pmr::memory_resource *r;
};

int main()
{
    auto r1 = std::experimental::pmr::new_delete_resource();
    auto r2 = std::experimental::pmr::null_memory_resource();
    boost::http::asio::experimental::poly_handler<void()>
        handler{MyHandler{r1}};

    std::experimental::pmr::polymorphic_allocator<void> fallback_a = r2;
    auto a = boost::asio::get_associated_allocator(handler, fallback_a);

    std::cout << "r1 = " << (void*)(r1) << std::endl;
    std::cout << "r2 = " << (void*)(r2) << std::endl;
    std::cout << "a = " << (void*)(a.resource()) << std::endl;

    return 0;
}
