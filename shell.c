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
#define NUM_BUILT_INS 7
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
int checkBuiltIn(char *[]);
int runBuiltIn(char *[]);

// List of the built-in commands
char *builtIns[] = { "cd", "clr", "dir", "environ",
		     "echo", "help", "pause" };

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
		int test = checkBuiltIn(args);
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

// checkBuiltIn() - Function will check the user input to see if the input is
//		    in fact a built in function or not, and will return a flag
// 		    designating whether or not it is a built in. The print statements
//		    are for testing purposes and will be removed.
int checkBuiltIn(char *args[]) {
	for (int i = 0; i < (sizeof(builtIns) / sizeof(builtIns[0])); i++) {
		if (strcmp(args[0], builtIns[i]) == 0) {
			printf( "Is a built in function\n" );
			return 1;
		}
	} printf( "Is NOT a built in function\n" );
	return 0;
}

// runBuiltIn() - Runs the built in commands using if statements, there will be
//		  individual functions for each command, that will be called according
//		  to what the user types in, and what is in the argument vector.
//		  Each command function will accept the argument vector as input.
int runBuiltIn(char *args[]) {
	if (strcmp(args[0], "cd") == 0)
		printf( "Run change directory\n" );
	else if (strcmp(args[0], "pause") == 0)
		printf( "Pause shell\n" );
	else if (strcmp(args[0], "quit") == 0)
		printf( "Quitting shell\n" );
	else if (strcmp(args[0], "help") == 0)
		printf( "Open help menu\n" );
	else if (strcmp(args[0], "clr") == 0)
		printf( "Clear the screen\n" );
	else if (strcmp(args[0], "environ") == 0)
		printf( "Run environment command\n" );
	else if (strcmp(args[0], "dir") == 0)
		printf( "Print directory contents\n" );
	else if (strcmp(args[0], "echo") == 0)
		printf( "Run echo command\n" );
	else if (strcmp(args[0], "cd") == 0)
		printf( "Change directory\n" );
	return 0;
}
