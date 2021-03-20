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
#define BATCH_FILE "batchfile.txt"
#define CWD_ERROR "--> ERROR: Cannot get CWD\n"
#define FORK_ERROR "--> ERROR: Fork function failed\n"
#define EXEC_ERROR "--> ERROR: Exec function failed\n"
#define WAIT_ERROR "--> ERROR: Wait function failed\n"
#define CD_ERROR "--> ERROR: Cannot change directory\n"
#define DIR_ERROR "--> ERROR: Directory cannot be opened\n"
#define PAUSE_MESSAGE "Shell paused... Press enter to continue\n"
#define RESUME_MESSAGE "Shell has resumed\n"
#define FILE_OPEN_ERROR "--> ERROR: File cannot be opened\n"
#define QUIT_MESSAGE "Shell finished...\n"
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
void evaluatePipe(char *[], char *[], int);
void cdCommand(char *[]);
void dirCommand(char *[]);
void pauseCommand(char *[]);
void echoCommand(char *[]);
void clearCommand(char *[]);
void helpCommand(char *[]);
void environCommand(char *[]);
void quitCommand();

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
int main(int argc, char *argv[]) {
	if (argc > 1) {							// Batchfile signal to read commands from batchfile
		char *batchFile = BATCH_FILE;				// Pointer to the batch file
		char input[ARRAY_SIZE];					// Holds the input from the batch file
		char *lineRead;						// Holds the input from the command file
		FILE *file = fopen(batchFile, "r");			// Opens the batch file to be read from
		while (fgets(input, sizeof(input), file) != NULL) {	// Reads each line from the batch file
			lineRead = input;				// Sets the input to the line read
			evaluateInput(lineRead);			// Passes line from the file to be evaluated in the evaluate function
		} fclose(file);						// Close the file at the end when finished
	} else {							// If there is no batchfile input by the user
		char input[ARRAY_SIZE];					// Create input to be read from the command line by the user
		system( "clear" );					// Clears the screen when first loaded up
		do {							// An endless loop that has a condition on the inside of it when user types "quit"
			printCurrentDir();				// Print the current working directory over and over again until shell exits
			readLine(input);				// Get the input from the user
			if (strcmp(input, "quit") == 0) {		// If the user types in "quit" then enter the quit command function that prints message and exits shell
				printf( "%s", QUIT_MESSAGE );		// Prints the quit message
				return EXIT_SUCCESS;			// Exit the shell
			} evaluateInput(input);				// Otherwise evaluate the input by the user
		} while (1);						// Endless while loop because exit variable is inside the loop
	} return EXIT_SUCCESS;						// Exit the shell
}

// evaluateInput() - handles all the cases of the user input.
//		     Takes as input the user input and parses it by calling the parse
//		     function and tests it for each instance of a token
int evaluateInput(char *input) {
	char *args[ARRAY_SIZE];				// Argument vector that will be filled with parsed input
	int flag = 0;					// A flag that will check the user input for the "&" character
	parse(input, args);				// Calls the parse function that will parse the user input and store it in the argument vector
	if (strcmp(args[argsNum - 1], "&") == 0) {	// If the final word in the argument vector is &
		flag = 1;				// Set the flag to true
		args[argsNum - 1] = NULL;		// Then remove the character
	}
	int test = checkRedirect(args, flag);		// Calls the check redirect function to test for any input or output tokens
	if (test == 2) {				// If the check redirect function returns 2, then it is a pipe flag
		return 1;				// Return and call the pipe function later on
	}
	if (test == 0) {				// If there are no redirect tokens
		if (args[0] == NULL) {			// If there is no input, then ignore it
			return 0;			// return and go to next iteration of shell
		} if (checkBuiltIn(args) == 1) {	// If the input by the user is a built-in function
			runBuiltIn(args);		// Run the said built-in function
		} else {				// If it is not a built-in and it is not a redirect
			forkProgram(args, flag);	// Execute the external command
			return 1;			// Return and go to the next iteration of the shell
		} return 1;
	} return 0;
}

