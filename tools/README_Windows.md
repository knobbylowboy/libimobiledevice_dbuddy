# Windows Version of mock_idevicebackup2

This is a Windows-compatible replica of the `mock_idevicebackup2` tool that simulates iOS device backup operations.

## Files

- `mock_idevicebackup2_win.c` - Source code for the Windows executable
- `mock_idevicebackup2_win.exe` - Compiled Windows executable
- `Makefile.win` - Makefile for cross-compilation on macOS/Linux
- `build_win.bat` - Windows batch file for building on Windows
- `build_win.ps1` - PowerShell script for building on Windows

## Building on Windows

### Option 1: Using PowerShell (Recommended)
```powershell
.\build_win.ps1
```

### Option 2: Using Batch File
```cmd
build_win.bat
```

### Option 3: Manual Compilation

#### With Visual Studio:
```cmd
cl /W3 /O2 /Fe:mock_idevicebackup2_win.exe mock_idevicebackup2_win.c
```

#### With MinGW-w64:
```cmd
x86_64-w64-mingw32-gcc -Wall -Wextra -std=c99 -O2 -o mock_idevicebackup2_win.exe mock_idevicebackup2_win.c
```

#### With regular MinGW:
```cmd
gcc -Wall -Wextra -std=c99 -O2 -o mock_idevicebackup2_win.exe mock_idevicebackup2_win.c
```

## Building on macOS/Linux (Cross-compilation)

### Prerequisites
Install MinGW-w64:
```bash
# macOS
brew install mingw-w64

# Ubuntu/Debian
sudo apt-get install mingw-w64
```

### Build
```bash
make -f Makefile.win
```

## Usage

The Windows version has the same command-line interface as the original Unix version:

```cmd
mock_idevicebackup2_win.exe [OPTIONS]
```

### Options

- `-u, --udid UDID` - Target specific device by UDID
- `-n, --network` - Connect to network device
- `-i, --info CMD` - Info command (e.g. 'encryption off')
- `-s, --scenario N` - Use specific scenario (1-3)
- `-t, --time N` - Total execution time in seconds
- `-c, --code N` - Exit with code N
- `-d, --delay N` - Wait N seconds before starting
- `-h, --help` - Print usage information
- `-v, --version` - Print version information

### Scenarios

1. **Encryption disabled successfully** - Backup encryption has been disabled successfully
2. **Encryption not enabled** - Backup encryption is not enabled
3. **Device locked** - Password protected

### Examples

```cmd
# Show help
mock_idevicebackup2_win.exe --help

# Show version
mock_idevicebackup2_win.exe --version

# Run with default settings (80 seconds)
mock_idevicebackup2_win.exe

# Run with custom time (30 seconds)
mock_idevicebackup2_win.exe --time 30

# Test encryption off with scenario 1
mock_idevicebackup2_win.exe --info "encryption off" --scenario 1

# Run with initial delay
mock_idevicebackup2_win.exe --delay 5
```

## Differences from Unix Version

1. **Sleep function**: Uses Windows `Sleep()` instead of Unix `sleep()`
2. **Command-line parsing**: Custom argument parser instead of `getopt_long()`
3. **Path handling**: Uses Windows-style paths and backslashes
4. **Environment variables**: Uses `USERNAME` instead of Unix user detection
5. **File buffering**: Maintains the same line buffering behavior

## Requirements

- Windows 7 or later
- No additional runtime dependencies (statically linked)

## Troubleshooting

### "No suitable C compiler found"
Install one of the following:
- Visual Studio Build Tools
- MinGW-w64
- MSYS2 with MinGW-w64
- TDM-GCC

### "Access denied" when running
- Right-click the executable and select "Run as administrator"
- Or run from an elevated command prompt

### Build errors
- Ensure you're running from the correct directory
- For Visual Studio, use the Developer Command Prompt
- For MinGW, ensure the compiler is in your PATH 