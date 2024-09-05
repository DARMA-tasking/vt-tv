#!/bin/bash

# This script builds vtk

set -ex

VTK_OPENGL_HAS_OSMESA=OFF
VTK_USE_X=OFF
VTK_USE_COCOA=OFF
VTK_SRC_DIR=${VTK_SRC_DIR:-"/opt/src/vtk"}
VTK_DIR=${VTK_DIR:-"/opt/build/vtk"}

mkdir -p $VTK_DIR
pushd $VTK_DIR

# if [[ $(uname -a) != *"Darwin"* ]]; then
#   . /etc/lsb-release
#   # Issue with Ubuntu 24.04 / OSMESA : glew build error. Use X instead.
#   if [ "$DISTRIB_RELEASE" == "24.04" ]; then
#     echo "Disabling VTK_OPENGL_HAS_OSMESA (not supported in Ubuntu 24.04). Use VTK_USE_X instead."
#     VTK_USE_X=ON
#   else
#     VTK_OPENGL_HAS_OSMESA=ON
#   fi
# else
#   VTK_USE_COCOA=ON
# fi

cmake \
  -DCMAKE_BUILD_TYPE:STRING=Release \
  -DBUILD_TESTING:BOOL=OFF \
  -DBUILD_SHARED_LIBS:BOOL=ON \
  -S "$VTK_SRC_DIR" -B "$VTK_DIR"
cmake --build "$VTK_DIR" -j$(nproc)

# -DVTK_OPENGL_HAS_OSMESA:BOOL=$VTK_OPENGL_HAS_OSMESA \
# -DVTK_DEFAULT_RENDER_WINDOW_OFFSCREEN:BOOL=ON \
# -DVTK_USE_X:BOOL=$VTK_USE_X \
# -DVTK_USE_WIN32_OPENGL:BOOL=OFF \
# -DVTK_USE_COCOA:BOOL=$VTK_USE_COCOA \
# -DVTK_USE_SDL2:BOOL=OFF \
# -DVTK_Group_Rendering:BOOL=OFF \

echo "VTK build success"

popd
