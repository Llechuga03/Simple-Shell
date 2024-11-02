#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "pipeline.h"
#include "trim_spaces.h"
#include "sigchild.h"
#include "collapse_spaces.h"

#define TRUE 1
#define STD_INPUT 0
#define STD_OUTPUT 1

// Function declarations
void trim_spaces(char* line);
void pipeline(char* args[], int cmd_count);
void collapse_spaces(char* line);
void sigchild(int val);

int main(int argc, char* argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0); //ensures stdout has no buffer

    // Setup signal handling for zombies
    struct sigaction s_zomb;
    s_zomb.sa_handler = sigchild; //setting signal handler to sigchild function
    sigemptyset(&s_zomb.sa_mask); //ensuring no signals are blocked while sigchild runs
    s_zomb.sa_flags = SA_RESTART | SA_NOCLDSTOP; //restart ensures interrupted system calls are reset if interrupted. nocldstop prevents sigchild from being called when a process stops, only when it terminates
    if (sigaction(SIGCHLD, &s_zomb, NULL) == -1) { //if call fails, issue error 
        perror("ERROR: sigaction");
        exit(1);
    }

    int dp = 1; // Display prompt
    int i;      // Counter variable

    // Check for "-n" to disable prompt
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            dp = 0;
        }
    }

    while (TRUE) {
        if (dp) {
            printf("my_shell$ ");
        }

        char buffer[512]; // Buffer for user input, max 512 characters
        char* input = buffer; // Input pointer
        char* args[32];   // Command tokens, max 32 arguments 

        // if user hits Ctrl+D, exit the while loop and terminate program
        if (fgets(input, sizeof(buffer), stdin) == NULL) {
            break; // Exit on EOF
        }

        // Remove newline
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }

        // Trim spaces
        trim_spaces(input);

        // Tokenize by '|'
        int cmd_count = 0;
        char* cursor = input;
        char* token;

        while ((token = strtok(cursor, "|")) != NULL) {
            trim_spaces(token); // Trim each command
            args[cmd_count++] = token;
            cursor = NULL;
        }
        args[cmd_count] = NULL; // Null-terminate

        // Handle piped commands
        if (cmd_count > 1) {
            pipeline(args, cmd_count);
        } else if (cmd_count == 1) {
            // Initialize parsing index
            i = 0; // Reset index

            char* fargs[32];
            int arg_count = 0;
            char* cmd = args[0];
            char* output = NULL;
            char* input_file = NULL;
            int len = strlen(cmd);

            // Manual parsing for redirection
            while (i < len) {
                while (i < len && isspace(cmd[i])) {
                    i++;
                }

                // Input redirection
                if (cmd[i] == '<') {
                    i++;
                    while (i < len && isspace(cmd[i])) { //skipping whitespaces
                        i++;
                    }
                    input_file = &cmd[i]; //stores filename in the input file
                    while (i < len && !isspace(cmd[i]) && cmd[i] != '\0') {
                        i++;
                    }
                    cmd[i] = '\0'; // Null-terminate the command
                    continue;
                }

                // Output redirection
                if (cmd[i] == '>') {
                    i++;
                    while (i < len && isspace(cmd[i])) { //move over whitespaces
                        i++;
                    }
                    output = &cmd[i]; //storing filename in the ouput file
                    while (i < len && !isspace(cmd[i]) && cmd[i] != '\0') {
                        i++;
                    }
                    cmd[i] = '\0'; // Null-terminate the command
                    continue;
                }

                // Get arguments
                if (!isspace(cmd[i])) {
                    fargs[arg_count++] = &cmd[i]; //move aruments into fargs and increment arg_count
                    while (i < len && !isspace(cmd[i]) && cmd[i] != '<' && cmd[i] != '>') { //skip over any redirection characters
                        i++;
                    }
                    cmd[i++] = '\0'; // Null-terminate the argument
                }
            }
            fargs[arg_count] = NULL; // Null-terminate fargs, useful when executing execvp

            // Handle background processes
            int amp = 0; //bit we will use to determine if an & character is present
            if (arg_count > 0 && fargs[arg_count - 1][strlen(fargs[arg_count - 1]) - 1] == '&') { //checking if last character is &
                amp = 1; //set bit to 1
                fargs[arg_count - 1][strlen(fargs[arg_count - 1]) - 1] = '\0'; //fix null terminator character
                if (strlen(fargs[arg_count - 1]) == 0) { //checking if & is the only character present, if so...
                    fargs[arg_count - 1] = NULL;
                    arg_count--;
                }
            }

            // Save original stdin/stdout, this was due to how i was having errors when writing or reading to a file
            int saved_stdin = dup(STD_INPUT);
            int saved_stdout = dup(STD_OUTPUT);

            // Handle input redirection
            if (input_file != NULL) { //if there is a valud input file
                int fd_input = open(input_file, O_RDONLY);
                if (fd_input < 0) {
                    perror("ERROR: Opening input file failed");
                    continue;
                }
                dup2(fd_input, STD_INPUT); //redirects stdin to the file we want to read from
                close(fd_input); //no longer need it so we close it
            }

            // Handle output redirection
            if (output != NULL) { //if there is a valid output file 
                int fd_output = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0644); //only want to write to it, we then open or create the file, and we want to truncate it back to zero if it already has content present
                if (fd_output < 0) {                                              //0644 permission bits, we can read and write to this file, while others can only read.
                    perror("ERROR: Opening output file failed");
                    continue;
                }
                dup2(fd_output, STD_OUTPUT); //redirects stdout to the file we want to write to
                close(fd_output); //no longer need this so we can close it
            }

            // Fork and execute command
            pid_t pid = fork(); //create child process
            if (pid < 0) { //if negative, fork didn't work correctly
                perror("ERROR: Fork failed");
            } else if (pid == 0) { //child process
                execvp(fargs[0], fargs); //executes the command, first argument, and then the list of arguments
                perror("ERROR: execvp failed");
                exit(1);
            } else { //parent process
                if (!amp) { //if there is no ampersand, amp = 0, we wait for pid. This is where we have to deal with zombie processes.
                    waitpid(pid, NULL, 0);
                }
            }

            // Restore original stdin/stdout
            dup2(saved_stdout, STD_OUTPUT); //restore original stdout
            close(saved_stdout);
            dup2(saved_stdin, STD_INPUT); //restore original stdin
            close(saved_stdin);
        }
    }
    return 0;
}
