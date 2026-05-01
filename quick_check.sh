#!/bin/bash
cd /workspaces/C-LHF-D-M

# Quick compile check
echo "Quick compilation check..."

# Compile config.c first
gcc -c -Wall -Wextra -std=c11 -D_GNU_SOURCE -Iinclude src/config.c 2>&1 | head -20

echo "---"

# Check for basic syntax errors in storage.c
gcc -c -Wall -Wextra -std=c11 -D_GNU_SOURCE -Iinclude src/storage.c 2>&1 | head -20
