#!/bin/bash

# Exit on error
set -e

# Create build directory
mkdir -p libusbmuxd_build
cd libusbmuxd_build

# Clone libusbmuxd repository
git clone https://github.com/libimobiledevice/libusbmuxd.git
cd libusbmuxd

# Configure for x86_64 architecture
./autogen.sh
./configure \
    --prefix=$(pwd)/install \
    --host=x86_64-apple-darwin \
    CFLAGS="-arch x86_64" \
    LDFLAGS="-arch x86_64"

# Build and install
make -j$(sysctl -n hw.ncpu)
make install

echo "libusbmuxd has been built and installed to $(pwd)/install"
echo "The x86_64 version of libusbmuxd-2.0.7.dylib can be found at:"
echo "$(pwd)/install/lib/libusbmuxd-2.0.7.dylib" 