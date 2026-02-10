#!/bin/bash
#
# build-windows-x86_64.sh - Cross-compile Infinite Tux for Windows x86_64
#
# Run this script from Linux to produce a Windows executable.
# SDL2 libraries are downloaded automatically.
#
# Prerequisites:
#   sudo apt-get install mingw-w64 cmake curl zip
#
# Usage:
#   ./build-windows-x86_64.sh
#

set -e

VERSION=$(cat VERSION 2>/dev/null | tr -d '[:space:]' || echo "1.0.0")
PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"
SDL2_DIR="${BUILD_DIR}/sdl2-mingw"
RELEASE_NAME="infinite-tux-${VERSION}-windows-x86_64"

# SDL2 versions - using older versions for better Wine compatibility
SDL2_VERSION="2.0.20"
SDL2_IMAGE_VERSION="2.0.5"
SDL2_MIXER_VERSION="2.0.4"

# Detect CPU cores for parallel build
NPROC=$(nproc 2>/dev/null || echo 4)

echo "=========================================="
echo "Infinite Tux v${VERSION} - Windows x86_64"
echo "=========================================="

# Check for required tools
if ! command -v x86_64-w64-mingw32-gcc &> /dev/null; then
    echo "ERROR: MinGW not found. Install with: sudo apt-get install mingw-w64"
    exit 1
fi

if ! command -v cmake &> /dev/null; then
    echo "ERROR: cmake not found. Install with: sudo apt-get install cmake"
    exit 1
fi

if ! command -v curl &> /dev/null; then
    echo "ERROR: curl not found. Install with: sudo apt-get install curl"
    exit 1
fi

