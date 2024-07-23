#!/bin/bash

set -ex

export DISPLAY=:99.0

# Start custom display with X virtual frame buffer
Xvfb :99 -screen 0 1024x768x24 -nolisten tcp > /dev/null 2>&1 &

sleep 1s

pushd /opt/build/vt-tv
mkdir -p /opt/src/vt-tv/output/

# run tests (allow failure but generate report to analyze failures later in CI)
ctest --output-junit Testing/Temporary/junit-report.xml --output-on-failure || true

# collect test logs for exporting
mkdir -p /tmp/artifacts/
cp /opt/build/vt-tv/Testing/Temporary/LastTest.log /tmp/artifacts/ || true
cp /opt/build/vt-tv/Testing/Temporary/junit-report.xml /tmp/artifacts/ || true
# output LastTest.log to screen
cat /opt/build/vt-tv/Testing/Temporary/LastTest.log

# output PNG images from the test_render tests
[ -f "/opt/src/vt-tv/output/test-render/test_vt_tv0.png" ] && cp "/opt/src/vt-tv/output/test-render/test_vt_tv0.png"  /tmp/artifacts/
[ -f "/opt/src/vt-tv/output/test-render/ccm_example0.png" ] && cp "/opt/src/vt-tv/output/test-render/ccm_example0.png"  /tmp/artifacts/

# coverage reporting
if [[ $VT_TV_COVERAGE_ENABLED=="ON" ]]; then
    pushd /opt/src/vt-tv/output
    lcov --capture --directory /opt/build/vt-tv --output-file lcov_vt-tv_test.info
    lcov --remove lcov_vt-tv_test.info -o lcov_vt-tv_test_no_deps.info '*/lib/*' '/usr/include/*' '*/vtk/*' '*/tests/*'
    lcov --list lcov_vt-tv_test_no_deps.info
    # add simple coverage artifact
    lcov --list lcov_vt-tv_test_no_deps.info > lcov-list-report.txt
    cp lcov_vt-tv_test_no_deps.info /tmp/artifacts/
    cp /opt/src/vt-tv/output/lcov-list-report.txt /tmp/artifacts/

    # extract total coverage (Lines) for later use in a badge in the CI
    LCOV_SUMMARY=$(lcov --summary lcov_vt-tv_test_no_deps.info)
    LCOV_TOTAL_LINES_COV=$(echo $LCOV_SUMMARY | grep -E -o 'lines......: ([0-9.]+)*' | grep -o -E '[0-9]+.[0-9]+')
    echo $LCOV_TOTAL_LINES_COV > lcov-lines-total.txt
    cp /opt/src/vt-tv/output/lcov-lines-total.txt /tmp/artifacts/

    ls /tmp/artifacts
    popd
fi

popd

# Clean and restore regular display
pkill Xvfb
rm -rf /tmp/.X11-unix/X99
export DISPLAY=:0
