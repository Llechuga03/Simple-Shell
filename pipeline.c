#define STD_INPUT 0
#define STD_OUTPUT 1
#include "pipeline.h"
#include "trim_spaces.h"
#include <unistd.h>  
#include <fcntl.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <sys/wait.h> 
#include <signal.h>
#include <stdlib.h>   
#include <stdio.h>    
#include <string.h>
#include <ctype.h>
#include "sigchild.h"

/*function to hanlde zombie processes*/
void sigchild(int val);

/*function to remove leading and trailing spaces*/
void trim_spaces(char* line);


void pipeline(char* args[], int cmd_count) {
    int fd[cmd_count - 1][2]; // Pipe file descriptors, we don't need it for the last command piped
    int i; // Counter variable for loop
    int amp = 0;

    // Check for background execution
    if (args[cmd_count - 1][strlen(args[cmd_count - 1]) - 1] == '&') {
        amp = 1;
        args[cmd_count - 1][strlen(args[cmd_count - 1]) - 1] = '\0'; // Remove '&'
        trim_spaces(args[cmd_count - 1]); // Trim spaces
    }

    // Create pipes
    for (i = 0; i < cmd_count - 1; i++) {
        if (pipe(fd[i]) == -1) {
            perror("ERROR: Pipe Failed");
            exit(1);
        }
    }

    for (i = 0; i < cmd_count; i++) {
        int pid = fork();
        if (pid < 0) { // Fork failed
            perror("ERROR: Fork Failed");
            exit(1);
        }

        if (pid == 0) { // Child process
            // If not the first command, read from the previous pipe
            if (i > 0) {
                dup2(fd[i - 1][0], STD_INPUT); // Read from previous pipe
            }
            // If not the last command, write to the next pipe
            if (i < cmd_count - 1) {
                dup2(fd[i][1], STD_OUTPUT); // Write to current pipe
            }

            // Close all pipe file descriptors in the child
            for (int j = 0; j < cmd_count - 1; j++) {
                close(fd[j][0]);
                close(fd[j][1]);
            }

            // Handle command and redirection logic
            char* cmd_args[32];
            char* token;
            int arg_count = 0;
            char* cmd = args[i];
            char* arg_cursor = cmd;
            char* output = NULL;
            char* input = NULL;

            // Tokenizing input
            while ((token = strtok(arg_cursor, " ")) != NULL) {
                if (strcmp(token, ">") == 0) {
                    output = strtok(NULL, " ");// Fetch output file
                    if(output == NULL)
                    {
                        perror("ERROR: no output file specified");
                        exit(1);
                    }
                } else if (strcmp(token, "<") == 0) {
                    input = strtok(NULL, " "); // Fetch input file
                    if(input == NULL)
                    {
                        perror("ERROR: No input file specified");
                        exit(1);
                    }
                } else {
                    cmd_args[arg_count++] = token; // Save token to command args
                }
                arg_cursor = NULL; // Continue tokenizing the same command
            }
            cmd_args[arg_count] = NULL; // Null-terminate the command arguments

            // Handle output redirection
            if (output != NULL) {
                int fd_output = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd_output < 0) {
                    perror("ERROR: Opening output file failed");
                    exit(1);
                }
                dup2(fd_output, STD_OUTPUT); // Redirect STDOUT to the file
                close(fd_output); // Close the output file descriptor
            }

            // Handle input redirection
            if (input != NULL) {
                int fd_input = open(input, O_RDONLY);
                if (fd_input < 0) {
                    perror("ERROR: Opening input file failed");
                    exit(1);
                }
                dup2(fd_input, STD_INPUT); // Redirect STDIN to the file
                close(fd_input); // Close the input file descriptor
            }

            // Execute the command
            if (execvp(cmd_args[0], cmd_args) == -1) {
                perror("ERROR: execvp failed");
                exit(1);
            }
        } else { // Parent process
            // Close unused pipe ends
            if (i > 0) {
                close(fd[i - 1][0]); // Close read end of previous pipe
            }
            if (i < cmd_count - 1) {
                close(fd[i][1]); // Close write end of current pipe
            }

            if (!amp) { // If not background execution, wait for child
                waitpid(pid, NULL, 0); // Wait for child to finish
            }
        }
    }

    // Close the last read end
    close(fd[cmd_count - 2][0]);

    // Wait for all child processes if not background
    if (!amp) {
        for (int i = 0; i < cmd_count; i++) {
            wait(NULL); // Wait for all child processes
        }
    }
}
