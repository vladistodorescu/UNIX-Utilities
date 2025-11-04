// This program is operated through a CLI and expects 3 arguments: [executable_name] [mode] [path]
// If mode = "parse" --> recursively go down the file tree, and print all contents, with recieved path as starting point
// If mode = "details" --> if it's a directory, then print out all the details about the files in the directory (similar to "ls -al" command), if it's a file print out it's details (similar to "ls -l" command)

#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>

bool isFile(char *filePath){
    struct stat st;
    return lstat(filePath, &st) == 0 && S_ISREG(st.st_mode);
}

bool isDir(char *filePath){
    struct stat st;
    return lstat(filePath, &st) == 0 && S_ISDIR(st.st_mode);
}

char *modeString(mode_t m){
    static char modeStr[11];

    modeStr[0] = S_ISDIR(m) ? 'd' :           // data type
                 S_ISLNK(m) ? 'l' :
                 S_ISCHR(m) ? 'c' :
                 S_ISBLK(m) ? 'b' :
                 S_ISFIFO(m) ? 'p' :
                 S_ISSOCK(m) ? 's' : '-';
    modeStr[1] = (m & S_IRUSR) ? 'r' : '-';   // user read permission
    modeStr[2] = (m & S_IWUSR) ? 'w' : '-';   // user write permission
    modeStr[3] = (m & S_IXUSR) ? 'x' : '-';   // user execute permission
    modeStr[4] = (m & S_IRGRP) ? 'r' : '-';   // group read permission
    modeStr[5] = (m & S_IWGRP) ? 'w' : '-';   // group write permission
    modeStr[6] = (m & S_IXGRP) ? 'x' : '-';   // group execute permission
    modeStr[7] = (m & S_IROTH) ? 'r' : '-';   // other read permission
    modeStr[8] = (m & S_IWOTH) ? 'w' : '-';   // other write permission
    modeStr[9] = (m & S_IXOTH) ? 'x' : '-';   // other execute permission
    modeStr[10] = '\0';                       // null to end string

    return modeStr;
}

void parse(char *path, int fileTreeIndent) {
    DIR *dir;  // variable DIR to store opened directory
    struct dirent *entry; // variable to store details of current directory

    // try to open directory recieved through the folder path
    if (!(dir = opendir(path))){
        printf("Unfortunately the following path cannot be parsed, since it is not a directory!\n Please try again!\n");
        return;
    }

    // go through each element of the directory
    while ((entry = readdir(dir)) != NULL){
        char pathCurrentEntry[512];  // create a buffer to store the new path
        bool isRegularFile = false; // variable to check if the current item in the directory is a regular file

        // check if the folder we want to enter is not parent or home, to avoid infinite loops
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // create path for current entry, formatted as expected
        snprintf(pathCurrentEntry, sizeof(pathCurrentEntry), "%s/%s", path, entry->d_name);

        // if regular file, flag it
        if (entry->d_type == DT_REG){
            isRegularFile = true;
        }
        // if DT different from REG, check if it's regular file with lstat and flag if yes
        else if (entry->d_type == DT_UNKNOWN || entry->d_type == DT_LNK){
            struct stat st;
            if (lstat(pathCurrentEntry, &st) == 0){
                int result = S_ISREG(st.st_mode);
                if (result != 0){
                    isRegularFile = true;
                }
            }
        }

        // regular files flagged are being printed
        if (isRegularFile == true){
            printf("%*s- %s\n", fileTreeIndent + 1, "", entry->d_name); // ex. "  - notes.txt"
        }
    }

    rewinddir(dir);
    while ((entry = readdir(dir)) != NULL){
        char pathCurrentEntry[512];
        bool isSubFolder = false;

        // check if the folder we want to enter is not parent or home, to avoid infinite loops
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // create path for current entry, formatted as expected
        snprintf(pathCurrentEntry, sizeof(pathCurrentEntry), "%s/%s", path, entry->d_name);

        // if directory, flag it
        if (entry->d_type == DT_DIR){
            isSubFolder = true;
        }
        // if DT different from DIR, check if it's directory with lstat and flag if yes
        else if (entry->d_type == DT_UNKNOWN){
            struct stat st;
            if (lstat(pathCurrentEntry, &st) == 0){
                int result = S_ISDIR(st.st_mode);
                if (result != 0){
                   isSubFolder = true;
                }
            }
        }

        // flagged directories are being printed and recursively traversed
        if (isSubFolder == true){
            printf("%*s[%s]\n", fileTreeIndent + 1, "", entry->d_name); // ex. "[OS]"
            parse(pathCurrentEntry, fileTreeIndent + 1);
        }


    }
    closedir(dir);
}

