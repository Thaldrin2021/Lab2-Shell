#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define ARRAY_SIZE 1024
#define CWD_ERROR "--> ERROR: Cannot get CWD\n"

void printCurrentDir();

int main(int argc, char argv[]) {
	printCurrentDir();
	return EXIT_SUCCESS;
}

void printCurrentDir() {
	char cwd[ARRAY_SIZE];
	if (getcwd(cwd, sizeof(cwd)) == NULL)
		printf( "%s", CWD_ERROR );
	else
		printf( "%s\n", cwd );
}
