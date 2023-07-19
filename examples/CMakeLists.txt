
macro(add_test_for_example_vt_tv test_name test_exec)
  add_test(
    ${test_name} ${test_exec} ${ARGN}
  )

  set_tests_properties(
    ${test_name}
    PROPERTIES TIMEOUT 300 FAIL_REGULAR_EXPRESSION "FAILED;WARNING"
  )
endmacro()

file(
  GLOB
  PROJECT_EXAMPLES
  RELATIVE
  "${PROJECT_EXAMPLE_DIR}"
  "${PROJECT_EXAMPLE_DIR}/*.cc"
)

foreach(EXAMPLE_FULL ${PROJECT_EXAMPLES})
  GET_FILENAME_COMPONENT(
    EXAMPLE
    ${EXAMPLE_FULL}
    NAME_WE
  )

  add_executable(
    ${EXAMPLE}
    ${PROJECT_EXAMPLE_DIR}/${EXAMPLE}.cc
  )

  target_include_directories(${EXAMPLE} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../src)
  target_include_directories(${EXAMPLE} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../lib/CLI)

  add_test_for_example_vt_tv(
    vt-tv:${EXAMPLE}
    ${CMAKE_CURRENT_BINARY_DIR}/${EXAMPLE}
  )

  target_link_libraries(
    ${EXAMPLE}
    PUBLIC
    ${VT_TV_LIBRARY_NS}
  )
endforeach()
