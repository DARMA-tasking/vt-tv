#!/bin/bash

# This script stops X virtual frame buffer display
# Additionally it removes temporary X11 files for the display passed as argument (e.g. ':99')

XVFB_DISPLAY="${1:-:99}"

echo "Stopping X Virtual Frame Buffer ($XVFB_DISPLAY)..."
pkill Xvfb
rm -rf /tmp/.X11-unix/X${XVFB_DISPLAY:1}
