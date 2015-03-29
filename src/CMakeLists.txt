set(library_SRC
  http_category.cpp
  file_server.cpp
  socket.cpp
  http_parser.c
)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../include" ${Boost_INCLUDE_DIR})

add_library("boost_http" SHARED ${library_SRC})

set_property(TARGET "boost_http" PROPERTY CXX_STANDARD 11)
set_property(TARGET "boost_http" PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries("boost_http"
  ${Boost_DATE_TIME_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY})

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
  add_subdirectory(example)
endif()

install(TARGETS "boost_http"
  DESTINATION "${libdir}"
)
