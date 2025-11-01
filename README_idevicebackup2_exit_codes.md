# idevicebackup2 Exit Codes Reference

This document provides a comprehensive reference for all exit codes returned by `idevicebackup2` when using the backup command with the `--full` flag.

## Command Syntax
```bash
idevicebackup2 backup --full [pathtobackup]
```

## Exit Code Categories

### Success Codes
| Exit Code | Meaning | Description |
|-----------|---------|-------------|
| `0` | Success | Backup operation completed successfully |

### Argument/Usage Errors (Exit Code 2)
These errors occur when the command is invoked incorrectly:

| Exit Code | Condition | Error Message |
|-----------|-----------|---------------|
| `2` | Empty UDID argument | `ERROR: UDID argument must not be empty!` |
| `2` | Empty SOURCE argument | `ERROR: SOURCE argument must not be empty!` |
| `2` | No command specified | `ERROR: No command specified.` |
| `2` | Invalid command | `ERROR: Unknown command '[command]'` |
| `2` | Missing backup directory | `ERROR: No target backup directory specified.` |
| `2` | Invalid command line options | Various usage-related error messages |

### Pre-Operation Failures (Exit Code -1)
These errors occur before the backup operation can begin:

| Exit Code | Condition | Error Message | Common Causes |
|-----------|-----------|---------------|---------------|
| `-1` | Backup directory doesn't exist | `ERROR: Backup directory "[path]" does not exist!` | • Typo in path<br>• Directory not created<br>• Incorrect permissions |
| `-1` | No device found | `No device found.` or `No device found with udid [udid].` | • Device not connected<br>• Device not trusted<br>• USB cable issues<br>• Device in wrong mode |
| `-1` | Password required in non-interactive mode | `ERROR: Can't get password input in non-interactive mode...` | • Encrypted backup requires password<br>• Not using `-i` flag<br>• No `BACKUP_PASSWORD` env var |
| `-1` | Invalid backup directory for restore | `ERROR: Backup directory "[path]" is invalid. No Info.plist found...` | • Corrupted backup directory<br>• Wrong directory specified |
| `-1` | Missing Manifest.plist | `ERROR: Backup directory "[path]" is invalid. No Manifest.plist found...` | • Incomplete backup<br>• Corrupted backup files |
| `-1` | Missing backup password | `ERROR: a backup password is required...` | • Encrypted backup<br>• No password provided<br>• Wrong password format |
| `-1` | Cannot connect to lockdownd | `ERROR: Could not connect to lockdownd, error code [code]` | • Device pairing issues<br>• lockdownd service problems<br>• Device communication failure |

### File System Error Codes (Mapped from errno)
These errors are mapped from system errno values using the `errno_to_device_error()` function:

| Exit Code | System Error | Description | Common Causes |
|-----------|--------------|-------------|---------------|
| `-6` | ENOENT | File or directory not found | • Missing backup files<br>• Corrupted backup directory |
| `-7` | EEXIST | File already exists | • Backup file conflicts<br>• Directory creation issues |
| `-8` | ENOTDIR | Not a directory | • Path component is not directory<br>• File system structure issues |
| `-9` | EISDIR | Is a directory | • Expected file but found directory<br>• Path resolution problems |
| `-10` | ELOOP | Too many symbolic links | • Circular symbolic links<br>• File system loop detected |
| `-11` | EIO | Input/output error | • Hardware failure<br>• Disk read/write errors<br>• USB connection issues<br>• Storage device problems |
| `-15` | ENOSPC | No space left on device | • Insufficient disk space<br>• Backup destination full |

### Operation Runtime Errors (Device Error Codes)
These errors occur during the backup operation and are returned as negative values of the device error codes:

| Exit Code | Device Error | Description | Common Causes |
|-----------|--------------|-------------|---------------|
| `-1` | Error Code 1 | Generic device error | • Device-specific issues<br>• Temporary device problems |
| `-2` | Error Code 2 | Device communication error | • Connection interrupted<br>• Device disconnected during backup |
| `-3` | Error Code 3 | Device storage error | • Insufficient device storage<br>• File system errors |
| `-4` | Error Code 4 | Device permission error | • Backup restrictions<br>• Device security policies |
| `-5` | Error Code 5 | Device timeout error | • Device became unresponsive<br>• Network timeout (if using network mode) |
| `-48` | Error Code 48 | Device-specific error | • iOS version compatibility issues<br>• Device firmware problems<br>• Backup protocol conflicts |
| `...` | Error Codes 6-254 | Various device-specific errors | • Depends on specific error code<br>• Check device logs for details |
| `-255` | Error Code 255 | Unknown/unspecified device error | • Unexpected device failure<br>• Protocol version mismatch<br>• Device firmware issues |

### Special Exit Codes

