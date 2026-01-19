#!/usr/bin/env bash
set -euo pipefail

CONDA_PATH=${CONDA_PATH:-"/opt/conda"}
PYTHON_VERSIONS=${1:-"3.9,3.10,3.11,3.12"}
PACKAGES=${PACKAGES:-"PyYAML Brotli schema nanobind"}

install_conda() {
  mkdir -p /tmp/miniconda
  case "$(uname -s)-$(uname -m)" in
    Darwin-arm64)  url="https://repo.anaconda.com/miniconda/Miniconda3-latest-MacOSX-arm64.sh"  ;;
    Darwin-*)      url="https://repo.anaconda.com/miniconda/Miniconda3-latest-MacOSX-x86_64.sh" ;;
    *)             url="https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh"  ;;
  esac
  curl -fsSL "$url" -o /tmp/miniconda/installer.sh
  bash /tmp/miniconda/installer.sh -b -u -p "$CONDA_PATH"
  rm -rf /tmp/miniconda
  "$CONDA_PATH/bin/conda" config --set always_yes yes
  "$CONDA_PATH/bin/conda" config --set changeps1 no
}

[ -x "$CONDA_PATH/bin/conda" ] || install_conda
export PATH="$CONDA_PATH/bin:$PATH"
export PIP_ROOT_USER_ACTION=ignore

conda tos accept --override-channels --channel https://repo.anaconda.com/pkgs/main
conda tos accept --override-channels --channel https://repo.anaconda.com/pkgs/r
eval "$("$CONDA_PATH/bin/conda" shell.bash hook)"

IFS=',' read -ra vers <<< "$PYTHON_VERSIONS"
for v in "${vers[@]}"; do
  v=$(echo "$v" | xargs)
  env="py${v}"
  env_dir="$CONDA_PATH/envs/$env"
  if [ -d "$env_dir" ]; then
    echo "::group::Update $env"
  else
    echo "::group::Create $env"
    conda create -y -n "$env" "python=$v"
  fi
  for p in $PACKAGES; do
    conda run -n "$env" pip install --no-cache-dir --upgrade "$p"
  done
  echo "::endgroup::"
done
