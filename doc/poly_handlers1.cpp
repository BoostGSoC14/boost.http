class my_socket
{
public:
    virtual ~my_socket() = default;

    virtual void async_write_some(
        boost::asio::const_buffer buf,
        std::function<void(boost::system::error_code, std::size_t)> handler
    ) = 0;

    virtual void async_read_some(
        std::vector<char> &buf,
        std::function<void(boost::system::error_code, std::size_t)> handler
    ) = 0;
};

void work()
{
    // ...

    auto handle_socket = boost::dll::import<void(std::shared_ptr<my_socket>)>(
        path_to_shared_library, "handle_socket"
    );

    // ...

    for (;;) {
        // ...

        handle_socket(sock);
    }
}
