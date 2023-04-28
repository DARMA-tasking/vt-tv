
# json library always included in the build
set(JSON_BuildTests OFF)
set(JSON_MultipleHeaders ON)
set(JSON_LIBRARY nlohmann_json)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/json)

# fmt always included in the build
set(FMT_LIBRARY fmt)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/fmt)

# brotli library always included in the build
set(BROTLI_DISABLE_TESTS ON)
# we need to disable bundled mode so it will install properly
set(BROTLI_BUNDLED_MODE OFF)
set(BROTLI_BUILD_PORTABLE ON)
set(BROTLI_LIBRARY brotlicommon-static brotlienc-static brotlidec-static)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/brotli)


include(cmake/load_vtk_package.cmake)
include(cmake/load_nanobind_package.cmake)
