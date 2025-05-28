#!/bin/bash

# Exit on error
set -e

# Create build directory
mkdir -p libplist_build
cd libplist_build

# Clone libplist repository
git clone https://github.com/libimobiledevice/libplist.git
cd libplist

# Configure for x86_64 architecture
./autogen.sh
./configure \
    --prefix=$(pwd)/install \
    --host=x86_64-apple-darwin \
    CFLAGS="-arch x86_64" \
    CXXFLAGS="-arch x86_64" \
    LDFLAGS="-arch x86_64"

# Build and install
make -j$(sysctl -n hw.ncpu)
make install

echo "libplist has been built and installed to $(pwd)/install"
echo "The x86_64 version of libplist-2.0.4.dylib can be found at:"
echo "$(pwd)/install/lib/libplist-2.0.4.dylib" 