// printCurrentDir() - Function uses unistd.h to get the currend working directory
// 		       otherwise will return an error message, will probably never happen
void printCurrentDir() {
	char cwd[ARRAY_SIZE];				// Holds the current working directory
	char host[ARRAY_SIZE];				// Holds the name of the host device
	if (getcwd(cwd, sizeof(cwd)) == NULL) {		// Gets the current working directory name, by useing getcwd()
		printf( "%s", CWD_ERROR );		// Print an error if it doens't return anything
	} else {					// Otherwise
		gethostname(host, ARRAY_SIZE);		// Get the host name
		printf( "%s:%s~$ ", host, cwd );	// Print host name and then cwd to the screen
	}
}

// readLine() - Function takes in the input string from main and fills in
// 		the string with the user input including spaces using pass
// 		by reference
void readLine(char *input) {
	scanf( "%[^\n]%*c", input );		// Gets the user input, including strings
}

// parse() - Uses the token function of string.h to parse through the
// 	     user input and tokenize each argument, saving them to the
// 	     argument vector args, both using pass by reference. This
// 	     function was inspired by a blog I read on geekstogeeks.com
void parse(char *input, char *args[]) {
	char *token;				// This is a pointer to the token, will fill up with each iteration
	const char delim[] = { DELIM };		// Delimiters used to determine how to divide the tokens
	int argCount = 0;			// Number of arguments
	token = strtok(input, delim);		// Gets the first token
	args[argCount++] = token;		// Increments the counter
	while (token != NULL) {			// While there are still tokens in input
		token = strtok(NULL, DELIM);	// Get the next token
		args[argCount++] = token;	// Increment again
	} argsNum = argCount - 1;		// Actual number used for indexing is 1 less than the numArgs
}

// forkProgram() - This function will handle forking the process, creating
//		   a parent and child process. It also uses execvp() to
//		   execute a command if it is not a built-in
//		   It takes in the argument vector as input
void forkProgram(char *args[], int flag) {
	pid_t pid = fork();						// Get the pid for the process and then fork it
	if (pid < 0) {							// If the pid < 0, then error occurred
		printf( "%s", FORK_ERROR );				// Print fork error message from above
	} else if (pid == 0) {						// This is the child process
		if (execvp(args[0], args) < 0) {			// Will check the execvp for any errors that occur
			printf( "%s %s\n", EXEC_ERROR, args[0] );	// Prints the exec error as well as the command that was input
			exit(0);					// Exits the process
		}
	} else if (pid > 0 && flag == 0) {				// This is the parent process
		int status;						// Keeps track of the waiting status of the parent
		if (waitpid(pid, &status, 0) < 0) {			// Will wait for the child process to finish running
			printf( "%s", WAIT_ERROR );			// Print error if waiting fails for any reason
		}
	}
}

// checkBuiltIn() - Function will check the user input to see if the input is
//		    in fact a built in function or not, and will return a flag
// 		    designating whether or not it is a built in. The print statements
//		    are for testing purposes and will be removed.
int checkBuiltIn(char *args[]) {
	for (int i = 0; i < (sizeof(builtIns) / sizeof(builtIns[0])); i++) {		// Will cycle through the list of the built-in commands from above
		if (strcmp(args[0], builtIns[i]) == 0)					// If there is a match
			return 1;							// Set the flag to true
	} return 0;									// Otherwise return a negative flag
}

// runBuiltIn() - Runs the built in commands using if statements, there will be
//		  individual functions for each command, that will be called according
//		  to what the user types in, and what is in the argument vector.
//		  Each command function will accept the argument vector as input.
int runBuiltIn(char *args[]) {
	if (strcmp(args[0], "cd") == 0)			// If the cd command is input
		cdCommand(args);			// Call the cd function from below
	else if (strcmp(args[0], "pause") == 0)		// If the pause command is input
		pauseCommand(args);			// Call the pause function
	else if (strcmp(args[0], "help") == 0)		// If the help command is input
		helpCommand(args);			// Call the help function
	else if (strcmp(args[0], "clr") == 0)		// If the clr command is input
		clearCommand(args);			// Call the clr function from below
	else if (strcmp(args[0], "environ") == 0)	// If the environ command is input
		environCommand(args);			// Call the environ function from below
	else if (strcmp(args[0], "dir") == 0)		// If the dir command is input
		dirCommand(args);			// Call the dir command
	else if (strcmp(args[0], "echo") == 0)		// If the echo command is input
		echoCommand(args);			// Call the echo command from below
	return 0;					// Return 0 that it ran and go to next shell iteration
}

