#!/bin/bash

# This script stops X virtual frame buffer display
XVFB_DISPLAY="${1:-:99}"

echo "Stopping X Virtual Frame Buffer ($XVFB_DISPLAY)..."
pkill Xvfb
rm -rf /tmp/.X11-unix/X${XVFB_DISPLAY:1}
