#include <vector>
#include <boost/asio/io_context.hpp>
#include <boost/asio/buffer.hpp>

class mock_socket
{
public:
    typedef boost::asio::io_context::executor_type executor_type;
    typedef mock_socket lowest_layer_type;

    mock_socket(boost::asio::io_context &io_context) :
        io_context(io_context)
    {}

    bool is_open() const
    {
        return true;
    }

    void close() {}

    template<class MutableBufferSequence, class CompletionToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken,
                                  void(boost::system::error_code, std::size_t))
    async_read_some(const MutableBufferSequence &buffers,
                    CompletionToken &&token)
    {
        using namespace boost;

        asio::async_completion<CompletionToken,
                               void(system::error_code, std::size_t)>
            init{token};
        auto handler(init.completion_handler);
        auto ex(boost::asio::get_associated_executor(handler, io_context));
        auto alloc(asio::get_associated_allocator(handler));

        if (input_buffer.size() == 0 || input_buffer.front().size() == 0) {
            ex.post([handler]() mutable {
                    handler(system::error_code(asio::error::eof), 0);
                }, alloc);
            return init.result.get();
        }

        auto bytes_transfered
            = asio::buffer_copy(buffers, asio::buffer(input_buffer.front()));

        if (bytes_transfered == input_buffer.front().size()) {
            input_buffer.erase(input_buffer.begin());
        } else {
            std::vector<char> v(input_buffer.front().begin() + bytes_transfered,
                                input_buffer.front().end());
            input_buffer.front() = std::move(v);
        }

        if (bytes_transfered == 0 && close_on_empty_input) {
            ex.post([handler, bytes_transfered]() mutable {
                    handler(system::error_code(asio::error::eof), 0);
                }, alloc);
        } else {
            ex.post([handler, bytes_transfered]() mutable {
                    handler(system::error_code(), bytes_transfered);
                }, alloc);
        }

        return init.result.get();
    }

    executor_type get_executor()
    {
        return io_context.get_executor();
    }

    template<class ConstBufferSequence, class CompletionToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken,
                                  void(boost::system::error_code, std::size_t))
    async_write_some(const ConstBufferSequence &buffers,
                     CompletionToken &&token)
    {
        using namespace boost;

        asio::async_completion<CompletionToken,
                               void(system::error_code, std::size_t)>
            init{token};
        auto handler(init.completion_handler);
        auto ex(boost::asio::get_associated_executor(handler, io_context));
        auto alloc(asio::get_associated_allocator(handler));

        auto more = asio::buffer_size(buffers);
        auto offset = output_buffer.size();
        output_buffer.resize(offset + more);
        asio::buffer_copy(asio::buffer(output_buffer.data() + offset, more),
                          buffers);

        ex.post([more,handler]() mutable {
                handler(system::error_code(), more);
            }, alloc);

        return init.result.get();
    }

    lowest_layer_type& lowest_layer()
    {
        return *this;
    }

    std::vector<std::vector<char>> input_buffer;
    std::vector<char> output_buffer;

    bool close_on_empty_input = false;

private:
    boost::asio::io_context &io_context;
};
