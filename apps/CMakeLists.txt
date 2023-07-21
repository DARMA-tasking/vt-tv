
macro(add_test_for_app_vt_tv test_name test_exec)
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
  PROJECT_APPS
  RELATIVE
  "${PROJECT_APP_DIR}"
  "${PROJECT_APP_DIR}/*.cc"
)

foreach(APP_FULL ${PROJECT_APPS})
  GET_FILENAME_COMPONENT(
    APP
    ${APP_FULL}
    NAME_WE
  )

  add_executable(
    ${APP}
    ${PROJECT_APP_DIR}/${APP}.cc
  )

  target_include_directories(${APP} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../src)
  target_include_directories(${APP} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../lib/CLI)

  add_test_for_app_vt_tv(
    vt-tv:${APP}
    ${CMAKE_CURRENT_BINARY_DIR}/${APP}
  )

  target_link_libraries(
    ${APP}
    PUBLIC
    ${VT_TV_LIBRARY_NS}
  )
endforeach()