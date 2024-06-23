#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

// long list of names in names.h
#include "names.h"

// preproc macros for version info for now, no build system yet
#define HOSTGEN_VERSION "v0.3a"

// char arrays
char *option_platform = "";

// platform type enum
enum EPlatformType { ANY, WINDOWS, MACBOOK, MACMINI };

// command line options
static struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"platform", required_argument, 0, 'p'},
    {"version", no_argument, 0, 'v'},
    {0, 0, 0, 0}
};

// display a help message
void display_help_message(char *program_name) {
    printf("Usage: %s [OPTIONS]...\n"
           "Generate randomized system hostnames mimicking various platforms.\n"
           "\n"
           "Options:\n"
           "  -h, --help                display this message\n"
           "  -p, --platform=PLATFORM   make the hostname emulate the default for PLATFORM\n"
           "  -v, --version             display version information\n"
           "\n"
           "Platforms:\n"
           "  mac           Alias to either macbook or macmini, random at runtime\n"
           "  macbook       [Name]s-Macbook-[Prod]; 'Name' is random, 'Prod' is Air or Pro\n"
           "  macmini       [Name]s-Mac-Mini; 'Name' is random\n"
           "  windows       DESKTOP-[XXXXXXX]; 'X' are random, capital alphanumeric chars\n"
           , program_name);
    exit(0);
}

// generate a random ASCII number/uppercase character
char get_random_char(bool is_ucase) {
    unsigned int rnum = rand() % 36;
    if (rnum < 10)
        return '0' + rnum;
    return (is_ucase == true ? ('A' + (rnum - 10)) : ('a' + (rnum - 10)));
}

// pick a random name from the first_names[] array
const char *get_random_name(void) {
    unsigned int name_ct = sizeof(first_names) / sizeof(first_names[0]);
    unsigned int name_index = rand() % (name_ct - 1);
    return first_names[name_index];
}

// set platform_type enum
enum EPlatformType set_platform_type(char *type_name) {
    if (strncmp(type_name, "macmini", 5) == 0)
        return MACMINI;
    if (strncmp(type_name, "macbook", 5) == 0)
        return MACBOOK;
    if (strncmp(type_name, "mac", 3) == 0)
        return (rand() % 2) == 0 ? MACBOOK : MACMINI;
    if (strncmp(type_name, "windows", 3) == 0)
        return WINDOWS;
    int platform_rng = rand() % 16;
    if (platform_rng <= 3) return MACBOOK;
    if (platform_rng <= 4) return MACMINI;
    if (platform_rng >= 5) return WINDOWS;
    return ANY;
}

// main
int main(int argc, char *argv[]) {
    // seed rng
    unsigned int random_seed;
    read(fileno(fopen("/dev/urandom", "rb")), &random_seed, sizeof(random_seed));
    srand(random_seed);

    // parse options
    int opt, option_index = 0;
    while ((opt = getopt_long(argc, argv, "ahn:o:p:P:s:v", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'h': display_help_message(argv[0]); break;
            case 'p': option_platform = strdup(optarg); break;
            case 'v': printf("hostgen %s\n", HOSTGEN_VERSION); return 0; break;
            default: break;
        }
    }

    // set/randomize and print the hostname
    enum EPlatformType platform_type = set_platform_type(option_platform);
    switch (platform_type) {
        case MACBOOK: printf("%ss-Macbook-%s\n", get_random_name(), (rand() % 2) == 0 ? "Air" : "Pro"); break;
        case MACMINI: printf("%ss-Mac-Mini\n", get_random_name()); break;
        case WINDOWS: char win_rand[8]; for (int i = 0; i < 7; i++) win_rand[i] = get_random_char(true); printf("%s%s\n", "DESKTOP-", win_rand); break;
        default: break;
    }
    return 0;
}
