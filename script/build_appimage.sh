#!/usr/bin/env bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

# build_appimage.sh - Build portable self-contained AppImage for rmap

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

BUILD_DIR="${PROJECT_ROOT}/build"
OUTPUT_DIR="${BUILD_DIR}/packages"
ARCH="${ARCH:-x86_64}"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --output-dir)
            OUTPUT_DIR="$2"
            shift 2
            ;;
        -h|--help)
            echo "Usage: $0 [--build-dir <dir>] [--output-dir <dir>]"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

echo "======================================================="
echo "Building rmap AppImage (${ARCH})"
echo "======================================================="
echo "Project Root: ${PROJECT_ROOT}"
echo "Build Dir:    ${BUILD_DIR}"
echo "Output Dir:   ${OUTPUT_DIR}"

APPDIR="${BUILD_DIR}/AppDir"
mkdir -p "${OUTPUT_DIR}"
rm -rf "${APPDIR}"
mkdir -p "${APPDIR}/usr"

# 1. Install project into AppDir
echo "--> Installing project into AppDir..."
cmake --install "${BUILD_DIR}" --prefix "${APPDIR}/usr"

# 2. Desktop file and icon integration
echo "--> Setting up desktop and icon metadata..."
cp "${PROJECT_ROOT}/res/rmap.desktop" "${APPDIR}/rmap.desktop"
cp "${PROJECT_ROOT}/res/images/app_icon.png" "${APPDIR}/rmap.png"
cp "${PROJECT_ROOT}/res/images/app_icon.png" "${APPDIR}/.DirIcon"

