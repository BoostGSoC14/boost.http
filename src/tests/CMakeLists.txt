enable_testing()

add_definitions(-DBOOST_TEST_DYN_LINK)

set(tests
  "firsttest"
  "socket_concept"
  "errc"
  "algorithm"
)

macro(add_test_target target)
  add_executable("${target}" "${target}.cpp")

  target_link_libraries("${target}"
    boost_http
    ${Boost_DATE_TIME_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

  add_test(NAME "${target}" COMMAND $<TARGET_FILE:${target}>)
endmacro()

foreach(test ${tests})
  add_test_target("${test}")
endforeach()
