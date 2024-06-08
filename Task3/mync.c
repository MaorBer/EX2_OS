#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <bits/getopt_core.h>


#define MAX_BUFFER_SIZE 1024
#define DEFAULT_TIMEOUT 10

void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s -e <command> [-i <input_option>] [-o <output_option>] [-b <bidirectional_option>] [-t <timeout>]\n", prog_name);
}

// Function to handle TCP server
void handle_tcp_server(int port, int input_fd, int timeout) {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[MAX_BUFFER_SIZE];

    // Create TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Bind socket to port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, 1) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Set timeout
    alarm(timeout);

    // Accept connection
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd == -1) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // Reset timeout
    alarm(0);

    // Close the listening socket
    close(server_fd);

    // Read input from the client and redirect to the specified file descriptor
    while (1) {
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer));
        if (bytes_read <= 0) {
            break;
        }
        write(input_fd, buffer, bytes_read);
    }

    // Close the client socket
    close(client_fd);
}

// Function to handle TCP client
void handle_tcp_client(const char *target, int port, int output_fd, int timeout) {
    int client_fd;
    struct sockaddr_in server_addr;
    char buffer[MAX_BUFFER_SIZE];

    // Create TCP socket
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, target, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }

    // Set timeout
    alarm(timeout);

    // Connect to server
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    // Reset timeout
    alarm(0);

    // Read input from the specified file descriptor and send it to the server
    while (1) {
        ssize_t bytes_read = read(output_fd, buffer, sizeof(buffer));
        if (bytes_read <= 0) {
            break;
        }
        write(client_fd, buffer, bytes_read);
    }

    // Close the client socket
    close(client_fd);
}

int main(int argc, char *argv[]) {
    int opt, input_fd = STDIN_FILENO, output_fd = STDOUT_FILENO;
    int tcp_server_port = -1;
    char *tcp_client_target = NULL;
    int tcp_client_port = -1;
    int timeout = DEFAULT_TIMEOUT;

    // Parse command-line arguments
    while ((opt = getopt(argc, argv, "e:i:o:b:t:")) != -1) {
        switch (opt) {
            case 'e':
                // Command to execute
                break;
            case 'i':
                // Redirect input option
                if (strncmp(optarg, "TCPS", 4) == 0) {
                    // Start TCP server for input
                    tcp_server_port = atoi(optarg + 4);
                } else {
                    fprintf(stderr, "Invalid input option\n");
                    print_usage(argv[0]);
                    return EXIT_FAILURE;
                }
                break;
            case 'o':
                // Redirect output option
                if (strncmp(optarg, "TCPC", 4) == 0) {
                    // Start TCP client for output
                    char *comma_pos = strchr(optarg + 4, ',');
                    if (comma_pos == NULL) {
                        fprintf(stderr, "Invalid output option\n");
                        print_usage(argv[0]);
                        return EXIT_FAILURE;
                    }
                    *comma_pos = '\0';
                    tcp_client_target = optarg + 4;
                    tcp_client_port = atoi(comma_pos + 1);
                } else {
                    fprintf(stderr, "Invalid output option\n");
                    print_usage(argv[0]);
                    return EXIT_FAILURE;
                }
                break;
            case 'b':
                // Bidirectional redirection option
                if (strncmp(optarg, "TCPS", 4) == 0) {
                    // Start TCP server for input
                    tcp_server_port = atoi(optarg + 4);
                    // Start TCP client for output
                    output_fd = socket(AF_INET, SOCK_STREAM, 0);
                    if (output_fd == -1) {
                        perror("socket");
                        return EXIT_FAILURE;
                    }
                    handle_tcp_client("localhost", tcp_server_port, output_fd, timeout);
                } else {
                    fprintf(stderr, "Invalid bidirectional option\n");
                    print_usage(argv[0]);
                    return EXIT_FAILURE;
                }
                break;
            case 't':
                // Timeout option
                timeout = atoi(optarg);
                break;
            default:
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (tcp_server_port != -1 && tcp_client_target != NULL && tcp_client_port != -1) {
        // Start TCP server for input
        int input_server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (input_server_fd == -1) {
            perror("socket");
            return EXIT_FAILURE;
        }

        // Start TCP client for output
        int output_client_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (output_client_fd == -1) {
            perror("socket");
            close(input_server_fd);
            return EXIT_FAILURE;
        }

        handle_tcp_server(tcp_server_port, input_fd, timeout);
        handle_tcp_client(tcp_client_target, tcp_client_port, output_fd, timeout);

        // Close sockets
        close(input_server_fd);
        close(output_client_fd);
    } else {
        // No TCP redirection options specified
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}