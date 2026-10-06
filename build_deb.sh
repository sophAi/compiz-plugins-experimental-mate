#!/usr/bin/env bash
# ==============================================================================
# Script: build_deb.sh
# Project: compiz-plugins-experimental-mate
# Description: Automated build & packaging script for Compiz Reloaded / Experimental
#              plugins on Linux Mint 22.3 MATE (Compiz 0.9.14.2 / Ubuntu 24.04 Noble base).
# ==============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PKG_NAME="compiz-plugins-experimental"
PKG_VERSION="0.8.18-4+mint22.3"
PKG_ARCH="amd64"
OUTPUT_DEB="${SCRIPT_DIR}/${PKG_NAME}_${PKG_VERSION}_${PKG_ARCH}.deb"
STAGING_DIR="/tmp/compiz-mate-deb-staging-$$"

BUILD_FROM_SOURCE=false
DO_INSTALL=false

print_usage() {
    cat <<EOF
Usage: $(basename "$0") [OPTIONS]

Options:
  -b, --build-source  Recompile elements and subplugins from C++ source code
  -i, --install       Automatically run 'sudo dpkg -i' after building the .deb
  -o, --output FILE   Specify custom output .deb path
  -h, --help          Show this help message
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        -b|--build-source)
            BUILD_FROM_SOURCE=true
            shift
            ;;
        -i|--install)
            DO_INSTALL=true
            shift
            ;;
        -o|--output)
            OUTPUT_DEB="$2"
            shift 2
            ;;
        -h|--help)
            print_usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            print_usage
            exit 1
            ;;
    esac
done

echo "=========================================================="
echo "  Compiz Experimental Plugins Package Builder for MATE"
echo "  Target: Linux Mint 22.3 MATE (Compiz 0.9.14.2)"
echo "=========================================================="
echo "Package Name    : ${PKG_NAME}"
echo "Package Version : ${PKG_VERSION}"
echo "Architecture    : ${PKG_ARCH}"
echo "Output Deb      : ${OUTPUT_DEB}"
echo "Staging Temp    : ${STAGING_DIR}"
echo "=========================================================="

# 1. Check prerequisite tools
echo "[1/6] Checking prerequisite tools..."
for cmd in dpkg-deb glib-compile-schemas xsltproc strip md5sum gzip; do
    if ! command -v "$cmd" >/dev/null 2>&1; then
        echo "ERROR: Required tool '$cmd' is not installed." >&2
        exit 1
    fi
done

# 2. Build elements from source if requested
if [ "$BUILD_FROM_SOURCE" = true ]; then
    echo "[2/6] Compiling plugins from source..."
    export PKG_CONFIG_PATH="/tmp/compiz-dev-sysroot/usr/lib/x86_64-linux-gnu/pkgconfig:/tmp/compiz-dev-sysroot/usr/lib/pkgconfig:${PKG_CONFIG_PATH:-}"

    echo "  -> Compiling elements and subplugins..."
    ELEMENTS_SRC="${SCRIPT_DIR}/src/elements"
    BUILD_DIR="${ELEMENTS_SRC}/build"
    rm -rf "${BUILD_DIR}"
    mkdir -p "${BUILD_DIR}"
    
    cd "${BUILD_DIR}"
    cmake .. \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DCMAKE_MODULE_PATH="${SCRIPT_DIR}/cmake" \
        -DCMAKE_PREFIX_PATH="/tmp/compiz-dev-sysroot/usr;/usr;${SCRIPT_DIR}/cmake" \
        -D_compiz_def_macro="${SCRIPT_DIR}/cmake" \
        -DCOMPIZ_PLUGIN_INSTALL_TYPE=local
    
    make -j"$(nproc)"
    
    # Update prebuilt binaries
    cp "${BUILD_DIR}/libelements.so" "${SCRIPT_DIR}/prebuilt/lib/"
    cp "${BUILD_DIR}/plugins/autumn/libautumn.so" "${SCRIPT_DIR}/prebuilt/lib/"
    cp "${BUILD_DIR}/plugins/bubbles/libbubbles.so" "${SCRIPT_DIR}/prebuilt/lib/"
    cp "${BUILD_DIR}/plugins/fireflies/libfireflies.so" "${SCRIPT_DIR}/prebuilt/lib/"
    cp "${BUILD_DIR}/plugins/snow/libsnow.so" "${SCRIPT_DIR}/prebuilt/lib/"
    cp "${BUILD_DIR}/plugins/stars/libstars.so" "${SCRIPT_DIR}/prebuilt/lib/"

    # Compile ported standalone plugins
    for p in stackswitch photo snowglobe atlantis cubemodel earth; do
        if [ -d "${SCRIPT_DIR}/src/${p}" ]; then
            echo "  -> Compiling ${p}..."
            P_BUILD="${SCRIPT_DIR}/src/${p}/build"
            rm -rf "${P_BUILD}"
            mkdir -p "${P_BUILD}"
            cd "${P_BUILD}"
            cmake .. \
                -DCMAKE_INSTALL_PREFIX=/usr \
                -DCMAKE_MODULE_PATH="${SCRIPT_DIR}/cmake" \
                -DCMAKE_PREFIX_PATH="/tmp/compiz-dev-sysroot/usr;/usr;${SCRIPT_DIR}/cmake" \
                -D_compiz_def_macro="${SCRIPT_DIR}/cmake" \
                -DCOMPIZ_PLUGIN_INSTALL_TYPE=local
            make -j"$(nproc)"
            cp "${P_BUILD}/lib${p}.so" "${SCRIPT_DIR}/prebuilt/lib/"
        fi
    done
    cd "${SCRIPT_DIR}"
