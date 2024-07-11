#!/usr/bin/env bash

set -e

export DISPLAY=:99.0

# Start X virtual frame buffer
Xvfb :99 -screen 0 1024x768x24 -nolisten tcp > /dev/null 2>&1 &

sleep 1s

# Activate conda environment
. /opt/conda/etc/profile.d/conda.sh && conda activate deves

# Build
pip install PyYAML
pip install /opt/src/vt-tv

# Test
python /opt/src/vt-tv/tests/test_bindings.py

pKill xvfb
rm -rf /tmp/.X11-unix/X99

export DISPLAY=:0
