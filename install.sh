#!/usr/bin/env bash
# ==============================================================================
# Script: install.sh
# Project: compiz-plugins-experimental-mate
# Description: Installs the compiz-plugins-experimental deb package for Linux Mint MATE.
# ==============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DEB=$(ls -t "${SCRIPT_DIR}"/compiz-plugins-experimental_*.deb 2>/dev/null | head -n 1 || true)

if [ -z "${DEB}" ] || [ ! -f "${DEB}" ]; then
    echo "No .deb package found in ${SCRIPT_DIR}."
    echo "Running build_deb.sh to create the package first..."
    "${SCRIPT_DIR}/build_deb.sh"
    DEB=$(ls -t "${SCRIPT_DIR}"/compiz-plugins-experimental_*.deb 2>/dev/null | head -n 1)
fi

echo "Installing ${DEB} using sudo dpkg -i..."
sudo dpkg -i "${DEB}"

echo "Checking package dependencies (apt-get -f install)..."
sudo apt-get -f install -y

echo "=========================================================="
echo "  Installation Complete!"
echo "=========================================================="
echo "Compiz Experimental Plugins are now installed."
echo "You can configure them in CompizConfig Settings Manager (ccsm)."
echo "=========================================================="
