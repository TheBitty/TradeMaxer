#!/bin/bash

set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

echo "Building Trading System..."

# Create build directory
mkdir -p "$BUILD_DIR"

# Install Python dependencies
echo "Installing Python dependencies..."
python3 -m pip install --user numpy

# Configure CMake
echo "Configuring CMake..."
cd "$BUILD_DIR"
cmake ..

# Build C++ application
echo "Building C++ application..."
make -j$(nproc)

echo "Build completed successfully!"
echo "Executable location: $BUILD_DIR/bin/trading_system"
echo ""
echo "To run the trading system:"
echo "  cd $BUILD_DIR/bin && ./trading_system"