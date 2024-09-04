#!/bin/bash

# Start custom display with X virtual frame buffer
XVFB_DISPLAY="${1:-:99}"
echo "Starting X Virtual Frame Buffer ($XVFB_DISPLAY)..."
Xvfb $XVFB_DISPLAY -screen 0 1024x768x24 -nolisten tcp > /dev/null 2>&1 &
sleep 1s
export DISPLAY=$XVFB_DISPLAY
echo "DISPLAY=$DISPLAY"