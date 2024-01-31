#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <getopt.h>

// long list of names in names.h
#include "names.h"

// preproc macros for version info for now, no build system yet
#define HOSTGEN_VERSION "v0.3a"

// char arrays
char *program_name, *option_output_file, *option_prefix = "", *option_suffix = "";

// platform type enum
enum EPlatformType {
    ANY = 0,
    WINDOWS = 1,
    MACBOOK = 2,
    MACMINI = 3,
};

// command line options
static struct option long_options[] = {
    {"output", required_argument, 0, 'o'},
    {"platform", required_argument, 0, 'p'},
    {"prefix", required_argument, 0, 'P'},
    {"suffix", required_argument, 0, 's'},
    {"help", no_argument, 0, 'h'},
    {"version", no_argument, 0, 'v'},
    {0, 0, 0, 0}
};

// display a help message
void display_help_message(void) {
    printf("Usage: %s [OPTIONS]...\n"
           "Generate randomized system hostnames mimicking various platforms.\n"
           "\n"
           "Options:\n"
           "  -h, --help                display this message\n"
           "  -o, --output=FILE         write the new hostname to FILE instead of stdout\n"
           "  -p, --platform=PLATFORM   make the hostname emulate the default for PLATFORM\n"
           "  -P, --prefix=PREFIX       prepend PREFIX to the hostname string\n"
           "  -s, --suffix=SUFFIX       append SUFFIX to the hostname string\n"
           "  -v, --version             display version information\n"
           "\n"
           "Platforms:\n"
           "  mac           Alias to either macbook or macmini, random at runtime\n"
           "  macbook       [Name]s-Macbook-[Prod]; 'Name' is random, 'Prod' is Air or Pro\n"
           "  macmini       [Name]s-Mac-Mini; 'Name' is random\n"
           "  macos         Alias to either macbook or macmini, random at runtime\n"
           "  osx           Alias to either macbook or macmini, random at runtime\n"
           "  windows       DESKTOP-[XXXXXXX]; 'X' are random, capital alphanumeric chars\n"
           "  win7          Alias to windows\n"
           "  win8          Alias to windows\n"
           "  win10         Alias to windows\n"
           "  win11         Alias to windows\n"
           , program_name);
    exit(0);
}

// display a version message
void display_version_message(void) {
    printf("hostgen %s\n", HOSTGEN_VERSION);
    exit(0);
}

// generate a random ASCII number/uppercase character
char get_random_char(bool is_ucase) {
    // get the random number
    unsigned int rnum = rand() % 36;

    // return it
    if (rnum < 10)
        return '0' + rnum;

    // return an upper- or lowercase letter
    return (is_ucase == true ? ('A' + (rnum - 10)) : ('a' + (rnum - 10)));
}

// pick a random name from the first_names[] array
const char *get_random_name(void) {
    // store the amount of names in the array
    unsigned int name_ct = sizeof(first_names) / sizeof(first_names[0]);

    // get a random position in the index
    unsigned int name_index = rand() % (name_ct - 1);

    // return it
    return first_names[name_index];
}

// print a hostname like DESKTOP-XXXXXXX: format_prefix is printed before, format_suffix is printed after
void hostname_windows(FILE *hostname_fd, char *format_prefix, char *format_suffix) {
    // char array containing the random part of the string
    char win_rand[8];

    // populate it
    for (int i = 0; i < 7; i++) {
        win_rand[i] = get_random_char(true);
    }

    // print the hostname
    fprintf(hostname_fd, "%s%s%s%s\n", format_prefix, "DESKTOP-", win_rand, format_suffix);
}

// print a hostname like [Name]s-Macbook-[Air|Pro]
void hostname_macbook(FILE *hostname_fd, char *format_prefix, char *format_suffix) {
    // type of macbook
    char mac_type[3];

    // decide between Air/Pro
    (rand() % 2) == 0 ? strncpy(mac_type, "Air", 3) : strncpy(mac_type, "Pro", 3);

    // print the hostname
    fprintf(hostname_fd, "%s%ss-Macbook-%s%s\n", format_prefix, get_random_name(), mac_type, format_suffix);
}

