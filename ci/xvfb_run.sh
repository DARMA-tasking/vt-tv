#!/bin/bash

# This script runs a command using Xvfb

DISPLAY_0=$(echo $DISPLAY)

XVFB_DISPLAY=":99"

echo "Starting X Virtual Frame Buffer ($XVFB_DISPLAY)..."
Xvfb $XVFB_DISPLAY -screen 0 1024x768x24 -nolisten tcp > /dev/null 2>&1 &
sleep 1

DISPLAY=$XVFB_DISPLAY $1

echo "Stopping X Virtual Frame Buffer ($XVFB_DISPLAY)..."
pkill Xvfb
rm -rf /tmp/.X11-unix/X${XVFB_DISPLAY:1}

export DISPLAY=$DISPLAY_0
