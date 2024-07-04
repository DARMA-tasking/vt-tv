#!/usr/bin/env bash

set -e

Xvfb :99 -screen 0 1024x768x24 > /dev/null 2>&1 &

# Build
. /opt/conda/etc/profile.d/conda.sh && conda activate deves
pip install PyYAML
pip install /opt/src/vt-tv

# Test
python /opt/src/vt-tv/tests/test_bindings.py
