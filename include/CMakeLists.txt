install(DIRECTORY .
  DESTINATION "${includedir}"
  FILES_MATCHING PATTERN *.hpp
  PATTERN CMakeLists.txt EXCLUDE
)
