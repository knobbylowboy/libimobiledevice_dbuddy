@echo off
REM Build script for mock_idevicebackup2_win.exe on Windows
REM This script tries different compilers in order of preference

echo Building mock_idevicebackup2_win.exe...

REM Try to use Visual Studio compiler if available
where cl >nul 2>&1
if %errorlevel% == 0 (
    echo Using Visual Studio compiler...
    cl /W3 /O2 /Fe:mock_idevicebackup2_win.exe mock_idevicebackup2_win.c
    if %errorlevel% == 0 (
        echo Build successful with Visual Studio compiler!
        goto :success
    )
)

REM Try to use MinGW-w64 if available
where x86_64-w64-mingw32-gcc >nul 2>&1
if %errorlevel% == 0 (
    echo Using MinGW-w64 compiler...
    x86_64-w64-mingw32-gcc -Wall -Wextra -std=c99 -O2 -o mock_idevicebackup2_win.exe mock_idevicebackup2_win.c
    if %errorlevel% == 0 (
        echo Build successful with MinGW-w64 compiler!
        goto :success
    )
)

REM Try to use regular MinGW if available
where gcc >nul 2>&1
if %errorlevel% == 0 (
    echo Using MinGW GCC compiler...
    gcc -Wall -Wextra -std=c99 -O2 -o mock_idevicebackup2_win.exe mock_idevicebackup2_win.c
    if %errorlevel% == 0 (
        echo Build successful with MinGW GCC compiler!
        goto :success
    )
)

REM Try to use MSYS2 MinGW if available
where mingw64-gcc >nul 2>&1
if %errorlevel% == 0 (
    echo Using MSYS2 MinGW compiler...
    mingw64-gcc -Wall -Wextra -std=c99 -O2 -o mock_idevicebackup2_win.exe mock_idevicebackup2_win.c
    if %errorlevel% == 0 (
        echo Build successful with MSYS2 MinGW compiler!
        goto :success
    )
)

echo ERROR: No suitable C compiler found!
echo.
echo Please install one of the following:
echo   - Visual Studio Build Tools
echo   - MinGW-w64
echo   - MSYS2 with MinGW-w64
echo.
goto :end

:success
echo.
echo mock_idevicebackup2_win.exe has been built successfully!
echo You can now run it with: mock_idevicebackup2_win.exe --help

:end
pause 