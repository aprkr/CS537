#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_INPUT_SIZE 2048
#define MAX_ARGS 128

// a struct to hold the local variables in a linked list
typedef struct shellVar{
    char* name;
    char* value;
    struct shellVar * nextVar;
    struct shellVar * prevVar;
} SHELLVAR;

// global head to our linked list
SHELLVAR* head = NULL;

// a history array to keep all the commands neatly organized
typedef struct historyNode{
    char* content;
    struct historyNode * nextNode;
} HISTORYNODE;

// head to enter the history 
HISTORYNODE* historyHead = NULL;
int historyLen = 5;

SHELLVAR* getLocalVar(char* var){
    SHELLVAR* curr = head;
    while(curr != NULL){
        if(strcmp(curr->name, var) == 0){
            return curr;
        }
        curr = curr->nextVar;
    }
    return NULL;
}

// Function to trim leading and trailing whitespace from a string
char *trim_whitespace(char *str) {
    while (isspace((unsigned char)*str)) str++; // Trim leading whitespace
    if (*str == 0) return str; // If the string is empty after trimming leading whitespace, return
    char *end = str + strlen(str) - 1; // Pointer to the last character of the string
    while (end > str && isspace((unsigned char)*end)) end--; // Trim trailing whitespace
    end[1] = '\0'; // Null-terminate the trimmed string
    return str;
}

void addLocalVar(int argCount, char* args[]){
    // Parse the input to extract the variable name and value
    char *input = args[1];
    char *equal_sign = strchr(input, '=');
    if (equal_sign == NULL) {
        printf("ERROR: Usage local VAR=<value\n");
        exit(-1);
    }
    // Replace '=' with '\0' to split the string into the two seperate args to setenv
    *equal_sign = '\0';
    // grab both the variable name and the value
    char *variable = (char*) malloc(strlen(input) * sizeof(char));
    strcpy(variable, input);
    char *value = (char*) malloc(strlen(equal_sign + 1) * sizeof(char));
    strcpy(value, equal_sign + 1);
    
    SHELLVAR* loc = getLocalVar(variable);
    if(loc != NULL){
        // if it is set to the empty string then delete it from the linked list
        if(strcmp(value, "") == 0){
        	if(loc == head){
        		head = loc->nextVar;
                free(loc);
        	}
        	else{
        		SHELLVAR* next = loc->nextVar;
        		SHELLVAR* prev = loc->prevVar;
                if(next != NULL){
					next->prevVar = prev;	
				}
				if(prev != NULL){
					prev->nextVar = next;
				}
        	}
        }
        else{
            loc->value = value;
        }
    }
    else{
        // create new shell var that will be stored in the linked list
        SHELLVAR* new = (SHELLVAR*) malloc(sizeof(SHELLVAR));
        if(new == NULL){
            printf("Failed to set the local variable\n");
            exit(-1);
        }
        // set the struct to hold info
        new->name = variable;
        new->value = value;
        new->nextVar = NULL;
        new->prevVar = NULL;
        // insert at the end of the linked list
        if(head == NULL){
            head = new;
        }
        else{
            SHELLVAR* curr = head;
            while(curr->nextVar != NULL){
                curr = curr->nextVar;
            }
            curr->nextVar = new;
            new->prevVar = curr;
        }
    }
}

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
        // check to see if the arg is a variable
        char* signPos = strchr(token, '$');
        if((signPos != NULL) && (signPos - token == 0)){
            char* var = token + sizeof(char);
            // if it is then check to see if it is a local var
            token = getenv(var);
            // if not check to see if it is a env var
            if(token == NULL){
                SHELLVAR* localVar = getLocalVar(var);
                // if not a local var then set token to be the default "" string
                if(localVar == NULL){
                    token = strtok(NULL, " ");
                    continue;
                }
                else{
                    token = localVar->value;
                }
            }
        }
        args[argc++] = token;
        token = strtok(NULL, " ");
    }
    // list of args must be null-terminated to work with execvp
    args[argc] = NULL;
    return argc;
}

