#!/usr/bin/env bash
set -euo pipefail

export CONDA_PATH=${CONDA_PATH:-/opt/conda}
export debian_chroot=""

CURRENT_DIR="$(dirname -- "$(realpath -- "$0")")"
PARENT_DIR="$(dirname "$CURRENT_DIR")"

VT_TV_SRC_DIR=${VT_TV_SRC_DIR:-$PARENT_DIR}
VT_TV_OUTPUT_DIR=${VT_TV_OUTPUT_DIR:-"$VT_TV_SRC_DIR/output"}

mkdir -p "$VT_TV_OUTPUT_DIR/python_tests"
cd "$VT_TV_SRC_DIR"

export PATH="$CONDA_PATH/bin:$PATH"
eval "$("$CONDA_PATH/bin/conda" shell.bash hook)"

echo "Conda path: $(command -v conda)"
echo "Conda version: $(conda --version)"

conda deactivate || true

for env in $(conda env list | grep -E '^py' | perl -lane 'print $F[-1]' | xargs ls -ld | perl -lane 'print $F[-1]' | sed -E 's|^.*/(.*)$|\1|'); do
  echo "::group::Test Python Bindings (${env})"

  conda activate "$env"

  rm -rf "$VT_TV_OUTPUT_DIR/python_tests"/*

  if [[ $(uname -s) == Linux ]]; then
    xvfb-run python "$VT_TV_SRC_DIR/tests/test_bindings.py"
  else
    python "$VT_TV_SRC_DIR/tests/test_bindings.py"
  fi

  conda deactivate
  echo "::endgroup::"
done
