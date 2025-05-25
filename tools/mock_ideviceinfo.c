#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#define TOOL_NAME "mock_ideviceinfo"

static void print_usage(int argc, char **argv, int is_error)
{
    char *name = strrchr(argv[0], '/');
    fprintf(is_error ? stderr : stdout, "Usage: %s [OPTIONS]\n", (name ? name + 1: argv[0]));
    fprintf(is_error ? stderr : stdout,
        "\n"
        "Mock version of ideviceinfo that returns predefined responses.\n"
        "\n"
        "OPTIONS:\n"
        "  -u, --udid UDID       target specific device by UDID\n"
        "  -n, --network         connect to network device\n"
        "  -q, --domain NAME     set domain of query to NAME\n"
        "  -k, --key NAME        only query key specified by NAME\n"
        "  -x, --xml             output information in XML property list format\n"
        "  -o, --output STRING   specify the output string to return\n"
        "  -c, --code N          exit with code N\n"
        "  -h, --help            prints usage information\n"
        "  -v, --version         prints version information\n"
        "\n"
    );
}

int main(int argc, char *argv[])
{
    const char *udid = NULL;
    const char *domain = NULL;
    const char *key = NULL;
    const char *output = NULL;
    int use_network = 0;
    int format_xml = 0;
    int exit_code = 0;
    int c = 0;

    const struct option longopts[] = {
        { "help", no_argument, NULL, 'h' },
        { "udid", required_argument, NULL, 'u' },
        { "network", no_argument, NULL, 'n' },
        { "domain", required_argument, NULL, 'q' },
        { "key", required_argument, NULL, 'k' },
        { "xml", no_argument, NULL, 'x' },
        { "output", required_argument, NULL, 'o' },
        { "code", required_argument, NULL, 'c' },
        { "version", no_argument, NULL, 'v' },
        { NULL, 0, NULL, 0}
    };

    while ((c = getopt_long(argc, argv, "hu:nq:k:xo:c:v", longopts, NULL)) != -1) {
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
        case 'q':
            if (!*optarg) {
                fprintf(stderr, "ERROR: 'domain' must not be empty!\n");
                print_usage(argc, argv, 1);
                return 2;
            }
            domain = optarg;
            break;
        case 'k':
            if (!*optarg) {
                fprintf(stderr, "ERROR: 'key' must not be empty!\n");
                print_usage(argc, argv, 1);
                return 2;
            }
            key = optarg;
            break;
        case 'x':
            format_xml = 1;
            break;
        case 'o':
            output = optarg;
            break;
        case 'c':
            exit_code = atoi(optarg);
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

    // If no output was specified, generate a default response based on domain/key
    if (!output) {
        if (domain && strcmp(domain, "com.apple.disk_usage") == 0) {
            // Return disk usage information in bytes
            printf("TotalDiskCapacity: 128849018880\n");  // 120 GB
            printf("TotalDataAvailable: 64424509440\n");  // 60 GB
            printf("TotalDataCapacity: 107374182400\n");  // 100 GB
            printf("TotalSystemCapacity: 21474836480\n"); // 20 GB
            printf("TotalSystemAvailable: 10737418240\n"); // 10 GB
            printf("AmountDataAvailable: 64424509440\n");  // 60 GB
        } else if (format_xml) {
            if (domain && key) {
                printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                       "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
                       "<plist version=\"1.0\">\n"
                       "<dict>\n"
                       "    <key>%s</key>\n"
                       "    <string>mock_value</string>\n"
                       "</dict>\n"
                       "</plist>", key);
            } else {
                printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                       "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
                       "<plist version=\"1.0\">\n"
                       "<dict>\n"
                       "    <key>DeviceName</key>\n"
                       "    <string>Mock Device</string>\n"
                       "    <key>ProductType</key>\n"
                       "    <string>iPhone14,3</string>\n"
                       "    <key>ProductVersion</key>\n"
                       "    <string>16.0</string>\n"
                       "</dict>\n"
                       "</plist>");
            }
        } else {
            if (domain && key) {
                printf("%s: mock_value\n", key);
            } else {
                printf("DeviceName: Mock Device\n"
                       "ProductType: iPhone14,3\n"
                       "ProductVersion: 16.0\n");
            }
        }
    } else {
        printf("%s\n", output);
    }

    return exit_code;
} 