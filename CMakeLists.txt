# Project Boost.Http
project(Boost.Http)

# Dependencies
cmake_minimum_required(VERSION 2.8.12)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})

find_package(CXX11 REQUIRED)
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

# Build options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX11_FLAGS}")

# Install info
set(includedir "include")
set(libdir "lib")

# Targets
add_subdirectory(include)
add_subdirectory(src)

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
