#!/bin/bash

echo "=== C-LHF&D-M Build Test ==="
echo "Current directory: $(pwd)"
echo ""

if cd /workspaces/C-LHF-D-M; then
    echo "✓ Changed to project directory"
    echo ""
    echo "Source files:"
    ls -la src/*.c
    echo ""
    echo "Header files:"
    ls -la include/*.h
    echo ""
    
    echo "Starting build..."
    make clean
    make all
    
    if [ -f "bin/clhfm" ]; then
        echo ""
        echo "✓ Build successful!"
        echo "Binary created: bin/clhfm"
        echo ""
        echo "Testing binary..."
        ./bin/clhfm help
    else
        echo "✗ Build failed - binary not created"
        exit 1
    fi
else
    echo "✗ Failed to change to project directory"
    exit 1
fi
