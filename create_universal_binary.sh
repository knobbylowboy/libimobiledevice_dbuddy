#!/bin/bash

# Script to create universal binaries from separate x86_64 and arm64 builds

set -e  # Exit immediately if a command exits with a non-zero status
set -x  # Print commands and their arguments as they are executed

# Check if the arm64 build exists
if [ ! -d "build/arm64" ]; then
    echo "Error: arm64 build not found."
    echo "Please run ./build_macos.sh first to create the arm64 build."
    exit 1
fi

# Check if the x86_64 build exists
HAVE_X86_64=0
if [ -d "build/x86_64" ]; then
    HAVE_X86_64=1
fi

# Create universal build directory
mkdir -p build/universal

# Copy the directory structure from arm64 build
echo "Creating directory structure..."
cp -R build/arm64/* build/universal/

# If we don't have x86_64 build, just inform the user and exit
if [ $HAVE_X86_64 -eq 0 ]; then
    echo "Note: Only arm64 build was found. The 'universal' build will only contain arm64 binaries."
    echo "Universal binaries (arm64 only) can be found in: $(pwd)/build/universal"
    exit 0
fi

# Process directories to create universal binaries
process_dir() {
    local dir=$1
    local rel_dir=${dir#build/universal/}
    
    # Process libraries (.dylib and .a files)
    for file in "$dir"/*.dylib "$dir"/*.a; do
        if [ -f "$file" ]; then
            echo "Creating universal binary for $(basename "$file")..."
            x86_file="build/x86_64/$rel_dir/$(basename "$file")"
            arm64_file="build/arm64/$rel_dir/$(basename "$file")"
            if [ -f "$x86_file" ] && [ -f "$arm64_file" ]; then
                lipo -create "$x86_file" "$arm64_file" -output "$file"
            fi
        fi
    done
    
    # Process executables
    for file in "$dir"/*; do
        if [ -f "$file" ] && [ -x "$file" ] && ! [[ "$file" == *.dylib ]] && ! [[ "$file" == *.a ]]; then
            # Check if it's a Mach-O file (executable)
            if file "$file" | grep -q "Mach-O"; then
                echo "Creating universal binary for $(basename "$file")..."
                x86_file="build/x86_64/$rel_dir/$(basename "$file")"
                arm64_file="build/arm64/$rel_dir/$(basename "$file")"
                if [ -f "$x86_file" ] && [ -f "$arm64_file" ]; then
                    lipo -create "$x86_file" "$arm64_file" -output "$file"
                fi
            fi
        fi
    done
    
    # Process subdirectories
    for subdir in "$dir"/*/; do
        if [ -d "$subdir" ]; then
            process_dir "$subdir"
        fi
    done
}

# Start processing from the build/universal directory
echo "Creating universal binaries..."
process_dir "build/universal/bin"
process_dir "build/universal/lib"

echo "Universal build created successfully!"
echo "Universal binaries can be found in: $(pwd)/build/universal" 