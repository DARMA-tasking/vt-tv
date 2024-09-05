#!/bin/bash

# This script builds VTK (required by vt-tv)

set -ex

# VTK_OPENGL_HAS_OSMESA=OFF
# VTK_USE_X=OFF
# VTK_USE_COCOA=OFF
VTK_SRC_DIR=${VTK_SRC_DIR:-"/opt/src/vtk"}
VTK_DIR=${VTK_DIR:-"/opt/build/vtk"}

mkdir -p $VTK_DIR
pushd $VTK_DIR

cmake \
  -DCMAKE_BUILD_TYPE:STRING=Release \
  -DBUILD_TESTING:BOOL=OFF \
  -DBUILD_SHARED_LIBS:BOOL=ON \
  -S "$VTK_SRC_DIR" -B "$VTK_DIR"
cmake --build "$VTK_DIR" -j$(nproc)

echo "VTK build success"

popd
