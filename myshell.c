#define TRUE 1
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Function to remove leading and trailing spaces
void trim_spaces(char* line) {
    char* start = line;
    char* end;

    // Remove leading spaces
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

    if (start != line) {
        memmove(line, start, end - start + 2); // +1 for '\0' and +1 because end is inclusive
    }
}

// Function to collapse multiple spaces into single spaces within the string
void collapse_spaces(char* line) {
    char* src = line;
    char* dest = line;
    int in_space = 0;

    // Remove leading spaces
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

int main(int argc, char* argv[]) {
    setvbuf(stdout,NULL,_IONBF,0);
    int dp = 1; // Display prompt
    int i;

    // Check for "-n" argument to disable prompt
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            dp = 0;
        }
    }

    while (TRUE) { // Repeat forever
        if (dp) {
            printf("my_shell$ ");
        }

        char buffer[512]; // Buffer to hold user input
        char* input = buffer;
        char* args[32];   // Array of command tokens separated by '|'
        char* fargs[32];  // Array of arguments for execvp, tokenized by ' '

        // Read input
        if (fgets(input, sizeof(buffer), stdin) == NULL) {
            break; // Exit on EOF
        }

        // Remove newline character if present
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }

        // Simplify the entire input line, only leading and trailing
        trim_spaces(input);

        // Tokenize input by '|'
        int cmd_count = 0;
        char* cursor = input;
        char* token;

        while ((token = strtok(cursor, "|")) != NULL) {
            trim_spaces(token); // Trim and collapse spaces in each command
            args[cmd_count++] = token;
            cursor = NULL; // For subsequent calls to strtok
        }
        args[cmd_count] = NULL; // Null-terminate the args array

        int c;
        for (c = 0; c < cmd_count; c++) {
            printf("%s\n",args[c]);
        }

        // For now, execute only the first command
        if (cmd_count > 0) {
            // Tokenize the first command by spaces
            int arg_count = 0;
            char* cmd = args[0];
            char* arg_cursor = cmd;
            char* arg_token;

            while ((arg_token = strtok(arg_cursor, " ")) != NULL) {
                collapse_spaces(arg_token); // Trim and collapse spaces in each argument
                fargs[arg_count++] = arg_token;
                arg_cursor = NULL; // For subsequent calls to strtok
            }
            fargs[arg_count] = NULL; // Null-terminate the fargs array

            // Execute the command
            pid_t pid = fork();

            if (pid < 0) {
                perror("ERROR: Fork failed");
                exit(1);
            }

            if (pid > 0) { // Parent process
                waitpid(pid, NULL, 0); // Wait for child to finish
            } else { // Child process
                if (execvp(fargs[0], fargs) == -1) { // Execute the command
                    perror("ERROR: execvp failed");
                    exit(1); // Exit if execvp fails
                }
            }
        }
    }

    return 0;
}
