#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>

#define TOOL_NAME "mock_idevicebackup2"
#define DEFAULT_TOTAL_TIME 80  // Current total time in seconds

static void print_usage(int argc, char **argv, int is_error)
{
    char *name = strrchr(argv[0], '/');
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
    int c = 0;

    const struct option longopts[] = {
        { "help", no_argument, NULL, 'h' },
        { "udid", required_argument, NULL, 'u' },
        { "network", no_argument, NULL, 'n' },
        { "info", required_argument, NULL, 'i' },
        { "scenario", required_argument, NULL, 's' },
        { "time", required_argument, NULL, 't' },
        { "code", required_argument, NULL, 'c' },
        { "delay", required_argument, NULL, 'd' },
        { "version", no_argument, NULL, 'v' },
        { NULL, 0, NULL, 0}
    };

    while ((c = getopt_long(argc, argv, "hu:ni:s:t:c:d:v", longopts, NULL)) != -1) {
        switch (c) {
        case 'u':
            if (!*optarg) {
                fprintf(stderr, "ERROR: UDID must not be empty!\n");
                print_usage(argc, argv, 1);
                return 2;
            }
            udid = optarg;
            break;
        case 'n':
            use_network = 1;
            break;
        case 'i':
            if (!*optarg) {
                fprintf(stderr, "ERROR: 'info' command must not be empty!\n");
                print_usage(argc, argv, 1);
                return 2;
            }
            info_cmd = optarg;
            break;
        case 's':
            scenario = atoi(optarg);
            if (scenario < 1 || scenario > 3) {
                fprintf(stderr, "ERROR: Scenario must be between 1 and 3!\n");
                print_usage(argc, argv, 1);
                return 2;
            }
            break;
        case 't':
            total_time = atoi(optarg);
            if (total_time < 1) {
                fprintf(stderr, "ERROR: Time must be greater than 0!\n");
                print_usage(argc, argv, 1);
                return 2;
            }
            break;
        case 'c':
            exit_code = atoi(optarg);
            break;
        case 'd':
            initial_delay = atoi(optarg);
            if (initial_delay < 0) {
                fprintf(stderr, "ERROR: Delay must be non-negative!\n");
                print_usage(argc, argv, 1);
                return 2;
            }
            break;
        case 'h':
            print_usage(argc, argv, 0);
            return 0;
        case 'v':
            printf("%s 1.0.0\n", TOOL_NAME);
            return 0;
        default:
            print_usage(argc, argv, 1);
            return 2;
        }
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
                srand(time(NULL));
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
        sleep(initial_delay);
    }

    // Calculate time distribution
    int initial_time = (total_time * 10) / 80;  // 10 seconds in original 80
    int passcode_time = (total_time * 5) / 80;  // 5 seconds in original 80
    int progress_time = (total_time * 60) / 80; // 60 seconds in original 80
    int final_time = (total_time * 5) / 80;     // 5 seconds in original 80

    // Initial messages
    printf("Backup directory is \"/Users/sethbell/Library/Caches/iosBackup\"\n");
    sleep(initial_time / 9);
    printf("Started \"com.apple.mobilebackup2\" service on port 50579.\n");
    sleep(initial_time / 9);
    printf("Negotiated Protocol Version 2.1\n");
    sleep(initial_time / 9);
    printf("Reading Info.plist from backup.\n");
    sleep(initial_time / 9);
    printf("Starting backup...\n");
    sleep(initial_time / 9);
    printf("Enforcing full backup from device.\n");
    sleep(initial_time / 9);
    printf("Backup will be unencrypted.\n");
    sleep(initial_time / 9);
    printf("Requesting backup from device...\n");
    sleep(initial_time / 9);
    printf("Incremental backup mode.\n");
    sleep(initial_time / 9);
    printf("*** Waiting for passcode to be entered on the device ***\n");
    sleep(passcode_time);

    // Progress updates
    for (int i = 0; i <= 100; i += 10) {
        print_progress(i);
        sleep(progress_time / 11); // 11 steps (0 to 100 in steps of 10)
    }

    // Final messages
    printf("Moving 128 files\n");
    sleep(final_time / 15);
    printf("Moving 128 files\n");
    sleep(final_time / 15);
    printf("Moving 128 files\n");
    sleep(final_time / 15);
    printf("Moving 70 files\n");
    sleep(final_time / 15);
    printf("Moving 1 file\n");
    sleep(final_time / 15);
    printf("Moving 1 file\n");
    sleep(final_time / 15);
    printf("Removing 1 file\n");
    sleep(final_time / 15);
    printf("Removing 1 file\n");
    sleep(final_time / 15);
    printf("Sending '00008110-000E785101F2401E/Status.plist' (189 Bytes)\n");
    sleep(final_time / 15);
    printf("Sending '00008110-000E785101F2401E/Manifest.plist' (253.2 KB)\n");
    sleep(final_time / 15);
    printf("Sending '00008110-000E785101F2401E/Manifest.db' (9.1 MB)\n");
    sleep(final_time / 15);
    printf("Received 2124 files from device.\n");
    sleep(final_time / 15);
    printf("Backup Successful.\n");

    return exit_code;
} 