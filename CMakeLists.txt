cmake_minimum_required(VERSION 3.17 FATAL_ERROR)
project(vt-tv VERSION 1.6.0)

# Try to find ccache to speed up compilation
find_program(ccache_binary ccache)
if (ccache_binary)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${ccache_binary}")
endif()

file(
  GLOB
  HEADER_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)

set(VT_TV_LIBRARY vt-tv CACHE INTERNAL "" FORCE)
set(VT_TV_LIBRARY_NS vt::lib::vt-tv)

set(
  CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/"
)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17 CACHE STRING "The C++ standard to use")
endif()
message(STATUS "CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")

option(VT_TV_PYTHON_BINDINGS_ENABLED "Build vt-tv with Python bindings" OFF)
option(VT_TV_OPENMP_ENABLED "Build vt-tv with openMP support" ON)
option(VT_TV_TESTS_ENABLED "Build vt-tv with unit tests" ON)
option(VT_TV_COVERAGE_ENABLED "Build vt-tv with coverage" OFF)
set(VT_TV_N_THREADS "2" CACHE STRING "Number of OpenMP threads to use")

# add -fPIC to all targets (if building with nanobind)
if(VT_TV_PYTHON_BINDINGS_ENABLED)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

option(VT_TV_WERROR_ENABLED "Build vt-tv with warnings as errors" OFF)

include(cmake/load_packages.cmake)

if(APPLE AND NOT CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
  add_compile_options(-ffat-lto-objects)
endif()

set(PROJECT_BIN_DIR      ${CMAKE_CURRENT_BINARY_DIR})
set(PROJECT_BASE_DIR     ${CMAKE_CURRENT_SOURCE_DIR})
set(PROJECT_LIB_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(PROJECT_EXAMPLE_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/examples)
set(PROJECT_APP_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/apps)
set(PROJECT_BINDINGS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bindings)

function(add_vttv_definitions TARGET)
  target_compile_definitions(${TARGET}
    PRIVATE
      VT_TV_N_THREADS=${VT_TV_N_THREADS}
      SRC_DIR="${PROJECT_BASE_DIR}"
      BUILD_DIR="${CMAKE_BINARY_DIR}"
  )

  if(VT_TV_OPENMP_ENABLED)
    target_compile_definitions(${TARGET} PRIVATE VT_TV_OPENMP_ENABLED=1)
  else()
    target_compile_definitions(${TARGET} PRIVATE VT_TV_OPENMP_ENABLED=0)
  endif()
endfunction(add_vttv_definitions TARGET)

add_subdirectory(src)
link_openmp()
add_subdirectory(examples)

add_subdirectory(apps)
add_subdirectory(bindings)

include(CTest) #adds option BUILD_TESTING (default ON)

message(STATUS "VT_TV_COVERAGE_ENABLED: ${VT_TV_COVERAGE_ENABLED}")
if (VT_TV_COVERAGE_ENABLED)
  add_compile_options(-fprofile-arcs -ftest-coverage -O0)
  add_link_options(--coverage)
endif()

message(STATUS "VT_TV_TESTS_ENABLED: ${VT_TV_TESTS_ENABLED}")
if (BUILD_TESTING AND VT_TV_TESTS_ENABLED)
  set(CTEST_SOURCE_DIRECTORY ${SOURCE_DIR}/src)
  add_subdirectory(tests)
endif()

configure_file(
  cmake/vtTVConfig.cmake.in
  "${PROJECT_BINARY_DIR}/vtTVConfig.cmake" @ONLY
)

install(
  FILES        "${PROJECT_BINARY_DIR}/vtTVConfig.cmake"
  DESTINATION  cmake
  COMPONENT    lib
)
