#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h> // Required for wait()

int main(int argc, char **argv)
{
    // 1. Check Arguments
    if (argc < 3){
        printf("Usage: %s <foldername> <content1> < \n", argv[0]);
        exit(1);
    }

    char *folderName = argv[1];

    // 2. Compile Task 1 (The new requirement)
    // We fork to run 'gcc' so it doesn't replace our current process yet.
    pid_t compilePid = fork();

    if (compilePid < 0) {
        perror("Fork failed");
        exit(1);
    }
    else if (compilePid == 0) {
        // --- CHILD PROCESS (The Compiler) ---
        // We compile the sibling project's main.c and output it as 'task1_exec'
        // Path adjustment: Assuming we are in 'midTermHWtask2', the other is '../midTermHWtask1'
        execlp("gcc", "gcc", "../midTermHWtask1/midTermHWtask1.c", "-o", "midTermHWtask1exec", NULL);

        // If gcc isn't found or fails:
        perror("Failed to run gcc");
        exit(1);
    }
    else {
        // --- PARENT PROCESS ---
        // Wait for compilation to finish before trying to run it
        wait(NULL);
    }

    // 3. Create the Directory
    if (mkdir(folderName, 0777) < 0){
        perror("Mkdir Error (folder might already exist)");
        // We generally continue even if it exists, or you can exit(2)
    }

    // 4. Change Directory
    // We go inside the new folder so Task 1 creates the file *inside* it
    if (chdir(folderName) < 0) {
        perror("chdir");
        exit(3);
    }

    // 5. Prepare Arguments for Task 1
    // We want to run: ../task1_exec myfolder Today is sunny
    // args[0]: The path to executable (it is now in the parent dir, so "../")
    // args[1]: The filename (same as folder name, which is argv[1])
    // args[2+]: The content

    char *args[argc + 1];

    args[0] = "../midTermHWtask1exec"; // Executable is one level up

    for (int i = 1; i < argc; i++){
        args[i] = argv[i];     // Copy "myfolder", "Today", "is", "sunny"
    }
    args[argc] = NULL;         // Null terminate the list

    // 6. Execute Task 1
    execv(args[0], args);

    // Only reached if execv fails
    perror("execv failed");
    return 4;
}
