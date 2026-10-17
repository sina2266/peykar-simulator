#!/bin/bash

# VCell Stochastic Build Script

set -e

# Default options
BUILD_TYPE="Release"
ENABLE_MESSAGING="OFF"
CLEAN_BUILD="false"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --messaging)
            ENABLE_MESSAGING="ON"
            shift
            ;;
        --clean)
            CLEAN_BUILD="true"
            shift
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --debug       Build in Debug mode (default: Release)"
            echo "  --messaging   Enable messaging support"
            echo "  --clean       Clean build directory before building"
            echo "  --help        Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Clean build directory if requested
if [ "$CLEAN_BUILD" = "true" ]; then
    echo "Cleaning build directory..."
    rm -rf build
fi

# Configure
echo "Configuring VCell Stochastic..."
echo "  Build Type: $BUILD_TYPE"
echo "  Messaging: $ENABLE_MESSAGING"

cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DOPTION_TARGET_MESSAGING=$ENABLE_MESSAGING

# Build
echo ""
echo "Building VCell Stochastic..."
cmake --build build --config $BUILD_TYPE

echo ""
echo "Build completed successfully!"
echo ""
echo "Artifacts:"
echo "  Static Library: build/bin/libVCellStochLib.a"
echo "  Executable:     build/bin/VCellStoch"