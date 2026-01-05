#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char **argv) {
    // If argument count wrong, throw error
    if (argc != 5) {
        perror("Exactly 5 arguments required for file compilation! ./[application-exec-name] [compiler] [file_name] [mode] [executable_name]");
        exit(1);
    }
  
    char compiler[10];
    strcpy(compiler, argv[1]);
  
    // If compiler incorrect, throw error
    if (strcmp(compiler, "gcc") != 0){
        perror("This application only supports gcc compilation, please correct the compiler name!");
        exit(2);
    } 

    char fileName[100];
    strcpy(fileName, argv[2]);

    char mode[10];
    strcpy(mode, argv[3]);

    char executableName[100];
    strcpy(executableName, argv[4]);
    
    pid_t pid = fork();

    if (pid < 0){
        perror("Fork failed"); 
        exit(3);
    }
    if (pid == 0) {
        printf("[Installer] Compiling hello.c...\n");
        // Searches the file gcc in the PATH folders
        // Equivalent to: gcc hello.c -o hello a.k.a compile file 'hello.c'
        execlp("gcc", compiler, fileName, mode, executableName, NULL);
        
        // If exec returns, it failed
        perror("Compilation failed");
        exit(4);
    } 
    else {
        // --- Parent Process ---
        int status;
        wait(&status); // Wait for compilation to finish

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("[Installer] Compilation successful. Launching program...\n");
            
            pid_t pid2 = fork();
            if (pid2 == 0) {
                // --- Child Process 2: The Application ---
                char runPath[150];
                snprintf(runPath, sizeof(runPath), "./%s", executableName);
              
                execl(runPath, executableName, NULL);
                
                perror("Execution failed");
                exit(1);
            }
            wait(NULL); // Wait for the program to finish
        } else {
            printf("[Installer] Compilation failed. Cannot run.\n");
        }
    }
    return 0;
}
