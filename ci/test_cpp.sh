#!/bin/bash

set -ex

CURRENT_DIR="$(dirname -- "$(realpath -- "$0")")"
PARENT_DIR="$(dirname "$CURRENT_DIR")"

VTK_DIR=${VTK_DIR:-"/opt/build/vtk"}

VT_TV_SRC_DIR=${VT_TV_SRC_DIR:-$PARENT_DIR}
VT_TV_BUILD_DIR=${VT_TV_BUILD_DIR:-"/opt/build/vt-tv"}
VT_TV_OUTPUT_DIR=${VT_TV_OUTPUT_DIR:-"$VT_TV_SRC_DIR/output"}
VT_TV_TESTS_OUTPUT_DIR=${VT_TV_TESTS_OUTPUT_DIR:-"$VT_TV_OUTPUT_DIR/tests"}

# Start virtual display (Linux)
CURRENT_DISPLAY=$(echo $DISPLAY)
if [[ $(uname -a) != *"Darwin"* ]]; then
    $CURRENT_DIR/xvfb_start.sh :99
    echo "DISPLAY(test)=$DISPLAY"
fi

# Run tests
bash -c "VTK_DIR=/opt/build/vtk \
    VT_TV_BUILD=OFF \
    VT_TV_BUILD_DIR=${VT_TV_BUILD_DIR} \
    VT_TV_COVERAGE_ENABLED=${VT_TV_COVERAGE_ENABLED:-OFF} \
    VT_TV_OUTPUT_DIR=$VT_TV_OUTPUT_DIR \
    VT_TV_RUN_TESTS=ON \
    $VT_TV_SRC_DIR/build.sh"

# Restore display (Linux)
if [[ $(uname -a) != *"Darwin"* ]]; then
    $CURRENT_DIR/xvfb_stop.sh :99 $CURRENT_DISPLAY
fi

# Add artifacts
VT_TV_ARTIFACTS_DIR="/tmp/artifacts"
mkdir -p $VT_TV_ARTIFACTS_DIR

# > go to output directory
pushd $VT_TV_OUTPUT_DIR

# > add the unit tests report artifact
cp "junit-report.xml" $VT_TV_ARTIFACTS_DIR/ || true

# > add mesh files and png artifacts
if [ -d "$VT_TV_TESTS_OUTPUT_DIR" ]; then
    cp "$VT_TV_TESTS_OUTPUT_DIR/"*".vtp"  $VT_TV_ARTIFACTS_DIR/
    cp "$VT_TV_TESTS_OUTPUT_DIR/"*".png"  $VT_TV_ARTIFACTS_DIR/
fi

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
