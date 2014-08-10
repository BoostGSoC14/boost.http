set(library_SRC
  http_category.cpp
  socket.cpp
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
add_subdirectory(example)

install(TARGETS "boost_http"
  DESTINATION "${libdir}"
)
