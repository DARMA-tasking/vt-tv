cmake_minimum_required(VERSION 3.17 FATAL_ERROR)
project(vt-tv)

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

# add -fPIC to all targets (if building with nanobind)
if(VT_TV_PYTHON_BINDINGS_ENABLED)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

option(VT_TV_WERROR_ENABLED "Build vt-tv with warnings as errors" OFF)

set(VT_TV_N_THREADS "2" CACHE STRING "Number of OpenMP threads to use")

include(cmake/load_packages.cmake)

if(APPLE)
  add_compile_options(-ffat-lto-objects)
endif()

add_definitions(-DVT_TV_N_THREADS=${VT_TV_N_THREADS})
add_definitions(-DVT_TV_OPENMP_ENABLED=${VT_TV_OPENMP_ENABLED})
add_compile_definitions(SRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

add_custom_target(vt_tv_examples)
add_custom_target(vt_tv_tests)
add_custom_target(vt_tv_apps)

set(PROJECT_BIN_DIR      ${CMAKE_CURRENT_BINARY_DIR})
set(PROJECT_BASE_DIR     ${CMAKE_CURRENT_SOURCE_DIR})
set(PROJECT_LIB_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(PROJECT_EXAMPLE_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/examples)
set(PROJECT_APP_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/apps)
set(PROJECT_BINDINGS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bindings)

include(CTest)

add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(tests)
add_subdirectory(apps)
add_subdirectory(bindings)

configure_file(
  cmake/vtTVConfig.cmake.in
  "${PROJECT_BINARY_DIR}/vtTVConfig.cmake" @ONLY
)

# configure_file(
#   ${PROJECT_BASE_DIR}/cmake/cmake_config.h.in
#   ${PROJECT_BASE_DIR}/tests/unit/cmake_config.h @ONLY
# )

install(
  FILES        "${PROJECT_BINARY_DIR}/vtTVConfig.cmake"
  DESTINATION  cmake
  COMPONENT    lib
)
