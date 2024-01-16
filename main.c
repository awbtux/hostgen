#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* generate a random ASCII character */
char get_rndchar(void) {
    /* get the random number */
    int rnum = rand() % 36;

    /* return a character or number based on it */
    if (rnum < 10) {
        return '0' + rnum;
    } else {
        return ('A' + (rnum - 10));
    }
}

/* windows function */
char *name_windows(void) {
    /* char arrays */
    char winrand[8], winfull[16];

    /* populate the random string */
    for (i = 0; i < 7; i++) {
        winrand[i] = get_rndchar();
    }
    winrand[7] = '\0';

    /* place the entire new hostname in winfull */
    strcpy(winfull, "DESKTOP-");
    strcat(winfull, winrand);

    /* return this value */
    return winfull;
}

/* main function yaaaaay les go */
int main(int argc, char *argv[]) {
    /* seed the random number generator with system time (GENERALLY INSECURE) */
    srand(time(NULL));

    printf("%s\n", name_windows());
    return 0;
}
