// Preprocessor Directives
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#define ARRAY_SIZE 1024
#define CWD_ERROR "--> ERROR: Cannot get CWD\n"
#define FORK_ERROR "--> ERROR: Fork function failed\n"
#define EXEC_ERROR "--> ERROR: Exec function failed\n"
#define WAIT_ERROR "--> ERROR: Wait function failed\n"
#define DELIM " \n"

// Function Prototypes
void printCurrentDir();
void readLine(char *);
void parse(char *, char *[]);
void forkProgram(char *[], int);

// Main function - invocation of all other functions
//		   input string is the input from the user
//		   args is the argument vector that will be
//		   filled up during the parsing function
//		   user input will be split up and saved here
//		   printCurrentDir will print the path to the
// 		   current directory over and over until quit
int main(int argc, char argv[]) {
	char input[ARRAY_SIZE];
	char *args[ARRAY_SIZE];
	while (1) {
		printCurrentDir();
		readLine(input);
		if (strcmp(input, "quit") == 0 || feof(stdin))
			return EXIT_SUCCESS;
		parse(input, args);
		forkProgram(args, 0);
	}
}

// printCurrentDir() - Function uses unistd.h to get the currend working directory
// 		       otherwise will return an error message, will probably never happen
void printCurrentDir() {
	char cwd[ARRAY_SIZE];
	if (getcwd(cwd, sizeof(cwd)) == NULL)
		printf( "%s", CWD_ERROR );
	else
		printf( "%s\n", cwd );
}

// readLine() - Function takes in the input string from main and fills in
// 		the string with the user input including spaces using pass
// 		by reference
void readLine(char *input) {
	scanf( "%[^\n]%*c", input );
}

// parse() - Uses the token function of string.h to parse through the
// 	     user input and tokenize each argument, saving them to the
// 	     argument vector args, both using pass by reference. This
// 	     function was inspired by a blog I read on geekstogeeks.com
void parse(char *input, char *args[]) {
	char *token;
	const char delim[] = { DELIM };
	int argCount = 0;
	token = strtok(input, delim);
	args[argCount++] = token;
	while (token != NULL) {
		token = strtok(NULL, DELIM);
		args[argCount++] = token;
	}
}

// forkProgram() - This function will handle forking the process, creating
//		   a parent and child process. It also uses execvp() to
//		   execute a command if it is not a built-in
//		   It takes in the argument vector as input
void forkProgram(char *args[], int flag) {
	pid_t pid = fork();
	if (pid < 0) {
		printf( "%s", FORK_ERROR );
	} else if (pid == 0) {
		if (execvp(args[0], args) < 0) {
			printf( "%s %s", EXEC_ERROR, args[0] );
			exit(0);
		}
	} else if (pid > 0 && flag == 0) {
		int status;
		if (waitpid(pid, &status, 0) < 0) {
			printf( "%s", WAIT_ERROR );
		}
	}
}
