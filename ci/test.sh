#!/usr/bin/env bash

set -e

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

# coverage reporting
pushd /opt/src/vt-tv/output
lcov --capture --directory /opt/build/vt-tv --output-file lcov_vt-tv_test.info
lcov --remove lcov_vt-tv_test.info -o lcov_vt-tv_test_no_deps.info '*/lib/*' '/usr/include/*' '*/vtk/*' '*/tests/*'
lcov --list lcov_vt-tv_test_no_deps.info
# add simple coverage artifact
lcov --list lcov_vt-tv_test_no_deps.info > lcov-list-report.txt
cp lcov_vt-tv_test_no_deps.info /tmp/artifacts/
cp /opt/src/vt-tv/output/lcov-list-report.txt /tmp/artifacts/
popd

popd