else
    echo "[2/6] Using prebuilt optimized libraries..."
fi

# 3. Create staging tree
echo "[3/6] Assembling package tree in staging directory..."
rm -rf "${STAGING_DIR}"
mkdir -p "${STAGING_DIR}/usr/lib/compiz"
mkdir -p "${STAGING_DIR}/usr/lib/x86_64-linux-gnu/compiz"
mkdir -p "${STAGING_DIR}/usr/share/compiz"
mkdir -p "${STAGING_DIR}/usr/share/compiz/elements/images"
mkdir -p "${STAGING_DIR}/usr/share/compiz/icons/hicolor/scalable/apps"
mkdir -p "${STAGING_DIR}/usr/share/glib-2.0/schemas"
mkdir -p "${STAGING_DIR}/usr/share/doc/${PKG_NAME}"
mkdir -p "${STAGING_DIR}/DEBIAN"

# Copy libraries & create multiarch compatibility symlinks
cp -a "${SCRIPT_DIR}/prebuilt/lib"/lib*.so* "${STAGING_DIR}/usr/lib/compiz/"
strip --strip-unneeded "${STAGING_DIR}/usr/lib/compiz"/*.so* 2>/dev/null || true

cd "${STAGING_DIR}/usr/lib/x86_64-linux-gnu/compiz"
for lib in "${STAGING_DIR}/usr/lib/compiz"/lib*.so*; do
    base=$(basename "$lib")
    ln -sf "../../compiz/${base}" "${base}"
done
cd "${SCRIPT_DIR}"

# Copy metadata XMLs
cp -a "${SCRIPT_DIR}/metadata"/*.xml "${STAGING_DIR}/usr/share/compiz/"

# Copy textures and data
cp -a "${SCRIPT_DIR}/data/elements"/*.png "${SCRIPT_DIR}/data/elements"/*.svg "${STAGING_DIR}/usr/share/compiz/elements/"
cp -a "${SCRIPT_DIR}/data/elements"/*.png "${SCRIPT_DIR}/data/elements"/*.svg "${STAGING_DIR}/usr/share/compiz/elements/images/"

for dir in earth fireflies snow stars; do
    if [ -d "${SCRIPT_DIR}/data/${dir}" ]; then
        mkdir -p "${STAGING_DIR}/usr/share/compiz/${dir}"
        cp -a "${SCRIPT_DIR}/data/${dir}"/* "${STAGING_DIR}/usr/share/compiz/${dir}/"
    fi
done

# Copy icons
cp -a "${SCRIPT_DIR}/icons/hicolor/scalable/apps"/*.svg "${STAGING_DIR}/usr/share/compiz/icons/hicolor/scalable/apps/"

# Copy localization (compiled mo files)
if [ -d "${SCRIPT_DIR}/po/mo/locale" ]; then
    cp -a "${SCRIPT_DIR}/po/mo/locale" "${STAGING_DIR}/usr/share/"
elif [ -d "${SCRIPT_DIR}/po/mo" ]; then
    cp -a "${SCRIPT_DIR}/po/mo"/* "${STAGING_DIR}/usr/share/locale/" 2>/dev/null || true
fi

# Copy doc files
if [ -f "${SCRIPT_DIR}/debian/copyright" ]; then
    cp -a "${SCRIPT_DIR}/debian/copyright" "${STAGING_DIR}/usr/share/doc/${PKG_NAME}/"
fi
if [ -f "${SCRIPT_DIR}/debian/changelog" ]; then
    gzip -9cn "${SCRIPT_DIR}/debian/changelog" > "${STAGING_DIR}/usr/share/doc/${PKG_NAME}/changelog.gz"
fi

# 4. Generate & Validate GSettings Schemas
echo "[4/6] Generating and validating GSettings schemas..."
XSLT_CONVERTER="${SCRIPT_DIR}/cmake/xslt/compiz_gsettings_schemas.xslt"
if [ ! -f "${XSLT_CONVERTER}" ]; then
    XSLT_CONVERTER="/usr/share/compiz/xslt/compiz_gsettings_schemas.xslt"
fi

if [ -f "${XSLT_CONVERTER}" ]; then
    for f in "${STAGING_DIR}/usr/share/compiz"/*.xml; do
        base=$(basename "$f" .xml)
        xsltproc "${XSLT_CONVERTER}" "$f" > "${STAGING_DIR}/usr/share/glib-2.0/schemas/org.compiz.${base}.gschema.xml"
    done
else
    # Fallback: copy verified schemas from repo
    cp -a "${SCRIPT_DIR}/schemas"/*.gschema.xml "${STAGING_DIR}/usr/share/glib-2.0/schemas/"
fi

glib-compile-schemas --strict --dry-run "${STAGING_DIR}/usr/share/glib-2.0/schemas"
echo "All 25 GSettings schemas validated successfully."

# 5. Setup DEBIAN control, maintainer scripts, and permissions
echo "[5/6] Generating package control files and file checksums..."
cp -a "${SCRIPT_DIR}/debian/postinst" "${STAGING_DIR}/DEBIAN/postinst"
cp -a "${SCRIPT_DIR}/debian/postrm" "${STAGING_DIR}/DEBIAN/postrm"
chmod 755 "${STAGING_DIR}/DEBIAN/postinst" "${STAGING_DIR}/DEBIAN/postrm"

# Calculate Installed-Size in KB
INSTALLED_SIZE=$(du -sk "${STAGING_DIR}/usr" | cut -f1)

# Generate control file
sed "s/^Installed-Size:.*/Installed-Size: ${INSTALLED_SIZE}/" "${SCRIPT_DIR}/debian/control" > "${STAGING_DIR}/DEBIAN/control"
chmod 644 "${STAGING_DIR}/DEBIAN/control"

