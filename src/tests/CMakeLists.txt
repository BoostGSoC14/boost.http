enable_testing()

add_definitions(-DBOOST_TEST_DYN_LINK)

set(tests
  "errc"
  "algorithm"
  "datetime"
)

macro(add_test_target target)
  add_executable("${target}" "${target}.cpp")

  set_property(TARGET "${target}" PROPERTY CXX_STANDARD 11)
  set_property(TARGET "${target}" PROPERTY CXX_STANDARD_REQUIRED ON)

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
