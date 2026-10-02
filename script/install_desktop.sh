#!/usr/bin/env bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

# install_desktop.sh - Install FreeDesktop desktop entry and application icons
# into the user's local XDG directories (~/.local/share/applications and
# ~/.local/share/icons/hicolor) without requiring root privileges.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

DATA_DIR="${XDG_DATA_HOME:-${HOME}/.local/share}"
APPS_DIR="${DATA_DIR}/applications"
ICONS_DIR="${DATA_DIR}/icons/hicolor"
PIXMAPS_DIR="${DATA_DIR}/pixmaps"

# Determine executable path to use in Exec=
RMAP_BIN="${PROJECT_ROOT}/build/bin/rmap"
if [ ! -x "${RMAP_BIN}" ]; then
  if command -v rmap >/dev/null 2>&1; then
    RMAP_BIN="$(command -v rmap)"
  else
    RMAP_BIN="rmap"
  fi
fi

mkdir -p "${APPS_DIR}" "${PIXMAPS_DIR}"

# Install icons for all resolutions
for size in 16 32 48 64 128 256 512; do
  icon_src="${PROJECT_ROOT}/res/images/app_icon_${size}.png"
  if [ -f "${icon_src}" ]; then
    target_dir="${ICONS_DIR}/${size}x${size}/apps"
    mkdir -p "${target_dir}"
    cp -f "${icon_src}" "${target_dir}/rmap.png"
  fi
done

# Install pixmap fallback
if [ -f "${PROJECT_ROOT}/res/images/app_icon.png" ]; then
  cp -f "${PROJECT_ROOT}/res/images/app_icon.png" "${PIXMAPS_DIR}/rmap.png"
fi

# Install desktop entry
cat << EOF > "${APPS_DIR}/rmap.desktop"
[Desktop Entry]
Type=Application
Name=rmap
GenericName=Hardware Register Map Designer
Comment=Hardware Register Map Designer & Model Generator
Exec="${RMAP_BIN}" %F
Icon=rmap
Terminal=false
Categories=Development;Engineering;Electronics;
MimeType=application/json;text/xml;text/csv;
Keywords=register;map;uvm;systemverilog;asic;fpga;hardware;
StartupWMClass=rmap
StartupNotify=true
EOF

chmod 644 "${APPS_DIR}/rmap.desktop"

# Refresh desktop database and icon cache if tools are present
if command -v update-desktop-database >/dev/null 2>&1; then
  update-desktop-database "${APPS_DIR}" >/dev/null 2>&1 || true
fi

if command -v gtk-update-icon-cache >/dev/null 2>&1; then
  gtk-update-icon-cache -f -t "${ICONS_DIR}" >/dev/null 2>&1 || true
fi

echo "rmap desktop integration installed successfully:"
echo "  Desktop entry: ${APPS_DIR}/rmap.desktop"
echo "  Icons:         ${ICONS_DIR}/*/apps/rmap.png"
echo "  Pixmaps:       ${PIXMAPS_DIR}/rmap.png"
