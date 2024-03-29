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

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17 CACHE STRING "The C++ standard to use")
endif()
message(STATUS "CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")

option(vt_tv_python_bindings_enabled "Build vt-tv with Python bindings" ON)
option(vt_tv_openmp_enabled "Build vt-tv with openMP support" ON)

include(cmake/load_packages.cmake)

if(APPLE)
  add_compile_options(-ffat-lto-objects)
endif()

if(openmp_enabled)
  set(VT_TV_NUM_THREADS "2" CACHE STRING "Number of threads to use")
  add_definitions(-DVT_TV_NUM_THREADS=${VT_TV_NUM_THREADS})
endif()

add_custom_target(vt_tv_examples)
add_custom_target(vt_tv_tests)
add_custom_target(vt_tv_apps)

set(PROJECT_BIN_DIR     ${CMAKE_CURRENT_BINARY_DIR})
set(PROJECT_BASE_DIR    ${CMAKE_CURRENT_SOURCE_DIR})
set(PROJECT_LIB_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(PROJECT_EXAMPLE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/examples)
set(PROJECT_APP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/apps)

include(CTest)

add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(tests)
add_subdirectory(apps)

configure_file(
  cmake/vtTVConfig.cmake.in
  "${PROJECT_BINARY_DIR}/vtTVConfig.cmake" @ONLY
)

configure_file(
  ${PROJECT_BASE_DIR}/cmake/cmake_config.h.in
  ${PROJECT_BASE_DIR}/tests/unit/cmake_config.h @ONLY
)

install(
  FILES        "${PROJECT_BINARY_DIR}/vtTVConfig.cmake"
  DESTINATION  cmake
  COMPONENT    lib
)

if (vt_tv_python_bindings_enabled)
  # Build the core parts of nanobind once
  nanobind_build_library(nanobind SHARED)

  # Compile an extension library
  add_library(my_ext MODULE ${CMAKE_CURRENT_SOURCE_DIR}/circle.cc)

  # .. and link it against the nanobind parts
  message(STATUS "vtk libraries: ${VTK_LIBRARIES}")
  target_link_libraries(my_ext PUBLIC ${VTK_LIBRARIES})
  target_link_libraries(my_ext PRIVATE nanobind)

  # .. enable size optimizations
  nanobind_opt_size(my_ext)

  # .. enable link time optimization
  nanobind_lto(my_ext)

  # .. disable the stack protector
  nanobind_disable_stack_protector(my_ext)

  # .. set the Python extension suffix
  nanobind_extension(my_ext)

  # .. set important compilation flags
  nanobind_compile_options(my_ext)

  # .. set important linker flags
  nanobind_link_options(my_ext)
endif()
