#!/bin/bash

# Script to fix library paths in macOS binaries to use relative paths

set -e

# Function to fix library paths for a binary
fix_library_paths() {
    local binary=$1
    local frameworks_dir=$2
    
    echo "Fixing library paths for $binary"
    
    # Get the list of libraries
    local libs=$(otool -L "$binary" | grep -v "$binary" | grep -v "/usr/lib" | grep -v "/System/Library" | awk '{print $1}')
    
    for lib in $libs; do
        # Get the library name
        local lib_name=$(basename "$lib")
        
        # Change the library path to use @rpath
        install_name_tool -change "$lib" "@rpath/$lib_name" "$binary"
    done
    
    # Add the frameworks directory to the rpath
    install_name_tool -add_rpath "@executable_path/../Frameworks" "$binary"
}

# Fix paths for arm64 build
if [ -d "build/arm64" ]; then
    echo "Fixing paths for arm64 build..."
    for binary in build/arm64/bin/*; do
        if [ -f "$binary" ] && [ -x "$binary" ]; then
            fix_library_paths "$binary" "@executable_path/../Frameworks"
        fi
    done
fi

# Fix paths for x86_64 build
if [ -d "build/x86_64" ]; then
    echo "Fixing paths for x86_64 build..."
    for binary in build/x86_64/bin/*; do
        if [ -f "$binary" ] && [ -x "$binary" ]; then
            fix_library_paths "$binary" "@executable_path/../Frameworks"
        fi
    done
fi

echo "Library paths fixed successfully!" 