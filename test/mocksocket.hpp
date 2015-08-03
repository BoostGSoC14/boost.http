#include <vector>
#include <boost/asio/io_service.hpp>
#include <boost/asio/buffer.hpp>

class mock_socket
{
public:
    mock_socket(boost::asio::io_service &io_service) :
        io_service(io_service)
    {}

    bool is_open() const
    {
        return true;
    }

    void close() {}

    template<class MutableBufferSequence, class CompletionToken>
    typename boost::asio::async_result<
        typename boost::asio::handler_type<
            CompletionToken, void(boost::system::error_code, std::size_t)
        >::type>::type
    async_read_some(const MutableBufferSequence &buffers,
                    CompletionToken &&token)
    {
        using namespace boost;

        typedef typename asio::handler_type<
        CompletionToken, void(system::error_code, std::size_t)>::type Handler;

        Handler handler(std::forward<CompletionToken>(token));
        asio::async_result<Handler> result(handler);

        if (input_buffer.size() == 0 || input_buffer.front().size() == 0) {
            io_service.post([handler]() mutable {
                    handler(system::error_code(asio::error::eof), 0);
                });
            return result.get();
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

        io_service.post([handler, bytes_transfered]() mutable {
                handler(system::error_code(), bytes_transfered);
            });

        return result.get();
    }

    boost::asio::io_service &get_io_service()
    {
        return io_service;
    }

    template<class ConstBufferSequence, class CompletionToken>
    typename boost::asio::async_result<
        typename boost::asio::handler_type<
            CompletionToken, void(boost::system::error_code, std::size_t)
        >::type>::type
    async_write_some(const ConstBufferSequence &buffers,
                     CompletionToken &&token)
    {
        using namespace boost;

        typedef typename asio::handler_type<
        CompletionToken, void(system::error_code, std::size_t)>::type Handler;

        Handler handler(std::forward<CompletionToken>(token));
        asio::async_result<Handler> result(handler);

        auto more = asio::buffer_size(buffers);
        auto offset = output_buffer.size();
        output_buffer.resize(offset + more);
        asio::buffer_copy(asio::buffer(output_buffer.data() + offset, more),
                          buffers);

        io_service.post([more,handler]() mutable {
                handler(system::error_code(), more);
            });

        return result.get();
    }

    std::vector<std::vector<char>> input_buffer;
    std::vector<char> output_buffer;

private:
    boost::asio::io_service &io_service;
};










