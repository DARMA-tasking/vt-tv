
find_package(
  VTK REQUIRED COMPONENTS
  RenderingCore
  IOExodus
  IOParallel
  CommonColor
  CommonCore
  CommonDataModel
  FiltersSources
  InteractionStyle
  RenderingFreeType
  RenderingOpenGL2
  RenderingGL2PSOpenGL2
)

message(STATUS "VTK libraries: ${VTK_LIBRARIES}")
