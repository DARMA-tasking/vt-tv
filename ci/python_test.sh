#!/bin/bash

# This script tests vt-tv pip package

set -ex

CONDA_PATH=${CONDA_PATH:-/opt/conda}
VT_TV_CONDA_ENV=${VT_TV_CONDA_ENV:-deves}

CURRENT_DIR="$(dirname -- "$(realpath -- "$0")")"

# Activate conda environment
. ${CONDA_PATH}/etc/profile.d/conda.sh && conda activate $VT_TV_CONDA_ENV

# Start virtual display (Linux)
CURRENT_DISPLAY=$(echo $DISPLAY)
if [[ $(uname -a) != *"Darwin"* ]]; then
    $CURRENT_DIR/xvfb_start.sh :99
fi

# Run test
python /opt/src/vt-tv/tests/test_bindings.py

# Restore display
if [[ $(uname -a) != *"Darwin"* ]]; then
    $CURRENT_DIR/xvfb_stop.sh :99 $CURRENT_DISPLAY
fi
