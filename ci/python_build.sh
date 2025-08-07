#!/bin/bash

# This script builds and install vt-tv as a pip package

set -euo pipefail

export PIP_ROOT_USER_ACTION=ignore

CURRENT_DIR="$(dirname -- "$(realpath -- "$0")")"
PARENT_DIR="$(dirname "$CURRENT_DIR")"
CONDA_PATH=${CONDA_PATH:-"/opt/conda"}
VTK_DIR="${VTK_DIR:-$PARENT_DIR/vtk/build}"

VT_TV_SRC_DIR=${VT_TV_SRC_DIR:-$PARENT_DIR}

eval "$("$CONDA_PATH/bin/conda" shell.bash hook)"

echo "Conda path: $(command -v conda)"
echo "Conda version: $(conda --version)"

conda deactivate || true

for env in $(conda env list | grep -E '^py' | perl -lane 'print $F[-1]' | xargs ls -ld | perl -lane 'print $F[-1]' | sed -E 's|^.*/(.*)$|\1|'); do
  echo "::group::Build Python Bindings ($env)"

  conda activate "$env"

  pip install --no-cache-dir --upgrade PyYAML Brotli schema nanobind
  pip install --no-cache-dir --upgrade "$VT_TV_SRC_DIR"

  conda deactivate

  echo "::endgroup::"
done
