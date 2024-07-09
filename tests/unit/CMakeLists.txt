set(BUILD_GMOCK ON CACHE BOOL "Builds the googlemock subproject" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)" FORCE)

if (NOT TARGET gtest)
  add_subdirectory(${PROJECT_LIB_DIR}/googletest ${PROJECT_BIN_DIR}/lib/googletest)
endif()
set(GOOGLETEST_LIBRARY gtest)

# Hide various options from UI-based property editors
mark_as_advanced(
  BUILD_GTEST INSTALL_GTEST BUILD_SHARED_LIBS
  gtest_build_samples gtest_build_tests
  gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
)

# Group targets, in IDEs supporting such
# set_target_properties(gtest PROPERTIES FOLDER lib)
# set_target_properties(gtest_main PROPERTIES FOLDER lib)

include(GoogleTest)

set(PROJECT_TEST_UNIT_DIR ${SRC_DIR}/unit)
set(GTEST_INCLUDE_DIR ${PROJECT_LIB_DIR}/googletest/googletest/include)
set(GMOCK_INCLUDE_DIR ${PROJECT_LIB_DIR}/googletest/googlemock/include)

# Collect sources
file(
  GLOB_RECURSE
  TEST_SOURCE_FILES
  "*.cc"
)

file(
  GLOB_RECURSE
  TEST_HEADER_FILES
  "*.h"
)

enable_testing()

# Macro to link vt-tv, gtest and gmock to a target
macro(vt_tv_link_target target)
  target_include_directories(${target} PRIVATE ${GTEST_INCLUDE_DIR})
  target_include_directories(${target} PRIVATE ${GMOCK_INCLUDE_DIR})
  target_link_libraries(${target} PRIVATE gtest)
  target_link_libraries(${target} PUBLIC ${VT_TV_LIBRARY})
endmacro()

foreach(TEST_FILENAME ${TEST_SOURCE_FILES})

  # Extract the test file base name to use as the CMake target name
  GET_FILENAME_COMPONENT(
    TEST
    ${TEST_FILENAME}
    NAME_WE
  )

  # Build test as an individual executable
  add_executable(
    ${TEST}
    main.cc
    ${TEST_HEADER_FILES}
    ${TEST_FILENAME}
  )

  vt_tv_link_target(${TEST})

  add_test(
    vt-tv:${TEST}
    ${TEST}
  )

  set_tests_properties(
    vt-tv:${TEST}
    PROPERTIES TIMEOUT 60
    FAIL_REGULAR_EXPRESSION "FAILED;should be deleted but never is"
    PASS_REGULAR_EXPRESSION "PASSED"
  )

endforeach()
