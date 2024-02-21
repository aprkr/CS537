#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

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
        for(int i = 0; i < 5; i++){
            if(strcmp(history[i],"") == 0){
                break;
            }
            else{
                printf("%i) %s\n", (i + 1), history[i]);
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
        // check to see if there was a builtin command being run, will return 1 if built in ran 0 if not
        if(check_builtin(argCount, args) == 1){
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
            exit(-1);
        } else {
            // Parent process - here we will wait for the child command to finish
            int status;
            waitpid(pid, &status, 0);
        }
        addCmdHist(args, argCount);
    }
    return 0;
}
