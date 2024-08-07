
# json library always included in the build
if (NOT TARGET nlohmann_json)
  set(JSON_BuildTests OFF)
  set(JSON_MultipleHeaders ON)
  set(JSON_LIBRARY nlohmann_json)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/json)
endif()

# use included fmt or external one
if(VT_TV_EXTERNAL_FMT)
  # user should provide 'fmt_DIR' or 'fmt_ROOT' to CMake (unless fmt is installed in system libs)
  if(fmt_ROOT)
    message(STATUS "VT_TV_EXTERNAL_FMT=ON. Using fmt located at ${fmt_ROOT}")
  elseif(fmt_DIR)
    message(STATUS "VT_TV_EXTERNAL_FMT=ON. Using fmt located at ${fmt_DIR}")
  else()
    message(STATUS "VT_TV_EXTERNAL_FMT=ON but neither fmt_DIR nor fmt_ROOT is provided!")
  endif()
  find_package(fmt 7.1.0 REQUIRED)
  set(FMT_LIBRARY fmt)
else()
  if (NOT TARGET fmt)
    set(FMT_LIBRARY fmt)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/fmt)
  endif()
endif()

if (NOT TARGET brotli)
  # brotli library always included in the build
  set(BROTLI_DISABLE_TESTS ON)
  # we need to disable bundled mode so it will install properly
  set(BROTLI_BUNDLED_MODE OFF)
  set(BROTLI_BUILD_PORTABLE ON)
  set(BROTLI_LIBRARY brotlicommon-static brotlienc-static brotlidec-static)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/brotli)
endif()

set(YAML_LIBRARY yaml-cpp)
if(NOT TARGET ${YAML_LIBRARY})
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/yaml-cpp)
endif()

include(cmake/load_vtk_package.cmake)

if (VT_TV_PYTHON_BINDINGS_ENABLED)
  include(cmake/load_nanobind_package.cmake)
endif()

if (VT_TV_OPENMP_ENABLED)
  include(cmake/load_openmp.cmake)
endif()
