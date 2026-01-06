#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char **argv) {
    // Check if we have commands to run
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <cmd1> <cmd2> <cmd3> ...\n", argv[0]);
        fprintf(stderr, "Example: %s ls date whoami\n", argv[0]);
        exit(1);
    }

    printf("--- Mini CLI Wrapper Started ---\n");

    // Iterate through all arguments passed to the program
    for (int i = 1; i < argc; i++) {
        
        printf("\n[Executor] Launching command: '%s'\n", argv[i]);

        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork failed");
            exit(1);
        }

        if (pid == 0) {
            // --- CHILD PROCESS ---
            // execlp searches the PATH for the command name
            execlp(argv[i], argv[i], NULL);
            
            fprintf(stderr, "Error: Command '%s' not found or invalid.\n", argv[i]);
            exit(1);
        } 
        else {
            // --- PARENT PROCESS ---
            // We wait for the current command to finish before starting the next one.
            int status;
            wait(&status);
            
            if (WIFEXITED(status)) {
                printf("[Executor] Command '%s' finished with exit code %d.\n", argv[i], WEXITSTATUS(status));
            }
        }
    }

    printf("\n--- All commands executed. ---\n");
    return 0;
}