// checkRedirect() - Will check the input by the user to see if there is any redirection parts
//		     of the input, and if so will run the redirect function for each. For now
//		     it is simply print statements because the redirect function hasn't been
//		     finished yet
int checkRedirect(char *args[], int flag) {
	int redirectFlag = 0;					// Flag will change to true if there is an I/O token in the user input
	char *left[ARRAY_SIZE];					// Holds the left side of the token
	char *right[ARRAY_SIZE];				// Holds the right side of the token
	for (int i = 0; args[i] != NULL; i++) {			// As long as there are strings left in the user input
		if (checkPipe(args, left, right) == 1) {	// Check for a pipe character in the user input
			evaluatePipe(left, right, flag);	// Evaluate the pipe
			return 2;				// Return the pipe flag
		}
		if (strcmp(args[i], ">") == 0) {		// If there is a > token
			redirect(args, 1, i);			// Call the redirect function with flag at 1
			redirectFlag = 1;			// Sets the redirect flag to true
		}
		if (strcmp(args[i], "<") == 0) {		// If there is a < token
			redirect(args, 2, i);			// Call the redirect function with flag set to 2
			redirectFlag = 1;			// Set redirect flag to 1
		}
		if (strcmp(args[i], ">>") == 0) {		// If there is a >> token
			redirect(args, 3, i);			// Call the redirect flag with flag set to 3
			redirectFlag = 1;			// Set redirect flag to 1
		}
	} return redirectFlag;					// Return the redirect flag
}

// redirect() - this function will handle all redirecting from the command to
//		the specified file, or to get input from the file, it all depends
//		on the token that is entered in by the user
void redirect(char *args[], int flag, int i) {
	int j = i;											// Will be used to cycle through input from where the token is
	int savedin = dup(0);										// Saves the read side
	int savedout = dup(1);										// Saves the write side
	pid_t pid = fork();										// Forks the process
	if (pid < 0) {											// If pid < 0, then error occurred
		printf( "%s", FORK_ERROR );								// Print the fork error from above
	} else if (pid == 0) {										// This is child process
		if (flag == 1) {									// If the redirect token is >
			int output = open(args[j + 1], O_WRONLY|O_CREAT,S_IRWXU|S_IRWXG|S_IRWXO);	// Opens the file for writing
			dup2(output, 1);								// Dup the write side
			close(output);									// Close the stdout
			args[j] = NULL;									// Sets the > token to NULL
			args[j + 1] = NULL;								// Sets the file to NULL
			++j;										// Increment
		}
		if (flag == 2) {									// If the redirect token is <
			int input = open(args[j + 1], O_CREAT | O_RDONLY, 0666);			// Open the file for reading from
			dup2(input, 0);									// Dups reading from the input file
			close(input);									// Close stdin
			args[j] = NULL;									// Set the token to NULL
			args[j + 1] = NULL;								// Set the file name to NULL
			++j;										// Increment
		}
		if (flag == 3) {									// If the redirect token is >>
			int output = open(args[j + 1], O_WRONLY | O_APPEND | O_CREAT, 0666);		// Open the file to be added to
			dup2(output, 1);								// Dups writing like with >
			close(output);									// Close stdin
			args[j] = NULL;									// Sets >> token to NULL
			args[j + 1] = NULL;								// Sets the file name to NULL
			++j;										// Increments
		} execvp(args[0], args);								// Execute the command from the input
	} else if (pid > 0) {										// This is the parent function
		waitpid(pid, NULL, WCONTINUED);								// Wait for the child process to finish running
	}
	dup2(savedin, 0);										// Restore reading
	close(savedin);											// Closes reading (stdin)
	dup2(savedout, 1);										// Restore writing
	close(savedout);										// Closes writing (stdout)
}

