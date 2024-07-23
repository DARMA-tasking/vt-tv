#!/bin/bash

# This script builds vt-tv for CI including tests and coverage

set -ex

# Calls build script with CI configuration
# (Specific path, enable tests and coverage)
bash -c "VT_TV_BUILD_DIR=/opt/build/vt-tv \
    VTK_DIR=/opt/build/vtk \
    VT_TV_TESTS_ENABLED=ON \
    VT_TV_COVERAGE_ENABLED=ON \
    /opt/src/vt-tv/build.sh"

echo "VT-TV build success"