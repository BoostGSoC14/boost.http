/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#include <boost/http/http_errc.hpp>

namespace boost {
namespace http {

class http_category_impl: public boost::system::error_category
{
public:
    const char* name() const noexcept override;
    std::string message(int condition) const noexcept override;
};

const char* http_category_impl::name() const noexcept
{
    return "http";
}

std::string http_category_impl::message(int condition) const noexcept
{
    switch (condition) {
    case static_cast<int>(http_errc::out_of_order):
        return "HTTP actions issued on the wrong order for some object";
    case static_cast<int>(http_errc::parsing_error):
        return "The parser encountered an error";
    case static_cast<int>(http_errc::buffer_exhausted):
        return "Need more buffer space to complete an operation";
    default:
        return "undefined";
    }
}

const boost::system::error_category& http_category()
{
    static http_category_impl category;
    return category;
}

} // namespace http
} // namespace boost
