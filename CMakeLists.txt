cmake_minimum_required(VERSION 3.17 FATAL_ERROR)
project(vt-dwv)

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

set(VT_DWV_LIBRARY vt-dwv CACHE INTERNAL "" FORCE)
set(VT_DWV_LIBRARY_NS vt::lib::vt-dwv)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17 CACHE STRING "The C++ standard to use")
endif()
message(STATUS "CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")

include(cmake/load_packages.cmake)

add_custom_target(vt_dwv_examples)
add_custom_target(vt_dwv_tests)

set(PROJECT_BIN_DIR     ${CMAKE_CURRENT_BINARY_DIR})
set(PROJECT_BASE_DIR    ${CMAKE_CURRENT_SOURCE_DIR})
set(PROJECT_LIB_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(PROJECT_EXAMPLE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/examples)

include(CTest)

add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(tests)

configure_file(
  cmake/vtDWVConfig.cmake.in
  "${PROJECT_BINARY_DIR}/vtDWVConfig.cmake" @ONLY
)

install(
  FILES        "${PROJECT_BINARY_DIR}/vtDWVConfig.cmake"
  DESTINATION  cmake
  COMPONENT    lib
)

