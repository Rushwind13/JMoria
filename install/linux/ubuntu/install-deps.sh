#!/bin/bash
# Install JMoria dependencies
# Run with: sudo ./install-deps.sh

set -euo pipefail

if [[ $EUID -ne 0 ]]; then
    echo "Run with sudo: sudo ./install-deps.sh"
    exit 1
fi

echo "Installing JMoria dependencies..."

apt-get update
apt-get install -y \
    build-essential \
    libsdl2-dev \
    libsdl2-image-dev \
    libgl1-mesa-dev

echo "Done. Now run 'make' to build JMoria."
