set(
  TOP_LEVEL_SUBDIRS
  api
  render
  utility
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

include(turn_on_warnings)
turn_on_warnings(${VT_TV_LIBRARY})

target_link_libraries(
  ${VT_TV_LIBRARY} PUBLIC ${VTK_LIBRARIES}
)

target_link_libraries(
  ${VT_TV_LIBRARY} PUBLIC ${JSON_LIBRARY}
)

if(${VT_TV_EXTERNAL_FMT})
  target_link_libraries(
    ${VT_TV_LIBRARY} PUBLIC fmt::fmt
  )
else()
  target_link_libraries(
    ${VT_TV_LIBRARY} PUBLIC ${FMT_LIBRARY}
  )
endif()

target_link_libraries(
  ${VT_TV_LIBRARY} PUBLIC ${BROTLI_LIBRARY}
)

target_link_libraries(
  ${VT_TV_LIBRARY} PUBLIC ${YAML_LIBRARY}
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

set(LIBRARIES_TO_INSTALL ${JSON_LIBRARY} ${BROTLI_LIBRARY} ${YAML_LIBRARY})

if(NOT VT_TV_EXTERNAL_FMT)
  list(APPEND LIBRARIES_TO_INSTALL ${FMT_LIBRARY})
endif()

foreach(LIBRARY ${LIBRARIES_TO_INSTALL})
  install(TARGETS ${LIBRARY} EXPORT ${VT_TV_LIBRARY})
endforeach()

export(
  TARGETS ${VT_TV_LIBRARY} ${LIBRARIES_TO_INSTALL}
  FILE "vtTVTargets.cmake"
  NAMESPACE vt::lib::
)
