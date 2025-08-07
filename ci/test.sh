#!/bin/bash

set -ex

CURRENT_DIR="$(dirname -- "$(realpath -- "$0")")"
PARENT_DIR="$(dirname "$CURRENT_DIR")"

VTK_DIR=${VTK_DIR:-"/opt/build/vtk"}

VT_TV_SRC_DIR=${VT_TV_SRC_DIR:-$PARENT_DIR}
VT_TV_BUILD_DIR=${VT_TV_BUILD_DIR:-"/opt/build/vt-tv"}
VT_TV_OUTPUT_DIR=${VT_TV_OUTPUT_DIR:-"$VT_TV_SRC_DIR/output"}
VT_TV_TESTS_OUTPUT_DIR=${VT_TV_TESTS_OUTPUT_DIR:-"$VT_TV_OUTPUT_DIR/tests"}

# Active conda env that contains needed python packages
source /opt/conda/etc/profile.d/conda.sh
conda activate py3.12

VT_TV_TEST_CMD="\
    pwd
    echo ------------------------------
    echo $CURRENT_DIR
    echo ------------------------------
    ls -ltra ci
    chmod +x $CURRENT_DIR/build.sh
    VTK_DIR=${VTK_DIR} \
    VT_TV_BUILD=ON \
    VT_TV_BUILD_DIR=${VT_TV_BUILD_DIR} \
    VT_TV_COVERAGE_ENABLED=${VT_TV_COVERAGE_ENABLED:-OFF} \
    VT_TV_OUTPUT_DIR=$VT_TV_OUTPUT_DIR \
    VT_TV_RUN_TESTS=ON \
    $CURRENT_DIR/build.sh"

# Run tests
if [[ $(uname -a) != *"Darwin"* ]]; then
    xvfb-run bash -c "$VT_TV_TEST_CMD"
else
    bash -c "$VT_TV_TEST_CMD"
fi

# Add artifacts
VT_TV_ARTIFACTS_DIR="/tmp/artifacts"
mkdir -p $VT_TV_ARTIFACTS_DIR

# > go to output directory
pushd "$VT_TV_OUTPUT_DIR"

# > add the unit tests report artifact
cp "junit-report.xml" $VT_TV_ARTIFACTS_DIR/ || true

# > add mesh files and png artifacts
if [ -d "$VT_TV_TESTS_OUTPUT_DIR" ]; then
    cp "$VT_TV_TESTS_OUTPUT_DIR/"*".vtp"  $VT_TV_ARTIFACTS_DIR/
    cp "$VT_TV_TESTS_OUTPUT_DIR/"*".png"  $VT_TV_ARTIFACTS_DIR/
fi

popd

# list artifacts dir content
ls $VT_TV_ARTIFACTS_DIR
