
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
