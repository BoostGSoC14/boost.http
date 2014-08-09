set(examples
  "spawn"
  "spawn-chunked"
  "spawn-with-runtime"
)

macro(add_example_target target)
  add_executable("${target}" "${target}.cpp")

  target_link_libraries("${target}"
    boost_http
    ${Boost_DATE_TIME_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_CONTEXT_LIBRARY}
    ${Boost_COROUTINE_LIBRARY})
endmacro()

foreach(example ${examples})
  add_example_target("${example}")
endforeach()
