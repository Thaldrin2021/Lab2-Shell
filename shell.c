#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define ARRAY_SIZE 1024
#define CWD_ERROR "--> ERROR: Cannot get CWD\n"
#define DELIM " \n"

void printCurrentDir();
void readLine(char *);
void parse(char *, char *[]);

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

void printCurrentDir() {
	char cwd[ARRAY_SIZE];
	if (getcwd(cwd, sizeof(cwd)) == NULL)
		printf( "%s", CWD_ERROR );
	else
		printf( "%s\n", cwd );
}

void readLine(char *input) {
	scanf( "%[^\n]%*c", input );
}

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
