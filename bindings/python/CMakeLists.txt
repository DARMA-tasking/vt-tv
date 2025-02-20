
# Source files for the Python bindings
file(
  GLOB
  PYTHON_BINDING_SRCS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
)

# Build the core parts of nanobind once
nanobind_build_library(nanobind SHARED)

# Create the Python bindings for the module
# small workaround here, use pyvttv target name to avoid conflict with binary executable
# target, called vttv as well; we want the python package to be called vttv too,
# thus the set_target_properties on OUTPUT_NAME.
nanobind_add_module(pyvttv ${PYTHON_BINDING_SRCS} tv.cc)
set_target_properties(${PYTHON_MODULE_TARGET} PROPERTIES OUTPUT_NAME "vttv")

# .. Link it to necessary libraries
target_link_libraries(pyvttv PUBLIC ${VT_TV_LIBRARY_NS} ${JSON_LIBRARY} ${FMT_LIBRARY})

vtk_module_autoinit(
  TARGETS pyvttv
  MODULES ${VTK_LIBRARIES}
)
