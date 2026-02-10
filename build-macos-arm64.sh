#!/bin/bash
#
# build-macos-arm64.sh - Build Infinite Tux for macOS arm64 (Apple Silicon)
#
# Prerequisites:
#   brew install cmake sdl2 sdl2_image sdl2_mixer
#
# Usage:
#   ./build-macos-arm64.sh
#

set -e

VERSION=$(cat VERSION 2>/dev/null | tr -d '[:space:]' || echo "1.0.0")
PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"
RELEASE_NAME="infinite-tux-${VERSION}-macos-arm64"

# Detect CPU cores for parallel build
NPROC=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo "=========================================="
echo "Infinite Tux v${VERSION} - macOS arm64"
echo "=========================================="

# Check for required tools
if ! command -v cmake &> /dev/null; then
    echo "ERROR: cmake not found. Install with: brew install cmake"
    exit 1
fi

# Check for SDL2
if ! pkg-config --exists sdl2 2>/dev/null; then
    echo "ERROR: SDL2 not found. Install with:"
    echo "  brew install sdl2 sdl2_image sdl2_mixer"
    exit 1
fi

# Clean and create build directory
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

echo ""
echo "Configuring..."
cmake "${PROJECT_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES=arm64

echo ""
echo "Building with ${NPROC} parallel jobs..."
make -j${NPROC}

echo ""
echo "Creating release package..."
RELEASE_DIR="${BUILD_DIR}/${RELEASE_NAME}"
mkdir -p "${RELEASE_DIR}"

cp InfiniteTux "${RELEASE_DIR}/"
cp -r "${PROJECT_DIR}/resources" "${RELEASE_DIR}/"
cp "${PROJECT_DIR}/README.md" "${RELEASE_DIR}/" 2>/dev/null || true
cp "${PROJECT_DIR}/VERSION" "${RELEASE_DIR}/" 2>/dev/null || true

# Create launcher script
cat > "${RELEASE_DIR}/run.command" << 'LAUNCHER'
#!/bin/bash
cd "$(dirname "$0")"
./InfiniteTux "$@"
LAUNCHER
chmod +x "${RELEASE_DIR}/run.command"

# Create tarball
cd "${BUILD_DIR}"
tar -czvf "${RELEASE_NAME}.tar.gz" "${RELEASE_NAME}"

echo ""
echo "=========================================="
echo "Build complete!"
echo "Binary:  ${BUILD_DIR}/InfiniteTux"
echo "Package: ${BUILD_DIR}/${RELEASE_NAME}.tar.gz"
echo ""
echo "To run: ./build/InfiniteTux"
echo "=========================================="
