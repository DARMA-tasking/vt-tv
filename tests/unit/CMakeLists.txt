# Help at https://google.github.io/googletest/quickstart-cmake.html

# 1. Add Google Test Dependency

include(CMakePrintHelpers)

# GoogleTest requires at least C++14
# set(CMAKE_CXX_STANDARD 14)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)



include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)
FetchContent_Declare(
  googletest
  DOWNLOAD_EXTRACT_TIMESTAMP True # Option needed with CMake <= 3.24 which introduced CMP0135 Policy
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

set(BUILD_GMOCK ON CACHE BOOL "Builds the googlemock subproject" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)" FORCE)

# Hide various options from UI-based property editors
mark_as_advanced(
  BUILD_GTEST INSTALL_GTEST BUILD_SHARED_LIBS
  gtest_build_samples gtest_build_tests
  gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
)

FetchContent_MakeAvailable(googletest)

# 2. Add tests

# Group targets, in IDEs supporting such
# set_target_properties(gtest PROPERTIES FOLDER extern)
# set_target_properties(gtest_main PROPERTIES FOLDER extern)

# set(PROJECT_GTEST_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/googletest/googletest/include)

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

# Print variables
cmake_print_variables(TEST_SOURCE_FILES)
cmake_print_variables(TEST_HEADER_FILES)
cmake_print_variables(FETCHCONTENT_BASE_DIR)

enable_testing()

# Macro to link vt-tv, gtest and gmock to a target
macro(vt_tv_link_target target)
  target_link_libraries(${target} PRIVATE gtest)
  target_link_libraries(${target} PRIVATE gmock)
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