// print a hostname like [Name]s-Mac-Mini
void hostname_macmini(FILE *hostname_fd, char *format_prefix, char *format_suffix) {
    // print the hostname
    fprintf(hostname_fd, "%s%ss-Mac-Mini%s\n", format_prefix, get_random_name(), format_suffix);
}

// perform some checks and initialize a file descriptor
FILE *prepare_file(char *file_path) {
    // try to open the file for writing/creating
    int fd = open(file_path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

    // print an error message and exit if the file can't be accessed
    if (fd == -1) {
        fprintf(stderr, "%s: cannot read or create %s: %s\n", program_name, file_path, strerror(errno));
        exit(errno);
    }

    // close the file descriptor if it was successfully opened
    close(fd);

    // return the file descriptor
    return fopen(file_path, "w+");
}

// main
int main(int argc, char *argv[]) {
    // make $0 usable in all functions
    program_name = argv[0];

    // seed the random number generator with a few entropy sources
    // not at all cryptographically secure but it's good enough here
    srand((clock() ^ getpgrp() ^ getpid() ^ geteuid() ^ getegid() ^ getuid() ^ getgid() ^ getppid()));

    // current option, index
    int opt, option_index = 0;

    // which platform to generate a hostname like
    enum EPlatformType platform_type = ANY;

    // parse options
    while ((opt = getopt_long(argc, argv, "hov:p:P:s:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'o':
                option_output_file = strdup(optarg);
                break;
            case 'p':
                // spaghetti ☠️
                if (strncmp(optarg, "mac", 4) == 0)
                    platform_type = (rand() % 2) == 0 ? MACBOOK : MACMINI;
                if (strncmp(optarg, "macos", 6) == 0)
                    platform_type = (rand() % 2) == 0 ? MACBOOK : MACMINI;
                if (strncmp(optarg, "osx", 4) == 0)
                    platform_type = (rand() % 2) == 0 ? MACBOOK : MACMINI;
                if (strncmp(optarg, "macbook", 4) == 0)
                    platform_type = MACBOOK;
                if (strncmp(optarg, "macmini", 4) == 0)
                    platform_type = MACMINI;
                if (strncmp(optarg, "windows", 8) == 0)
                    platform_type = WINDOWS;
                if (strncmp(optarg, "win7", 5) == 0)
                    platform_type = WINDOWS;
                if (strncmp(optarg, "win8", 5) == 0)
                    platform_type = WINDOWS;
                if (strncmp(optarg, "win10", 6) == 0)
                    platform_type = WINDOWS;
                if (strncmp(optarg, "win11", 6) == 0)
                    platform_type = WINDOWS;
                break;
            case 'P':
                option_prefix = strdup(optarg);
                //printf("prefix: %s\n", optarg);
                break;
            case 's':
                option_suffix = strdup(optarg);
                //printf("suffix: %s\n", optarg);
                break;
            case 'h':
                display_help_message();
                break;
            case 'v':
                display_version_message();
                break;
            default:
                // Handle other cases
                break;
        } // yeah I used strdup() without freeing memory, sue me 🙂
    }

    // open the chosen file or point to stdout
    FILE *write_location = (option_output_file) ? prepare_file(option_output_file) : stdout;

    // if the platform type is to be chosen by chance, do so
    if (platform_type == ANY) {
        // get a number
        int platform_rng = rand() % 12;

        // 33% chance to be macbook
        if (platform_rng >= 0)
            platform_type = MACBOOK;

        // 16% chance to be mac mini
        if (platform_rng >= 4)
            platform_type = MACMINI;

        // 50% chance to be windows
        if (platform_rng >= 6)
            platform_type = WINDOWS;
    }

    // run the specified hostname function
    switch (platform_type) {
        case MACBOOK: hostname_macbook(write_location, option_prefix, option_suffix); break;
        case MACMINI: hostname_macmini(write_location, option_prefix, option_suffix); break;
        case WINDOWS: hostname_windows(write_location, option_prefix, option_suffix); break;
        default: break;
    }

    // close the file descriptor
    fclose(write_location);

    return 0;
}
