#!/bin/bash
set -e

echo "Building C-LHF&D-M..."

cd /workspaces/C-LHF-D-M

# Check if gcc is available
if ! command -v gcc &> /dev/null; then
    echo "Error: gcc not found"
    exit 1
fi

gcc --version

echo "Compilation test - checking syntax..."

# Try to compile just the main components
gcc -c -Wall -Wextra -std=c11 -D_GNU_SOURCE -Iinclude src/config.c -o /tmp/config.o 2>&1 && echo "✓ config.c compiles"
gcc -c -Wall -Wextra -std=c11 -D_GNU_SOURCE -Iinclude src/concurrency.c -o /tmp/concurrency.o 2>&1 && echo "✓ concurrency.c compiles"
gcc -c -Wall -Wextra -std=c11 -D_GNU_SOURCE -Iinclude src/storage.c -o /tmp/storage.o 2>&1 && echo "✓ storage.c compiles"
gcc -c -Wall -Wextra -std=c11 -D_GNU_SOURCE -Iinclude src/package_manager.c -o /tmp/package_manager.o 2>&1 && echo "✓ package_manager.c compiles"
gcc -c -Wall -Wextra -std=c11 -D_GNU_SOURCE -Iinclude src/main.c -o /tmp/main.o 2>&1 && echo "✓ main.c compiles"

echo ""
echo "Linking..."
gcc -Wall -Wextra -std=c11 -D_GNU_SOURCE /tmp/config.o /tmp/concurrency.o /tmp/storage.o /tmp/package_manager.o /tmp/main.o -o /tmp/clhfm -lm 2>&1 && echo "✓ Linking successful"

echo ""
echo "Testing binary..."
/tmp/clhfm help

echo ""
echo "✓ All build checks passed!"
