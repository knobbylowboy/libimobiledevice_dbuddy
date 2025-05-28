#!/bin/bash

# Script to fix library paths in macOS binaries to use relative paths

set -e

# Function to fix library paths for a binary
fix_library_paths() {
    local binary=$1
    
    echo "Fixing library paths for $binary"
    
    # Get the list of libraries
    local libs=$(otool -L "$binary" | grep -v "$binary" | grep -v "/usr/lib" | grep -v "/System/Library" | awk '{print $1}')
    
    for lib in $libs; do
        # Get the library name
        local lib_name=$(basename "$lib")
        
        # Change the library path to use @rpath
        install_name_tool -change "$lib" "@rpath/$lib_name" "$binary"
    done
    
    # Check if rpath already exists
    if ! otool -l "$binary" | grep -q "@executable_path/../Frameworks"; then
        # Add the frameworks directory to the rpath only if it doesn't exist
        install_name_tool -add_rpath "@executable_path/../Frameworks" "$binary"
    fi
}

# Paths for the Flutter/macOS app
RESOURCES_DIR="/Users/sethbell/Dev/dispute_buddy/macos/Runner/Resources"

# Specific files to process
FILES=("idevice_id" "ideviceinfo" "idevicebackup2")

echo "Fixing library paths for specific files..."
for file in "${FILES[@]}"; do
    filepath="$RESOURCES_DIR/$file"
    if [ -f "$filepath" ] && [ -x "$filepath" ]; then
        fix_library_paths "$filepath"
    else
        echo "Warning: $file not found or not executable"
    fi
done

echo "Library paths fixed successfully!" 