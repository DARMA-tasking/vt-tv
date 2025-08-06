#!/bin/bash

set -euo pipefail

# This script installs Conda and setup conda environments on the host machine for the given python versions
# Example: `setup_conda.sh 3.8,3.9,3.10,3.11,3.12`

CONDA_PATH=${CONDA_PATH:-"/opt/conda"}
PYTHON_VERSIONS=${1:-"3.8,3.9,3.10,3.11,3.12"}
PACKAGES=${PACKAGES:-"PyYAML Brotli schema nanobind"}

echo "::group::Install conda"
mkdir -p ~/miniconda3

# Detect platform
if [[ $(uname -a) == *"Darwin"* ]]; then
    if [[ $(arch) == 'arm64' ]]; then
        INSTALLER_URL="https://repo.anaconda.com/miniconda/Miniconda3-latest-MacOSX-arm64.sh"
    else
        INSTALLER_URL="https://repo.anaconda.com/miniconda/Miniconda3-latest-MacOSX-x86_64.sh"
    fi
else
    INSTALLER_URL="https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh"
fi

curl -fsSL "$INSTALLER_URL" -o ~/miniconda.sh
bash ~/miniconda.sh -b -u -p "$CONDA_PATH"
rm -f ~/miniconda.sh

"$CONDA_PATH/bin/conda" tos accept --override-channels --channel https://repo.anaconda.com/pkgs/main
"$CONDA_PATH/bin/conda" tos accept --override-channels --channel https://repo.anaconda.com/pkgs/r

"$CONDA_PATH/bin/conda" config --set always_yes yes
"$CONDA_PATH/bin/conda" config --set changeps1 no

"$CONDA_PATH/bin/conda" init bash
"$CONDA_PATH/bin/conda" init zsh 2>/dev/null || true

export PATH="$CONDA_PATH/bin:$PATH"
source "$CONDA_PATH/etc/profile.d/conda.sh" 2>/dev/null || true

echo "Conda version: $(conda --version)"
conda deactivate || true
echo "::endgroup::"

IFS=',' read -ra versions <<< "$PYTHON_VERSIONS"
for python_version in "${versions[@]}"; do
    python_version=$(echo "$python_version" | xargs)

    echo "::group::Create conda environment (py${python_version})"
    conda create -y -n "py${python_version}" "python=${python_version}"

    echo "Python: $(conda run -n py${python_version} which python)"
    echo "Version: $(conda run -n py${python_version} python --version)"

    for package in $PACKAGES; do
        conda run -n "py${python_version}" pip install --no-cache-dir "$package"
    done

    echo "::endgroup::"
done

echo "Created environments: $(echo $PYTHON_VERSIONS | tr ',' ' ' | sed 's/[0-9.]\+/py&/g')"
