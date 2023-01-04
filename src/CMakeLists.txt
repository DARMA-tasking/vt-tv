set(
  TOP_LEVEL_SUBDIRS
  api
)

set(VT_TV_INSTALL_DESTINATION "include/vt-tv")
set(VT_TV_EXTERNAL_DESTINATION "include")

foreach(DIR ${TOP_LEVEL_SUBDIRS})
  install(
    DIRECTORY "vt-tv/${DIR}"
    DESTINATION ${VT_TV_INSTALL_DESTINATION}
    FILES_MATCHING PATTERN "*.h"
  )
endforeach()

file(GLOB TOP_HEADERS "vt-tv/*.h")

install(
  FILES ${TOP_HEADERS}
  DESTINATION ${VT_TV_INSTALL_DESTINATION}
)

file(
  GLOB
  HEADER_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/vt-tv/*.h
)

file(
  GLOB
  SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/vt-tv/*.cc
)

foreach(SUB_DIR ${TOP_LEVEL_SUBDIRS})
  file(
    GLOB
    "${SUB_DIR}_HEADER_FILES"
    RELATIVE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/vt-tv/${SUB_DIR}/*.h"
  )

  file(
    GLOB
    "${SUB_DIR}_SOURCE_FILES"
    RELATIVE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/vt-tv/${SUB_DIR}/*.cc"
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
  ${VT_TV_LIBRARY}
  STATIC
  ${HEADER_FILES} ${SOURCE_FILES}
)

add_library(${VT_TV_LIBRARY_NS} ALIAS ${VT_TV_LIBRARY})

target_link_libraries(
  ${VT_TV_LIBRARY} PUBLIC ${VTK_LIBRARIES}
)

target_include_directories(
  ${VT_TV_LIBRARY} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  $<INSTALL_INTERFACE:include>
)

target_include_directories(
  ${VT_TV_LIBRARY} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)

install(
  TARGETS                   ${VT_TV_LIBRARY}
  EXPORT                    ${VT_TV_LIBRARY}
  LIBRARY DESTINATION       lib
  ARCHIVE DESTINATION       lib
  RUNTIME DESTINATION       bin
  INCLUDES DESTINATION      ${VT_TV_EXTERNAL_DESTINATION}
)

install(
  EXPORT                    ${VT_TV_LIBRARY}
  DESTINATION               cmake
  FILE                      "vtTVTargets.cmake"
  NAMESPACE                 vt::lib::
  COMPONENT                 runtime
)

export(
  TARGETS                   ${VT_TV_LIBRARY}
  FILE                      "vtTVTargets.cmake"
  NAMESPACE                 vt::lib::
)
