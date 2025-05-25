#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s -u <string> -c <exit_code>\n", program_name);
    fprintf(stderr, "  -u: String to output (can be empty)\n");
    fprintf(stderr, "  -c: Exit code to return\n");
}

int main(int argc, char *argv[]) {
    char *output_string = NULL;
    int exit_code = 0;
    int opt;

    while ((opt = getopt(argc, argv, "u:c:")) != -1) {
        switch (opt) {
            case 'u':
                output_string = optarg;
                break;
            case 'c':
                exit_code = atoi(optarg);
                break;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    // Output the string if provided
    if (output_string != NULL) {
        printf("%s\n", output_string);
    }

    return exit_code;
} 