#!/usr/bin/env bash

# Set X11 display
Xvfb :99 -screen 0 1024x768x24 > /dev/null 2>&1 &
export DISPLAY=:99.0;
export VTKI_OFF_SCREEN=True;