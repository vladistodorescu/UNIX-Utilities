#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>   
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

int main (int argc, char **argv){
	  if (argc < 2){
		    printf("Error: No arguments found");
		    return 0;
	  }

	  char currentWord[40];
	
	  for (int i = 1; i < argc; i++) {
		    pid_t pid = fork();

		    if (pid < 0) {
			      perror("Error when forking!");
			      exit(1);
		    }
		    else if (pid == 0) {
			      strncpy(currentWord, argv[i], sizeof(currentWord) - 1);
			      currentWord[sizeof(currentWord) - 1] = '\0';
			      int lenCurrentWord = strlen(currentWord);
			      for (int j = 0; j < lenCurrentWord; j++){
			          currentWord[j] = currentWord[j] + 1;
			      }
			      printf("%s", currentWord);
			      if ( i < argc - 1){
				        printf("%c", ' ');
			      }
			      exit(0);
		        }
		    else {
			      wait(NULL);
		    } 
	}
	printf("\n");
	return 0;
	
}
