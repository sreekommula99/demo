#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 80 /* The maximum length command */
#define MAX_COMMANDS 10 /* The maximum number of commands */

int main(int argc, char *argv[])
{
    char *commands[MAX_COMMANDS][MAX_LINE/2 + 1]; /* commands */
    int should_run = 1; /* flag to determine when to exit program */

    while (should_run) {
        printf("SHELL$ ");
        fflush(stdout);

        char input[MAX_LINE];
        fgets(input, MAX_LINE, stdin);
        input[strcspn(input, "\n")] = 0; // remove newline character

        int i = 0;
        int background = 0;
        char *command = strtok(input, ";");
        while (command != NULL && i < MAX_COMMANDS) {
            int j = 0;
            commands[i][j] = strtok(command, " ");
            while (commands[i][j] != NULL) {
                j++;
                commands[i][j] = strtok(NULL, " ");
            }
            if (strcmp(commands[i][j-1], "&") == 0) {
                background = 1;
                commands[i][j-1] = NULL;
            }
            i++;
            command = strtok(NULL, ";");
        }

        for (int k = 0; k < i; k++) {
            if (strcmp(commands[k][0], "quit") == 0) {
                should_run = 0;
                break;
            }

            pid_t pid = fork();
            if (pid < 0) {
                fprintf(stderr, "Fork failed\n");
                exit(1);
            } else if (pid == 0) {
                if (execvp(commands[k][0], commands[k]) < 0) {
                    fprintf(stderr, "Error: command not found\n");
                    exit(1);
                }
            } else {
                if (!background) {
                    wait(NULL);
                }
            }
        }
    }

    return 0;
}