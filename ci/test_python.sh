#!/bin/bash

set -ex

export DISPLAY=:99.0

# Start custom display with X virtual frame buffer
Xvfb :99 -screen 0 1024x768x24 -nolisten tcp > /dev/null 2>&1 &

sleep 1s

TEST_RESULT=0

# Activate conda environment
. /opt/conda/etc/profile.d/conda.sh && conda activate deves

# Build
pip install PyYAML
pip install /opt/src/vt-tv

# Test
python /opt/src/vt-tv/tests/test_bindings.py

# Clean and restore regular display
pkill Xvfb
rm -rf /tmp/.X11-unix/X99
export DISPLAY=:0
