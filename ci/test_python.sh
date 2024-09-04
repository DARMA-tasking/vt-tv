#!/bin/bash

# This script is running tests
# > setup as pip package (internally build VT-TV with Python binding)
# > run tests

set -ex

VT_TV_CONDA_ENV=${VT_TV_CONDA_ENV:-deves}

CURRENT_DIR="$(dirname -- "$(realpath -- "$0")")"

# Activate conda environment
. /opt/conda/etc/profile.d/conda.sh && conda activate $VT_TV_CONDA_ENV

# Build
pip install PyYAML
pip install /opt/src/vt-tv

# Start virtual display
CURRENT_DISPLAY=$(echo $DISPLAY)
if [ "$(echo  $(uname -a))" != *"Darwin"* ]; then
    $CURRENT_DIR/xvfb_start.sh :99
fi

# Run test
python /opt/src/vt-tv/tests/test_bindings.py

# Restore display
if [ "$(echo  $(uname -a))" != *"Darwin"* ]; then
    $CURRENT_DIR/xvfb_stop.sh :99 $CURRENT_DISPLAY
fi
