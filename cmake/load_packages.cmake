
# json library always included in the build
set(JSON_BuildTests OFF)
set(JSON_MultipleHeaders ON)
set(JSON_LIBRARY nlohmann_json)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/json)

# fmt always included in the build
set(FMT_LIBRARY fmt)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/fmt)

include(cmake/load_vtk_package.cmake)
include(cmake/load_nanobind_package.cmake)
