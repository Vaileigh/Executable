#include <sys/stat.h>	// stat()
#include <unistd.h>		// getcwd()
#include <stdlib.h>		// malloc(), exit()
#include <limits.h>		// CHAR_MAX, PATH_MAX
#include <string.h>		// strcpy(), strcat(), strlen(), strcmp()
#include <stdio.h>		// printf(), popen(), pclose(), FILE, fgets()

// Read content on specific path
#define CMDLS " && ls"	
#define CMDCD "cd "

// Record the total executables file
int total_exe = 0;
int total_nonexe = 0;

// investigate in file executables
int investigate(char *argv)	 {
	struct stat sb;
	// S_IXUSR - User have execute permission | st_mode - File type and mode
	if (stat(argv, &sb) == 0 && sb.st_mode & S_IXUSR) {		// File or Directory is executable
		printf("[executable] %s\n", argv);
		total_exe++;
		return 0;
	}
	else {		// File or Directory is non-executable
		total_nonexe++;
		return 1;
	}
}

// list all file by argument - path
int list_file	(char *direc) {		// Pass path eg: "/nfs/pihome"
	
	FILE *valid;					// For file
	char valid_cmd[CHAR_MAX];		// Store valid file in path
	
	int count_exe = 0, count_nonexe = 0;	// Total of executables file from a path

	char *argv;		//Store key in right format by strtok() break by New line

	char *directory = malloc(strlen(CMDCD) + strlen(direc) + strlen(CMDLS) + 1);
	if (directory == NULL) {	//malloc success or not
		printf("Memory allocation failed\n\n");
		exit(EXIT_FAILURE);		// exit
	}

	// Get exact cmd - eg: 'cd' + PATH + 'ls'
	strcpy(directory, CMDCD);
	strcat(directory, direc);
	strcat(directory, CMDLS);

	valid = popen(directory, "r");	//Start another process "cd PATH && ls" and read
	if (valid == NULL) {			//popen() error handling
		printf("Failed to run command\n");
		exit(EXIT_FAILURE);
	}

	//Read the output screen - 'ls'
	while (fgets(valid_cmd, sizeof(valid_cmd), valid) != NULL) { //Read the line from stream and store into valid_cmd
		
		argv = strtok(valid_cmd, "\n");		//Break the key value from /bin by New line
		
		char *w_path = malloc(strlen(direc) + strlen(argv)+ 2);
		if (w_path == NULL) {				//malloc success or not
			printf("Memory allocation failed\n\n");
			exit(EXIT_FAILURE);				// exit
		}
		
		// Combine to full path - eg: PATH + argv
		strcpy(w_path, direc);
		strcat(w_path, "/");
		strcat(w_path, argv);

		if (investigate(w_path)) {
			count_nonexe++;		// Increase the count if not executable
		}
		else
			count_exe++;		// Increase the count if executable
	}

	printf("\nexecutable(s): %d  |  non-executable(s): %d\n", count_exe, count_nonexe);
	pclose(valid); // close the file
}

// Validate include ':'
int pathOnot	(char *arg) {

	char *path = malloc(strlen(arg) + 1);
	if (path == NULL) {				//malloc success or not
		printf("Memory allocation failed\n\n");
		exit(EXIT_FAILURE);				// exit
	}

	path = strtok(arg, ":");
	path = strtok(NULL, ":");

	if (path == NULL) {	// Not include semicolon - ':'
		return 0;
	}
	else {
		return 1;		// Include semicolon - ':'
	}
}

int main(int argc, char *argv[]) {

	int i = 0;			// Path counter
	char *arg;			// store each path
	char *saveptr;		// Thread-safe pointer
	char cwd[PATH_MAX]; // Current path

	// Validate user input
	if (argc != 2) {
		if (argc == 1) {// NUll on argv[1] - User doesn't not enter arguments
			if (getcwd(cwd, sizeof(cwd)) != NULL) {		// Get current path as default
				printf("Default PATH --\n");
				argv[1] = malloc(strlen(cwd)+1);
				strcpy(argv[1], "pwd");
			}
		}
		else {		// User enter more than one arguments exclu. command
			printf("Too many arguments --\n");
			return 1;
		}
			
	}

	// User argv
	char *user_argv = malloc(strlen(argv[1]) + 1);
	strcpy(user_argv, argv[1]);
	int check = pathOnot(user_argv);	// Validate include semicolon ':' - pathOnot() function
	
	if (check) {	// Include semicolon ':'
		arg = strtok_r(argv[1], ":", &saveptr);
	}
	else {			// Not include semicolon
		if (getcwd(cwd, sizeof(cwd)) != NULL) {	// Get current path as root
			
			// Temporary path store
			char *result = malloc(strlen(cwd) + strlen(argv[1]) + 2);
			if (!result) {		// malloc error handling
				printf("Memory allocation failed\n\n");
				exit(EXIT_FAILURE);
			}

			// Not include slash '/'
			if (argv[1][0] != '/') {
				int pwd = strcmp(argv[1], "pwd");
				if (!pwd) {	// User input are 'pwd' - current path
					strcpy(result, cwd);
				}
				else {		// Not 'pwd'
					// Combine current path and user input directory name
					strcpy(result, cwd);
					strcat(result, "/");
					strcat(result, argv[1]);
				}
				arg = strtok_r(result, ":", &saveptr);
			}
			else { // Include slash '/'
				strcpy(result, cwd);
				strcat(result, argv[1]);
				arg = strtok_r(result, ":", &saveptr);
			}
		}
	}
	
	// Pass each path to validate executables
	while (arg != NULL)
	{
		i++;	// Every lap increase path counter

		// Validate even pwd with semicolon - ':'
		int pwd = strcmp(arg, "pwd");
		
		printf("\n\n[loading]%s\n", arg);
		printf("--------------------\n");

		if (!pwd){		// Mixed path with pwd in user input - eg: "pwd:$PATH:/nfs/pihome"
			if (getcwd(cwd, sizeof(cwd)) != NULL)
				list_file(cwd);
		}else			// Not Mixed path with pwd in user input - eg: "$PATH:/nfs/pihome"
			list_file(arg);
		
		//	Point to next path
		arg = strtok_r(NULL, ":", &saveptr);
	}
	
	printf("Number of executable(s) [%d]\n", total_exe);
	printf("Number of Non-executable(s) [%d]\n", total_nonexe);

	printf("from [%d] path(s)\n\n", i);

	return EXIT_SUCCESS;
}