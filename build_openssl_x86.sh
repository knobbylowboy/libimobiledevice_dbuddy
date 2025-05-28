#!/bin/bash

# Exit on error
set -e

# Create build directory
mkdir -p openssl_build
cd openssl_build

# Download OpenSSL 3.1.1 (or latest 3.x version)
curl -LO https://www.openssl.org/source/openssl-3.1.1.tar.gz
tar xzf openssl-3.1.1.tar.gz
cd openssl-3.1.1

# Configure for x86_64 architecture
./Configure darwin64-x86_64-cc \
    --prefix=$(pwd)/install \
    --openssldir=$(pwd)/install/openssl \
    shared \
    enable-ec_nistp_64_gcc_128 \
    no-ssl3 \
    no-weak-ssl-ciphers

# Build and install
make -j$(sysctl -n hw.ncpu)
make install

echo "OpenSSL has been built and installed to $(pwd)/install"
echo "The x86_64 version of libssl.3.dylib can be found at:"
echo "$(pwd)/install/lib/libssl.3.dylib" 