/* Copyright (c) 2014 Vinícius dos Santos Oliveira

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) */

#ifndef BOOST_HTTP_DETAIL_CONFIG_H
#define BOOST_HTTP_DETAIL_CONFIG_H

#include <boost/config.hpp>

#ifdef BOOST_HTTP_DECL
#  undef BOOST_HTTP_DECL
#endif

#if defined(BOOST_ALL_DYN_LINK) || defined(BOOST_HTTP_DYN_LINK)
#  if defined(BOOST_HTTP_SOURCE)
#    define BOOST_HTTP_DECL BOOST_SYMBOL_EXPORT
#  else
#    define BOOST_HTTP_DECL BOOST_SYMBOL_IMPORT
#  endif
#endif

#if ! defined(BOOST_HTTP_DECL)
#  define BOOST_HTTP_DECL
#endif

#endif // BOOST_HTTP_DETAIL_CONFIG_H
