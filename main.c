#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

// long list of names in names.h
static const char *first_names[] = {
#include "names.h"
};

// generate a random ASCII number/uppercase character
char get_random_char(bool is_upper) {
    // get the random number
    int rnum = rand() % 36;

    // return it
    if (rnum < 10)
        return '0' + rnum;

    // return an upper- or lowercase letter
    return (is_upper == true ? ('A' + (rnum - 10)) : ('a' + (rnum - 10)));
}

// pick a random name from the first_names[] array
const char *get_random_name(void) {
    // store the amount of names in the array
    int name_ct = sizeof(first_names) / sizeof(first_names[0]);

    // get a random position in the index
    int name_index = rand() % (name_ct - 1);

    // return it
    return first_names[name_index];
}

// print a hostname like DESKTOP-XXXXXXX: format_prefix is printed before, format_suffix is printed after
void hostname_windows(char *format_prefix, char *format_suffix) {
    // char array containing the random part of the name
    char win_rand[8];

    // populate the random part of the string
    for (int i = 0; i < 7; i++) {
        win_rand[i] = get_random_char(true);
    }

    // print the hostname
    printf("%s%s%s%s\n", format_prefix, "DESKTOP-", win_rand, format_suffix);
}

// print a hostname like NAMEs-Macbook-[Air|Pro]: format_prefix is printed before, format_suffix is printed after
void hostname_macbook(char *format_prefix, char *format_suffix) {
    // type of macbook
    char mac_type[3];

    // decide between Air/Pro
    (rand() % 2) == 0 ? strncpy(mac_type, "Air", 3) : strncpy(mac_type, "Pro", 3);

    // print the hostname
    printf("%s%ss-Macbook-%s%s\n", format_prefix, get_random_name(), mac_type, format_suffix);
}

// main
int main(int argc, char *argv[]) {
    // seed the random number generator with a few entropy sources
    // not at all cryptographically secure but it's good enough here
    srand((clock() ^ getpgrp() ^ getpid() ^ geteuid() ^ getegid() ^ getuid() ^ getgid() ^ getppid()));

    // option parsing/output redirection/etc is yet to be done; for now, just print hostnames
    hostname_windows("", "");
    hostname_macbook("", "");

    return 0;
}
