#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

int main() {
    // Test different buffering modes
    printf("Testing buffering behavior...\n");
    
    // Line buffered (like our mock_idevicebackup2_win.exe)
    setvbuf(stdout, NULL, _IOLBF, 0);
    
    printf("Line 1 - should appear immediately\n");
    Sleep(1000);
    printf("Line 2 - should appear immediately\n");
    Sleep(1000);
    
    // Test progress bar style output (like our executable)
    printf("[");
    for (int i = 0; i < 10; i++) {
        printf("=");
        Sleep(200);
    }
    printf("] 20%% Complete\n");
    
    Sleep(1000);
    printf("Final message\n");
    
    return 0;
} 