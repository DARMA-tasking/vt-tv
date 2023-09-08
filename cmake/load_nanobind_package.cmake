find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)

if(NOT (${Python_VERSION_MAJOR} EQUAL 3 AND 
       (${Python_VERSION_MINOR} EQUAL 8 OR ${Python_VERSION_MINOR} EQUAL 9)))
    message(FATAL_ERROR "Python version must be 3.8 or 3.9")
endif()


# Detect the installed nanobind package and import it into CMake
message(STATUS "Python executable: ${Python_EXECUTABLE}")
execute_process(
  COMMAND ${Python_EXECUTABLE} -m nanobind --cmake_dir
  OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE NANOBIND_DIR)
list(APPEND CMAKE_PREFIX_PATH "${NANOBIND_DIR}")
find_package(nanobind CONFIG REQUIRED)
