#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS 64

void execute_command(char *cmd, char **args);
int parse_command(char *input, char **cmd, char **args);
void run_shell();

int main() {
    run_shell();
    return 0;
}

void run_shell() {
    char *input;
    char *cmd;
    char *args[MAX_ARGS];

    while (1) {
        input = readline("mysh> ");
        if (input == NULL || strlen(input) == 0) {
            free(input);
            continue;
        }

        if (parse_command(input, &cmd, args)) {
            if (strcmp(cmd, "exit") == 0) {
                free(input);
                break;
            }
            execute_command(cmd, args);
        }

        free(input);
    }
}

int parse_command(char *input, char **cmd, char **args) {
    int i = 0;
    char *token = strtok(input, " ");

    if (!token) {
        return 0;
    }

    *cmd = token;
    token = strtok(NULL, " ");

    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    return 1;
}

void execute_command(char *cmd, char **args) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
    } else if (pid == 0) {
        // Child process
        if (execvp(cmd, args) == -1) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
    }
}
