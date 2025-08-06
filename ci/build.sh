#!/bin/bash

# This script builds vt-tv for CI including tests and coverage

set -ex

CURRENT_DIR="$(dirname -- "$(realpath -- "$0")")"
PARENT_DIR="$(dirname "$CURRENT_DIR")"
VTK_DIR=${VTK_DIR:-"/opt/build/vtk"}

# Calls build script with CI configuration
# (Specific path, enable tests and coverage, warnings as errors)
bash -c "VT_TV_BUILD_DIR=/opt/build/vt-tv \
    VTK_DIR=${VTK_DIR} \
    VT_TV_TESTS_ENABLED=ON \
    VT_TV_COVERAGE_ENABLED=${VT_TV_COVERAGE_ENABLED:-OFF} \
    VT_TV_WERROR_ENABLED=ON \
    VT_TV_INSTALL=ON \
    ${PARENT_DIR}/build.sh"

echo "VT-TV build success"
