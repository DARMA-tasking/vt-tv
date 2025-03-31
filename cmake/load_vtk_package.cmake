
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
