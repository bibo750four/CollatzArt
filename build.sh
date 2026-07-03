#!/bin/bash

# CollatzArt Build Script
# This script rebuilds the CollatzArt application

set -e

echo "Building CollatzArt..."

# Create build directory if it doesn't exist
mkdir -p build

# Configure and build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Copy executable to Debug directory for convenience
mkdir -p Debug
cp build/CollatzArt Debug/CollatzArt

echo "Build complete!"
echo ""
echo "To run the application:"
echo "  ./build/CollatzArt"
echo "  or"
echo "  ./Debug/CollatzArt"
