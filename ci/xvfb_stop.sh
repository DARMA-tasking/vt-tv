#!/bin/bash

# This script stops X virtual frame buffer display
XVFB_DISPLAY="${1:-:99}"
NEW_DISPLAY="${2:-:0}"

echo "Stopping X Virtual Frame Buffer ($XVFB_DISPLAY)..."
pkill Xvfb
export DISPLAY=$NEW_DISPLAY
echo "DISPLAY=$DISPLAY"
rm -rf /tmp/.X11-unix/X${XVFB_DISPLAY:1}
