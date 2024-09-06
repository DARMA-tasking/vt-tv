#!/bin/bash

# This script builds VTK (required by vt-tv)

set -ex

VTK_SRC_DIR=${VTK_SRC_DIR:-"/opt/src/vtk"}
VTK_DIR=${VTK_DIR:-"/opt/build/vtk"}
VTK_VERSION=${VTK_VERSION:-"9.3.1"}

echo "Setup VTK $VTK_VERSION from source..."
git clone --recursive --branch v${VTK_VERSION} https://gitlab.kitware.com/vtk/vtk.git /opt/src/vtk

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

echo "VTK $VTK_VERSION has been installed successfully."
