#!/bin/bash

# This script tests vt-tv pip package

set -ex

CURRENT_DIR="$(dirname -- "$(realpath -- "$0")")"
PARENT_DIR="$(dirname "$CURRENT_DIR")"

CONDA_PATH=${CONDA_PATH:-/opt/conda}
VT_TV_CONDA_ENV=${VT_TV_CONDA_ENV:-deves}
VT_TV_SRC_DIR=${VT_TV_SRC_DIR:-$PARENT_DIR}
VT_TV_OUTPUT_DIR=${VT_TV_OUTPUT_DIR:-"$VT_TV_SRC_DIR/output"}

# Activate conda environment
. ${CONDA_PATH}/etc/profile.d/conda.sh && conda activate $VT_TV_CONDA_ENV


# Run test
if [[ $(uname -a) != *"Darwin"* ]]; then
    # Start virtual display (Linux)
    xvfb-run python $VT_TV_SRC_DIR/tests/test_bindings.py
else
    python $VT_TV_SRC_DIR/tests/test_bindings.py
fi