void showDetails(char *path){
    // if the Path leads to a file
    if (isFile(path)){
        struct stat st;
        if (lstat(path, &st) != 0){
            printf("Error at one of the child paths!");
            return;
        }

        char detailsStr[128];
        size_t used = 0;

        char *modeStr = modeString(st.st_mode);
        used += snprintf(detailsStr + used, sizeof(detailsStr) - used, "%s ", modeStr);

        /* 2) links and size */
        used += snprintf(detailsStr + used, sizeof(detailsStr) - used,
                         "%lu %lld ",
                         (unsigned long)st.st_nlink,
                         (long long)st.st_size);

        // filePath
        used += snprintf(detailsStr + used, sizeof(detailsStr) - used, "%s", path);

        printf("%s\n", detailsStr);

    }

    // if the Path leads to a directory
    if (isDir(path)){
        DIR *dir;
        struct dirent *entry;

        if (!(dir = opendir(path))){
            printf("Unfortunately the details of the directory at the given path could not be extracted!\n"
                   "Please try again!\n");
            return;
        }

        while ((entry = readdir(dir)) != NULL){
            char childPath[512];
            char childDetailsStr[128];
            struct stat st;

            snprintf(childPath, sizeof(childPath), "%s/%s", path, entry->d_name);

            if (lstat(childPath, &st) != 0){
                printf("Error at one of the child paths!");
                continue;
            }

            // build one "details" line for this child
            childDetailsStr[0] = '\0';                 // start empty
            size_t used = 0;

            const char *mstr = modeString(st.st_mode); // <- pass mode_t
            used += snprintf(childDetailsStr + used, sizeof(childDetailsStr) - used, "%s ", mstr);


            used += snprintf(childDetailsStr + used, sizeof(childDetailsStr) - used, "%lu %lld ", (unsigned long)st.st_nlink, (long long)st.st_size);

            if (S_ISLNK(st.st_mode)) {
                char target[256];
                ssize_t n = readlink(childPath, target, sizeof(target) - 1);
                if (n >= 0) { target[n] = '\0';
                    used += snprintf(childDetailsStr + used, sizeof(childDetailsStr) - used,
                                     "%s -> %s", entry->d_name, target);
                } else {
                    used += snprintf(childDetailsStr + used, sizeof(childDetailsStr) - used,
                                     "%s", entry->d_name);
                }
            } else {
                used += snprintf(childDetailsStr + used, sizeof(childDetailsStr) - used,
                                 "%s", entry->d_name);
            }

            printf("%s\n", childDetailsStr);
        }
    }
}

int main(int argc, char *argv[])
{
    int fileTreeIndent = 0;  // create a file tree indent, so that output looks smooth and structure maintained

    // if more or less than 3 arguments return error
    if (argc != 3)
    {
        printf("This executable only allows exactly 3 arguments in this format!\n ./[executable_name] [mode] [path]\n Please try again!\n");
        return 1;
    }

    // if mode is "parse", start parsing with given path and file tree indent 0
    if (strcmp(argv[1], "parse") == 0){
        parse(argv[2], fileTreeIndent);
    }
    // if mode is "details", show the details of the folder/path
    else if (strcmp(argv[1], "details") == 0){
        showDetails(argv[2]);
    }
    // if the mode is wrong, warn user and exit with error code
    else{
        printf("Unfortunately the mode you have chosen doesn't exist!\n Please try again!\n");
        return 1;
    }

    return 0;
}
