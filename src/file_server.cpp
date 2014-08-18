/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#include <boost/http/file_server.hpp>

namespace boost {
namespace http {

class file_server_category_impl: public boost::system::error_category
{
public:
    const char* name() const noexcept override;
    std::string message(int condition) const noexcept override;
};

const char* file_server_category_impl::name() const noexcept
{
    return "file_server";
}

std::string file_server_category_impl::message(int condition) const noexcept
{
    switch (condition) {
    case static_cast<int>(file_server_errc::io_error):
        return "IO failed";
    case static_cast<int>(file_server_errc::irrecoverable_io_error):
        return "IO failed after some channel operation already was issued";
    case static_cast<int>(file_server_errc::write_state_not_supported):
        return "Cannot operate on channels with this write_state";
    case static_cast<int>(file_server_errc::file_not_found):
        return "The requested file wasn't found";
    case static_cast<int>(file_server_errc::file_type_not_supported):
        return "Cannot process type for the found file";
    case static_cast<int>(file_server_errc::filter_set):
        return "The user custom filter aborted the operation";
    default:
        return "undefined";
    }
}

const system::error_category& file_server_category()
{
    static file_server_category_impl category;
    return category;
}

} // namespace http
} // namespace boost
