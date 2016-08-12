# Meta
project(examples)

# Dependencies
cmake_minimum_required(VERSION 3.1.0)

find_package(Boost 1.55 COMPONENTS
  date_time
  filesystem
  system
  context
  coroutine
  REQUIRED)

# Config

link_directories("${target}" ${Boost_LIBRARIES_DIRS})

# Examples' stuff

set(examples
  "spawn"
)

macro(add_example_target target)
  add_executable("${target}" "${target}.cpp")

  set_property(TARGET "${target}" PROPERTY CXX_STANDARD 11)
  set_property(TARGET "${target}" PROPERTY CXX_STANDARD_REQUIRED ON)

  target_include_directories("${target}"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include" ${Boost_INCLUDE_DIR})

  target_link_libraries("${target}"
    ${Boost_DATE_TIME_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_COROUTINE_LIBRARY}
    ${Boost_CONTEXT_LIBRARY})
endmacro()

foreach(example ${examples})
  add_example_target("${example}")
endforeach()
