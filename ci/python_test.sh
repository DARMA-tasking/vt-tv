#!/bin/bash

# This script tests vt-tv pip package

set -ex

CURRENT_DIR="$(dirname -- "$(realpath -- "$0")")"
PARENT_DIR="$(dirname "$CURRENT_DIR")"

VT_TV_SRC_DIR=${VT_TV_SRC_DIR:-$PARENT_DIR}
VT_TV_OUTPUT_DIR=${VT_TV_OUTPUT_DIR:-"$VT_TV_SRC_DIR/output"}

pushd $VT_TV_SRC_DIR

# Create vizualization output directory (required).
mkdir -p $VT_TV_OUTPUT_DIR/python_tests

for env in $(conda env list | grep -E '^py' | perl -lane 'print $F[-1]' | xargs ls -ld | perl -lane 'print $F[-1]' | sed -E 's|^.*/(.*)$|\1|'); do
    # Clear vizualization output directory
    rm -rf $VT_TV_OUTPUT_DIR/python_tests/*

    echo "::group::Test Python Bindings (${env})"

    # Activate conda environment
    . $CONDA_PATH/etc/profile.d/conda.sh && conda activate $env

    # Run test
    if [[ $(uname -a) != *"Darwin"* ]]; then
        # Start virtual display (Linux)
        xvfb-run python $VT_TV_SRC_DIR/tests/test_bindings.py
    else
        python $VT_TV_SRC_DIR/tests/test_bindings.py
    fi

    # Deactivate conda environment
    conda deactivate

    echo "::endgroup::"
done

popd