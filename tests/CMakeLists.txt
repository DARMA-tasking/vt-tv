set(BUILD_GMOCK OFF CACHE BOOL "Builds the googlemock subproject" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)" FORCE)

add_subdirectory(extern/googletest)
set(GOOGLETEST_LIBRARY gtest)

# Hide various options from UI-based property editors
mark_as_advanced(
  BUILD_GTEST INSTALL_GTEST BUILD_SHARED_LIBS
  gtest_build_samples gtest_build_tests
  gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
)

# Group targets, in IDEs supporting such
set_target_properties(gtest PROPERTIES FOLDER extern)
set_target_properties(gtest_main PROPERTIES FOLDER extern)

include(GoogleTest)

set(PROJECT_TEST_UNIT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/unit)
set(PROJECT_GTEST_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/googletest/googletest/include)

file(
  GLOB
  PROJECT_TEST_LIST
  RELATIVE
  "${PROJECT_TEST_UNIT_DIR}"
  "${PROJECT_TEST_UNIT_DIR}/*.cc"
)

macro(vt_tv_link_target target has_mpi)
  target_include_directories(${target} PUBLIC ${PROJECT_TEST_UNIT_DIR})
  target_include_directories(${target} PRIVATE ${PROJECT_GTEST_INCLUDE_DIR})
  target_link_libraries(${target} PRIVATE gtest)
  target_link_libraries(${target} PUBLIC ${VT_TV_LIBRARY})
endmacro()

set(
  TEST_HEADER_FILES
  ${PROJECT_TEST_UNIT_DIR}/test_harness.h
)

set(
  TEST_SOURCE_FILES
  ${PROJECT_TEST_UNIT_DIR}/main.cc
)

foreach(TEST_FULL ${PROJECT_TEST_LIST})
  GET_FILENAME_COMPONENT(
    TEST
    ${TEST_FULL}
    NAME_WE
  )

  add_executable(
    ${TEST}
    ${TEST_SOURCE_FILES}
    ${TEST_HEADER_FILES}
    ${PROJECT_TEST_UNIT_DIR}/${TEST}.cc
  )

  vt_tv_link_target(${TEST} 0)

  add_test(
    vt-tv:${TEST}
    ${CMAKE_CURRENT_BINARY_DIR}/${TEST}
  )

  set_tests_properties(
    vt-tv:${TEST}
    PROPERTIES TIMEOUT 60
    FAIL_REGULAR_EXPRESSION "FAILED;should be deleted but never is"
    PASS_REGULAR_EXPRESSION "PASSED"
  )
endforeach()
