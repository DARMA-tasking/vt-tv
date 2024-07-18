set(BUILD_GMOCK ON CACHE BOOL "Builds the googlemock subproject" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)" FORCE)

if (NOT TARGET gtest)
  # include GoogleTest and enable testing
  add_subdirectory(${PROJECT_LIB_DIR}/googletest ${PROJECT_BIN_DIR}/lib/googletest)
endif()
set(GOOGLETEST_LIBRARY gtest)

include(GoogleTest)

# Hide various options from UI-based property editors
mark_as_advanced(
  BUILD_GTEST INSTALL_GTEST BUILD_SHARED_LIBS
  gtest_build_samples gtest_build_tests
  gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
)

# Group targets, in IDEs supporting such
set_target_properties(gtest PROPERTIES FOLDER lib)
set_target_properties(gtest_main PROPERTIES FOLDER lib)

set(PROJECT_TEST_UNIT_DIR ${SRC_DIR}/unit)

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

include(turn_on_warnings)

# Macro to link vt-tv, gtest and gmock to a target
macro(vt_tv_link_target target)
  target_include_directories(${target} PRIVATE
    ${gtest_SOURCE_DIR}/include
    ${gmock_SOURCE_DIR}/include
  )
  target_link_libraries(${target} PRIVATE GTest::gtest_main GTest::gmock_main)
  target_link_libraries(${target} PUBLIC ${VT_TV_LIBRARY})
endmacro()

option(VT_TV_TESTS_ALL_IN_ONE "Integrates all tests in a single build target named AllTests." OFF)

if (VT_TV_TESTS_ALL_IN_ONE)

  add_executable(
    AllTests
    main.cc
    ${TEST_HEADER_FILES}
    ${TEST_SOURCE_FILES}
  )

  turn_on_warnings(AllTests)

  # Initialize VTK modules
  vtk_module_autoinit(
    TARGETS AllTests
    MODULES ${VTK_LIBRARIES}
  )

  vt_tv_link_target(AllTests)
  gtest_discover_tests(AllTests)

else()

  foreach(TEST_FILENAME ${TEST_SOURCE_FILES})

    # Extract the test file base name to use as the CMake target name
    GET_FILENAME_COMPONENT(
      TEST
      ${TEST_FILENAME}
      NAME_WE
    )

    # Build test as an individual target
    add_executable(
      ${TEST}
      ${TEST_FILENAME}
      ${TEST_HEADER_FILES}
      ${TEST_FILENAME}
    )

    turn_on_warnings(${TEST})

    # Initialize VTK modules
    vtk_module_autoinit(
      TARGETS ${TEST}
      MODULES ${VTK_LIBRARIES}
    )

    vt_tv_link_target(${TEST})

    gtest_discover_tests(${TEST})

  endforeach()

endif()

# ERROR (gcc): The vtkWindowToImageFilter class generated segfault when running a unit test that should generates PNG output through VTK.
# --> SKIPPED TESTS: test_render, test_parse_render and test_vtk
# add_compile_options(-g -ggdb)
# A detailed report can be generated using valgrind utility. A valgrind report has already been added to #32 comment on GH.
# `valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=output/valgrind-report.txt ./build/tests/unit/test_vtk`
