
# Use the same external JSON target that VTK uses. Adding lib/json here can
# shadow VTK's JSON headers and mix the nlohmann and vtknlohmann namespaces.
set(JSON_LIBRARY nlohmann_json::nlohmann_json)
if (NOT TARGET ${JSON_LIBRARY})
  find_package(nlohmann_json CONFIG REQUIRED)
endif()

# fmt always included in the build
if (NOT TARGET fmt)
  set(FMT_LIBRARY fmt)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/fmt)
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
