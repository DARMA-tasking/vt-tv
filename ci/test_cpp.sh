#!/bin/bash

set -ex

export DISPLAY=:99.0

# Start custom display with X virtual frame buffer
Xvfb :99 -screen 0 1024x768x24 -nolisten tcp > /dev/null 2>&1 &

sleep 1s

VT_TV_DIR="/opt/src/vt-tv"
VT_TV_OUTPUT_DIR="$VT_TV_DIR/output"
VT_TV_BUILD_DIR="/opt/build/vt-tv"
VT_TV_TEST_REPORT_PATH="$VT_TV_BUILD_DIR/Testing/Temporary/junit-report.xml"
VT_TV_ARTIFACTS_DIR="/tmp/artifacts"

pushd $VT_TV_BUILD_DIR
mkdir -p $VT_TV_OUTPUT_DIR

# run tests (allow failure but generate report to analyze failures later in CI)
"$VT_TV_BUILD_DIR/tests/unit/AllTests" --gtest_output="xml:$VT_TV_TEST_REPORT_PATH"  || true
mkdir -p $VT_TV_ARTIFACTS_DIR

# add artifact: unit tests report
cp "$VT_TV_TEST_REPORT_PATH" $VT_TV_ARTIFACTS_DIR/ || true

# add artifact: *.png generated images (by the ParseRenderTest class)
[ -d "$VT_TV_OUTPUT_DIR/tests" ] && cp "$VT_TV_OUTPUT_DIR/tests/"*".png"  $VT_TV_ARTIFACTS_DIR/

# coverage reporting
if [[ $VT_TV_COVERAGE_ENABLED == "ON" ]]; then
    pushd $VT_TV_OUTPUT_DIR
    lcov --capture --directory $VT_TV_BUILD_DIR --output-file lcov_vt-tv_test.info
    lcov --remove lcov_vt-tv_test.info -o lcov_vt-tv_test_no_deps.info '*/lib/*' '/usr/include/*' '*/vtk/*' '*/tests/*'
    lcov --list lcov_vt-tv_test_no_deps.info
    # add simple coverage artifact
    lcov --list lcov_vt-tv_test_no_deps.info > lcov-list-report.txt

    # add artifacts: coverage list
    cp lcov_vt-tv_test_no_deps.info $VT_TV_ARTIFACTS_DIR/
    cp lcov-list-report.txt $VT_TV_ARTIFACTS_DIR/

    # add artifacts: coverage total (percentage / lines)
    # might be useful for later badging
    LCOV_SUMMARY=$(lcov --summary lcov_vt-tv_test_no_deps.info)
    LCOV_TOTAL_LINES_COV=$(echo $LCOV_SUMMARY | grep -E -o 'lines......: ([0-9.]+)*' | grep -o -E '[0-9]+.[0-9]+')
    echo $LCOV_TOTAL_LINES_COV > lcov-lines-total.txt
    cp lcov-lines-total.txt $VT_TV_ARTIFACTS_DIR/

    ls $VT_TV_ARTIFACTS_DIR
    popd
fi

popd

# Clean and restore regular display
pkill Xvfb
rm -rf /tmp/.X11-unix/X99
export DISPLAY=:0
