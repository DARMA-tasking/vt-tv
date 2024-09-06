#!/bin/bash

# This script run glxinfo | grep OpenGL version" in Xbfb

DISPLAY_0=$(echo $DISPLAY)
bash ci/xvfb_start.sh :99
export DISPLAY=:99

glxinfo | grep "OpenGL version"
sudo add-apt-repository ppa:oibaf/graphics-drivers -y
sudo apt update
sudo apt upgrade
glxinfo | grep "OpenGL version"

# Restore display (none)
bash ci/xvfb_stop.sh :99
export DISPLAY=DISPLAY_0