// checkPipe() - checks for a pipe in the user's input, and if there is
//		 then the left and right argument vectors will be filled
//		 with the left and the right sides of the input by the user and will
//		 be handled accordingly by calling the piping function
int checkPipe(char *args[], char *left[], char *right[]) {
	int pipeFlag = 0;								// Flag used to test if there is a pipe resent
	int i = 0, j, k;								// Counters to cycle through the argument vector, left and right side
	while (args[i] != NULL) {							// While input still has arguments
		if (strcmp(args[i], "|") == 0) {					// If argument is the pipe character
			for (j = 0; j < i; j++) {					// Cycle through the arguments on the left side of the pipe
				left[j] = malloc(sizeof(char) * sizeof(args[i]));	// Malloc space for argument
				char *strArg = args[j];					// Set the token to string
				left[j] = strArg;					// Store it in left side
			}
			int m = 0;							// Counter for the right side since not starting at 0
			for (k = i + 1; k < argsNum; k++) {				// Until the end of the input is reached
				right[m] = malloc(sizeof(char) * sizeof(args[k]));	// Like the left side, malloc the space for the argument
				char *strArg = args[k];					// Set the token to the string
				right[m++] = strArg;					// Store it in the right side
			} pipeFlag = 1;							// Set the pipe flag to true, since there is a pipe
		} i++;									// Increment the counter for the entire input
	} return pipeFlag;								// Return the pipe flag, 1 if there is a pipe, 0 otherwise
}

// evaluatePipe() - handles running the pipe command. Takes the left and the
//		    right side of a command that was parsed in the previous function. Then runs two processes keeping track of them along the way
void evaluatePipe(char *left[], char *right[], int flag) {
	int fd[2];								// Read and write ends of the pipe
	pid_t pid1, pid2;							// PID for the first child process and the second child process
	pipe(fd);								// Create the pipe using the fd
	pid1 = fork();								// Fork the first process
	if (pid1 < 0) {								// If pid < 0, error occured
		printf( "%s", FORK_ERROR );					// Print fork error message
		return;
	}
	if (pid1 == 0) {							// If pid == 0, then this is the child process
		dup2(fd[1], STDOUT_FILENO);					// Dup the stdout
		close(fd[0]);							// Close the read side of the pipe
		if (checkBuiltIn(left) == 1) {					// Check for a built in on the left side
			runBuiltIn(left);					// Run it if there is one
		} else {							// If external command
			if (execvp(left[0], left) < 0) {			// Execute and check for an error
				printf( "%s", EXEC_ERROR );			// Print error message
				exit(1);
			}
		}
	} else {								// This is the parent process
		pid2 = fork();							// Run fork function again for the second child process
		if (pid2 < 0) {							// If pid < 0, an error occurred
			printf( "%s", FORK_ERROR );				// Print fork error message
			exit(1);						// Return and continue to next shell iteration
		} else if (pid2 == 0) {						// This is the child process
			dup2(fd[0], STDIN_FILENO);				// Dup the read end of the pipe
			close(fd[1]);						// Close the write end of the pipe
			if (checkBuiltIn(right) == 1) {				// Check for a built in again, this time on the right side
				runBuiltIn(right);				// Run the built in function if it is one
			} else {						// Otherwise its an external command
				if (execvp(right[0], right) < 0) {		// Execute it using the right side of the pipe
					printf( "%s", EXEC_ERROR );		// If there is an error then print the error message
					exit(1);
				}
			}
		} else if (pid2 > 2 && flag == 0) {				// This is the parent process again
			close(fd[0]);						// Close the read side of pipe
			close(fd[1]);						// Close the write side of pipe
			waitpid(pid1, NULL, 0);					// Wait for the first child to finish running
			waitpid(pid2, NULL, 0);					// Wait for the second child process to finish running
		}
	}
}

