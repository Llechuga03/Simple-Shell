#include "collapse_spaces.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// function to collapse multiple spaces into single spaces within the string
void collapse_spaces(char* line) {
    char* src = line;
    char* dest = line;
    int in_space = 0;

    // Skipping over leading spaces
    while (isspace((unsigned char)*src)) src++;

    while (*src != '\0') {
        if (isspace((unsigned char)*src)) {
            if (!in_space) {
                *dest++ = ' ';
                in_space = 1;
            }
        } else {
            *dest++ = *src;
            in_space = 0;
        }
        src++;
    }

    // Remove trailing space if any
    if (in_space && dest > line) {
        dest--;
    }

    *dest = '\0';
}
