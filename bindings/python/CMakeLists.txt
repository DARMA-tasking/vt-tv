
# Source files for the Python bindings
file(
  GLOB
  PYTHON_BINDING_SRCS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
)

# # List of modules to bind
# set(VT_TV_PYTHON_MODULES
#   tv_from_json
# )

# Build the core parts of nanobind once
nanobind_build_library(nanobind SHARED)

# foreach(module ${VT_TV_PYTHON_MODULES})
#   # Create the Python bindings for the module
#   nanobind_add_module(${module} ${PYTHON_BINDING_SRCS})

#   # .. Link it to necessary libraries
#   target_link_libraries(${module} PUBLIC ${VT_TV_LIBRARY_NS} ${JSON_LIBRARY} ${FMT_LIBRARY})

# endforeach()

# Create the Python bindings for the module
nanobind_add_module(vttv ${PYTHON_BINDING_SRCS} tv.cc)

# .. Link it to necessary libraries
target_link_libraries(vttv PUBLIC ${VT_TV_LIBRARY_NS} ${JSON_LIBRARY} ${FMT_LIBRARY})

vtk_module_autoinit(
  TARGETS vttv
  MODULES ${VTK_LIBRARIES}
)
