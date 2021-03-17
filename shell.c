// Preprocessor Directives
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define ARRAY_SIZE 1024
#define CWD_ERROR "--> ERROR: Cannot get CWD\n"
#define DELIM " \n"

// Function Prototypes
void printCurrentDir();
void readLine(char *);
void parse(char *, char *[]);

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
	printCurrentDir();
	readLine(input);
	parse(input, args);
	for (int i = 0; args[i] != NULL; i++)
		printf( "%s\n", args[i] );
	return EXIT_SUCCESS;
}

// Function uses unistd.h to get the currend working directory
// otherwise will return an error message, will probably never happen
void printCurrentDir() {
	char cwd[ARRAY_SIZE];
	if (getcwd(cwd, sizeof(cwd)) == NULL)
		printf( "%s", CWD_ERROR );
	else
		printf( "%s\n", cwd );
}

// Function takes in the input string from main and fills in
// the string with the user input including spaces using pass
// by reference
void readLine(char *input) {
	scanf( "%[^\n]%*c", input );
}

// Uses the token function of string.h to parse through the
// user input and tokenize each argument, saving them to the
// argument vector args, both using pass by reference. This
// function was inspired by a blog I read on geekstogeeks.com
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

