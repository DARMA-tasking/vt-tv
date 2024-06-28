#!/bin/bash

set -ex

# Paths
VT_TV_DIR="/path/to/vt-tv/source"
BUILD_DIR="/path/to/vt-tv/build"
VTK_DIR="/path/to/compiled/VTK"
JOBS="2"  # for instance, modify as needed

mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Remove CMakeCache for fresh build
# rm -rf CMakeCache.txt

cmake -B "${BUILD_DIR}" \
  -DVTK_DIR=${VTK_DIR} \
  -DPython_EXECUTABLE="$(which python)" \
  -DPython_INCLUDE_DIRS=$(python -c "import sysconfig; print(sysconfig.get_path('include'))") \
  "${VT_TV_DIR}"

time cmake --build . --parallel -j"${JOBS}"