# Function to download SDL2 libraries
download_sdl2() {
    echo ""
    echo "Downloading SDL2 libraries..."
    
    local DOWNLOAD_DIR="${BUILD_DIR}/sdl2-download"
    mkdir -p "${DOWNLOAD_DIR}"
    mkdir -p "${SDL2_DIR}/include/SDL2"
    mkdir -p "${SDL2_DIR}/lib"
    mkdir -p "${SDL2_DIR}/bin"
    
    cd "${DOWNLOAD_DIR}"
    
    # Download SDL2
    echo "  Downloading SDL2 ${SDL2_VERSION}..."
    curl -L -o sdl2.tar.gz "https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/SDL2-devel-${SDL2_VERSION}-mingw.tar.gz"
    tar -xzf sdl2.tar.gz
    cp -r SDL2-${SDL2_VERSION}/x86_64-w64-mingw32/lib/* "${SDL2_DIR}/lib/"
    cp -r SDL2-${SDL2_VERSION}/x86_64-w64-mingw32/bin/* "${SDL2_DIR}/bin/"
    cp -r SDL2-${SDL2_VERSION}/x86_64-w64-mingw32/include/SDL2/* "${SDL2_DIR}/include/SDL2/"
    
    # Download SDL2_image
    echo "  Downloading SDL2_image ${SDL2_IMAGE_VERSION}..."
    curl -L -o sdl2_image.tar.gz "https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL2_IMAGE_VERSION}/SDL2_image-devel-${SDL2_IMAGE_VERSION}-mingw.tar.gz"
    tar -xzf sdl2_image.tar.gz
    cp -r SDL2_image-${SDL2_IMAGE_VERSION}/x86_64-w64-mingw32/lib/* "${SDL2_DIR}/lib/"
    cp -r SDL2_image-${SDL2_IMAGE_VERSION}/x86_64-w64-mingw32/bin/* "${SDL2_DIR}/bin/"
    # Headers may be in include/ or include/SDL2/
    if [ -d "SDL2_image-${SDL2_IMAGE_VERSION}/x86_64-w64-mingw32/include/SDL2" ]; then
        cp -r SDL2_image-${SDL2_IMAGE_VERSION}/x86_64-w64-mingw32/include/SDL2/* "${SDL2_DIR}/include/SDL2/"
    else
        cp SDL2_image-${SDL2_IMAGE_VERSION}/x86_64-w64-mingw32/include/*.h "${SDL2_DIR}/include/SDL2/" 2>/dev/null || true
    fi
    
    # Download SDL2_mixer
    echo "  Downloading SDL2_mixer ${SDL2_MIXER_VERSION}..."
    curl -L -o sdl2_mixer.tar.gz "https://github.com/libsdl-org/SDL_mixer/releases/download/release-${SDL2_MIXER_VERSION}/SDL2_mixer-devel-${SDL2_MIXER_VERSION}-mingw.tar.gz"
    tar -xzf sdl2_mixer.tar.gz
    cp -r SDL2_mixer-${SDL2_MIXER_VERSION}/x86_64-w64-mingw32/lib/* "${SDL2_DIR}/lib/"
    cp -r SDL2_mixer-${SDL2_MIXER_VERSION}/x86_64-w64-mingw32/bin/* "${SDL2_DIR}/bin/"
    # Headers may be in include/ or include/SDL2/
    if [ -d "SDL2_mixer-${SDL2_MIXER_VERSION}/x86_64-w64-mingw32/include/SDL2" ]; then
        cp -r SDL2_mixer-${SDL2_MIXER_VERSION}/x86_64-w64-mingw32/include/SDL2/* "${SDL2_DIR}/include/SDL2/"
    else
        cp SDL2_mixer-${SDL2_MIXER_VERSION}/x86_64-w64-mingw32/include/*.h "${SDL2_DIR}/include/SDL2/" 2>/dev/null || true
    fi
    
    # Remove CMake config files with broken paths
    rm -rf "${SDL2_DIR}/lib/cmake"
    
    # Clean up downloads
    cd "${PROJECT_DIR}"
    rm -rf "${DOWNLOAD_DIR}"
    
    echo "  SDL2 libraries installed to ${SDL2_DIR}"
}

# Clean and create build directory
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"

# Download SDL2 if needed
if [ ! -f "${SDL2_DIR}/lib/libSDL2.dll.a" ]; then
    download_sdl2
fi

cd "${BUILD_DIR}"

# Create CMake toolchain file for MinGW
cat > mingw-toolchain.cmake << 'TOOLCHAIN'
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
TOOLCHAIN

echo ""
echo "Configuring..."
cmake "${PROJECT_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=mingw-toolchain.cmake \
    -DSDL2_FOUND=TRUE \
    -DSDL2_INCLUDE_DIR="${SDL2_DIR}/include" \
    -DSDL2_INCLUDE_DIRS="${SDL2_DIR}/include" \
    -DSDL2_LIBRARY="${SDL2_DIR}/lib/libSDL2.dll.a" \
    -DSDL2_LIBRARIES="${SDL2_DIR}/lib/libSDL2.dll.a" \
    -DSDL2_IMAGE_INCLUDE_DIRS="${SDL2_DIR}/include" \
    -DSDL2_IMAGE_LIBRARY="${SDL2_DIR}/lib/libSDL2_image.dll.a" \
    -DSDL2_IMAGE_LIBRARIES="${SDL2_DIR}/lib/libSDL2_image.dll.a" \
    -DSDL2_MIXER_INCLUDE_DIRS="${SDL2_DIR}/include" \
    -DSDL2_MIXER_LIBRARY="${SDL2_DIR}/lib/libSDL2_mixer.dll.a" \
    -DSDL2_MIXER_LIBRARIES="${SDL2_DIR}/lib/libSDL2_mixer.dll.a"

echo ""
echo "Building with ${NPROC} parallel jobs..."
make -j${NPROC}

echo ""
echo "Creating release package..."
RELEASE_DIR="${BUILD_DIR}/${RELEASE_NAME}"
mkdir -p "${RELEASE_DIR}"

cp InfiniteTux.exe "${RELEASE_DIR}/"
cp -r "${PROJECT_DIR}/resources" "${RELEASE_DIR}/"
cp "${PROJECT_DIR}/README.md" "${RELEASE_DIR}/" 2>/dev/null || true
cp "${PROJECT_DIR}/VERSION" "${RELEASE_DIR}/" 2>/dev/null || true

# Copy SDL2 DLLs
for dll in SDL2.dll SDL2_image.dll SDL2_mixer.dll; do
    if [ -f "${SDL2_DIR}/bin/${dll}" ]; then
        cp "${SDL2_DIR}/bin/${dll}" "${RELEASE_DIR}/"
    fi
done

# Copy image format DLLs
for dll in libjpeg-62.dll libpng16-16.dll libtiff-5.dll libwebp-7.dll zlib1.dll; do
    if [ -f "${SDL2_DIR}/bin/${dll}" ]; then
        cp "${SDL2_DIR}/bin/${dll}" "${RELEASE_DIR}/"
    fi
done

# Copy audio format DLLs
for dll in libFLAC.dll libFLAC-8.dll libmodplug-1.dll libmpg123-0.dll libogg-0.dll libopus-0.dll libopusfile-0.dll libvorbis-0.dll libvorbisfile-3.dll; do
    if [ -f "${SDL2_DIR}/bin/${dll}" ]; then
        cp "${SDL2_DIR}/bin/${dll}" "${RELEASE_DIR}/"
    fi
done

# Copy MinGW runtime DLLs
for dll in libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll; do
    for path in \
        "/usr/lib/gcc/x86_64-w64-mingw32/$(x86_64-w64-mingw32-gcc -dumpversion 2>/dev/null)" \
        "/usr/x86_64-w64-mingw32/lib" \
        "/usr/x86_64-w64-mingw32/bin" \
        "/usr/lib/gcc/x86_64-w64-mingw32/10-posix" \
        "/usr/lib/gcc/x86_64-w64-mingw32/10-win32"
    do
        if [ -f "${path}/${dll}" ]; then
            cp "${path}/${dll}" "${RELEASE_DIR}/"
            break
        fi
    done
done

# Create zip
cd "${BUILD_DIR}"
zip -r "${RELEASE_NAME}.zip" "${RELEASE_NAME}"

echo ""
echo "=========================================="
echo "Build complete!"
echo "Binary:  ${BUILD_DIR}/InfiniteTux.exe"
echo "Package: ${BUILD_DIR}/${RELEASE_NAME}.zip"
echo ""
echo "To test with Wine: wine ${BUILD_DIR}/InfiniteTux.exe"
echo "=========================================="
