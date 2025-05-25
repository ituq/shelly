#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
char buf[255];
void printArr(char** ptr);
char** parsePureCommand(char* string);

#define MAX_COMMAND_LENGTH 255
int main(){
    while(1){
        printf("shelly $ ");

        char input[MAX_COMMAND_LENGTH];
        fgets(input,MAX_COMMAND_LENGTH,stdin);

        // Remove \n
        size_t len = strnlen(input,MAX_COMMAND_LENGTH);
        input[len-1] = '\0';
        //check for piping final result to file (>)
        int output_fd= STDOUT_FILENO;
        int stdout_saved =-1;
        char* token=strtok(input, ">");
        char* fileName= input+len; //start of file name if applicable
        if(token!=input){
            fileName=strtok(NULL, " >"); //start of file name if applicable
            printf("file to write to: '%s'\n",fileName);
            output_fd=open(fileName, O_CREAT | O_WRONLY | O_TRUNC, 0644);
            stdout_saved=dup(STDOUT_FILENO);
            dup2(output_fd, STDOUT_FILENO);
        }
        int numCommands=0;
        char* iter=input;
        while(iter!=fileName){
            if(*(iter++) == '|')
                numCommands++;
        }
        numCommands++;
        char* command = strtok(input, "|");
        int prevPipeOut =STDIN_FILENO;
        for (int i=0; i<numCommands; i++) {
            int pipefd[2];
            if (i < numCommands - 1) {
                    pipe(pipefd); // create a pipe for all but the last command
            }
            char** args;
            pid_t pid = fork();
            if (pid==0) { //child
                if(i>0){
                    dup2(prevPipeOut, STDIN_FILENO);
                    close(prevPipeOut);
                }
                if(i<numCommands-1){
                    close(pipefd[0]);
                    dup2(pipefd[1],STDOUT_FILENO);
                    close(pipefd[1]);
                }
                if(i==numCommands-1 && output_fd != STDOUT_FILENO){
                    dup2(output_fd, STDOUT_FILENO);
                }
                int n= strnlen(command, MAX_COMMAND_LENGTH);
                char copy[MAX_COMMAND_LENGTH];
                strncpy(copy, command, MAX_COMMAND_LENGTH);
                args = parsePureCommand(copy);
                char* program =args[0];
                execvp(program, args);

                exit(1);
            }
            else{ //parent
                if(i>0) close(prevPipeOut);
                if (i < numCommands - 1) {
                    close(pipefd[1]); // close write end in parent
                    prevPipeOut = pipefd[0]; // save read end for next command
                }
                if(i>0 && i==numCommands-1 && output_fd != STDOUT_FILENO) close(output_fd);
                wait(NULL);
            }
            free(args);
            command =strtok(NULL, "|");
        }
        
        // Restore stdout if it was redirected
        if(stdout_saved != -1) {
            dup2(stdout_saved, STDOUT_FILENO);
            close(stdout_saved);
            close(output_fd);
            stdout_saved = -1;
        }
    }
    return 0;
}

char** parsePureCommand(char* string){
    //parses pure command without pipes and alike
    char** args = calloc(8, sizeof(char*));
    char *saveptr;
    char* token =strtok_r(string, " ", &saveptr);
    args[0]=token;
    int i=1,n=8;
    while(token!=NULL){
        if(n==i) //allocate more space for arguments
            args= realloc(args, (n*=2)*sizeof(char*));
        token = strtok_r(NULL, " ",&saveptr);
        args[i] = token;
        i++;
    }
    assert(args[i]==NULL);
    return args;
}
void printArr(char** ptr){
    while(*ptr){
        printf("%s,",*ptr);
        ptr++;
    }
}
