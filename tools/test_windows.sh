#!/bin/bash

# Test script for mock_idevicebackup2_win.exe
# This script tests various command-line options to ensure the Windows executable works correctly

echo "Testing mock_idevicebackup2_win.exe..."
echo "======================================"

# Check if executable exists
if [ ! -f "mock_idevicebackup2_win.exe" ]; then
    echo "ERROR: mock_idevicebackup2_win.exe not found!"
    echo "Please build it first using: make -f Makefile.win"
    exit 1
fi

# Test 1: Help
echo -e "\n1. Testing --help"
./mock_idevicebackup2_win.exe --help
if [ $? -eq 0 ]; then
    echo "✓ Help test passed"
else
    echo "✗ Help test failed"
fi

# Test 2: Version
echo -e "\n2. Testing --version"
./mock_idevicebackup2_win.exe --version
if [ $? -eq 0 ]; then
    echo "✓ Version test passed"
else
    echo "✗ Version test failed"
fi

# Test 3: Encryption off scenario 1
echo -e "\n3. Testing encryption off scenario 1"
./mock_idevicebackup2_win.exe --info "encryption off" --scenario 1
if [ $? -eq 0 ]; then
    echo "✓ Encryption off scenario 1 test passed"
else
    echo "✗ Encryption off scenario 1 test failed"
fi

# Test 4: Encryption off scenario 2
echo -e "\n4. Testing encryption off scenario 2"
./mock_idevicebackup2_win.exe --info "encryption off" --scenario 2
if [ $? -eq 0 ]; then
    echo "✓ Encryption off scenario 2 test passed"
else
    echo "✗ Encryption off scenario 2 test failed"
fi

# Test 5: Encryption off scenario 3
echo -e "\n5. Testing encryption off scenario 3"
./mock_idevicebackup2_win.exe --info "encryption off" --scenario 3
if [ $? -eq 1 ]; then
    echo "✓ Encryption off scenario 3 test passed (expected exit code 1)"
else
    echo "✗ Encryption off scenario 3 test failed"
fi

# Test 6: Custom exit code
echo -e "\n6. Testing custom exit code"
./mock_idevicebackup2_win.exe --code 42
if [ $? -eq 42 ]; then
    echo "✓ Custom exit code test passed"
else
    echo "✗ Custom exit code test failed"
fi

# Test 7: Invalid scenario
echo -e "\n7. Testing invalid scenario"
./mock_idevicebackup2_win.exe --scenario 99 2>/dev/null
if [ $? -eq 2 ]; then
    echo "✓ Invalid scenario test passed (expected exit code 2)"
else
    echo "✗ Invalid scenario test failed"
fi

# Test 8: Invalid time
echo -e "\n8. Testing invalid time"
./mock_idevicebackup2_win.exe --time 0 2>/dev/null
if [ $? -eq 2 ]; then
    echo "✓ Invalid time test passed (expected exit code 2)"
else
    echo "✗ Invalid time test failed"
fi

# Test 9: Quick backup simulation (5 seconds)
echo -e "\n9. Testing quick backup simulation (5 seconds)"
echo "This will take 5 seconds..."
./mock_idevicebackup2_win.exe --time 5
if [ $? -eq 0 ]; then
    echo "✓ Quick backup simulation test passed"
else
    echo "✗ Quick backup simulation test failed"
fi

echo -e "\n======================================"
echo "All tests completed!"
echo "The Windows executable appears to be working correctly." 