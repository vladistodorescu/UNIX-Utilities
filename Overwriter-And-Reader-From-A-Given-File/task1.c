#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
    if (argc < 3){
        printf("Not enough arguments! Format: %s <filename> <content_1> <content_2> ...", argv[0]);
        exit(1);
    }

    char fileName[256];
    char content[256] = "";

    for (int i = 0; i < argc; i++){
        if (i == 1){
            snprintf(fileName, sizeof(fileName), "%s", argv[i]);
        }

        if (i > 1){
            strcat(content, argv[i]);
        }
    }

    FILE *file = fopen(fileName, "w+");
    if (file == NULL){
        perror("Error at fopen");
        exit(2);
    }

    pid_t pid = fork();

    // Error when forking
    if (pid < 0){
        perror("fork");
        exit(3);
    }
    // Child process writes to file
    else if (pid == 0){
        fputs(content, file);
        fclose(file);
        exit(0);
    }
    // Parent for file to be written too, then reads it
    else {
        wait(NULL);
        rewind(file);

        int ch;
        while ((ch = fgetc(file)) != EOF) {
            printf("%c", ch);
        }
        printf("\n");
        fclose(file);
    }

    return 0;
}

