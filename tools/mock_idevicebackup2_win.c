#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>

#define TOOL_NAME "mock_idevicebackup2_win"
#define DEFAULT_TOTAL_TIME 80  // Current total time in seconds

static void print_usage(int argc, char **argv, int is_error)
{
    char *name = strrchr(argv[0], '\\');
    if (!name) name = strrchr(argv[0], '/');
    fprintf(is_error ? stderr : stdout, "Usage: %s [OPTIONS]\n", (name ? name + 1: argv[0]));
    fprintf(is_error ? stderr : stdout,
        "\n"
        "Mock version of idevicebackup2 that simulates backup operations.\n"
        "\n"
        "OPTIONS:\n"
        "  -u, --udid UDID       target specific device by UDID\n"
        "  -n, --network         connect to network device\n"
        "  -i, --info CMD        info command (e.g. 'encryption off')\n"
        "  -s, --scenario N      use specific scenario (1-3)\n"
        "  -t, --time N          total execution time in seconds\n"
        "  -c, --code N          exit with code N\n"
        "  -d, --delay N         wait N seconds before starting\n"
        "  -h, --help            prints usage information\n"
        "  -v, --version         prints version information\n"
        "\n"
        "Scenarios:\n"
        "  1: Encryption disabled successfully\n"
        "  2: Encryption not enabled\n"
        "  3: Device locked\n"
        "\n"
    );
}

void print_progress(int percent) {
    int bars = (percent * 50) / 100;
    printf("[");
    for (int i = 0; i < 50; i++) {
        if (i < bars) printf("=");
        else printf(" ");
    }
    printf("] %d%% Finished\n", percent);
}

// Simple command-line argument parser for Windows
int parse_args(int argc, char **argv, char **udid, char **info_cmd, int *use_network, 
               int *exit_code, int *scenario, int *total_time, int *initial_delay) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argc, argv, 0);
            return 1; // Indicate help was requested
        }
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("%s 1.0.0\n", TOOL_NAME);
            return 1; // Indicate version was requested
        }
        else if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--udid") == 0) {
            if (i + 1 >= argc || !argv[i + 1] || !*argv[i + 1]) {
                fprintf(stderr, "ERROR: UDID must not be empty!\n");
                print_usage(argc, argv, 1);
                return 2;
            }
            *udid = argv[++i];
        }
        else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--network") == 0) {
            *use_network = 1;
        }
        else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--info") == 0) {
            if (i + 1 >= argc || !argv[i + 1] || !*argv[i + 1]) {
                fprintf(stderr, "ERROR: 'info' command must not be empty!\n");
                print_usage(argc, argv, 1);
                return 2;
            }
            *info_cmd = argv[++i];
        }
        else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--scenario") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "ERROR: Scenario value required!\n");
                print_usage(argc, argv, 1);
                return 2;
            }
            *scenario = atoi(argv[++i]);
            if (*scenario < 1 || *scenario > 3) {
                fprintf(stderr, "ERROR: Scenario must be between 1 and 3!\n");
                print_usage(argc, argv, 1);
                return 2;
            }
        }
        else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--time") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "ERROR: Time value required!\n");
                print_usage(argc, argv, 1);
                return 2;
            }
            *total_time = atoi(argv[++i]);
            if (*total_time < 1) {
                fprintf(stderr, "ERROR: Time must be greater than 0!\n");
                print_usage(argc, argv, 1);
                return 2;
            }
        }
        else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--code") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "ERROR: Code value required!\n");
                print_usage(argc, argv, 1);
                return 2;
            }
            *exit_code = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--delay") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "ERROR: Delay value required!\n");
                print_usage(argc, argv, 1);
                return 2;
            }
            *initial_delay = atoi(argv[++i]);
            if (*initial_delay < 0) {
                fprintf(stderr, "ERROR: Delay must be non-negative!\n");
                print_usage(argc, argv, 1);
                return 2;
            }
        }
        else {
            fprintf(stderr, "ERROR: Unknown option '%s'\n", argv[i]);
            print_usage(argc, argv, 1);
            return 2;
        }
    }
    return 0; // Success
}

