enable_testing()

add_definitions(-DBOOST_TEST_DYN_LINK)

add_executable("firsttest" firsttest.cpp)

target_link_libraries("firsttest"
  boost_http
  ${Boost_DATE_TIME_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_test(NAME firsttest COMMAND $<TARGET_FILE:firsttest>)
