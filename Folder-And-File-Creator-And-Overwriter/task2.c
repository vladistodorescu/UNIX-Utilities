#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>   
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
    if (argc < 3){
        perror("At least 3 arguments required!");
        exit(1);
    }
    
    char fileAndFolderName[256];
    strncpy(fileAndFolderName, argv[1], sizeof(fileAndFolderName) - 1);
    fileAndFolderName[sizeof(fileAndFolderName) - 1] = '\0';
    
    if (mkdir(fileAndFolderName, 0777) < 0){
        perror("Mkdir Error!");
        exit(2);
    }
    
    if (chdir(fileAndFolderName) < 0) {
        perror("chdir");
        exit(3);
    }
    
    char *args[argc + 1];
    args[0] = "../../Task1/task1exec";
    args[1] = fileAndFolderName;
    for (int i = 2; i < argc ; i++){
        args[i] = argv[i];
    }
    args[argc] = NULL;
    
    execv(args[0], args);
    
    // only reached if execv failed
    perror("execv");
    return 4;
}
