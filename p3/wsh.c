#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
    while(1){
        // interactive mode
        if(argc == 1){
            // get and store cmd from stdin
            char* cmd;
            size_t cmd_len = 250;
            cmd = (char*) malloc(cmd_len * sizeof(char));
            if(cmd == NULL){
                printf("Could not malloc the buffer to hold the command");
                exit(1);
            }
            // prompt user for input
            printf("wsh> ");
            getline(&cmd, &cmd_len, stdin);
            // split up the input so the args and cmd can be passed seperatly
            strtok();
            execvp(cmd);
        }
        // batch mode
        else{

        }
    }
}
