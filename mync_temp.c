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

void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s -e <command> [-i <input>] [-o <output>] [-b <both>]\n", prog_name);
}

int s_socket_tcp(int port)
{

    int server_socket = 0;
    struct sockaddr_in server_address;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    int optval = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
    {
        perror("setsockopt");
        exit(0);
    }

    return server_socket;
}

void handle_tcp_server(int port, int port2, char opt, char opt2, char **args)
{
    int server_socket = s_socket_tcp(port);
    int server_socket2;
    int client_socket2, client_socket;

    struct sockaddr_in client_address;
    socklen_t client_address_length = sizeof(client_address);

    if (listen(server_socket, 5) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_length);
    if (client_socket == -1)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    if (port2 != 0)
    {
        server_socket2 = s_socket_tcp(port2);
        if (listen(server_socket2, 1) == -1)
        {
            perror("listen");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in client_address2;
        socklen_t client_address_length2 = sizeof(client_address2);

        client_socket2 = accept(server_socket2, (struct sockaddr *)&client_address2, &client_address_length2);
        if (client_socket2 == -1)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
    }

    if (opt == 'i' && opt2 == ' ')
    {
        dup2(client_socket, STDIN_FILENO);
        execv(args[0], args);
    }

    else if (opt == 'o' && opt2 == ' ')
    {
        dup2(client_socket, STDOUT_FILENO);
        execv(args[0], args);
    }

    else if (opt == 'b' && opt2 == ' ')
    {
        dup2(client_socket, STDIN_FILENO);
        dup2(client_socket, STDOUT_FILENO);
        execv(args[0], args);
    }

    else if (opt == 'i' && opt2 == 'o')
    {
        dup2(client_socket, STDIN_FILENO);
        dup2(client_socket2, STDOUT_FILENO);
        execv(args[0], args);
    }

    else if (opt == 'o' && opt2 == 'i')
    {
        dup2(client_socket2, STDIN_FILENO);
        dup2(client_socket, STDOUT_FILENO);
        execv(args[0], args);
    }

    close(server_socket);
    close(client_socket);
    close(server_socket2);
    close(client_socket2);
}

void parse_tcpc_string(const char *str, char *ip, int *port)
{
    // Find the position of the comma
    const char *comma_pos = strchr(str, ',');
    if (!comma_pos)
    {
        // Comma not found, unable to parse
        *port = -1;
        return;
    }

    // Calculate the length of the IP address or hostname
    int ip_length = comma_pos - (str + 4); // Length of "TCPC"
    if (ip_length <= 0)
    {
        // Invalid IP address or hostname
        *port = -1;
        return;
    }

    // Copy the IP address or hostname
    strncpy(ip, str + 4, ip_length);
    ip[ip_length] = '\0'; // Null-terminate the string

    // Extract the port number and convert it to an integer
    *port = atoi(comma_pos + 1);
}

void split_tcpc_strings(const char *str, char *str1, char *str2)
{
    const char *tcpc_start = strstr(str, "TCPC");

    if (tcpc_start == NULL)
    {
        strcpy(str1, str);
        str2[0] = '\0'; // Empty string for str2
        return;
    }

    // Copy the first part before the first TCPC occurrence
    strncpy(str1, str, tcpc_start - str);
    str1[tcpc_start - str] = '\0'; // Null-terminate the first string

    // Find the second TCPC occurrence
    const char *tcpc_end = strstr(tcpc_start + 1, "TCPC");
    if (tcpc_end == NULL)
    {
        strcpy(str2, tcpc_start); // Copy the rest of the string if only one TCPC occurrence
    }
    else
    {
        // Copy the part between the first and second TCPC occurrences
        strncpy(str2, tcpc_start, tcpc_end - tcpc_start);
        str2[tcpc_end - tcpc_start] = '\0'; // Null-terminate the second string
    }
}

void handle_tcp_client(const char *host, const char *host2, int port, int port2, char opt, char opt2, char **args)
{
    printf("%d %s\n", port2, host2);

    int client_socket;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server;
    socklen_t socklen = sizeof(server);
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(host);

    if (connect(client_socket, (struct sockaddr *)&server, socklen) == -1)
    {
        perror("connect");
        exit(1);
    }

    if (opt == 'i' && opt2 == ' ')
    {
        dup2(client_socket, STDIN_FILENO);
        execv(args[0], args);
    }

    else if (opt == 'o' && opt2 == ' ')
    {
        dup2(client_socket, STDOUT_FILENO);
        execv(args[0], args);
    }

    else if (opt == 'b' && opt2 == ' ')
    {
        dup2(client_socket, STDOUT_FILENO);
        dup2(client_socket, STDIN_FILENO);
        execv(args[0], args);
    }

    if (port2 != '0')
    {
        int client_socket2;
        client_socket2 = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in server2;
        socklen_t socklen2 = sizeof(server2);
        client_socket2 = socket(AF_INET, SOCK_STREAM, 0);

        server2.sin_family = AF_INET;
        server2.sin_port = htons(port2);
        server2.sin_addr.s_addr = inet_addr(host2);

        if (connect(client_socket2, (struct sockaddr *)&server2, socklen) == -1)
        {
            perror("connect");
            exit(1);
        }

        if (opt == 'i' && opt2 == 'o')
        {
            dup2(client_socket, STDIN_FILENO);
            dup2(client_socket2, STDOUT_FILENO);
            execv(args[0], args);
        }

        else if (opt == 'o' && opt2 == 'i')
        {
            dup2(client_socket2, STDIN_FILENO);
            dup2(client_socket, STDOUT_FILENO);
            execv(args[0], args);
        }
    }

    close(client_socket);
}

int main(int argc, char *argv[])
{
    int opt;
    char *command = NULL;
    char *input = NULL;
    char *output = NULL;
    int port = 0;
    char host[120] = {0};
    char host2[120] = {0};
    int server_socket;
    int both = 0;
    int port2 = 0;

    // Parse command-line arguments
    while ((opt = getopt(argc, argv, "e:i:o:b:")) != -1)
    {
        switch (opt)
        {
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
            both = 1;
            input = optarg;
            output = optarg;
            break;
        default:
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (command == NULL || (input == NULL && output == NULL))
    {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
    // Create a child process
    pid_t pid = fork();

    if (pid == -1)
    {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0)
    {
        char *args[100]; // Assuming we won't have more than 100 arguments
        int arg_count = 0;

        char *token = strtok(command, " ");
        while (token != NULL)
        {
            args[arg_count++] = token;
            token = strtok(NULL, " ");
        }
        args[arg_count] = NULL;
        // Child process

        if (output == NULL && input == NULL)
            execv(args[0], args);

        else if (both == 1 && (input, "TCPS", 4) == 0)
        {
            port = atoi(input + 4);
            handle_tcp_server(port, 0, 'b', ' ', args);
        }

        else if (input != NULL && strncmp(input, "TCPS", 4) == 0) // TCPS AND I
        {
            port = atoi(input + 4);
            handle_tcp_server(port, 0, 'i', ' ', args);
        }

        else if (output != NULL && strncmp(output, "TCPS", 4) == 0)
        {
            port = atoi(output + 4);
            handle_tcp_server(port, 0, 'o', ' ', args);
        }

        else if (output != NULL && input != NULL && strncmp(input, "TCPS", 4) == 0 && strncmp(output, "TCPS", 4) == 0)
        {
            port = atoi(input + 4);
            port2 = atoi(output + 4);
            handle_tcp_server(port, port2, 'i', 'o', args);
        }

        else if (input != NULL && strncmp(input, "TCPC", 4) == 0)
        {
            parse_tcpc_string(input, host, &port);
            handle_tcp_client(host, 0, port, 0, 'i', ' ', args);
        }

        else if (output != NULL && strncmp(output, "TCPC", 4) == 0)
        {
            parse_tcpc_string(output, host, &port);
            handle_tcp_client(host, 0, port, 0, 'o', ' ', args);
        }

        else if (both == 1 && strncmp(output, "TCPC", 4) == 0)
        {
            parse_tcpc_string(output, host, &port);
            handle_tcp_client(host, 0, port, 0, 'b', ' ', args);
        }
        else if (output != NULL && input != NULL && strncmp(input, "TCPC", 4) == 0 && strncmp(output, "TCPC", 4) == 0)
        {
            // Split input into two parts containing TCPC strings
            char str1_input[256];                              // Allocate memory for the first part of input
            char str2_input[256];                              // Allocate memory for the second part of input
            split_tcpc_strings(input, str1_input, str2_input); // Split input string into two parts
            parse_tcpc_string(str1_input, host, &port);        // Parse the first TCPC string in input
            handle_tcp_client(host, 0, port, 0, 'i', ' ', args);

            // Reset host and port for the second TCPC string in input
            memset(host, 0, sizeof(host));
            port = 0;

            // Split output into two parts containing TCPC strings
            char str1_output[256];                                // Allocate memory for the first part of output
            char str2_output[256];                                // Allocate memory for the second part of output
            split_tcpc_strings(output, str1_output, str2_output); // Split output string into two parts
            parse_tcpc_string(str1_output, host2, &port2);        // Parse the first TCPC string in output
            handle_tcp_client(host2, 0, port2, 0, 'o', ' ', args);
        }

        else
        {
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    else
    {
        // Parent process
        int status;
        if (waitpid(pid, &status, 0) == -1)
        {
            perror("waitpid");
            return EXIT_FAILURE;
        }

        if (WIFEXITED(status))
        {
            printf("Child exited with status %d\n", WEXITSTATUS(status));
        }

        else if (WIFSIGNALED(status))
        {
            printf("Child killed by signal %d\n", WTERMSIG(status));
        }
    }

    return EXIT_SUCCESS;
}
