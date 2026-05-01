#!/bin/bash

# C-LHF&D-M Build Verification Script
# This script tests all components of the build

set -e

PROJECT_DIR="/workspaces/C-LHF-D-M"
BUILD_DIR="/tmp/clhfm_build"
CFLAGS="-Wall -Wextra -g -Iinclude -std=c11 -D_GNU_SOURCE"

echo "========================================="
echo "C-LHF&D-M - Build Verification"
echo "========================================="
echo ""

# Prepare build directory
mkdir -p "$BUILD_DIR"
cd "$PROJECT_DIR"

echo "1. Verifying source files..."
echo "   Source files found:"
for f in src/*.c; do
    echo "     - $(basename $f)"
done
echo ""

echo "2. Verifying header files..."
echo "   Header files found:"
for f in include/*.h; do
    echo "     - $(basename $f)"
done
echo ""

echo "3. Compiling individual modules..."
echo ""

CC=${CC:-gcc}
echo "   Using compiler: $CC"
echo "   Compiler version:"
$CC --version | head -1
echo ""

# Compile each module separately
MODULES=(
    "config"
    "concurrency"
    "storage"
)

for module in "${MODULES[@]}"; do
    echo "   Compiling $module.c..."
    if $CC -c $CFLAGS src/$module.c -o "$BUILD_DIR/$module.o" 2>&1; then
        echo "   ✓ $module.c compiled successfully"
    else
        echo "   ✗ Failed to compile $module.c"
        exit 1
    fi
done

echo ""
echo "4. Compiling package_manager.c..."
if $CC -c $CFLAGS src/package_manager.c -o "$BUILD_DIR/package_manager.o" 2>&1; then
    echo "✓ package_manager.c compiled successfully"
else
    echo "✗ Failed to compile package_manager.c"
    exit 1
fi

echo ""
echo "5. Compiling main.c..."
if $CC -c $CFLAGS src/main.c -o "$BUILD_DIR/main.o" 2>&1; then
    echo "✓ main.c compiled successfully"
else
    echo "✗ Failed to compile main.c"
    exit 1
fi

echo ""
echo "6. Linking executable..."
if $CC "$BUILD_DIR"/*.o -o "$BUILD_DIR/clhfm" -lm 2>&1; then
    echo "✓ Linking successful"
else
    echo "✗ Linking failed"
    exit 1
fi

echo ""
echo "7. Testing executable..."
if "$BUILD_DIR/clhfm" help | head -5; then
    echo "✓ Executable runs successfully"
else
    echo "✗ Executable test failed"
    exit 1
fi

echo ""
echo "========================================="
echo "✓ Build verification successful!"
echo "========================================="
echo ""
echo "Binary location: $BUILD_DIR/clhfm"