# 3. Create AppRun launcher
echo "--> Generating AppRun launcher..."
cat > "${APPDIR}/AppRun" << 'EOF'
#!/bin/sh
SELF=$(readlink -f "$0")
HERE=${SELF%/*}

export PATH="${HERE}/usr/bin:${PATH}"
export LD_LIBRARY_PATH="${HERE}/usr/lib:${HERE}/usr/lib64:${HERE}/usr/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH:-}"
export QT_PLUGIN_PATH="${HERE}/usr/plugins:${QT_PLUGIN_PATH:-}"
export QML_IMPORT_PATH="${HERE}/usr/qml:${QML_IMPORT_PATH:-}"
export QML2_IMPORT_PATH="${HERE}/usr/qml:${QML2_IMPORT_PATH:-}"
export XDG_DATA_DIRS="${HERE}/usr/share:${XDG_DATA_DIRS:-/usr/local/share:/usr/share}"

# Ensure rmap discovers bundled assets inside the AppImage
export RMAP_TEMPLATES_DIR="${HERE}/usr/share/rmap/templates"
export RMAP_EXAMPLES_DIR="${HERE}/usr/share/rmap/examples"
export RMAP_THEMES_PATH="${HERE}/usr/share/rmap/themes"
export RMAP_TRANSLATIONS_PATH="${HERE}/usr/share/rmap/translations"
export RMAP_DOCS_DIR="${HERE}/usr/share/doc/rmap/html"

exec "${HERE}/usr/bin/rmap" "$@"
EOF
chmod +x "${APPDIR}/AppRun"

# 4. Bundle Qt6 plugins and runtime dependencies
echo "--> Bundling Qt6 plugins and runtime dependencies..."
mkdir -p "${APPDIR}/usr/plugins"
mkdir -p "${APPDIR}/usr/lib"
ln -sf lib "${APPDIR}/usr/lib64" 2>/dev/null || true

QT_PLUGIN_DIR=""
if command -v qtpaths6 >/dev/null 2>&1; then
    QT_PLUGIN_DIR="$(qtpaths6 --plugin-dir 2>/dev/null || true)"
elif command -v qtpaths-qt6 >/dev/null 2>&1; then
    QT_PLUGIN_DIR="$(qtpaths-qt6 --plugin-dir 2>/dev/null || true)"
elif command -v qtpaths >/dev/null 2>&1; then
    QT_PLUGIN_DIR="$(qtpaths --plugin-dir 2>/dev/null || true)"
elif command -v qmake6 >/dev/null 2>&1; then
    QT_PLUGIN_DIR="$(qmake6 -query QT_INSTALL_PLUGINS 2>/dev/null || true)"
fi

if [ -z "${QT_PLUGIN_DIR}" ] || [ ! -d "${QT_PLUGIN_DIR}" ]; then
    for cand in /usr/lib64/qt6/plugins /usr/lib/qt6/plugins /usr/lib/x86_64-linux-gnu/qt6/plugins; do
        if [ -d "$cand" ]; then
            QT_PLUGIN_DIR="$cand"
            break
        fi
    done
fi

if [ -n "${QT_PLUGIN_DIR}" ] && [ -d "${QT_PLUGIN_DIR}" ]; then
    echo "    Discovered Qt plugin directory: ${QT_PLUGIN_DIR}"
    for plugin_sub in platforms imageformats platformthemes iconengines styles xcbglintegrations wayland-shell-integration; do
        if [ -d "${QT_PLUGIN_DIR}/${plugin_sub}" ]; then
            mkdir -p "${APPDIR}/usr/plugins/${plugin_sub}"
            cp -r "${QT_PLUGIN_DIR}/${plugin_sub}"/* "${APPDIR}/usr/plugins/${plugin_sub}/" 2>/dev/null || true
        fi
    done
fi

# Collect dependent shared libraries
echo "--> Collecting shared library dependencies..."
collect_libs() {
    local target="$1"
    if [ -f "$target" ]; then
        ldd "$target" 2>/dev/null | grep "=>" | awk '{print $3}' | while read -r lib; do
            if [ -n "$lib" ] && [ -f "$lib" ]; then
                local basename
                basename=$(basename "$lib")
                # Do not bundle fundamental glibc/system low-level libraries
                case "$basename" in
                    ld-linux*|libc.so*|libpthread.so*|libm.so*|libdl.so*|libresolv.so*|librt.so*|libutil.so*|libnss*|libX11*|libGL*|libdrm*|libasound*)
                        continue
                        ;;
                    *)
                        if [ ! -f "${APPDIR}/usr/lib/${basename}" ]; then
                            cp -L "$lib" "${APPDIR}/usr/lib/" 2>/dev/null || true
                        fi
                        ;;
                esac
            fi
        done
    fi
}

collect_libs "${APPDIR}/usr/bin/rmap"

# Collect dependencies of copied Qt plugins (especially libqxcb.so)
find "${APPDIR}/usr/plugins" -type f -name "*.so" | while read -r plugin_lib; do
    collect_libs "$plugin_lib"
done

# 5. Extract version from version header or git
APP_VERSION="0.2.0"
if [ -f "${PROJECT_ROOT}/src/version.h" ]; then
    APP_VERSION=$(grep -E 'RMAP_VERSION_STRING' "${PROJECT_ROOT}/src/version.h" | head -n 1 | awk -F'"' '{print $2}' || echo "0.2.0")
fi
APPIMAGE_OUTPUT="${OUTPUT_DIR}/rmap-${APP_VERSION}-${ARCH}.AppImage"

# 6. Locate or download appimagetool
APPIMAGETOOL=""
if command -v appimagetool >/dev/null 2>&1; then
    APPIMAGETOOL="$(command -v appimagetool)"
elif [ -f "${BUILD_DIR}/appimagetool" ]; then
    APPIMAGETOOL="${BUILD_DIR}/appimagetool"
fi

if [ -z "${APPIMAGETOOL}" ]; then
    echo "--> Downloading appimagetool..."
    TOOL_URL="https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-${ARCH}.AppImage"
    if curl -sSL -f -o "${BUILD_DIR}/appimagetool" "${TOOL_URL}" 2>/dev/null || wget -q -O "${BUILD_DIR}/appimagetool" "${TOOL_URL}" 2>/dev/null; then
        chmod +x "${BUILD_DIR}/appimagetool"
        APPIMAGETOOL="${BUILD_DIR}/appimagetool"
    fi
fi

if [ -n "${APPIMAGETOOL}" ] && [ -x "${APPIMAGETOOL}" ]; then
    echo "--> Packaging AppDir with appimagetool..."
    export ARCH
    export APPIMAGE_EXTRACT_AND_RUN=1
    # Use --appimage-extract-and-run for environments without FUSE (Docker, GitHub Actions)
    if "${APPIMAGETOOL}" --appimage-extract-and-run --version >/dev/null 2>&1; then
        "${APPIMAGETOOL}" --appimage-extract-and-run "${APPDIR}" "${APPIMAGE_OUTPUT}"
    else
        "${APPIMAGETOOL}" "${APPDIR}" "${APPIMAGE_OUTPUT}"
    fi
    chmod +x "${APPIMAGE_OUTPUT}"
    echo "✓ AppImage generated successfully: ${APPIMAGE_OUTPUT}"
else
    echo "⚠️ appimagetool not found or unavailable. AppDir staged at: ${APPDIR}"
    echo "  To package into an AppImage manually:"
    echo "    ARCH=${ARCH} appimagetool --appimage-extract-and-run ${APPDIR} ${APPIMAGE_OUTPUT}"
fi

echo "======================================================="
echo "AppImage Stage Completed!"
echo "======================================================="
