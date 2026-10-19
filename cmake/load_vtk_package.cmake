
find_package(
  VTK REQUIRED COMPONENTS
  RenderingCore
  IOExodus
  IOParallel
  IOXML
  CommonColor
  CommonCore
  CommonDataModel
  FiltersSources
  FiltersGeneral
  InteractionStyle
  RenderingFreeType
  RenderingOpenGL2
  RenderingGL2PSOpenGL2
  RenderingAnnotation
)

message(STATUS "VTK version: ${VTK_VERSION}")

# Since VTK 9.4, JSON appears in VTK's public headers. Its bundled copy uses a
# different namespace and cannot safely share a translation unit with ours.
if (VTK_VERSION VERSION_GREATER_EQUAL "9.4")
  include(CheckCXXSourceCompiles)
  include(CMakePushCheckState)
  cmake_push_check_state(RESET)
  set(CMAKE_REQUIRED_LIBRARIES VTK::nlohmannjson)
  # Recheck when a different VTK build is selected in an existing build tree.
  unset(VT_TV_VTK_USES_EXTERNAL_JSON CACHE)
  check_cxx_source_compiles("
    #include <vtk_nlohmannjson.h>
    #if !VTK_MODULE_USE_EXTERNAL_vtknlohmannjson
    #error VTK must use external nlohmann_json
    #endif
    #include <nlohmann/json.hpp>
    int main() { return 0; }
  " VT_TV_VTK_USES_EXTERNAL_JSON)
  cmake_pop_check_state()
  if (NOT VT_TV_VTK_USES_EXTERNAL_JSON)
    message(FATAL_ERROR
      "VTK must use the same external nlohmann_json package as vt-tv. "
      "Rebuild VTK with -DVTK_MODULE_USE_EXTERNAL_VTK_nlohmannjson=ON "
      "and select the same package with nlohmann_json_DIR or CMAKE_PREFIX_PATH.")
  endif()
endif()

message(STATUS "VTK libraries: ${VTK_LIBRARIES}")

foreach(VTK_LIBRARY ${VTK_LIBRARIES})
  get_target_property(VTK_RUNTIME_LIBRARY_DIR ${VTK_LIBRARY} LOCATION)

  list(
    APPEND
    VTK_RUNTIME_LIBRARY_DIRS
    "${VTK_RUNTIME_LIBRARY_DIR}"
  )
endforeach()

if(NOT VTK_RUNTIME_LIBRARY_DIRS)
  message(FATAL_ERROR "VTK runtime library paths could not be determined!")
endif()

install(
    FILES ${VTK_RUNTIME_LIBRARY_DIRS}
    DESTINATION lib
)
