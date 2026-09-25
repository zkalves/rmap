#!/usr/bin/env bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

BUILD_DIR="${PROJECT_ROOT}/build"
OUTPUT_DIR="${PROJECT_ROOT}/build/packages"

# Parse CLI arguments
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
            echo "Unknown argument: $1" >&2
            exit 1
            ;;
    esac
done

VERSION="$(cat "${PROJECT_ROOT}/VERSION" | tr -d '[:space:]')"
RMAP_BIN="${BUILD_DIR}/bin/rmap"

if [[ ! -f "${RMAP_BIN}" ]]; then
    # Check alternate location directly in build root
    if [[ -f "${BUILD_DIR}/rmap" ]]; then
        RMAP_BIN="${BUILD_DIR}/rmap"
    else
        echo "ERROR: rmap executable not found at ${RMAP_BIN}. Build the project first." >&2
        exit 1
    fi
fi

mkdir -p "${OUTPUT_DIR}"

TMP_DIR="$(mktemp -d -t rmap_module_XXXXXX)"
trap 'rm -rf "${TMP_DIR}"' EXIT

PACKAGE_NAME="rmap-${VERSION}-module-linux-x86_64"
STAGE_DIR="${TMP_DIR}/${PACKAGE_NAME}"

echo "=================================================="
echo "Creating rmap Environment Module Package (${VERSION})"
echo "=================================================="
echo "Source Binary: ${RMAP_BIN}"
echo "Staging Root:  ${STAGE_DIR}"

mkdir -p "${STAGE_DIR}/bin"
mkdir -p "${STAGE_DIR}/share/rmap/templates"
mkdir -p "${STAGE_DIR}/share/rmap/examples"
mkdir -p "${STAGE_DIR}/share/rmap/doc"
mkdir -p "${STAGE_DIR}/modulefiles/rmap"

# 1. Copy binary
cp -p "${RMAP_BIN}" "${STAGE_DIR}/bin/rmap"
chmod 755 "${STAGE_DIR}/bin/rmap"

# 2. Copy templates & examples
cp -rp "${PROJECT_ROOT}/templates/"* "${STAGE_DIR}/share/rmap/templates/"
cp -rp "${PROJECT_ROOT}/examples/"* "${STAGE_DIR}/share/rmap/examples/"

# 3. Copy documentation & licenses
cp -p "${PROJECT_ROOT}/LICENSE" "${STAGE_DIR}/share/rmap/doc/"
cp -p "${PROJECT_ROOT}/README.md" "${STAGE_DIR}/share/rmap/doc/"
if [[ -d "${PROJECT_ROOT}/_site" ]]; then
    cp -rp "${PROJECT_ROOT}/_site" "${STAGE_DIR}/share/rmap/doc/html"
fi

# 4. Copy and populate modulefiles
sed "s/0\.2\.0/${VERSION}/g" "${PROJECT_ROOT}/packaging/modules/rmap.lua" > "${STAGE_DIR}/modulefiles/rmap/${VERSION}.lua"
sed "s/0\.2\.0/${VERSION}/g" "${PROJECT_ROOT}/packaging/modules/rmap.tcl" > "${STAGE_DIR}/modulefiles/rmap/${VERSION}"

# Create default version marker (.version)
cat <<EOF > "${STAGE_DIR}/modulefiles/rmap/.version"
#%Module1.0
set ModulesVersion "${VERSION}"
EOF

# 5. Create Tarball archive
TARBALL="${OUTPUT_DIR}/${PACKAGE_NAME}.tar.gz"
tar -czf "${TARBALL}" -C "${TMP_DIR}" "${PACKAGE_NAME}"

echo "✓ Successfully generated module package: ${TARBALL} ($(du -h "${TARBALL}" | cut -f1))"
echo "=================================================="
