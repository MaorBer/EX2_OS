#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <bits/getopt_core.h>

void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s -e <command>\n", prog_name);
}

int main(int argc, char *argv[]) {
    int opt;
    char *command = NULL;

    // Parse command-line arguments
    while ((opt = getopt(argc, argv, "e:")) != -1) {
        switch (opt) {
            case 'e':
                command = optarg;
                break;
            default:
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (command == NULL) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    // Create a child process
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        // Child process
        // Execute the command
        execlp("sh", "sh", "-c", command, (char *) NULL);
        perror("execlp");
        return EXIT_FAILURE;
    } else {
        // Parent process
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            return EXIT_FAILURE;
        }

        if (WIFEXITED(status)) {
            printf("Child exited with status %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Child killed by signal %d\n", WTERMSIG(status));
        }
    }

    return EXIT_SUCCESS;
}
