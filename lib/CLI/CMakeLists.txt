add_library(CLI INTERFACE)
target_include_directories(CLI INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/CLI)

set_target_properties(CLI PROPERTIES OUTPUT_NAME CLI-vt-tv)

install(
  DIRECTORY                CLI
  DESTINATION              include
  CONFIGURATIONS           ${build_type_list}
  FILES_MATCHING PATTERN   "*"
)
