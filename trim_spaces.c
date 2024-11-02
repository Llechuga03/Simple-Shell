#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "trim_spaces.h"

/*function to remove leading and trailing spaces*/
void trim_spaces(char* line) {
    char* start = line;
    char* end;

    // Skip over leading spaces
    while (isspace((unsigned char)*start)) start++;

    // If all spaces or empty string
    if (*start == 0) {
        *line = '\0';
        return;
    }

    // Remove trailing spaces
    end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;

    *(end + 1) = '\0';

    if (start != line) 
    {
        memmove(line, start, end - start + 2); // +1 for '\0' and +1 because end is inclusive
    }
}
