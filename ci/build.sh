#!/bin/bash

set -ex

cmake -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=gcc-11 \
    -DCMAKE_CXX_COMPILER=g++-11 \
    -DCMAKE_CXX_FLAGS="-Werror" \
    -DVTK_DIR=/opt/build/vtk-build \
    -S /opt/src/vt-tv -B /opt/build/vt-tv

time cmake --build /opt/build/vt-tv --parallel $(nproc)
