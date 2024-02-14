#!/bin/bash

set -ex

# Paths
VT-TV_DIR="/path/to/vt-tv/source"
BUILD_DIR="/path/to/vt-tv/build"
VTK_DIR="/path/to/compiled/VTK"
JOBS="2"  # for instance, modify as needed

mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Remove CMakeCache for fresh build
# rm -rf CMakeCache.txt

cmake -B "${BUILD_DIR}" \
  -DVTK_DIR="${VTK_DIR}" \
  "${VT-TV_DIR}"

time cmake --build . --parallel -j"${JOBS}"
