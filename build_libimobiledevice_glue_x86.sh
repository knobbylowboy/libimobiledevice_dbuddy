#!/bin/bash

# Exit on error
set -e

# Create build directory
mkdir -p libimobiledevice_glue_build
cd libimobiledevice_glue_build

# Clone libimobiledevice-glue repository
git clone https://github.com/libimobiledevice/libimobiledevice-glue.git
cd libimobiledevice-glue

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

echo "libimobiledevice-glue has been built and installed to $(pwd)/install"
echo "The x86_64 version of libimobiledevice-glue-1.0.0.dylib can be found at:"
echo "$(pwd)/install/lib/libimobiledevice-glue-1.0.0.dylib" 