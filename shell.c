// Preprocessor Directives
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#define ARRAY_SIZE 1024
#define NUM_BUILT_INS 7
#define HELP_FILE "manfile.txt"
#define CWD_ERROR "--> ERROR: Cannot get CWD\n"
#define FORK_ERROR "--> ERROR: Fork function failed\n"
#define EXEC_ERROR "--> ERROR: Exec function failed\n"
#define WAIT_ERROR "--> ERROR: Wait function failed\n"
#define CD_ERROR "--> ERROR: Cannot change directory\n"
#define DIR_ERROR "--> ERROR: Directory cannot be opened\n"
#define PAUSE_MESSAGE "Shell paused... Press enter to continue\n"
#define RESUME_MESSAGE "Shell has resumed\n"
#define FILE_OPEN_ERROR "--> ERROR: File cannot be opened\n"
#define DELIM " \n"

// Function Prototypes
int evaluateInput(char *);
void printCurrentDir();
void readLine(char *);
void parse(char *, char *[]);
void forkProgram(char *[], int);
int checkBuiltIn(char *[]);
int runBuiltIn(char *[]);
int checkRedirect(char *[], int);
void redirect(char *[], int, int);
int checkPipe(char *[], char *[], char *[]);
int cdCommand(char *[]);
int dirCommand(char *[]);
int pauseCommand(char *[]);
int echoCommand(char *[]);
int clearCommand(char *[]);
int helpCommand(char *[]);
int environCommand(char *[]);

// List of the built-in commands
char *builtIns[] = { "cd", "clr", "dir", "environ",
		     "echo", "help", "pause" };
extern char **environ;
int argsNum;

// Main function - invocation of all other functions
//		   input string is the input from the user
//		   args is the argument vector that will be
//		   filled up during the parsing function
//		   user input will be split up and saved here
//		   printCurrentDir will print the path to the
// 		   current directory over and over until quit
int main(int argc, char argv[]) {
	char input[ARRAY_SIZE];
	system( "clear" );
	do {
		printCurrentDir();
		readLine(input);
		int test = evaluateInput(input);
	} while (strcmp(input, "quit") != 0);
	return EXIT_SUCCESS;
}

// evaluateInput() - handles all the cases of the user input.
//		     Takes as input the user input and parses it by calling the parse
//		     function and tests it for each instance of a token
int evaluateInput(char *input) {
	char *args[ARRAY_SIZE];
	int flag = 0;
	parse(input, args);
	if (strcmp(args[argsNum - 1], "&") == 0) {
		flag = 1;
		args[argsNum - 1] = NULL;
	}
	int test = checkRedirect(args, flag);
	if (test == 2) {
		return 1;
	}
	if (test == 0) {
		if (args[0] == NULL) {
			return 0;
		} if (checkBuiltIn(args) == 1) {
			runBuiltIn(args);
		} else {
			forkProgram(args, flag);
			return 1;
		} return 1;
	}
}

// printCurrentDir() - Function uses unistd.h to get the currend working directory
// 		       otherwise will return an error message, will probably never happen
void printCurrentDir() {
	char cwd[ARRAY_SIZE];
	char host[ARRAY_SIZE];
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
		printf( "%s", CWD_ERROR );
	} else {
		gethostname(host, ARRAY_SIZE);
		printf( "%s:%s~$ ", host, cwd );
	}
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
	} argsNum = argCount - 1;
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
		if (strcmp(args[0], builtIns[i]) == 0)
			return 1;
	} return 0;
}

// runBuiltIn() - Runs the built in commands using if statements, there will be
//		  individual functions for each command, that will be called according
//		  to what the user types in, and what is in the argument vector.
//		  Each command function will accept the argument vector as input.
int runBuiltIn(char *args[]) {
	if (strcmp(args[0], "cd") == 0) {
		cdCommand(args);
	} else if (strcmp(args[0], "pause") == 0)
		pauseCommand(args);
	else if (strcmp(args[0], "quit") == 0)
		printf( "Quitting shell\n" );
	else if (strcmp(args[0], "help") == 0)
		helpCommand(args);
	else if (strcmp(args[0], "clr") == 0)
		clearCommand(args);
	else if (strcmp(args[0], "environ") == 0)
		environCommand(args);
	else if (strcmp(args[0], "dir") == 0)
		dirCommand(args);
	else if (strcmp(args[0], "echo") == 0)
		echoCommand(args);
	return 0;
}

// checkRedirect() - Will check the input by the user to see if there is any redirection parts
//		     of the input, and if so will run the redirect function for each. For now
//		     it is simply print statements because the redirect function hasn't been
//		     finished yet
int checkRedirect(char *args[], int flag) {
	int redirectFlag = 0;
	char *left[ARRAY_SIZE];
	char *right[ARRAY_SIZE];
	for (int i = 0; args[i] != NULL; i++) {
		if (checkPipe(args, left, right) == 1) {
			printf( "Evaluate the Pipe\n" );
			return 2;
		}
		if (strcmp(args[i], ">") == 0) {
			redirect(args, 1, i);
			redirectFlag = 1;
		}
		if (strcmp(args[i], "<") == 0) {
			redirect(args, 2, i);
			redirectFlag = 1;
		}
		if (strcmp(args[i], ">>") == 0) {
			redirect(args, 3, i);
			redirectFlag = 1;
		}
	} return redirectFlag;
}

