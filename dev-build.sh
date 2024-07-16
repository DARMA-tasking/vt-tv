#!/bin/bash

usage() { echo "Usage: $0 [-c clean output directory]" 1>&2; exit 1; }

# Paths

PROJECT_DIR="$(dirname -- "$(realpath -- "$0")")"
REPOSITORIES_DIR="$(dirname "$PROJECT_DIR")"

# consider by default that all projects resides in a same directory
# (like "repositories" or "dev") as it is a common case. Change it if needed.
VT_TV_DIR=$REPOSITORIES_DIR/vt-tv
BUILD_DIR=$REPOSITORIES_DIR/vt-tv/build
VTK_DIR=$REPOSITORIES_DIR/vtk/build
JOBS=10  # for instance, modify as needed

while getopts ":ch" opt; do
 case $opt in
  c)
    # Remove CMakeCache for fresh build
    rm -rf CMakeCache.txt
    rm -rf ${BUILD_DIR} # recreate clean and build
    ;;
  h)
    usage
  ;;
 esac
done

set -ex

mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

# export CC=/usr/bin/clang
# export CXX=/usr/bin/clang++

# -DCMAKE_C_COMPILER="$(which clang)" \
# -DCMAKE_CXX_COMPILER=$(which clang++) \
# -DCMAKE_CXX_FLAGS_INIT="-std=c++17" \

# -DCMAKE_C_COMPILER="$(which clang)" \
# -DCMAKE_CXX_COMPILER=$(which clang++) \

cmake -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE=RELEASE \
  -DVTK_DIR=${VTK_DIR} \
  \
  -DBUILD_TESTING=ON \
  -DVT_TV_TESTS_ENABLED=ON \
  -DVT_TV_COVERAGE_ENABLED=ON \
  \
  -DVT_TV_TESTS_ALL_IN_ONE=OFF \
  -DVT_TV_PYTHON_BINDINGS_ENABLED=OFF \
  \
  -DPython_EXECUTABLE="$(which python)" \
  -DPython_INCLUDE_DIRS=$(python -c "import sysconfig; print(sysconfig.get_path('include'))") \
  \
  "${VT_TV_DIR}"

time cmake --build . --parallel -j"${JOBS}"

# Tests and coverage
cd ${PROJECT_DIR}

ctest --test-dir build -T Coverage -T Test || true 
#  || true to catch some strange error:
# Error(s) while accumulating results:
#   Problem reading source file: /home/thomas/repositories/vt-tv/lib/yaml-cpp/include/yaml-cpp/node/detail/impl.h line:235  out total: 384
#   Looks like there are more lines in the file: /home/thomas/repositories/vt-tv/lib/yaml-cpp/include/yaml-cpp/node/detail/node.h

lcov --capture --directory build --output-file output/lcov_vt-tv_test.info
lcov --remove output/lcov_vt-tv_test.info -o output/lcov_vt-tv_test_no_deps.info '*/lib/*' '/usr/include/*' '*/vtk/*' '*/tests/*'
genhtml --prefix ./src --ignore-errors source ./output/lcov_vt-tv_test_no_deps.info --legend --title "$(git rev-parse HEAD)" --output-directory=output/lcov_vt-tv_html
