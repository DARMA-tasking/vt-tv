
# Source files for the Python bindings
file(
  GLOB
  PYTHON_BINDING_SRCS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
)

# Build the core parts of nanobind once
nanobind_build_library(nanobind SHARED)

# Create the Python bindings for the module
nanobind_add_module(vttv ${PYTHON_BINDING_SRCS} tv.cc)

# .. Link it to necessary libraries
target_link_libraries(vttv PUBLIC ${VT_TV_LIBRARY_NS} ${JSON_LIBRARY} ${FMT_LIBRARY})

vtk_module_autoinit(
  TARGETS vttv
  MODULES ${VTK_LIBRARIES}
)
