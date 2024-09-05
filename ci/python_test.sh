#!/bin/bash

# This script tests vt-tv pip package

set -ex

CURRENT_DIR="$(dirname -- "$(realpath -- "$0")")"
PARENT_DIR="$(dirname "$CURRENT_DIR")"

CONDA_PATH=${CONDA_PATH:-/opt/conda}
VT_TV_CONDA_ENV=${VT_TV_CONDA_ENV:-deves}
VT_TV_SRC_DIR=${VT_TV_SRC_DIR:-$PARENT_DIR}
VT_TV_OUTPUT_DIR=${VT_TV_OUTPUT_DIR:-"$VT_TV_SRC_DIR/output"}
VT_TV_PYTHON_TESTS_OUTPUT_DIR=${VT_TV_PYTHON_TESTS_OUTPUT_DIR:-"$VT_TV_OUTPUT_DIR/tests/python_tests"}

# Activate conda environment
. ${CONDA_PATH}/etc/profile.d/conda.sh && conda activate $VT_TV_CONDA_ENV

# Start virtual display (Linux)
DISPLAY_0=$(echo $DISPLAY)
if [[ $(uname -a) != *"Darwin"* ]]; then
    $CURRENT_DIR/xvfb_start.sh :99
    export DISPLAY=:99
fi

export LIBGL_ALWAYS_INDIRECT=1

# Run test
mkdir -p $VT_TV_PYTHON_TESTS_OUTPUT_DIR
python $VT_TV_SRC_DIR/tests/test_bindings.py

# Restore display
if [[ $(uname -a) != *"Darwin"* ]]; then
    $CURRENT_DIR/xvfb_stop.sh :99
    export DISPLAY=DISPLAY_0
fi
