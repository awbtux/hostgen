#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <getopt.h>

// long list of names in names.h
#include "names.h"

// preproc macros for version info for now, no build system yet
#define HOSTGEN_VERSION "v0.3a"

// file descriptors
FILE *names_location,
     *random_file,
     *write_location;

// char arrays
char *program_name,
     *option_names_file,
     *option_output_file,
     *option_platform = "",
     *option_prefix = "",
     *option_suffix = "";

// affects fopen permissions in file_prepare()
bool option_append_writes = false;

// platform type enum
enum EPlatformType {
    ANY = 0,
    WINDOWS = 1,
    MACBOOK = 2,
    MACMINI = 3,
};

// command line options
static struct option long_options[] = {
    {"append", no_argument, 0, 'a'},
    {"help", no_argument, 0, 'h'},
    {"names", required_argument, 0, 'n'},
    {"output", required_argument, 0, 'o'},
    {"platform", required_argument, 0, 'p'},
    {"prefix", required_argument, 0, 'P'},
    {"suffix", required_argument, 0, 's'},
    {"version", no_argument, 0, 'v'},
    {0, 0, 0, 0}
};

// display a help message
void display_help_message(void) {
    printf("Usage: %s [OPTIONS]...\n"
           "Generate randomized system hostnames mimicking various platforms.\n"
           "\n"
           "Options:\n"
           "  -a, --append              append to FILE, instead of overwriting or trying\n"
           "                            substitution in it\n"
           "  -h, --help                display this message\n"
           "  -n, --names=NAMEFILE      choose a list of firstnames other than the defaults\n"
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


// count the lines in a file
int file_count_lines(FILE *file) {
    // char and line
    int ch = 0, ln = 0;

    // if an invalid file descriptor is passed, return a negative line count
    if (!file)
        return -1;

    // rewind the file to the beginning
    rewind(file);

    // increment the line count
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') {
            ln++;
        }
    }

    // return the line number
    return ln;
}

// get a random line from a file
char *file_get_random_line(FILE *file) {
    // buffer holding the chosen line
    char line_bfr[1024], *line = line_bfr;

    // get the total number of lines in the file
    int lines = file_count_lines(file);

    // return null if lines can't be counted
    if (lines <= 0)
        return NULL;

    // generate a random line number
    int random_line = (rand() % lines) + 1;

    // seek to the beginning of the file
    rewind(file);

    // read and discard lines until the randomly chosen line
    for (int i = 1; i < random_line; i++) {
        char bfr[1024];
        if (fgets(bfr, sizeof(bfr), file) == NULL) {
            fprintf(stderr, "%s: get_random_line(): error: %s\n", program_name, strerror(errno));
            exit(errno);
        }
    }

    // read and process the chosen line
    if (fgets(line_bfr, sizeof(line_bfr), file) == NULL) {
        fprintf(stderr, "%s: get_random_line(): error: %s\n", program_name, strerror(errno));
        exit(errno);
    }

    // get the length of the string without the newline
    size_t length = strcspn(line_bfr, "\n");

    // Replace the newline character with null terminator
    if (line[length] == '\n')
        line[length] = '\0';

    // trim the newline char and null terminate the buffer
    line = line_bfr;

    // Print the randomly chosen line
    return line;
}

// perform some checks and initialize a file descriptor
FILE *file_prepare(char *file_path, bool read_only, bool use_binary) {
    // try to open the file for writing/creating
    int fd = open(file_path, (read_only == true ? O_RDONLY : O_WRONLY | O_CREAT), S_IRUSR | S_IWUSR);

    // print an error message and exit if the file can't be accessed
    if (fd == -1) {
        fprintf(stderr, "%s: cannot %s %s: %s\n", program_name, (read_only == true ? "read" : "create or read"), file_path, strerror(errno));
        exit(errno);
    }

    // close the file descriptor if it was successfully opened
    close(fd);

    // open the file as a text or binary stream
    if (use_binary)
        return fopen(file_path, (read_only == true ? "rb" : (option_append_writes == true ? "a+b" : "w+b")));
    else
        return fopen(file_path, (read_only == true ? "r" : (option_append_writes == true ? "a+" : "w+")));
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
    // use the first_names[] array or a provided file in names_file
    if (names_location)
        return file_get_random_line(names_location);

    // store the amount of names in the array
    unsigned int name_ct = sizeof(first_names) / sizeof(first_names[0]);

    // get a random position in the index
    unsigned int name_index = rand() % (name_ct - 1);

    // return it
    return first_names[name_index];
}

