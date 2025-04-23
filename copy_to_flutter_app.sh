#!/bin/bash

# Script to copy built binaries and libraries to Flutter app

set -e

# Path to Flutter app
FLUTTER_APP_PATH="/Users/sethbell/Dev/dispute_buddy/macos/Runner"

# List of binaries to copy
BINARIES=("idevicebackup2" "ideviceinfo" "idevice_id" "ideviceinstaller" "ideviceprovision" "idevicescreenshot" "idevicesyslog")

# Check if Flutter app directory exists
if [ ! -d "$FLUTTER_APP_PATH" ]; then
    echo "Error: Flutter app directory not found at $FLUTTER_APP_PATH"
    exit 1
fi

# Create necessary directories if they don't exist
sudo mkdir -p "$FLUTTER_APP_PATH/Resources"
sudo mkdir -p "$FLUTTER_APP_PATH/Frameworks"

# Copy required libraries to Frameworks (only if not already present)
echo "Copying libraries to Frameworks..."
if [ ! -f "$FLUTTER_APP_PATH/Frameworks/libimobiledevice-1.0.6.dylib" ]; then
    sudo cp "build/arm64/lib/libimobiledevice-1.0.6.dylib" "$FLUTTER_APP_PATH/Frameworks/"
fi
if [ ! -f "$FLUTTER_APP_PATH/Frameworks/libusbmuxd-2.0.7.dylib" ]; then
    sudo cp "/opt/homebrew/lib/libusbmuxd-2.0.7.dylib" "$FLUTTER_APP_PATH/Frameworks/"
fi
if [ ! -f "$FLUTTER_APP_PATH/Frameworks/libimobiledevice-glue-1.0.0.dylib" ]; then
    sudo cp "/opt/homebrew/lib/libimobiledevice-glue-1.0.0.dylib" "$FLUTTER_APP_PATH/Frameworks/"
fi
if [ ! -f "$FLUTTER_APP_PATH/Frameworks/libplist-2.0.4.dylib" ]; then
    sudo cp "/opt/homebrew/lib/libplist-2.0.4.dylib" "$FLUTTER_APP_PATH/Frameworks/"
fi
if [ ! -f "$FLUTTER_APP_PATH/Frameworks/libssl.3.dylib" ]; then
    sudo cp "/opt/homebrew/opt/openssl@3/lib/libssl.3.dylib" "$FLUTTER_APP_PATH/Frameworks/"
fi
if [ ! -f "$FLUTTER_APP_PATH/Frameworks/libcrypto.3.dylib" ]; then
    sudo cp "/opt/homebrew/opt/openssl@3/lib/libcrypto.3.dylib" "$FLUTTER_APP_PATH/Frameworks/"
fi

# Copy and fix each binary
for binary in "${BINARIES[@]}"; do
    if [ -f "build/arm64/bin/$binary" ]; then
        echo "Copying $binary to Resources..."
        sudo cp "build/arm64/bin/$binary" "$FLUTTER_APP_PATH/Resources/"

        # Fix library paths in the binary
        echo "Fixing library paths in $binary..."
        sudo install_name_tool -change "/Users/sethbell/Dev/libimobiledevice_dbuddy/build/arm64/lib/libimobiledevice-1.0.6.dylib" "@rpath/libimobiledevice-1.0.6.dylib" "$FLUTTER_APP_PATH/Resources/$binary"
        sudo install_name_tool -change "/opt/homebrew/opt/openssl@3/lib/libssl.3.dylib" "@rpath/libssl.3.dylib" "$FLUTTER_APP_PATH/Resources/$binary"
        sudo install_name_tool -change "/opt/homebrew/opt/openssl@3/lib/libcrypto.3.dylib" "@rpath/libcrypto.3.dylib" "$FLUTTER_APP_PATH/Resources/$binary"
        sudo install_name_tool -change "/opt/homebrew/lib/libusbmuxd-2.0.7.dylib" "@rpath/libusbmuxd-2.0.7.dylib" "$FLUTTER_APP_PATH/Resources/$binary"
        sudo install_name_tool -change "/opt/homebrew/lib/libimobiledevice-glue-1.0.0.dylib" "@rpath/libimobiledevice-glue-1.0.0.dylib" "$FLUTTER_APP_PATH/Resources/$binary"
        sudo install_name_tool -change "/opt/homebrew/lib/libplist-2.0.4.dylib" "@rpath/libplist-2.0.4.dylib" "$FLUTTER_APP_PATH/Resources/$binary"
    else
        echo "Warning: $binary not found in build directory"
    fi
done

echo "Files copied and paths fixed successfully!" 