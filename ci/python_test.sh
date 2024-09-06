#!/bin/bash

# This script tests vt-tv pip package

set -ex

CURRENT_DIR="$(dirname -- "$(realpath -- "$0")")"
PARENT_DIR="$(dirname "$CURRENT_DIR")"

VT_TV_SRC_DIR=${VT_TV_SRC_DIR:-$PARENT_DIR}
VT_TV_OUTPUT_DIR=${VT_TV_OUTPUT_DIR:-"$VT_TV_SRC_DIR/output"}

pushd $VT_TV_SRC_DIR
chmod +x ./ci/python_test.sh

# Create vizualization output directory (required).
mkdir -p $VT_TV_OUTPUT_DIR/python_tests

for env in $(conda env list | grep 'py*' | perl -lane 'print $F[-1]' | xargs ls -lrt1d |  perl -lane 'print $F[-1]' | sed -r 's/^.*\/(.*)$/\1/'); do
    # Clear vizualization output directory
    rm -rf $VT_TV_OUTPUT_DIR/python_tests/*

    echo "::group::Test Python Bindings (${python_version})"

    # Activate conda environment
    . $CONDA_PREFIX/etc/profile.d/conda.sh && conda activate env

    # Run test
    if [[ $(uname -a) != *"Darwin"* ]]; then
        # Start virtual display (Linux)
        xvfb-run python $VT_TV_SRC_DIR/tests/test_bindings.py
    else
        python $VT_TV_SRC_DIR/tests/test_bindings.py
    fi

    conda deactivate

    echo "::endgroup::"
done

popd