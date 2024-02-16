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
    char* res = fgets(input, MAX_INPUT_SIZE, stdin);
    // if EOF is recieved end gracefully
    if(res == NULL){
        exit(0);
    }
    // remove the trailing newline
    input[strcspn(input, "\n")] = '\0';
} 

int parse_input(char* input, char *args[]){
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
    return argc;
}

int main() {
    char input[MAX_INPUT_SIZE];
    char *args[MAX_ARGS];

    while (1) {
        // Read input from the user
        read_input(input);
        // Parse input into arguments
        int argCount = parse_input(input, args);
        // if nothing was entered just prompt again
        if(argCount == 0){
            continue;
        }
        // Execute the command
        pid_t pid = fork();
        // exec the program in the child process so the shell or parent process can wait for it to finish
        if (pid == 0) {
            // Child process - here we will execute the command
            execvp(args[0], args);
            // If execvp returns, it means an error occurred
            perror("execvp");
        } else {
            // Parent process - here we will wait for the child command to finish
            int status;
            waitpid(pid, &status, 0);
        }
    }
    return 0;
}
