#!/bin/bash

# This script builds and install vt-tv as a pip package

set -ex

CURRENT_DIR="$(dirname -- "$(realpath -- "$0")")"
PARENT_DIR="$(dirname "$CURRENT_DIR")"
CONDA_PATH=${CONDA_PATH:-"/opt/conda"}
VTK_DIR="${VTK_DIR:-$PARENT_DIR/vtk/build}"

VT_TV_SRC_DIR=${VT_TV_SRC_DIR:-$PARENT_DIR}

for env in $(conda env list | grep -E '^py' | perl -lane 'print $F[-1]' | xargs ls -ld | perl -lane 'print $F[-1]' | sed -E 's|^.*/(.*)$|\1|'); do
    echo "::group::Build Python Bindings (${env})"

    # Activate conda environment
    . $CONDA_PATH/etc/profile.d/conda.sh && conda activate $env

    # Build VT-TV python package
    pip install PyYAML
    pip install Brotli
    pip install schema
    pip install nanobind
    pip install $VT_TV_SRC_DIR

    # Deactivate conda environment
    conda deactivate

    echo "::endgroup::"
done