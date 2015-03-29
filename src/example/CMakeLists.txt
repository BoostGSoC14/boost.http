set(examples
  "spawn"
  "spawn-chunked"
  "spawn-with-runtime"
  "spawn-with-runtime2"
  "spawn-fileserver"
  "spawn-fileserver2"
)

macro(add_example_target target)
  add_executable("${target}" "${target}.cpp")

  set_property(TARGET "${target}" PROPERTY CXX_STANDARD 11)
  set_property(TARGET "${target}" PROPERTY CXX_STANDARD_REQUIRED ON)

  target_link_libraries("${target}"
    boost_http
    ${Boost_DATE_TIME_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_COROUTINE_LIBRARY}
    ${Boost_CONTEXT_LIBRARY})
endmacro()

foreach(example ${examples})
  add_example_target("${example}")
endforeach()
