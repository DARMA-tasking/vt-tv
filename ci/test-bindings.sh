#!/usr/bin/env bash

set -e

# Start X virtual frame buffer
Xvfb :99 -screen 0 1024x768x24 > /dev/null 2>&1 &

sleep 1s

# Activate conda environment
. /opt/conda/etc/profile.d/conda.sh && conda activate deves

# Build
pip install PyYAML
pip install /opt/src/vt-tv

# Test
python /opt/src/vt-tv/tests/test_bindings.py
