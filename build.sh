#!/bin/bash
# HelixForge Build Script

set -e

BUILD_TYPE=${1:-Release}
BUILD_DIR="build"

echo "==================================="
echo "  HelixForge Build Script"
echo "==================================="
echo "Build Type: $BUILD_TYPE"
echo ""

# Create build directory
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Configure
echo "Configuring with CMake..."
cmake .. \
  -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build
echo ""
echo "Building..."
cmake --build . --config $BUILD_TYPE -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo ""
echo "==================================="
echo "✓ Build complete!"
echo "==================================="
echo "Executable: $BUILD_DIR/helixforge"
echo ""
echo "Run tests:"
echo "  ./helixforge --version"
echo ""
echo "See QUICKSTART.md for usage examples"