| Exit Code | Condition | Description |
|-----------|-----------|-------------|
| `-6` | Protocol version mismatch | `ERROR: Could not start backup process: backup protocol version mismatch!` |
| `-7` | Device refused backup | `ERROR: Could not start backup process: device refused to start the backup process.` |
| `-256` | Unknown mobilebackup2 error | `ERROR: Could not start backup process: unspecified error occurred` |

## Signal Interruption
If the backup process is interrupted by a signal (SIGINT, SIGTERM, SIGQUIT), the tool will:
- Set `quit_flag` to indicate interruption
- Display "Backup Aborted." message
- Return the current `result_code` (typically `-1` if no other error occurred)

## Environment Variables
The following environment variables can affect exit codes:

- `BACKUP_PASSWORD`: Used for encrypted backups to avoid interactive password prompts

## Common Exit Code Scenarios

### Successful Backup
```bash
$ idevicebackup2 backup --full /path/to/backup
Full backup mode.
[==================================================] 100%
Received 1234 files from device.
Backup Successful.
$ echo $?
0
```

### Device Not Found
```bash
$ idevicebackup2 backup --full /path/to/backup
No device found.
$ echo $?
255  # (Shell converts -1 to 255)
```

### Invalid Backup Directory
```bash
$ idevicebackup2 backup --full /nonexistent/path
ERROR: Backup directory "/nonexistent/path" does not exist!
$ echo $?
255  # (Shell converts -1 to 255)
```

### Missing Command Arguments
```bash
$ idevicebackup2 backup --full
ERROR: No target backup directory specified.
$ echo $?
2
```

### Device Error During Backup
```bash
$ idevicebackup2 backup --full /path/to/backup
Full backup mode.
[===============                     ] 35%
ErrorCode 13: Insufficient storage space
Backup Failed (Error Code 13).
$ echo $?
243  # (Shell converts -13 to 243)
```

### I/O Error During Backup (Exit Code -11)
```bash
$ idevicebackup2 backup --full /path/to/backup
Full backup mode.
[========                            ] 20%
ErrorCode 11: Input/output error
Backup Failed (Error Code 11).
$ echo $?
245  # (Shell converts -11 to 245)
```

### Device-Specific Error (Exit Code 48)
```bash
$ idevicebackup2 backup --full /path/to/backup
Full backup mode.
ErrorCode 48: Device-specific error
Backup Failed (Error Code 48).
$ echo $?
208  # (Shell converts -48 to 208)
```

### Unknown Device Error (Exit Code 255)
```bash
$ idevicebackup2 backup --full /path/to/backup
Full backup mode.
[==================                  ] 45%
ErrorCode 255: (Unknown)
Backup Failed (Error Code 255).
$ echo $?
1    # (Shell converts -255 to 1)
```

## Troubleshooting Guide

### Exit Code -1 (appears as 255)
1. **Check device connection**: Ensure device is connected and trusted
2. **Verify backup directory**: Make sure the directory exists and is writable
3. **Check device pairing**: Try `idevicepair pair` if connection issues persist
4. **Verify passwords**: For encrypted backups, ensure password is provided

### Exit Code 2
1. **Check command syntax**: Ensure all required arguments are provided
2. **Verify paths**: Make sure backup directory path is correct
3. **Check command spelling**: Ensure 'backup' and '--full' are spelled correctly

### Exit Code -11 (appears as 245) - I/O Error
1. **Check USB connection**: Try different USB cable or port
2. **Check disk health**: Run disk utility to check for bad sectors
3. **Free up space**: Ensure both device and backup destination have adequate space
4. **Restart both devices**: Restart both computer and iOS device
5. **Check hardware**: Test with different storage device if using external drive

### Exit Code 48 (appears as 208) - Device-Specific Error
1. **Update iOS**: Ensure device is running compatible iOS version
2. **Update libimobiledevice**: Use latest version for better device support
3. **Check device restrictions**: Verify backup restrictions aren't enabled
4. **Try different backup method**: Consider using iTunes/Finder backup to isolate issue
5. **Reset device settings**: Reset network and backup settings if persistent

### Exit Codes -2 to -255 (appears as 254 to 1)
1. **Check device storage**: Ensure device has sufficient free space
2. **Check host storage**: Ensure backup directory has enough space
3. **Restart device**: Sometimes resolves temporary device issues
4. **Try different USB port/cable**: Hardware issues can cause various errors
5. **Check device logs**: Use Console.app (macOS) or device logs for more details

## Exit Code Conversion
Note that shells typically convert negative exit codes to positive values by adding 256:
- `-1` becomes `255`
- `-13` becomes `243`
- `-255` becomes `1`

To get the actual error code from the shell exit code:
```bash
exit_code=$?
if [ $exit_code -gt 128 ]; then
    actual_error=$((exit_code - 256))
    echo "Device error code: ${actual_error#-}"
fi
```

## See Also
- `idevicebackup2 --help` - Command usage information
- `man idevicebackup2` - Manual page (if available)
- Device system logs for detailed error information
