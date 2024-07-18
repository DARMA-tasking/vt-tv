#!/usr/bin/env bash

set -ex

# Change display to X virtual frame buffer
export DISPLAY=:99.0
Xvfb :99 -screen 0 1024x768x24 -nolisten tcp > /dev/null 2>&1 &
sleep 1s

# Run tests
# > Unit tests
pushd /opt/build/vt-tv
mkdir -p /opt/src/vt-tv/output/
success_flag=0
ctest --output-junit Testing/Temporary/junit-report.xml --output-on-failure || success_flag=1
# We collect the test logs for exporting
echo "this is the Success flag: ${success_flag}"
mkdir -p /tmp/artifacts/
cp /opt/build/vt-tv/Testing/Temporary/LastTest.log /tmp/artifacts/
cp /opt/build/vt-tv/Testing/Temporary/junit-report.xml /tmp/artifacts/
echo ${success_flag} > /tmp/artifacts/success_flag.txt
ls /tmp/artifacts
# Simply output the LastTest.log to screen
cat /opt/build/vt-tv/Testing/Temporary/LastTest.log

# > Report coverage tests
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

popd
popd

# Clean and restore regular display
pkill Xvfb
rm -rf /tmp/.X11-unix/X99
export DISPLAY=:0