#!/bin/bash

# Stop custom display with X virtual frame buffer
XVFB_DISPLAY="${1:-:99}"
NEW_DISPLAY="${2:-:0}"

echo "Stopping X Virtual Frame Buffer ($XVFB_DISPLAY)..."
pkill Xvfb
rm -rf /tmp/.X11-unix/X${XVFB_DISPLAY:1}
export DISPLAY=$NEW_DISPLAY
echo "DISPLAY=$DISPLAY"