
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
  InteractionStyle
  RenderingFreeType
  RenderingOpenGL2
  RenderingGL2PSOpenGL2
  RenderingAnnotation
)

message(STATUS "VTK libraries: ${VTK_LIBRARIES}")
