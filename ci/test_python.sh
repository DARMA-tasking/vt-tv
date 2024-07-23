#!/bin/bash

# This script is running tests
# > setup as pip package (internally build VT-TV with Python binding) 
# > run tests


set -ex

export DISPLAY=:99.0

# Activate conda environment
. /opt/conda/etc/profile.d/conda.sh && conda activate deves

# Build
pip install PyYAML
pip install /opt/src/vt-tv

# Start custom display with X virtual frame buffer
Xvfb :99 -screen 0 1024x768x24 -nolisten tcp > /dev/null 2>&1 &
sleep 1s

# Test (needs display)
python /opt/src/vt-tv/tests/test_bindings.py

# Clean and restore regular display
pkill Xvfb
rm -rf /tmp/.X11-unix/X99
export DISPLAY=:0