// redirect() - this function will handle all redirecting from the command to
//		the specified file, or to get input from the file, it all depends
//		on the token that is entered in by the user
void redirect(char *args[], int flag, int i) {
	int j = i;
	int in, out;
	int savedin = dup(0);
	int savedout = dup(1);
	pid_t pid = fork();
	if (pid < 0) {
		printf( "%s", FORK_ERROR );
	} else if (pid == 0) {
		if (flag == 1) {
			int output = open(args[j + 1], O_WRONLY|O_CREAT,S_IRWXU|S_IRWXG|S_IRWXO);
			dup2(output, 1);
			close(output);
			args[j] = NULL;
			args[j + 1] = NULL;
			++j;
		}
		if (flag == 2) {
			int input = open(args[j + 1], O_CREAT | O_RDONLY, 0666);
			dup2(input, 0);
			close(input);
			args[j] = NULL;
			args[j + 1] = NULL;
			++j;
		}
		if (flag == 3) {
			int output = open(args[j + 1], O_WRONLY | O_APPEND | O_CREAT, 0666);
			dup2(output, 1);
			close(output);
			args[j] = NULL;
			args[j + 1] = NULL;
			++j;
		} execvp(args[0], args);
	} else if (pid > 0) {
		waitpid(pid, NULL, WCONTINUED);
	}
	dup2(savedin, 0);
	close(savedin);
	dup2(savedout, 1);
	close(savedout);
}

// checkPipe() - checks for a pipe in the user's input, and if there is
//		 then the left and right argument vectors will be filled
//		 with the left and the right sides of the input by the user and will
//		 be handled accordingly by calling the piping function
int checkPipe(char *args[], char *left[], char *right[]) {
	int pipeFlag = 0;
	int i = 0, j, k;
	while (args[i] != NULL) {
		if (strcmp(args[i], "|") == 0) {
			for (j = 0; j < i; j++) {
				left[j] = malloc(sizeof(char) * sizeof(args[i]));
				char *strArg = args[j];
				left[j] = strArg;
			}
			int m = 0;
			for (k = i + 1; k < argsNum; k++) {
				right[m] = malloc(sizeof(char) * sizeof(args[k]));
				char *strArg = args[k];
				right[m] = strArg;
				m++;
			} pipeFlag = 0;
		} i++;
	} return pipeFlag;
}

// cdCommand() - changes the directory by calling the chdir command, if
//		 the user only types cd, then the cwd is printed, otherwise
//		 will call the chdir function and if it fails, will print
//		 an error message, otherwise will change the directory
int cdCommand(char *args[]) {
	if (args[1] == NULL) {
		char cwd[ARRAY_SIZE];
        	getcwd(cwd, sizeof(cwd));
                printf( "\n%s\n", cwd );
		return 0;
	} else {
		if (chdir(args[1]) != 0)
			printf( "%s", CD_ERROR );
	} return 0;
}

// dirCommand() - opens the directory, and prints the contents of it
//		  one after the other, then closes the directory
int dirCommand(char *args[]) {
	char path[ARRAY_SIZE];
	getcwd(path, sizeof(path));
	DIR *directory;
	struct dirent *bat;
	if ((directory = opendir(path)) == NULL)
		printf( "%s", DIR_ERROR );
	else {
		int k = 0;
		while ((bat = readdir(directory)) != NULL) {
			printf( "%s\n", bat->d_name );
			k++;
		} closedir(directory);
	} return 0;
}

// pauseCommand() - command will pause the shell, and will continue to be
//		    paused until the user types in ENTER or \n key
int pauseCommand(char *args[]) {
	char input[ARRAY_SIZE];
	do {
		printf( "%s", PAUSE_MESSAGE );
		fgets(input, sizeof(input), stdin);
	} while (strcmp(input, "\n") != 0);
	printf( "%s", RESUME_MESSAGE );
}

// echoCommand() - will take input by the user and echo it to the screen,
//		   or print it to the screen
int echoCommand(char *args[]) {
	int i = 1;
	while (args[i] != NULL)
		printf( "%s ", args[i++] );
	printf( "\n" );
}

// clearCommand() - uses the ioctl header to call functions that get the size
//		    of the window, and clears the screen, putting the prompt
//		    up at the top of the screen
int clearCommand(char *args[]) {
	int i;
	struct winsize w;
	ioctl(0, TIOCGWINSZ, &w);
	for (i = 0; i < w.ws_row; i++)
		puts("");
	printf( "\033[%dA", w.ws_row );
}

// helpCommand() - clears the screen, and then calls the manual text file that contains
//		   information about the shell itself and how to use it
int helpCommand(char *args[]) {
	int test = clearCommand(args);
	int i = 0;
	struct winsize w;
	ioctl(0, TIOCGWINSZ, &w);
	char manFileLine[ARRAY_SIZE];
	FILE *file = fopen( HELP_FILE, "r" );
	if (file == NULL) {
		printf( "%s", FILE_OPEN_ERROR );
		return 0;
	}
	while (fgets(manFileLine, ARRAY_SIZE, file) != NULL) {
		printf( "%s", manFileLine );
		i++;
		if (i == w.ws_row - 1) {
			char input[ARRAY_SIZE];
			while (fgets(manFileLine, ARRAY_SIZE, file) != NULL) {
				fgets(input, sizeof(input), stdin);
				if (strcmp(input, "\n") == 0) {
					char *token = strtok(manFileLine, "\n");
					printf( "%s", manFileLine );
				}
				if (strcmp(input, "q\n") == 0) {
					fclose(file);
					return 1;
				}
			} fclose(file);
			return 1;
		}
	}
}

// environCommand() - function that prints the environment variables, such
//		      as global and local variables
int environCommand(char *args[]) {
	char *environVariable = *environ;
	for (int i = 1; environVariable; i++) {
		printf( "%s\n", environVariable );
		environVariable = *(environ + i);
	} return 0;
}
