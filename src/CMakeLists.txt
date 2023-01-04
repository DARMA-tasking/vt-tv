set(
  TOP_LEVEL_SUBDIRS
  api
)

set(VT_DWV_INSTALL_DESTINATION "include/vt-dwv")
set(VT_DWV_EXTERNAL_DESTINATION "include")

foreach(DIR ${TOP_LEVEL_SUBDIRS})
  install(
    DIRECTORY "vt-dwv/${DIR}"
    DESTINATION ${VT_DWV_INSTALL_DESTINATION}
    FILES_MATCHING PATTERN "*.h"
  )
endforeach()

file(GLOB TOP_HEADERS "vt-dwv/*.h")

install(
  FILES ${TOP_HEADERS}
  DESTINATION ${VT_DWV_INSTALL_DESTINATION}
)

file(
  GLOB
  HEADER_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/vt-dwv/*.h
)

file(
  GLOB
  SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/vt-dwv/*.cc
)

foreach(SUB_DIR ${TOP_LEVEL_SUBDIRS})
  file(
    GLOB
    "${SUB_DIR}_HEADER_FILES"
    RELATIVE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/vt-dwv/${SUB_DIR}/*.h"
  )

  file(
    GLOB
    "${SUB_DIR}_SOURCE_FILES"
    RELATIVE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/vt-dwv/${SUB_DIR}/*.cc"
  )

  #message("headers=${${SUB_DIR}_HEADER_FILES}")
  #message("source=${${SUB_DIR}_SOURCE_FILES}")

  list(
    APPEND
    HEADER_FILES
    "${${SUB_DIR}_HEADER_FILES}"
  )

  list(
    APPEND
    SOURCE_FILES
    "${${SUB_DIR}_SOURCE_FILES}"
  )
endforeach()

add_library(
  ${VT_DWV_LIBRARY}
  STATIC
  ${HEADER_FILES} ${SOURCE_FILES}
)

add_library(${VT_DWV_LIBRARY_NS} ALIAS ${VT_DWV_LIBRARY})

target_include_directories(
  ${VT_DWV_LIBRARY} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  $<INSTALL_INTERFACE:include>
)

target_include_directories(
  ${VT_DWV_LIBRARY} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)

install(
  TARGETS                   ${VT_DWV_LIBRARY}
  EXPORT                    ${VT_DWV_LIBRARY}
  LIBRARY DESTINATION       lib
  ARCHIVE DESTINATION       lib
  RUNTIME DESTINATION       bin
  INCLUDES DESTINATION      ${VT_DWV_EXTERNAL_DESTINATION}
)

install(
  EXPORT                    ${VT_DWV_LIBRARY}
  DESTINATION               cmake
  FILE                      "vtDWVTargets.cmake"
  NAMESPACE                 vt::lib::
  COMPONENT                 runtime
)

export(
  TARGETS                   ${VT_DWV_LIBRARY}
  FILE                      "vtDWVTargets.cmake"
  NAMESPACE                 vt::lib::
)
