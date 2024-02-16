#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_INPUT_SIZE 2048
#define MAX_ARGS 128

void read_input(char *input){
    // prompt user and get input from them
    printf("wsh> ");
    fgets(input, MAX_INPUT_SIZE, stdin);
    // remove the trailing newline
    input[strcspn(input, "\n")] = '\0';
} 

void parse_input(char* input, char *args[]){
    // keeps track of the amount of args present
    int argc = 0;
    // seperates the args by spaces
    char *token = strtok(input, " ");
    while (token != NULL && argc < MAX_ARGS - 1) {
        args[argc++] = token;
        token = strtok(NULL, " ");
    }
    // list of args must be null-terminated to work with execvp
    args[argc] = NULL;
}

int main() {
    char input[MAX_INPUT_SIZE];
    char *args[MAX_ARGS];

    while (1) {
        // Read input from the user
        read_input(input);

        // Parse input into arguments
        parse_input(input, args);

        // Execute the command
        pid_t pid = fork();

        if(*args[0] == EOF){
            printf("exiting\n");
            exit(0);
        }
        if (pid < 0) {
            perror("fork");
        } else if (pid == 0) {
            // Child process
            execvp(args[0], args);
            // If execvp returns, it means an error occurred
            perror("execvp");
        } else {
            // Parent process
            int status;
            waitpid(pid, &status, 0);
        }
    }

    return 0;
}