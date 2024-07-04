#!/usr/bin/env bash

set -e

# Build
. /opt/conda/etc/profile.d/conda.sh && conda activate deves
pip install PyYAML
pip install /opt/src/vt-tv

# Test
export VTKI_OFF_SCREEN=True
python /opt/src/vt-tv/tests/test_bindings.py