void executePipe(char* args[], int argc){
    // Calculate the total length of the concatenated string
    int totalLength = 0;
    for (int i = 0; i < argc; i++) {
        totalLength += strlen(args[i]);
    }

    // Space between strings
    totalLength += argc - 1;

    // Allocate memory for the concatenated string (+1 for the null terminator)
    char* concatenatedString = (char*)malloc(totalLength + 1);
    // Copy each string from the array into the concatenated string
    int currentIndex = 0;
    for (int i = 0; i < argc; i++) {
        strcpy(concatenatedString + currentIndex, args[i]);
        currentIndex += strlen(args[i]);
        // add back the space in between the args
        if (i < argc - 1) {
            concatenatedString[currentIndex++] = ' '; // Add space
        }
    }

    // Add null terminator at the end
    concatenatedString[currentIndex] = '\0';
    
    char *token;
    char *commands[10]; // Max 10 commands, adjust as needed
    int i = 0;

    // Split the command into individual commands based on the pipe character "|"
    token = strtok(concatenatedString, "|");
    while (token != NULL) {
        commands[i++] = trim_whitespace(token);
        token = strtok(NULL, "|");
    }
    commands[i] = NULL;

    int fd[2]; // File descriptors for pipe
    pid_t pid;
    int prev_read = 0; // File descriptor to store the previous command's output

    // Iterate over each command
    for (int j = 0; j < i; j++) {
        pipe(fd); // Create a pipe for communication between commands

        if ((pid = fork()) == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process

            // Redirect stdin if not the first command
            if (j != 0) {
                dup2(prev_read, 0);
                close(prev_read);
            }

            // Redirect stdout if not the last command
            if (j != i - 1) {
                dup2(fd[1], 1);
            }

            char *argsCMD[MAX_ARGS];
            parse_input(commands[j], argsCMD);

            close(fd[0]); // Close unused read end
            execvp(argsCMD[0], argsCMD);
            perror(commands[j]);
            exit(EXIT_FAILURE);
        } else {
            // Parent process
            wait(NULL); // Wait for child process to finish
            close(fd[1]); // Close write end to avoid deadlock
            prev_read = fd[0]; // Store read end for the next command
        }
    }
}

int check_builtin(int argCount, char* args[]){
    // EXIT
    if(strcmp(args[0], "exit") == 0){
        exit(0);
    }
    // CD
    else if(strcmp(args[0], "cd") == 0){
        // check to make sure that the cd command only has one arg after it
        if(argCount != 2){
            printf("ERROR: Usage cd <path>\n");
            exit(-1);
        }
        else{
            // if chdir is sucessfull it will return zero, if non-zero error out
            if (chdir(args[1]) != 0){
                printf("cd to %s failed\n", args[1]);
                exit(-1);
            }
            // return 1 to notify main loop to prompt again
            return 1;
        }
    }
    // EXPORT
    else if(strcmp(args[0], "export") == 0){
        // check to make sure that there is only two arguments supplied
        if(argCount != 2){
            printf("ERROR: Usage export VAR=<value>\n");
            exit(-1);
        }
        else{
            // Parse the input to extract the variable name and value
            char *input = args[1];
            char *equal_sign = strchr(input, '=');
            if (equal_sign == NULL) {
                printf("ERROR: Usage export VAR=<value>\n");
                exit(-1);
            }
            // Replace '=' with '\0' to split the string into the two seperate args to setenv
            *equal_sign = '\0'; 
            char *variable = input;
            char *value = equal_sign + 1;
            // Remove quotes from the value if present
            if (value[0] == '\"' && value[strlen(value) - 1] == '\"') {
                value[strlen(value) - 1] = '\0';
                value++;
            }
            // Set the environment variable
            if (setenv(variable, value, 1) != 0) {
                printf("Failed to set the enviroment variable\n");
                exit(-1);
            }
        }
        return 1;
    }
    // LOCAL
    else if(strcmp(args[0], "local") == 0){
        // check to make sure that there is only two arguments supplied
        if(argCount != 2){
            printf("ERROR: Usage local VAR=<value>\n");
            exit(-1);
        }
        addLocalVar(argCount, args);
        return 1;
    }
    // VARS
    else if(strcmp(args[0], "vars") == 0){
        SHELLVAR* curr = head;
        // if there are none then just print the newline
        if(curr == NULL){
            return 1;
        }
        // iterate through the linked list to print the local vars
        else{
            while(curr != NULL){
                printf("%s=%s\n", curr->name, curr->value);
                curr = curr->nextVar;
            }
        }
        return 1;
    }
    // HISTORY
    else if(strcmp(args[0], "history") == 0){
        HISTORYNODE* curr = historyHead;
        if(curr == NULL && argCount != 3){
            return 1;
        }
        if(argCount == 1){
            for(int i = 0; i < historyLen; i++){
                printf("%i) %s\n", (i + 1), curr->content);
                curr = curr->nextNode;
                if(curr == NULL){
                    break;
                }
            }
        }
        else if((argCount == 2)){
            for(int i = 0; i < strtol(args[1], NULL, 10) - 1; i++){
                curr = curr->nextNode;
                if(curr == NULL){
                    return 1;
                }
            }
            char *args[MAX_ARGS];
            // Parse input into arguments
            char* cmd = (char*) malloc(sizeof(char) * strlen(curr->content));
            strcpy(cmd, curr->content);
            parse_input(cmd, args);
            // Execute the command
            pid_t pid = fork();
            // exec the program in the child process so the shell or parent process can wait for it to finish
            if (pid == 0) {
                // Child process - here we will execute the command
                execvp(args[0], args);
                // If execvp returns, it means an error occurred
                perror("execvp");
                exit(-1);
            } else {
                // Parent process - here we will wait for the child command to finish
                int status;
                waitpid(pid, &status, 0);
            }
            free(cmd);
        }
        else if(argCount == 3){
            historyLen = strtol(args[2], NULL, 10);
            for(int i = 0; i < strtol(args[2], NULL, 10) - 1; i++){
                if(curr == NULL){
                    break;
                }
                curr = curr->nextNode;
            }
            if(curr != NULL){
                curr->nextNode = NULL;
            }
        }
        return 1;
    }
    else{
        // command was not built in so we will use execvp
        return 0;
    }
}

