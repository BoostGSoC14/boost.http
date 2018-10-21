#include <boost/asio/spawn.hpp>

// inspired by
// https://github.com/facebook/folly/blob/master/folly/fibers/Baton.h
// (it's just a 1:1 semaphore)
class fiber_baton
{
public:
    fiber_baton(boost::asio::io_context &ctx)
        : executor(ctx.get_executor())
    {}

    fiber_baton(const fiber_baton&) = delete;

    void post()
    {
        if (!pending) {
            posted = true;
            return;
        }

        executor.on_work_finished();
        std::allocator<void> alloc;
        auto p = std::move(pending);
        pending = nullptr;
        executor.dispatch(std::move(p), alloc);
    }

    void wait(boost::asio::yield_context &yield)
    {
        if (posted) {
            posted = false;
            return;
        }

        // suspend
        boost::asio::async_completion<
            decltype(yield), void(boost::system::error_code)
        > init{yield};
        pending = std::move(init.completion_handler);
        executor.on_work_started();
        init.result.get();
    }

private:
    boost::asio::io_context::executor_type executor;
    bool posted = false;
    std::function<void()> pending;
};
