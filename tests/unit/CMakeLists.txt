set(BUILD_GMOCK ON CACHE BOOL "Builds the googlemock subproject" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)" FORCE)

if (NOT TARGET gtest)
  # include GoogleTest and enable testing
  add_subdirectory(${PROJECT_LIB_DIR}/googletest ${PROJECT_BIN_DIR}/lib/googletest)
endif()
set(GOOGLETEST_LIBRARY gtest)

include(GoogleTest)

# Profiling and debug tests
# if(CMAKE_BUILD_TYPE MATCHES "DEBUG")
  #   -ltcmalloc links profiling tools library installed in googleperf-tools package

  # add_compile_options(-fsanitize=address -fno-omit-frame-pointer -fprofile-arcs -ltcmalloc)
  # add_link_options(-fsanitize=address -fno-omit-frame-pointer -fprofile-arcs -ltcmalloc)

  # add_compile_options(-fsanitize=address,undefined -fprofile-arcs -fno-omit-frame-pointer) # -ltcmalloc
  # add_link_options(-fsanitize=address,undefined -fprofile-arcs -fno-omit-frame-pointer) # -ltcmalloc
  # set(ENV{ASAN_OPTIONS} intercept_tls_get_addr=0)

#   add_compile_options(-fsanitize=leak,address,undefined -fno-omit-frame-pointer -fno-common -O1)
#   link_libraries(-fsanitize=leak,address,undefined)
# endif()

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

# Macro to link vt-tv, gtest and gmock to a target
macro(vt_tv_link_target target)
  target_include_directories(${target} PRIVATE
    ${gtest_SOURCE_DIR}/include
    ${gmock_SOURCE_DIR}/include
    # /usr/include/gperftools
  )
  target_link_libraries(${target} PRIVATE GTest::gtest_main) # profiling: tcmalloc pthread
  target_link_libraries(${target} PUBLIC ${VT_TV_LIBRARY})
endmacro()

option(VT_TV_TESTS_ALL_IN_ONE "Integrates all tests in a single build target." OFF)

if (VT_TV_TESTS_ALL_IN_ONE)

  add_executable(
    AllTests
    main.cc
    ${TEST_HEADER_FILES}
    ${TEST_SOURCE_FILES}
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
      main.cc
      ${TEST_HEADER_FILES}
      ${TEST_FILENAME}
    )

    vt_tv_link_target(${TEST})

    gtest_discover_tests(${TEST})

  endforeach()

endif()