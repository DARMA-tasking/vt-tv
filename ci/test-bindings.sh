#!/usr/bin/env bash

set -e

# bash -c ". /opt/conda/etc/profile.d/conda.sh && conda activate deves"
. /opt/conda/etc/profile.d/conda.sh && conda activate deves
pip install PyYAML
python /opt/src/vt-tv/tests/test_bindings.py"
