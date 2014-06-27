set(library_SRC
  http_category.cpp
  embedded_server_socket.cpp
  embedded_server_socket_acceptor.cpp
  http_parser.c
)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../include" ${Boost_INCLUDE_DIR})

add_library("boost_http" SHARED ${library_SRC})

target_link_libraries("boost_http"
  ${Boost_DATE_TIME_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY})

enable_testing()
add_subdirectory(tests)

install(TARGETS "boost_http"
  DESTINATION "${libdir}"
)
