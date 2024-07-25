#!/bin/bash

set -ex

export DISPLAY=:99.0

# Start custom display with X virtual frame buffer
Xvfb :99 -screen 0 1024x768x24 -nolisten tcp > /dev/null 2>&1 &
sleep 1s

VT_TV_OUTPUT_DIR=/opt/build/vtk/output

# call build script with options to only run tests without building
# (in CI the docker image define the build and the test stages separately).
bash -c "VTK_DIR=/opt/build/vtk \
    VT_TV_BUILD=OFF \
    VT_TV_BUILD_DIR=/opt/build/vt-tv \
    VT_TV_COVERAGE_ENABLED=${VT_TV_COVERAGE_ENABLED:-OFF} \
    VT_TV_OUTPUT_DIR=$VT_TV_OUTPUT_DIR \
    VT_TV_RUN_TESTS=ON \
    /opt/src/vt-tv/build.sh"

# Add artifacts

VT_TV_ARTIFACTS_DIR="/tmp/artifacts"
mkdir -p $VT_TV_ARTIFACTS_DIR

# > go to output directory
pushd $VT_TV_OUTPUT_DIR

# > add the unit tests report artifact
cp "junit-report.xml" $VT_TV_ARTIFACTS_DIR/ || true

# > add some generated images artifacts
[ -d "tests" ] && cp "tests/"*".png"  $VT_TV_ARTIFACTS_DIR/

if [[ $VT_TV_COVERAGE_ENABLED == "ON" ]]; then
    # > add `coverage --list` file artifact
    lcov --list lcov_vt-tv_test_no_deps.info > $VT_TV_ARTIFACTS_DIR/lcov-list-report.txt

    # > add total lines coverage file artifact (percentage of lines covered)
    # might be useful for generating later a badge in ci
    LCOV_SUMMARY=$(lcov --summary lcov_vt-tv_test_no_deps.info)
    LCOV_TOTAL_LINES_COV=$(echo $LCOV_SUMMARY | grep -E -o 'lines......: ([0-9.]+)*' | grep -o -E '[0-9]+.[0-9]+')
    echo $LCOV_TOTAL_LINES_COV > lcov-lines-total.txt
    cp lcov-lines-total.txt $VT_TV_ARTIFACTS_DIR/
fi

popd

# list artifacts dir content
ls $VT_TV_ARTIFACTS_DIR

# Clean and restore regular display
pkill Xvfb
rm -rf /tmp/.X11-unix/X99
export DISPLAY=:0
