#!/bin/bash

set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

if [ ! -f "$BUILD_DIR/bin/trading_system" ]; then
    echo "Trading system not built. Running build script..."
    "$PROJECT_DIR/build.sh"
fi

echo "Starting Trading System..."
cd "$BUILD_DIR/bin"
./trading_system