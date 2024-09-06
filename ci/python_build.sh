#!/bin/bash

# This script builds and install vt-tv as a pip package

set -ex

CURRENT_DIR="$(dirname -- "$(realpath -- "$0")")"
PARENT_DIR="$(dirname "$CURRENT_DIR")"

VT_TV_SRC_DIR=${VT_TV_SRC_DIR:-$PARENT_DIR}

for env in $(conda env list | grep 'py*' | perl -lane 'print $F[-1]' | xargs ls -lrt1d |  perl -lane 'print $F[-1]' | sed -r 's/^.*\/(.*)$/\1/'); do
    # Clear vizualization output directory
    rm -rf $VT_TV_OUTPUT_DIR/python_tests/*

    echo "::group::Test Python Bindings (${python_version})"

    # Activate conda environment
    . $CONDA_PATH/etc/profile.d/conda.sh && conda activate env

    # Build
    pip install PyYAML
    pip install $VT_TV_SRC_DIR

    conda deactivate

    echo "::endgroup::"
done