int main(int argc, char **argv)
{
    // Force line buffering on stdout
    setvbuf(stdout, NULL, _IOLBF, 0);
    setvbuf(stderr, NULL, _IOLBF, 0);

    const char *udid = NULL;
    const char *info_cmd = NULL;
    int use_network = 0;
    int exit_code = 0;
    int scenario = 0;
    int total_time = DEFAULT_TOTAL_TIME;
    int initial_delay = 0;

    // Parse command line arguments
    int parse_result = parse_args(argc, argv, (char**)&udid, (char**)&info_cmd, 
                                 &use_network, &exit_code, &scenario, &total_time, &initial_delay);
    if (parse_result == 1) {
        return 0; // Help or version requested
    } else if (parse_result == 2) {
        return 2; // Error in arguments
    }

    // Handle encryption command
    if (info_cmd && strcmp(info_cmd, "encryption off") == 0) {
        switch (scenario) {
            case 1:
                // Encryption disabled successfully
                printf("Backup encryption has been disabled successfully\n");
                return 0;
            case 2:
                // Encryption not enabled
                fprintf(stderr, "ERROR: Backup encryption is not enabled\n");
                return 0;
            case 3:
                // Device locked
                fprintf(stderr, "ERROR: Password protected\n");
                return 1;
            default:
                // Randomly choose a scenario if none specified
                srand((unsigned int)time(NULL));
                scenario = (rand() % 3) + 1;
                switch (scenario) {
                    case 1:
                        printf("Backup encryption has been disabled successfully\n");
                        return 0;
                    case 2:
                        fprintf(stderr, "ERROR: Backup encryption is not enabled\n");
                        return 0;
                    case 3:
                        fprintf(stderr, "ERROR: Password protected\n");
                        return 1;
                }
        }
    }

    // Apply initial delay if specified
    if (initial_delay > 0) {
        Sleep(initial_delay * 1000); // Convert seconds to milliseconds
    }

    // Calculate time distribution
    int initial_time = (total_time * 10) / 80;  // 10 seconds in original 80
    int passcode_time = (total_time * 5) / 80;  // 5 seconds in original 80
    int progress_time = (total_time * 60) / 80; // 60 seconds in original 80
    int final_time = (total_time * 5) / 80;     // 5 seconds in original 80

    // Initial messages
    printf("Backup directory is \"C:\\Users\\%s\\AppData\\Local\\iosBackup\"\n", getenv("USERNAME") ? getenv("USERNAME") : "User");
    Sleep((initial_time / 9) * 1000);
    printf("Started \"com.apple.mobilebackup2\" service on port 50579.\n");
    Sleep((initial_time / 9) * 1000);
    printf("Negotiated Protocol Version 2.1\n");
    Sleep((initial_time / 9) * 1000);
    printf("Reading Info.plist from backup.\n");
    Sleep((initial_time / 9) * 1000);
    printf("Starting backup...\n");
    Sleep((initial_time / 9) * 1000);
    printf("Enforcing full backup from device.\n");
    Sleep((initial_time / 9) * 1000);
    printf("Backup will be unencrypted.\n");
    Sleep((initial_time / 9) * 1000);
    printf("Requesting backup from device...\n");
    Sleep((initial_time / 9) * 1000);
    printf("Incremental backup mode.\n");
    Sleep((initial_time / 9) * 1000);
    printf("*** Waiting for passcode to be entered on the device ***\n");
    Sleep(passcode_time * 1000);

    // Progress updates
    for (int i = 0; i <= 100; i += 10) {
        print_progress(i);
        Sleep((progress_time / 11) * 1000); // 11 steps (0 to 100 in steps of 10)
    }

    // Final messages
    printf("Moving 128 files\n");
    Sleep((final_time / 15) * 1000);
    printf("Moving 128 files\n");
    Sleep((final_time / 15) * 1000);
    printf("Moving 128 files\n");
    Sleep((final_time / 15) * 1000);
    printf("Moving 70 files\n");
    Sleep((final_time / 15) * 1000);
    printf("Moving 1 file\n");
    Sleep((final_time / 15) * 1000);
    printf("Moving 1 file\n");
    Sleep((final_time / 15) * 1000);
    printf("Removing 1 file\n");
    Sleep((final_time / 15) * 1000);
    printf("Removing 1 file\n");
    Sleep((final_time / 15) * 1000);
    printf("Sending '00008110-000E785101F2401E/Status.plist' (189 Bytes)\n");
    Sleep((final_time / 15) * 1000);
    printf("Sending '00008110-000E785101F2401E/Manifest.plist' (253.2 KB)\n");
    Sleep((final_time / 15) * 1000);
    printf("Sending '00008110-000E785101F2401E/Manifest.db' (9.1 MB)\n");
    Sleep((final_time / 15) * 1000);
    printf("Received 2124 files from device.\n");
    Sleep((final_time / 15) * 1000);
    printf("Backup Successful.\n");

    return exit_code;
} 