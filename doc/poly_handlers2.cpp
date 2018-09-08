void work()
{
    // ...

    auto handler = boost::asio::bind_executor(
        boost::asio::io_context::strand{ctx},
        [](boost::system::error_code, std::size_t) {}
    );

    boost::asio::async_write(*sock, handler);
}
