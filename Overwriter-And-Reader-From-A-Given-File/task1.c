#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
    if (argc < 3){
        exit(1);
    }
    
    char fileName[256];
    char content[256];
    content[0] = '\0';

    for (int i = 0; i < argc; i++){
        if (i == 1){
            strncpy(fileName, argv[i], sizeof(fileName) - 1);
            fileName[sizeof(fileName) - 1] = '\0';   
        }
        
        if (i > 1){
            strcat(content, argv[i]);
        }
    }
        
    FILE *file = fopen(fileName, "w+");
    if (f == NULL){
        perror("Error at fopen");
        exit(2);
    }
    
    pid_t pid = fork();
    
    // error when forking
    if (pid < 0){
        perror("fork");
        exit(3);
    }
    // child process
    else if (pid == 0){
        fputs(content, file);
        fclose(file);
        exit(0);
    }
    else {
        wait(NULL);  
        rewind(file);
        
        int ch;
        while ((ch = fgetc(file)) != EOF) {
            putchar(ch);
        }
        putchar('\n');
        fclose(file);
    } 
        
    return 0;
}
