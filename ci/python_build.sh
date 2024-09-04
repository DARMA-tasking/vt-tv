#!/bin/bash

# This script builds and install vt-tv as a pip package

set -ex

CURRENT_DIR="$(dirname -- "$(realpath -- "$0")")"
PARENT_DIR="$(dirname "$CURRENT_DIR")"

CONDA_PATH=${CONDA_PATH:-/opt/conda}
VT_TV_CONDA_ENV=${VT_TV_CONDA_ENV:-deves}
VT_TV_SRC_DIR=${VT_TV_SRC_DIR:-$PARENT_DIR}

# Activate conda environment
. ${CONDA_PATH}/etc/profile.d/conda.sh && conda activate $VT_TV_CONDA_ENV

# Build
pip install PyYAML
pip install $VT_TV_SRC_DIR

# Deactivate conda environment
conda deactivate