// print a hostname like DESKTOP-XXXXXXX: format_prefix is printed before, format_suffix is printed after
void hostname_windows(char *format_prefix, char *format_suffix) {
    // char array containing the random part of the string
    char win_rand[8];

    // populate it
    for (int i = 0; i < 7; i++) {
        win_rand[i] = get_random_char(true);
    }

    // print the hostname
    fprintf(write_location, "%s%s%s%s\n", format_prefix, "DESKTOP-", win_rand, format_suffix);
}

// print a hostname like [Name]s-Macbook-[Air|Pro]
void hostname_macbook(char *format_prefix, char *format_suffix) {
    // type of macbook
    char mac_type[3];

    // decide between Air/Pro
    (rand() % 2) == 0 ? strncpy(mac_type, "Air", 3) : strncpy(mac_type, "Pro", 3);

    // print the hostname
    fprintf(write_location, "%s%ss-Macbook-%s%s\n", format_prefix, get_random_name(), mac_type, format_suffix);
}

// print a hostname like [Name]s-Mac-Mini
void hostname_macmini(char *format_prefix, char *format_suffix) {
    // print the hostname
    fprintf(write_location, "%s%ss-Mac-Mini%s\n", format_prefix, get_random_name(), format_suffix);
}

// set platform_type enum
enum EPlatformType set_platform_type(char *type_name) {
    // spaghetti ☠️  (doesn't matter it's not in main() anymore)
    if (strncmp(type_name, "mac", 4) == 0)
        return (rand() % 2) == 0 ? MACBOOK : MACMINI;
    if (strncmp(type_name, "macos", 6) == 0)
        return (rand() % 2) == 0 ? MACBOOK : MACMINI;
    if (strncmp(type_name, "osx", 4) == 0)
        return (rand() % 2) == 0 ? MACBOOK : MACMINI;
    if (strncmp(type_name, "macbook", 4) == 0)
        return MACBOOK;
    if (strncmp(type_name, "macmini", 4) == 0)
        return MACMINI;
    if (strncmp(type_name, "windows", 8) == 0)
        return WINDOWS;
    if (strncmp(type_name, "win7", 5) == 0)
        return WINDOWS;
    if (strncmp(type_name, "win8", 5) == 0)
        return WINDOWS;
    if (strncmp(type_name, "win10", 6) == 0)
        return WINDOWS;
    if (strncmp(type_name, "win11", 6) == 0)
        return WINDOWS;

    // if the platform type is to be chosen by chance, do so
    int platform_rng = rand() % 12;

    // 33% chance to be macbook
    if (platform_rng >= 0)
        return MACBOOK;

    // 16% chance to be mac mini
    if (platform_rng >= 4)
        return MACMINI;

    // 50% chance to be windows
    if (platform_rng >= 6)
        return WINDOWS;

    // fallback (this shouldn't happen)
    return ANY;
}

// main
int main(int argc, char *argv[]) {
    // seed for the random number generator
    unsigned int random_seed;

    // make $0 usable in all functions
    program_name = argv[0];

    // open /dev/urandom for reading
    random_file = file_prepare("/dev/urandom", true, true);

    // read bytes from /dev/urandom
    read(fileno(random_file), &random_seed, sizeof(random_seed));

    // seed the random number generator with entropy
    srand(random_seed);

    // current option, index
    int opt, option_index = 0;

    // which platform to generate a hostname like
    enum EPlatformType platform_type = ANY;

    // parse options
    while ((opt = getopt_long(argc, argv, "ahn:o:p:P:s:v", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'a':
                option_append_writes = true;
                break;
            case 'h':
                display_help_message();
                break;
            case 'o':
                option_output_file = strdup(optarg);
                break;
            case 'n':
                option_names_file = strdup(optarg);
                break;
            case 'p':
                option_platform = strdup(optarg);
                break;
            case 'P':
                option_prefix = strdup(optarg);
                //printf("prefix: %s\n", optarg);
                break;
            case 's':
                option_suffix = strdup(optarg);
                //printf("suffix: %s\n", optarg);
                break;
            case 'v':
                display_version_message();
                break;
            default:
                break;
        } // yeah I used strdup() without freeing memory, sue me 🙂
    }

    // open the name file if one is provided
    names_location = (option_names_file) ? file_prepare(option_names_file, true, false) : NULL;

    // open the chosen file or point to stdout
    write_location = (option_output_file) ? file_prepare(option_output_file, false, false) : stdout;

    // set or randomize platform_type
    platform_type = set_platform_type(option_platform);

    // run the specified hostname function
    switch (platform_type) {
        case MACBOOK: hostname_macbook(option_prefix, option_suffix); break;
        case MACMINI: hostname_macmini(option_prefix, option_suffix); break;
        case WINDOWS: hostname_windows(option_prefix, option_suffix); break;
        default: break;
    }

    // close the file descriptor
    fclose(write_location);

    return 0;
}
