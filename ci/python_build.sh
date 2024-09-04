#!/bin/bash

# This script builds and install vt-tv as a pip package

set -ex

CONDA_PATH=${CONDA_PATH:-/opt/conda}
VT_TV_CONDA_ENV=${VT_TV_CONDA_ENV:-deves}

CURRENT_DIR="$(dirname -- "$(realpath -- "$0")")"

# Activate conda environment
. ${CONDA_PATH}/etc/profile.d/conda.sh && conda activate $VT_TV_CONDA_ENV

# Build
pip install PyYAML

pushd /opt/src/vt-tv
pip install .
popd

# Deactivate conda environment
conda deactivate