#!/usr/bin/env bash

set -e

pushd /opt/build/vt-tv
success_flag=0
ctest --output-on-failure || success_flag=1
# We collect the test logs for exporting
echo "this is the Success flag: ${success_flag}"
mkdir -p /opt/src/vt-tv/output/
mkdir -p /tmp/artifacts/
cp /opt/build/vt-tv/Testing/Temporary/LastTest.log /tmp/artifacts/
echo ${success_flag} > /tmp/artifacts/success_flag.txt
ls /tmp/artifacts
# Simply output the LastTest.log to screen
cat /opt/build/vt-tv/Testing/Temporary/LastTest.log
popd
