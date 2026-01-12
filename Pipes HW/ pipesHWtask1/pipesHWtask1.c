#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>

// --- THE DATA STRUCTURE ---
// We cannot just send a 'char' because we need to attach PIDs.
// We send this "Folder" through the pipes instead.
struct Packet {
    char value;         // The actual character from the file
    int pids[100];      // Room for children to sign their names (PIDs)
    int step;           // Counter to know who is signing next (0, 1, 2...)
};

int main(int argc, char **argv) {
    // --- 1. ARGUMENT CHECK ---
    if (argc != 3){
        fprintf(stderr, "Usage: %s <nr_of_subprocesses> <path_to_file_F>\n", argv[0]);
        exit(1);
    }

    int N = atoi(argv[1]);
    if (N <= 0){
        fprintf(stderr, "Error: N must be > 0\n");
        exit(1);
    }

    char *filename = argv[2];
    int file_fd = open(filename, O_RDONLY);
    if(file_fd < 0) {
        perror("Error opening file");
        exit(1);
    }

    // --- 2. SETUP PIPES ---
    // We need N+1 pipes for the full ring: Parent -> C0 -> C1 ... -> CN -> Parent
    int num_pipes = N + 1;
    int pipes[num_pipes][2];

    for (int i = 0; i < num_pipes; i++){
        if (pipe(pipes[i]) < 0) {
            perror("Error creating pipe");
            exit(1);
        }
    }

    // --- 3. CREATE CHILDREN ---
    for (int i = 0; i < N; i++){
        int pid = fork();

        if (pid < 0){
            perror("Fork failed");
            exit(1);
        }

        // === CHILD PROCESS LOGIC ===
        if (pid == 0){
            // 1. Connect Left Hand (Read from pipes[i])
            dup2(pipes[i][0], STDIN_FILENO);

            // 2. Connect Right Hand (Write to pipes[i+1])
            dup2(pipes[i + 1][1], STDOUT_FILENO);

            // 3. Close ALL raw pipe descriptors to prevent deadlocks
            for (int j = 0; j < num_pipes; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // 4. Processing Loop
            struct Packet pkg;
            // Read a full packet struct, not just a char
            while (read(STDIN_FILENO, &pkg, sizeof(pkg)) > 0) {

                // --- VALIDATION STEP ---
                // Attach my PID to the list
                pkg.pids[pkg.step] = getpid();
                pkg.step++; // Move to next slot for the next child

                // Pass the folder to the next person
                write(STDOUT_FILENO, &pkg, sizeof(pkg));
            }

            // Clean exit
            close(STDIN_FILENO);
            close(STDOUT_FILENO);
            exit(0);
        }
    }

    // === PARENT LOGIC ===

    // We split the Parent into two parts: 
    // A. "Sender" (puts data into the ring)
    // B. "Receiver" (gets data out of the ring)
    // We fork a helper for the Sender part so the Parent doesn't get stuck.

    int sender_pid = fork();

    if (sender_pid == 0) {
        // --- SENDER PROCESS ---
        // Writes to pipes[0] (The start of the ring)

        close(pipes[0][0]); // We don't read, we write

        char c;
        struct Packet pkg;

        while(read(file_fd, &c, 1) > 0) {
            // Requirement: Process even-value characters
            if (c % 2 == 0) {
                pkg.value = c;
                pkg.step = 0; // Reset counter

                // Send the packet into the ring
                write(pipes[0][1], &pkg, sizeof(pkg));
            }
        }

        // Close up shop
        close(pipes[0][1]);
        // Close other unused pipes
        for(int j=1; j<num_pipes; j++) { close(pipes[j][0]); close(pipes[j][1]); }
        close(file_fd);
        exit(0);
    }

    // --- RECEIVER PROCESS (Main Parent) ---
    // Reads from pipes[N] (The end of the ring)

    // 1. Close all pipes except the one we read from (pipes[N])
    for (int i = 0; i < num_pipes; i++) {
        if (i != N) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        } else {
            close(pipes[i][1]); // Close write-end of our input
        }
    }
    close(file_fd); // Sender has this, we don't need it

    // 2. Read Results
    struct Packet result_pkg;
    printf("\n--- FINAL RESULTS ---\n");

    while(read(pipes[N][0], &result_pkg, sizeof(result_pkg)) > 0) {
        // Requirement: Double the value and print Hex
        int doubled = result_pkg.value * 2;

        printf("Original: '%c' | Doubled (Hex): 0x%X | Validated by PIDs: ",
               result_pkg.value, doubled);

        // Print the PIDs attached by the children
        for(int k=0; k < result_pkg.step; k++) {
            printf("%d ", result_pkg.pids[k]);
        }
        printf("\n");
    }

    // 3. Wait for everyone to finish
    wait(NULL); // Wait for sender helper
    for(int i=0; i<N; i++) wait(NULL); // Wait for children

    return 0;
}
