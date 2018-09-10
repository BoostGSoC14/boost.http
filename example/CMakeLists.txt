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

find_package(Threads)

# Config

link_directories("${target}" ${Boost_LIBRARIES_DIRS})

# Examples' stuff

set(examples
  "spawn"
  "regex_router"
  "basic_router"
  "spawn-chunked"
  "spawn-with-runtime"
  "spawn-with-runtime2"
  "spawn-with-runtime3"
  "spawn-with-runtime4"
  "spawn-fileserver"
  "spawn-fileserver2"
  "spawn-settings"
)

macro(add_example_target target)
  add_executable("${target}" "${target}.cpp")

  set_property(TARGET "${target}" PROPERTY CXX_STANDARD 17)
  set_property(TARGET "${target}" PROPERTY CXX_STANDARD_REQUIRED ON)

  if(WIN32)
    if(MSVC)
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
      set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
      set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /SAFESEH:NO")
    endif()
  endif()

  target_compile_definitions("${target}"
    PRIVATE BOOST_COROUTINES_NO_DEPRECATION_WARNING BOOST_ASIO_NO_DEPRECATED)

  target_include_directories("${target}"
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include" ${Boost_INCLUDE_DIR})

  target_link_libraries("${target}"
    ${Boost_DATE_TIME_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_COROUTINE_LIBRARY}
    ${Boost_CONTEXT_LIBRARY}
     ${CMAKE_THREAD_LIBS_INIT})

  #additional libraries for Windows builds
  if(MSVC OR MINGW)
      target_link_libraries("${target}" ws2_32 mswsock)
  endif()
endmacro()

foreach(example ${examples})
  add_example_target("${example}")
endforeach()
