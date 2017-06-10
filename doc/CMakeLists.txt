add_custom_target(adoc ALL
    COMMAND asciidoctor -a toc=left ${CMAKE_CURRENT_SOURCE_DIR}/http.adoc
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building ${CMAKE_CURRENT_SOURCE_DIR}/http.adoc")
