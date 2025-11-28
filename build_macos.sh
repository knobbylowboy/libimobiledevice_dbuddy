#!/bin/bash

# Script to build libimobiledevice for macOS on both x86_64 and arm64 architectures

set -x  # Print commands and their arguments as they are executed
# Note: We don't use 'set -e' because we want to continue even if x86_64 build fails

# Check if Homebrew is installed
if ! command -v brew &> /dev/null; then
    echo "Homebrew is required but not installed. Please install Homebrew first."
    exit 1
fi

# Get Homebrew prefix
BREW_PREFIX=$(brew --prefix)
echo "Homebrew prefix: $BREW_PREFIX"

# Detect system architecture
SYSTEM_ARCH=$(uname -m)
echo "System architecture: $SYSTEM_ARCH"

# On arm64 systems, skip x86_64 build by default unless explicitly requested
# Set BUILD_X86_64=1 environment variable to force x86_64 build
if [ "$SYSTEM_ARCH" = "arm64" ]; then
    if [ "${BUILD_X86_64:-0}" = "1" ]; then
        echo "x86_64 build explicitly requested, will attempt cross-compilation..."
        BUILD_X86_64=1
    else
        echo "On arm64 system, skipping x86_64 build (set BUILD_X86_64=1 to enable)"
        BUILD_X86_64=0
    fi
else
    # On x86_64 systems, build both architectures
    BUILD_X86_64=1
fi

# Create build directories
mkdir -p build/arm64
if [ $BUILD_X86_64 -eq 1 ]; then
    mkdir -p build/x86_64
fi

# Clean any previous build artifacts
if [ -f Makefile ]; then
    make clean || true
fi

# Function to check for required dependencies
check_deps() {
    local missing_deps=()
    
    # Check for pkg-config separately because it might be installed via different means
    if ! command -v pkg-config &> /dev/null; then
        echo "ERROR: pkg-config is required but not installed or not in PATH."
        echo "Please install it with: brew install pkg-config"
        exit 1
    fi
    
    # Check for other Homebrew packages
    for pkg in autoconf automake libtool libplist libusbmuxd libimobiledevice-glue libtatsu "openssl@3"; do
        if ! brew list --formula | grep -q "^${pkg}$"; then
            missing_deps+=("$pkg")
        fi
    done
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        echo "Missing dependencies: ${missing_deps[*]}"
        echo "Please install them with: brew install ${missing_deps[*]}"
        exit 1
    fi
}

# Check for required dependencies
check_deps

# Make sure autogen.sh is executable
chmod +x ./autogen.sh

# Build for x86_64 if supported
if [ $BUILD_X86_64 -eq 1 ]; then
    echo "Building for x86_64 architecture..."
    export PKG_CONFIG_PATH="$BREW_PREFIX/lib/pkgconfig"
    export CFLAGS="-arch x86_64 -mmacosx-version-min=10.15"
    export LDFLAGS="-arch x86_64 -L$BREW_PREFIX/lib"
    export CPPFLAGS="-I$BREW_PREFIX/include"

    ./autogen.sh --prefix="$(pwd)/build/x86_64" --with-openssl
    if ! make -j$(sysctl -n hw.ncpu); then
        echo "Warning: x86_64 build failed. Continuing with arm64 build only..."
        make clean || true
    else
        make install
        make clean
    fi

    # Reset environment variables
    unset CFLAGS
    unset LDFLAGS
    unset CPPFLAGS
fi

# Build for arm64
echo "Building for arm64 architecture..."
export PKG_CONFIG_PATH="$BREW_PREFIX/lib/pkgconfig"
export CFLAGS="-arch arm64 -mmacosx-version-min=11.0"
export LDFLAGS="-arch arm64 -L$BREW_PREFIX/lib"
export CPPFLAGS="-I$BREW_PREFIX/include"

./autogen.sh --prefix="$(pwd)/build/arm64" --with-openssl
make -j$(sysctl -n hw.ncpu)
make install
make clean

echo "Build completed successfully!"
if [ $BUILD_X86_64 -eq 1 ]; then
    echo "x86_64 build can be found in: $(pwd)/build/x86_64"
fi
echo "arm64 build can be found in: $(pwd)/build/arm64"

# Reset environment variables
unset PKG_CONFIG_PATH
unset CFLAGS
unset LDFLAGS
unset CPPFLAGS

# Make the fix script executable and run it
chmod +x ./fix_library_paths.sh
./fix_library_paths.sh 