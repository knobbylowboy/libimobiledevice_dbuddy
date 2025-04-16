#!/bin/bash

# Script to check if all dependencies for building libimobiledevice are properly installed

set -e

echo "Checking dependencies for libimobiledevice build..."

# Check if Homebrew is installed
if ! command -v brew &> /dev/null; then
    echo "ERROR: Homebrew is required but not installed."
    echo "Please install Homebrew first: https://brew.sh/"
    exit 1
fi

# Get Homebrew prefix
BREW_PREFIX=$(brew --prefix)
echo "Homebrew prefix: $BREW_PREFIX"

# Check for pkg-config directly
if command -v pkg-config &> /dev/null; then
    echo "✓ pkg-config is available ($(which pkg-config))"
else
    echo "ERROR: pkg-config is required but not installed or not in PATH."
    echo "Please install it with: brew install pkg-config"
    exit 1
fi

# Required packages
REQUIRED_PACKAGES=(
    "autoconf"
    "automake"
    "libtool"
    "libplist"
    "libusbmuxd"
    "libimobiledevice-glue"
    "libtatsu"
    "openssl@3"
)

# Check for each required package
MISSING_PACKAGES=()
for package in "${REQUIRED_PACKAGES[@]}"; do
    if ! brew list --formula | grep -q "^${package}$"; then
        MISSING_PACKAGES+=("$package")
    else
        echo "✓ $package is installed"
    fi
done

# Report missing packages
if [ ${#MISSING_PACKAGES[@]} -ne 0 ]; then
    echo "ERROR: Missing dependencies: ${MISSING_PACKAGES[*]}"
    echo "Please install them with: brew install ${MISSING_PACKAGES[*]}"
    exit 1
fi

# Check for required libraries with pkg-config
REQUIRED_LIBS=(
    "libplist-2.0"
    "libusbmuxd-2.0"
    "libimobiledevice-glue-1.0"
    "libtatsu-1.0"
)

for lib in "${REQUIRED_LIBS[@]}"; do
    if ! pkg-config --exists "$lib"; then
        echo "ERROR: $lib not found with pkg-config."
        echo "Make sure the library is installed and pkg-config is properly configured."
        exit 1
    else
        echo "✓ $lib found with pkg-config"
        # Show lib version
        VERSION=$(pkg-config --modversion "$lib")
        echo "  Version: $VERSION"
        # Show lib path
        LIBPATH=$(pkg-config --variable=libdir "$lib")
        echo "  Library path: $LIBPATH"
    fi
done

# Check for OpenSSL
if ! pkg-config --exists openssl; then
    echo "WARNING: openssl not found with pkg-config."
    echo "Make sure OpenSSL is installed and pkg-config is properly configured."
else
    echo "✓ openssl found with pkg-config"
    # Show OpenSSL version
    VERSION=$(pkg-config --modversion openssl)
    echo "  Version: $VERSION"
    # Show OpenSSL path
    LIBPATH=$(pkg-config --variable=libdir openssl)
    echo "  Library path: $LIBPATH"
fi

# Check for Xcode Command Line Tools
if ! xcode-select -p &> /dev/null; then
    echo "ERROR: Xcode Command Line Tools are not installed."
    echo "Please install them with: xcode-select --install"
    exit 1
fi
echo "✓ Xcode Command Line Tools are installed"

# Check for architectures in a critical library
echo "Checking architectures in libraries..."
for lib in "${REQUIRED_LIBS[@]}"; do
    if pkg-config --exists "$lib"; then
        LIBPATH=$(pkg-config --variable=libdir "$lib")
        if [ -d "$LIBPATH" ]; then
            # Find first .dylib in the directory that matches the lib name
            DYLIB=$(find "$LIBPATH" -name "lib${lib%%%-*}*.dylib" -o -name "lib${lib%%%-*}*.a" | head -n 1)
            if [ -n "$DYLIB" ]; then
                echo "Checking architectures in $DYLIB:"
                lipo -info "$DYLIB"
            fi
        fi
    fi
done

echo "All required dependencies are installed."
echo "You should be able to build libimobiledevice for macOS." 