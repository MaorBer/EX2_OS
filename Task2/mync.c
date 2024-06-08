#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <getopt.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s -e <command> [-i <input>] [-o <output>] [-b <both>]\n", prog_name);
}

void handle_tcp_server(int port) {
    int server_socket;
    struct sockaddr_in server_address;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    int client_socket;
    struct sockaddr_in client_address;
    socklen_t client_address_length = sizeof(client_address);

    client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_length);
    if (client_socket == -1) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    dup2(client_socket, STDOUT_FILENO);
    close(client_socket);
}

void handle_tcp_client(const char *host, int port) {
    int client_socket;
    struct sockaddr_in server_address;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &server_address.sin_addr) <= 0) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    dup2(client_socket, STDIN_FILENO);
    close(client_socket);
}

int main(int argc, char *argv[]) {
    int opt;
    char *command = NULL;
    char *input = NULL;
    char *output = NULL;
    int port = 0;
    char *host = NULL;

    // Parse command-line arguments
    while ((opt = getopt(argc, argv, "e:i:o:b:")) != -1) {
        switch (opt) {
            case 'e':
                command = optarg;
                break;
            case 'i':
                input = optarg;
                break;
            case 'o':
                output = optarg;
                break;
            case 'b':
                input = optarg;
                output = optarg;
                break;
            default:
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (command == NULL || (input == NULL && output == NULL)) {
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
        if (input != NULL && strncmp(input, "TCPS", 4) == 0) {
            // Handle TCP server input
            port = atoi(input + 4);
            handle_tcp_server(port);
        } else if (output != NULL && strncmp(output, "TCPC", 4) == 0) {
            // Handle TCP client output
            char *delim = strchr(output + 4, ',');
            if (delim == NULL) {
                print_usage(argv[0]);
                return EXIT_FAILURE;
            }
            *delim = '\0';
            host = output + 4;
            port = atoi(delim + 1);
            handle_tcp_client(host, port);
        } else if (input != NULL && output != NULL && strncmp(input, "TCPS", 4) == 0 && strncmp(output, "TCPC", 4) == 0) {
            // Handle both TCP server and client
            port = atoi(input + 4);
            handle_tcp_server(port);
            char *delim = strchr(output + 4, ',');
            if (delim == NULL) {
                print_usage(argv[0]);
                return EXIT_FAILURE;
            }
            *delim = '\0';
            host = output + 4;
            port = atoi(delim + 1);
            handle_tcp_client(host, port);
        } else {
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }

        // Parse the command into program and arguments
        char *args[100]; // Assuming we won't have more than 100 arguments
        int arg_count = 0;

        char *token = strtok(command, " ");
        while (token != NULL) {
            args[arg_count++] = token;
            token = strtok(NULL, " ");
        }
        args[arg_count] = NULL; // Null-terminate the array

        // Execute the command
        execv(args[0], args);
        perror("execv");
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
