/* shell.c */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARG_COUNT 64

extern char **environ; // To access environment variables

// Function prototypes
void execute_command(char *input);
void change_directory(char *path);
void print_working_directory();
void echo(char **args);
void set_environment_variable(char *var, char *value);
void print_environment(char **args);
void sigint_handler(int sig);
void alarm_handler(int sig);

int main() {
    char input[MAX_INPUT_SIZE];
    char cwd[1024];

    // Set up signal handlers
    signal(SIGINT, sigint_handler); // Handle Ctrl+C for the shell
    signal(SIGALRM, alarm_handler); // Handle alarm for timing out processes

    while (1) {
        // Display prompt with current directory
        getcwd(cwd, sizeof(cwd));
        printf("%s> ", cwd);

        // Get input from user
        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
            break; // Exit on EOF or error
        }

        // Remove newline at end of input
        input[strcspn(input, "\n")] = '\0';

        // Process the command
        execute_command(input);
    }

    return 0;
}

void execute_command(char *input) {
    char *args[MAX_ARG_COUNT];
    char *token = strtok(input, " ");
    int i = 0;

    // Tokenize the input
    while (token != NULL && i < MAX_ARG_COUNT - 1) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL; // Null-terminate the array of arguments

    if (args[0] == NULL) {
        return; // Ignore empty input
    }

    // Check for built-in commands
    if (strcmp(args[0], "cd") == 0) {
        change_directory(args[1]);
    } else if (strcmp(args[0], "pwd") == 0) {
        print_working_directory();
    } else if (strcmp(args[0], "echo") == 0) {
        echo(args);
    } else if (strcmp(args[0], "env") == 0) {
        print_environment(args);
    } else if (strcmp(args[0], "setenv") == 0 && args[1] && args[2]) {
        set_environment_variable(args[1], args[2]);
    } else if (strcmp(args[0], "exit") == 0) {
        exit(0);
    } else {
        // Non-built-in commands (external commands)
        int is_background = 0;
        if (i > 0 && strcmp(args[i - 1], "&") == 0) {
            is_background = 1;
            args[--i] = NULL; // Remove '&' from args
        }

        pid_t pid = fork();
        if (pid == 0) { // Child process
            // Handle I/O redirection
            for (int j = 0; j < i; j++) {
                if (strcmp(args[j], ">") == 0) {
                    int fd = open(args[j + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd < 0) {
                        perror("open error");
                        exit(1);
                    }
                    dup2(fd, STDOUT_FILENO); // Redirect stdout to the file
                    close(fd);
                    args[j] = NULL;
                    break;
                } else if (strcmp(args[j], "<") == 0) {
                    int fd = open(args[j + 1], O_RDONLY);
                    if (fd < 0) {
                        perror("open error");
                        exit(1);
                    }
                    dup2(fd, STDIN_FILENO); // Redirect stdin from the file
                    close(fd);
                    args[j] = NULL;
                    break;
                }
            }

            alarm(10); // Set a 10-second timeout for the process

            if (execvp(args[0], args) < 0) {
                perror("execvp error");
                exit(1);
            }
        } else if (pid > 0) { // Parent process
            if (!is_background) {
                waitpid(pid, NULL, 0); // Wait for foreground processes
            }
            alarm(0); // Disable alarm after the process completes
        } else {
            perror("fork failed");
        }
    }
}

// Handle built-in commands
void change_directory(char *path) {
    if (chdir(path) != 0) {
        perror("chdir failed");
    }
}

void print_working_directory() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("getcwd error");
    }
}

void echo(char **args) {
    for (int i = 1; args[i] != NULL; i++) {
        if (args[i][0] == '$') {
            char *env_var = getenv(args[i] + 1); // Skip the $ and get env var
            if (env_var) {
                printf("%s ", env_var);
            }
        } else {
            printf("%s ", args[i]);
        }
    }
    printf("\n");
}

void set_environment_variable(char *var, char *value) {
    if (setenv(var, value, 1) != 0) {
        perror("setenv error");
    }
}

void print_environment(char **args) {
    if (args[1]) { // env <var>
        char *value = getenv(args[1]);
        if (value) {
            printf("%s\n", value);
        } else {
            printf("Variable %s not found\n", args[1]);
        }
    } else { // env
        for (char **env = environ; *env != NULL; env++) {
            printf("%s\n", *env);
        }
    }
}

// Signal handler for SIGINT (Ctrl+C)
void sigint_handler(int sig) {
    printf("\nCaught signal %d. Type 'exit' to quit.\n", sig);
}

// Signal handler for process timeout
void alarm_handler(int sig) {
    printf("\nProcess exceeded time limit and was terminated.\n");
    kill(0, SIGKILL); // Kill the timed-out process
}