# Normalize directory and file permissions
find "${STAGING_DIR}/usr" -type d -exec chmod 755 {} +
find "${STAGING_DIR}/usr/share" -type f -exec chmod 644 {} +
find "${STAGING_DIR}/usr/lib" -type f -name "*.so*" -exec chmod 644 {} +

# Generate md5sums
cd "${STAGING_DIR}"
find usr -type f -exec md5sum {} + | sort -k2 > DEBIAN/md5sums
chmod 644 DEBIAN/md5sums
cd "${SCRIPT_DIR}"

# 6. Build the debian package
echo "[6/6] Packaging Debian archive..."
dpkg-deb --root-owner-group --build "${STAGING_DIR}" "${OUTPUT_DEB}"
rm -rf "${STAGING_DIR}"

DEB_SIZE=$(du -h "${OUTPUT_DEB}" | cut -f1)
echo "=========================================================="
echo "  BUILD SUCCESSFUL!"
echo "=========================================================="
echo "Package File : ${OUTPUT_DEB} (${DEB_SIZE})"
echo "To install   : sudo dpkg -i \"${OUTPUT_DEB}\""
echo "=========================================================="

if [ "$DO_INSTALL" = true ]; then
    echo "Installing ${OUTPUT_DEB}..."
    sudo dpkg -i "${OUTPUT_DEB}"
fi
