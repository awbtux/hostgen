#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

// char array to place randomized windows hostnames in
char winhostname[16];

// generate a random ASCII number/uppercase character
char get_rndchar(bool isupper) {
    // get the random number
    int rnum = rand() % 36;

    // return a character or number based on it
    if (rnum < 10)
        return '0' + rnum;

    // return an upper- or lowercase letter
    switch (isupper) {
        case true:
            return ('A' + (rnum - 10));
            break;
        default:
            return ('a' + (rnum - 10));
            break;
    }
}

// DESKTOP-XXXXXXX
void gen_windows(char *win_rand) {
    // copy the DESKTOP- prefix
    strcpy(win_rand, "DESKTOP-");

    // populate the random part of the string
    for (int i = 0; i < 7; i++) {
        win_rand[i + 8] = get_rndchar(true);
    }
    win_rand[15] = '\0';
}

// main function yaaaaay les go
int main(int argc, char *argv[]) {
    // seed the random number generator with a few entropy sources
    // not at all cryptographically secure but it's good enough here
    srand((clock() ^ getpgrp() ^ getpid() ^ geteuid() ^ getegid() ^ getuid() ^ getgid() ^ getppid()));

    // generate a windows hostname
    gen_windows(winhostname);

    printf("%s\n", winhostname);
    return 0;
}
