#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
char buf[255];
void printArr(char** ptr);
int main(){
    while(1){
        printf("shelly $ ");
        char input[255];
        fgets(input,255,stdin);
        // Remove \n
        size_t len = strlen(input);
        input[len-1] = '\0';
        char* token =strtok(input, " ");
        char* program =token;
        char** args = calloc(8, sizeof(char*));
        args[0]=program;
        int i=1,n=8;
        while(token!=NULL){
            if(n==i) //allocate more space for arguments
                args= realloc(args, (n*=2)*sizeof(char*));
            token = strtok(NULL, " ");
            args[i] = token;
            i++;
        }
        assert(args[i]==NULL);
        pid_t p= fork();
        if(p<0){
            perror("fork fail");
            exit(1);
        }
        else if (p==0){
            execvp(program, args);
            perror("execvp");
        }
        int status;
        wait(&status);
        free(args);
    }
    return 0;
}

void printArr(char** ptr){
    while(*ptr){
        printf("%s,",*ptr);
        ptr++;
    }
}
