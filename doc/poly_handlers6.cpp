class my_socket
{
public:
    virtual ~my_socket() = default;

    virtual void async_write_some(
        boost::asio::const_buffer buf,
        boost::http::asio::experimental::poly_handler<
            void(boost::system::error_code, std::size_t)
        > handler
    ) = 0;

    virtual void async_read_some(
        std::vector<char> &buf,
        boost::http::asio::experimental::poly_handler<
            void(boost::system::error_code, std::size_t)
        > handler
    ) = 0;
};