void addCmdHist(char* args[], int argc){
    // Calculate the total length of the concatenated string
    int totalLength = 0;
    for (int i = 0; i < argc; i++) {
        totalLength += strlen(args[i]);
    }

    // Space between strings
    totalLength += argc - 1;

    // Allocate memory for the concatenated string (+1 for the null terminator)
    char* concatenatedString = (char*)malloc(totalLength + 1);
    // Copy each string from the array into the concatenated string
    int currentIndex = 0;
    for (int i = 0; i < argc; i++) {
        strcpy(concatenatedString + currentIndex, args[i]);
        currentIndex += strlen(args[i]);
        // add back the space in between the args
        if (i < argc - 1) {
            concatenatedString[currentIndex++] = ' '; // Add space
        }
    }

    // Add null terminator at the end
    concatenatedString[currentIndex] = '\0';

    // do not add to list if is already at the head
    if(historyHead != NULL && strcmp(historyHead->content, concatenatedString) == 0){
        return;
    }

    // create new node in the history linked list
    HISTORYNODE* new = (HISTORYNODE*) malloc(sizeof(HISTORYNODE));
    // assign values to the created struct
    new->content=concatenatedString;
    new->nextNode = NULL;

    // create the list that will hold all of the history entries
    if(historyHead == NULL){
        historyHead = new;
    }
    else{
        new->nextNode = historyHead;
        historyHead = new;
    }
}

int main(int argc, char* args[]) {
    // launch interactive mode
    if(argc == 1){
        char input[MAX_INPUT_SIZE];
        char *argsCMD[MAX_ARGS];
        while (1) {
            // Read input from the user
            read_input(input);
            // Parse input into arguments
            int argCount = parse_input(input, argsCMD);
            // if nothing was entered just prompt again
            if(argCount == 0){
                continue;
            }
            // check to see if there was a builtin command being run, will return 1 if built in ran 0 if not
            if(check_builtin(argCount, argsCMD) == 1){
                continue;
             }
            // check to see if it is a piped command
            int piped = 0;
            for(int i = 0; i < argCount; i++){
                char* signPipe = strchr(argsCMD[i], '|');
                if(signPipe != NULL){
                    piped = 1;
                    executePipe(argsCMD, argCount);
                    addCmdHist(argsCMD, argCount);
                    continue;
                }
            }
            if(piped == 1){
                continue;
            }
            // Execute the command
            pid_t pid = fork();
            // exec the program in the child process so the shell or parent process can wait for it to finish
            if (pid == 0) {
                // Child process - here we will execute the command
                execvp(argsCMD[0], argsCMD);
                // If execvp returns, it means an error occurred
                perror("execvp");
                exit(-1);
            } else {
                // Parent process - here we will wait for the child command to finish
                int status;
                waitpid(pid, &status, 0);
            }
            addCmdHist(argsCMD, argCount);
        }
        return 0;
    }
    // launch batch mode
    else{
        FILE *file;
        char line[MAX_INPUT_SIZE];
        char *argsCMD[MAX_ARGS];

        file = fopen(args[1], "r");

        if (file == NULL) {
            printf("Error opening file\n");
            return 1;
        }
        while (fgets(line, sizeof(line), file) != NULL) {
            line[strcspn(line, "\n")] = '\0';
            int argCount = parse_input(line, argsCMD);
            // if nothing was entered just prompt again
            if(argCount == 0){
                continue;
            }
            // check to see if there was a builtin command being run, will return 1 if built in ran 0 if not
            if(check_builtin(argCount, argsCMD) == 1){
                continue;
            }
            // check to see if it is a piped command
            for(int i = 0; i > argCount; i++){
                char* signPipe = strchr(argsCMD[i], '|');
                if(signPipe != NULL){
                    executePipe(argsCMD, argCount);
                    addCmdHist(argsCMD, argCount);
                    continue;
                }
            }
            // Execute the command
            pid_t pid = fork();
            // exec the program in the child process so the shell or parent process can wait for it to finish
            if (pid == 0) {
                // Child process - here we will execute the command
                execvp(argsCMD[0], argsCMD);
                // If execvp returns, it means an error occurred
                perror("execvp");
                exit(-1);
            } else {
                // Parent process - here we will wait for the child command to finish
                int status;
                waitpid(pid, &status, 0);
            }
            addCmdHist(argsCMD, argCount);
        }
        return 0;
    }
    
}
