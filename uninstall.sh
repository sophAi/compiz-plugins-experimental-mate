#!/usr/bin/env bash
# ==============================================================================
# Script: uninstall.sh
# Project: compiz-plugins-experimental-mate
# Description: Uninstalls compiz-plugins-experimental and cleans up schemas.
# ==============================================================================

set -euo pipefail

echo "Uninstalling compiz-plugins-experimental package..."
sudo apt-get remove --purge -y compiz-plugins-experimental

echo "Recompiling GLib GSettings schemas..."
sudo glib-compile-schemas /usr/share/glib-2.0/schemas/ || true

echo "=========================================================="
echo "  Uninstallation Complete!"
echo "=========================================================="
