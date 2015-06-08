# Project Boost.Http
project(Boost.Http)

# Dependencies
cmake_minimum_required(VERSION 3.1.0)

find_package(Boost 1.55 COMPONENTS
  date_time
  filesystem
  system
  unit_test_framework
  context
  coroutine
  REQUIRED)

# Configure options
option(BUILD_TESTS
  "Build tests" YES
)

option(BUILD_EXAMPLES
  "Build examples" YES
)

# Install info
set(includedir "include")
set(libdir "lib")

# Targets
add_subdirectory(include)

# src
set(library_SRC
  src/http_category.cpp
  src/file_server.cpp
  src/socket.cpp
  src/http_parser.c
)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include" ${Boost_INCLUDE_DIR})

add_library("boost_http" ${library_SRC})

set_property(TARGET "boost_http" PROPERTY CXX_STANDARD 11)
set_property(TARGET "boost_http" PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries("boost_http"
  ${Boost_DATE_TIME_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY})

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()

if(BUILD_EXAMPLES)
  add_subdirectory(example)
endif()

install(TARGETS "boost_http"
  DESTINATION "${libdir}"
)

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

# CTest
if(BUILD_TESTS)
  include(CTest)
endif()
