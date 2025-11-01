# PowerShell build script for mock_idevicebackup2_win.exe
# This script tries different compilers in order of preference

Write-Host "Building mock_idevicebackup2_win.exe..." -ForegroundColor Green

# Function to test if a command exists
function Test-Command($cmdname) {
    return [bool](Get-Command -Name $cmdname -ErrorAction SilentlyContinue)
}

# Function to build with a specific compiler
function Build-WithCompiler($compiler, $args, $description) {
    Write-Host "Trying $description..." -ForegroundColor Yellow
    
    if (Test-Command $compiler) {
        try {
            $process = Start-Process -FilePath $compiler -ArgumentList $args -Wait -PassThru -NoNewWindow
            if ($process.ExitCode -eq 0) {
                Write-Host "Build successful with $description!" -ForegroundColor Green
                return $true
            } else {
                Write-Host "Build failed with $description (exit code: $($process.ExitCode))" -ForegroundColor Red
            }
        } catch {
            Write-Host "Error running $description: $($_.Exception.Message)" -ForegroundColor Red
        }
    } else {
        Write-Host "$description not found" -ForegroundColor Gray
    }
    return $false
}

# Try Visual Studio compiler
$vsArgs = @("/W3", "/O2", "/Fe:mock_idevicebackup2_win.exe", "mock_idevicebackup2_win.c")
if (Build-WithCompiler "cl" $vsArgs "Visual Studio compiler") {
    goto success
}

# Try MinGW-w64
$mingw64Args = @("-Wall", "-Wextra", "-std=c99", "-O2", "-o", "mock_idevicebackup2_win.exe", "mock_idevicebackup2_win.c")
if (Build-WithCompiler "x86_64-w64-mingw32-gcc" $mingw64Args "MinGW-w64 compiler") {
    goto success
}

# Try regular MinGW GCC
if (Build-WithCompiler "gcc" $mingw64Args "MinGW GCC compiler") {
    goto success
}

# Try MSYS2 MinGW
if (Build-WithCompiler "mingw64-gcc" $mingw64Args "MSYS2 MinGW compiler") {
    goto success
}

# Try TDM-GCC
if (Build-WithCompiler "tdm-gcc" $mingw64Args "TDM-GCC compiler") {
    goto success
}

Write-Host "`nERROR: No suitable C compiler found!" -ForegroundColor Red
Write-Host "`nPlease install one of the following:" -ForegroundColor Yellow
Write-Host "  - Visual Studio Build Tools" -ForegroundColor White
Write-Host "  - MinGW-w64" -ForegroundColor White
Write-Host "  - MSYS2 with MinGW-w64" -ForegroundColor White
Write-Host "  - TDM-GCC" -ForegroundColor White
Write-Host "`nFor Visual Studio, run from Developer Command Prompt" -ForegroundColor Cyan
Write-Host "For MinGW/MSYS2, ensure the compiler is in your PATH" -ForegroundColor Cyan
exit 1

:success
Write-Host "`nmock_idevicebackup2_win.exe has been built successfully!" -ForegroundColor Green
Write-Host "You can now run it with: .\mock_idevicebackup2_win.exe --help" -ForegroundColor Cyan
exit 0 