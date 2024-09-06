#!/bin/bash

# This script installs Mesa libraries and utilities

. /etc/lsb-release

# Ubuntu 24.04 FIX.
# Error: MESA: error: ZINK: vkCreateInstance failed (VK_ERROR_INCOMPATIBLE_DRIVER)
if [ "$DISTRIB_RELEASE" == "24.04" ]; then
    echo "FIX: Using latest MESA drivers (dev) for Ubuntu 24.04 to fix MESA errors !"
    add-apt-repository ppa:oibaf/graphics-drivers -y
    apt update
fi

apt install -y -q --no-install-recommends \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    mesa-common-dev \
    libosmesa6-dev \
    mesa-utils
