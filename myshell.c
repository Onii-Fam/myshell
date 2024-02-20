#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define RED "\x1b[31m"
#define GRN "\x1B[32m"
#define BLU "\x1b[34m"
#define YEL "\x1b[33m"
#define MAG "\x1b[35m"
#define CYN "\x1b[36m"
#define COLOR_RESET "\x1b[0m"

#define MAX_COMMAND_LENGTH 80
#define MAX_TOKENS 10
#define MAX_PATH_LENGTH 1024
#define PID_HISTORY_SIZE 5

void displayPrompt(); //displays the directory and prompt in the terminal
char* readCommand(); //uses get line to read the user command input
char** parseCommand(char* command); //parses the command and tokenizes it so long as it isn't empty
int executeCommand(char** args, pid_t* pidHistory, int* pidIndex); //will use execvp to run the command and its arguments as well as 
//checking to make sure fork is sucessful and waits for child process to finish before returning to main prompt
//void handleBuiltInCommands();
void updatePidHistory(pid_t pid, pid_t* pidHistory, int* pidIndex);
void showPidHistory(pid_t* pidHistory, int pidIndex);
int changeDirectory(char* path);



int main() {
    char* command;
    char** args;
    int status = 1;
    pid_t pidHistory[PID_HISTORY_SIZE] = {0};
    int pidIndex = 0;
    static int cCount = 1;

    do {
        displayPrompt();
        command = readCommand();
        args = parseCommand(command);

        if (args[0] != NULL) {
          //checks for built in commands first before using exec
            if (strcmp(args[0], "cd") == 0) {
                status = changeDirectory(args[1]);
            } else if (strcmp(args[0], "exit") == 0) {
                printf("exit\n");
                exit(0);
            } else if (strcmp(args[0], "showpid") == 0) {
                showPidHistory(pidHistory, pidIndex);
                status = 1; // Continue the loop
            } else {
                status = executeCommand(args, pidHistory, &pidIndex);
            }
        }
        //cleaining up
        free(command);
        free(args);
    } while (status);

    return 0;
}

void displayPrompt() {
    char cwd[MAX_PATH_LENGTH];
    static int cCount = 0;
    const char* colors[] = {RED, GRN, YEL, BLU, MAG, CYN}; // Array of color codes
    int numColors = sizeof(colors) / sizeof(colors[0]); // Number of colors

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
       printf("%s%s$%s ", colors[cCount], cwd, COLOR_RESET); 
    } else {
        perror("getcwd() error");
    }
      cCount = (cCount + 1) % numColors; // Move to the next color, cycling back to 0 if at the end
}

char* readCommand() {
    char* line = NULL;
    size_t bufsize = 0;
    getline(&line, &bufsize, stdin);
    return line;
}

char** parseCommand(char* command) {
    int bufsize = MAX_TOKENS, position = 0;
    char** tokens = malloc(bufsize * sizeof(char*));
    char* token;

    token = strtok(command, " \t\r\n\a");
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += MAX_TOKENS;
            tokens = realloc(tokens, bufsize * sizeof(char*));
        }

        token = strtok(NULL, " \t\r\n\a");
    }
    tokens[position] = NULL;
    return tokens;
}

int executeCommand(char** args, pid_t* pidHistory, int* pidIndex) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
          printf(RED "Error: Command '%s' could not be executed\n" COLOR_RESET, args[0]);
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Fork error handling
        perror("fork error");
    } else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status)); //checking for normal exit of child
        updatePidHistory(pid, pidHistory, pidIndex);
    }

    return 1;
}

void updatePidHistory(pid_t pid, pid_t* pidHistory, int* pidIndex) {
    pidHistory[*pidIndex % PID_HISTORY_SIZE] = pid; //hashing the pid so that i don't have to worry about 
    (*pidIndex)++;                                  //overflowing this. Circular buffering basically
}

void showPidHistory(pid_t* pidHistory, int pidIndex) {
    printf("Last %d child PIDs:\n", PID_HISTORY_SIZE);
    int start = pidIndex < PID_HISTORY_SIZE ? 0 : pidIndex - PID_HISTORY_SIZE; //conditional checking if there is anything in the history
    for (int i = start; i < pidIndex; i++) {
        printf("%d\n", pidHistory[i % PID_HISTORY_SIZE]);
    }
}

int changeDirectory(char* path) {
    if (path == NULL) {
        fprintf(stderr, RED "cd: expected argument to \"cd\"\n" COLOR_RESET);
        return 1;
    } else {
        if (chdir(path) != 0) { //chdir error handling
            perror("cd error");
            return 1; 
        }
    }
    return 1; 
}
