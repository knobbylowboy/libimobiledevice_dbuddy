# Building libimobiledevice for macOS

This README provides instructions for building libimobiledevice on macOS for both Intel (x86_64) and Apple Silicon (arm64) architectures.

## Prerequisites

Before building, make sure you have the following dependencies installed:

```bash
# Using Homebrew
brew install \
  autoconf \
  automake \
  libtool \
  pkg-config \
  libplist \
  libusbmuxd \
  libimobiledevice-glue \
  libtatsu \
  openssl@3
```

Ensure that you have Xcode Command Line Tools installed:

```bash
xcode-select --install
```

If you're building on Apple Silicon (M1/M2) and want to build x86_64 binaries, you'll need Rosetta 2:

```bash
softwareupdate --install-rosetta
```

## Checking Dependencies

Before attempting to build, you can use the included dependency checker script to verify that all required components are correctly installed:

```bash
./check_dependencies.sh
```

This script will:
- Check if all required Homebrew packages are installed
- Verify that pkg-config can find all necessary libraries
- Show the versions and paths of the libraries
- Check which architectures are supported in the installed libraries
- Confirm that Xcode Command Line Tools are installed

## Building for Specific Architectures

This repository includes two scripts to help with building:

1. `build_macos.sh`: Builds separate x86_64 and arm64 versions
2. `create_universal_binary.sh`: Creates universal binaries combining both architectures

### Building for Both Architectures Separately

Run:

```bash
./build_macos.sh
```

The script will:
- Check for required dependencies using Homebrew
- Create two build directories: `build/x86_64` and `build/arm64`
- Build libimobiledevice for each architecture with architecture-specific flags
- Install each build into its respective directory

When complete, you'll have:
- `build/x86_64`: Contains binaries that run on Intel Macs
- `build/arm64`: Contains binaries that run on Apple Silicon Macs

### Creating Universal Binaries

After running the architecture-specific build script, you can create universal binaries by running:

```bash
./create_universal_binary.sh
```

This will:
- Create a `build/universal` directory
- Copy the directory structure from one of the architecture builds
- Create universal binaries by combining the x86_64 and arm64 versions using `lipo`

The resulting `build/universal` directory will contain binaries that run natively on both Intel and Apple Silicon Macs.

## Installation

To install the universal binaries system-wide, you can run:

```bash
sudo cp -R build/universal/* /usr/local/
```

Or for a specific architecture:

```bash
# For Intel Macs
sudo cp -R build/x86_64/* /usr/local/

# For Apple Silicon Macs
sudo cp -R build/arm64/* /usr/local/
```

## Troubleshooting

### Missing Symbols Error

If you encounter the error `ld: symbol(s) not found for architecture x86_64` or similar, this usually indicates:

1. Missing dependencies
2. Architecture mismatch in dependencies
3. Wrong path to libraries

Solutions:
- Run `./check_dependencies.sh` to diagnose issues with dependencies
- Make sure all dependencies are installed: `brew install libplist libusbmuxd libimobiledevice-glue libtatsu openssl@3`
- Verify that the dependencies support the architecture you're building for
- Check if pkg-config is finding the right libraries: `pkg-config --libs libplist-2.0`

### General Troubleshooting

If you encounter other issues during the build:

1. Make sure all dependencies are properly installed and up to date: `brew update && brew upgrade`
2. Check that XCode Command Line Tools are installed: `xcode-select --install`
3. Clean the build environment and start fresh: `git clean -xdf` (Warning: This removes all untracked files)
4. If building on Apple Silicon, ensure Rosetta 2 is installed for building x86_64 binaries: `softwareupdate --install-rosetta`

## Notes

- The builds use `-mmacosx-version-min=10.15` for x86_64 and `-mmacosx-version-min=11.0` for arm64
- The universal binary combines both architectures for improved compatibility across Mac systems
- The build script explicitly uses OpenSSL for encryption support via the `--with-openssl` configure flag 