// cdCommand() - changes the directory by calling the chdir command, if
//		 the user only types cd, then the cwd is printed, otherwise
//		 will call the chdir function and if it fails, will print
//		 an error message, otherwise will change the directory
void cdCommand(char *args[]) {
	if (args[1] == NULL) {					// If there is only one argument
		char cwd[ARRAY_SIZE];				// Hold the name of the directory and path
        	getcwd(cwd, sizeof(cwd));			// Get the current directoy and path
                printf( "\n%s\n", cwd );			// Print it to the screen
	} else {						// If there are more than one argument
		if (chdir(args[1]) != 0)			// Use chdir function with the second argument to change the directory
			printf( "%s", CD_ERROR );		// Print error message if this fails
	}
}

// dirCommand() - opens the directory, and prints the contents of it
//		  one after the other, then closes the directory
void dirCommand(char *args[]) {
	char path[ARRAY_SIZE];					// Holds the path of the directory
	getcwd(path, sizeof(path));				// Get the current directory and store in path[]
	DIR *directory;						// Directory variable from <dirent.h>
	struct dirent *bat;					// Bat is variable used to read from the directory
	if ((directory = opendir(path)) == NULL)		// Open the directory and check that it is not empty
		printf( "%s", DIR_ERROR );			// If this failes print error message
	else {							// Otherwise it opened and not empty
		int k = 0;					// Used to iterate through directory
		while ((bat = readdir(directory)) != NULL) {	// Reads each item in the open directory
			printf( "%s\n", bat->d_name );		// Prints the directory item name
			k++;					// Iterates through
		} closedir(directory);				// Closes the directory
	}
}

// pauseCommand() - command will pause the shell, and will continue to be
//		    paused until the user types in ENTER or \n key
void pauseCommand(char *args[]) {
	char input[ARRAY_SIZE];					// Holds the user input
	do {							// Do-while loop to iterate until the uer hits enter
		printf( "%s", PAUSE_MESSAGE );			// Print the pause message from above
		fgets(input, sizeof(input), stdin);		// Get the input from the user using stdin
	} while (strcmp(input, "\n") != 0);			// While the user doesn't type in enter key
	printf( "%s", RESUME_MESSAGE );				// Print shell is resumed
}

// echoCommand() - will take input by the user and echo it to the screen,
//		   or print it to the screen
void echoCommand(char *args[]) {
	int i = 1;					// Counter for each argument
	while (args[i] != NULL)				// While the arguments are not NULL
		printf( "%s ", args[i++] );		// Print the argument to the screen and increment
	printf( "\n" );					// Print newline
}

// clearCommand() - uses the ioctl header to call functions that get the size
//		    of the window, and clears the screen, putting the prompt
//		    up at the top of the screen
void clearCommand(char *args[]) {
	struct winsize w;			// Get the window dimensions
	ioctl(0, TIOCGWINSZ, &w);		// Used to hold the window's dimensions
	for (int i = 0; i < w.ws_row; i++)	// Cycles through each row on the screen
		puts("");			// Put empty line
	printf( "\033[%dA", w.ws_row );		// Used to revert back to the top of the screen, otherwise prompt is at bottom of the screen
}

// helpCommand() - clears the screen, and then calls the manual text file that contains
//		   information about the shell itself and how to use it
void helpCommand(char *args[]) {
	clearCommand(args);							// Clears the screen using command from above
	struct winsize w;							// Get the window dimensions
	ioctl(0, TIOCGWINSZ, &w);						// Get the window dimensions using <ioctl.h>
	char manFileLine[ARRAY_SIZE];						// Holds each line in the Man file
	FILE *file = fopen( HELP_FILE, "r" );					// Open the man file
	if (file == NULL) {							// If there is an issue, print error message
		printf( "%s", FILE_OPEN_ERROR );				// Error message for the file
		return;
	} while (fgets(manFileLine, ARRAY_SIZE, file) != NULL) {		// While the end of the file hasn't been reached
		printf( "%s", manFileLine );					// Print the line from the file
	}
}

// environCommand() - function that prints the environment variables, such
//		      as global and local variables
void environCommand(char *args[]) {
	char *environVariable = *environ;		// Environment pointer
	for (int i = 1; environVariable; i++) {		// Cycle until NULL is reached
		printf( "%s\n", environVariable );	// Prints the variable
		environVariable = *(environ + i);	// Sets the environment variable to the next
	}
}
