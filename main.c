#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

// long list of names in names.h
#include "names.h"

// file-scope char pointer to argv[0]
char *program_name;

// function declarations
void hostname_windows(FILE *fd, char *prefix, char *suffix);
void hostname_macbook(FILE *fd, char *prefix, char *suffix);
void hostname_macmini(FILE *fd, char *prefix, char *suffix);
FILE *prepare_file(char *path);
char get_random_char(bool is_ucase);
const char *get_random_name(void);

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

    // open the file descriptor
    FILE *write_location = prepare_file("hostname.txt");

    // option parsing/etc is yet to be done; for now, just print hostnames
    hostname_windows(write_location, "", "");
    hostname_macbook(write_location, "", "");
    hostname_macmini(write_location, "", "");

    // close the file descriptor
    fclose(write_location);

    return 0;
}
