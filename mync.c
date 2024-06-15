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
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/unix_socket_example"
#define BUFFER_SIZE 1024

char *socket_path;
int udp_socket;
volatile sig_atomic_t time_up = 0;

void signal_handler(int signum)
{
    printf("Alarm triggered. Closing server.\n");
    close(udp_socket);
    exit(0);
}

void uds_client_datagram(char *socket_path, char **args)
{
    // create a socket
    int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        perror("error creating socket");
        exit(EXIT_FAILURE);
    }

    // connect to the server
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, socket_path);

    // connect the socket to the server
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("error connecting socket");
        exit(EXIT_FAILURE);
    }

    dup2(sockfd, STDIN_FILENO);
    execv(args[0], args);
}

void uds_client_stream(char *socket_path, char **args)
{
    // create a socket
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("error creating socket");
        exit(EXIT_FAILURE);
    }

    // connect to the server
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, socket_path);

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("error connecting to server");
        exit(EXIT_FAILURE);
    }

    dup2(sockfd, STDOUT_FILENO);
    execv(args[0], args);
}

void uds_server_datagram(char *socket_path, char **args)
{
    // create a socket
    int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        perror("error creating socket");
        exit(EXIT_FAILURE);
    }

    // bind the socket to the address
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, socket_path);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        printf("%s\n", addr.sun_path);
        perror("error binding socket");
        exit(EXIT_FAILURE);
    }

    // receive dummy data to get the client address
    char buffer[1024];
    struct sockaddr_un client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int bytes_received = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &client_addr_len);
    if (bytes_received == -1)
    {
        perror("error receiving data");
        exit(EXIT_FAILURE);
    }

    dup2(sockfd, STDIN_FILENO);
    execv(args[0], args);
}

void uds_server_stream(char *socket_path, char **args)
{
    // create a socket
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("error creating socket");
        exit(EXIT_FAILURE);
    }

    // bind the socket to the address
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, socket_path);


    unlink(addr.sun_path); // remove the socket file if it already exists
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("error binding socket");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // listen for incoming connections - at most 1
    if (listen(sockfd, 1) == -1)
    {
        perror("error listening on socket");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening...\n");


    // accept the connection and change the input_fd to the new socket
    struct sockaddr_un client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);

    if (client_fd == -1)
    {
        perror("error accepting connection");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    dup2(client_fd, STDIN_FILENO);
    execv(args[0], args);
}

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
        dup2(client_socket, STDOUT_FILENO);

        dup2(client_socket, STDIN_FILENO);
        execv(args[0], args);
    }

    else if (opt == 'i' && opt2 == 'u')
    {
        dup2(client_socket, STDIN_FILENO);
        uds_client_stream(socket_path, args);
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

void handle_udp_server(int port, int port2, int seconds, char **args, char opt)
{
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    int bytes_received;

    // Create UDP socket
    if ((udp_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Fill in server address details
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    // Bind socket to address
    if (bind(udp_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Socket bind failed");
        close(udp_socket);
        exit(EXIT_FAILURE);
    }

    // Set the signal handler for SIGALRM
    if (seconds != 0)
    {
        signal(SIGALRM, signal_handler);

        // Set the alarm
        alarm(seconds);
    }

    printf("UDP server is running...\n");
    int count = 0;
    while (1)
    {
        // Receive data from client
        bytes_received = recvfrom(udp_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_len);

        if (bytes_received == -1)
        {
            perror("Receive failed");
            close(udp_socket);
        exit(EXIT_FAILURE);

        }
        count++;
        if (count > 4)
        {   
            dup2(udp_socket, STDIN_FILENO);

            if(opt == 'u')
                uds_client_datagram(socket_path, args);
            if(opt == 't')
                handle_tcp_server(port, 0,  'o', ' ', args);
            if(opt == 'z')
                uds_client_stream(socket_path, args);
            if(opt == 's')
                uds_server_stream(socket_path, args);
            
            else    
                execv(args[0], args);
        }
    }
}

void parse_string(const char *str, char *ip, int *port)
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

    if(strcmp(ip, "localhost") == 0)
    {
        ip = "127.0.0.1";
    }
}

void handle_udp_clinet(int port, int port2, char *host, char **args, int seconds, char opt)
{
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int bytes_sent;
    socklen_t addr_len = sizeof(server_addr);



    // Create UDP socket
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket == -1)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Fill in server address details
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (connect(client_socket, (struct sockaddr *)&server_addr, addr_len) == -1)
    {
        perror("connect");
        exit(1);
    }
    if (seconds != 0)
    {
       signal(SIGALRM, signal_handler);

        // Set the alarm
         alarm(seconds);
    }
    
    dup2(client_socket, STDOUT_FILENO);

    if(opt == 'u')
        uds_server_datagram(socket_path, args);
    
    else if(opt == 's')
        handle_udp_server(port2, 0, seconds, args, ' ');
    
    else
        execv(args[0],args);
    


    while (1)
    {
        bytes_sent = sendto(client_socket, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (bytes_sent == -1)
        {
            perror("Send failed");
            close(client_socket);
            exit(EXIT_FAILURE);
        }
    }
    // Close socket
    close(client_socket);
}



void handle_tcp_client(char *host, char *host2, int port, int port2, char opt, char opt2, char **args)
{

    int client_socket;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server;
    socklen_t socklen = sizeof(server);

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

    else if(opt = 'o' && opt2 == 's'){
        dup2(client_socket, STDOUT_FILENO);
        uds_server_stream(socket_path, args);
    }

    else if(opt = 'o' && opt2 == 't'){
        dup2(client_socket, STDOUT_FILENO);
        handle_tcp_server(port2,port,'i', ' ', args);
    }

    else if(opt = 'i' && opt2 == 'c'){
        dup2(client_socket, STDIN_FILENO);
        uds_client_stream(socket_path, args);
    }

    else if(opt = 'i' && opt2 == 'd'){
        dup2(client_socket, STDIN_FILENO);
        handle_udp_clinet(port2, 0, host2, args, 0, ' ');
    }

    else if(opt = 'o' && opt2 == 'u'){
        dup2(client_socket, STDOUT_FILENO);
        handle_udp_server(port2, 0, 0, args, ' ');
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

        else if (opt == 'i' && opt2 == 's')
        {
            dup2(client_socket, STDIN_FILENO);
            handle_tcp_server(port2, port, 'o', ' ', args);
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
    char *seconds = NULL;
    // Parse command-line arguments
    while ((opt = getopt(argc, argv, "e:i:o:b:t:")) != -1)
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
        case 't':
            seconds = optarg;

        default:
            continue;
        }
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
        if (command != NULL)
        {
            char *token = strtok(command, " ");
            while (token != NULL)
            {
                args[arg_count++] = token;
                token = strtok(NULL, " ");
            }

            args[arg_count] = NULL;
            // Child process
            if (input == NULL && output == NULL){
                execv(args[0], args);
            }

            //input and output with TCPS
            else if (both == 1 && strncmp(input, "TCPS", 4) == 0)
            {
                port = atoi(input + 4);
                handle_tcp_server(port, 0, 'b', ' ', args);
            }

            //output with TCPS
            else if (output != NULL && input == NULL && strncmp(output, "TCPS", 4) == 0)
            {
                port = atoi(output + 4);
                handle_tcp_server(port, 0, 'o', ' ', args);
            }

            //input with TCPS
            else if (input != NULL && output == NULL && strncmp(input, "TCPS", 4) == 0) // TCPS AND I
            {
                port = atoi(input + 4);
                handle_tcp_server(port, 0, 'i', ' ', args);
            }

            //input and output with TCPC
            else if (both == 1 && strncmp(output, "TCPC", 4) == 0)  
            {
                parse_string(output, host, &port);
                handle_tcp_client(host, 0, port, 0, 'b', ' ', args);
            }

            //input with TCPC
            else if (input != NULL && output == NULL && strncmp(input, "TCPC", 4) == 0)
            {
                parse_string(input, host, &port);
                handle_tcp_client(host, 0, port, 0, 'i', ' ', args);
            }

            //output with TCPC
            else if (output != NULL && input == NULL && strncmp(output, "TCPC", 4) == 0)
            {
                parse_string(output, host, &port);
                handle_tcp_client(host, 0, port, 0, 'o', ' ', args);
            }

            //output with UDPC
            else if (output != NULL && input == NULL && strncmp(output, "UDPC", 4) == 0)
            {
                parse_string(output, host, &port);
                handle_udp_clinet(port, 0, host, args, atoi(seconds), ' ');
            }

            //input with UDPS
            else if (input != NULL && output == NULL && strncmp(input, "UDPS", 4) == 0)
            {
                port = atoi(input + 4);
                handle_udp_server(port, 0, atoi(seconds), args, ' ');
            }

            //input UDSSD
            else if (input != NULL && output == NULL && strncmp(input, "UDSSD", 5) == 0)
            {
                uds_server_datagram(input + 5, args);
            }

            //input UDSCD
            else if (input != NULL && output == NULL && strncmp(input, "UDSCD", 5) == 0)
            {
                uds_client_datagram(input + 5, args);
            }

            //output UDSSS
            else if (input == NULL && output != NULL && strncmp(output, "UDSSS", 5) == 0)
            {
                uds_server_stream(output + 5, args);
            }


            //input with TCPC and output with TCPC
            else if (output != NULL && input != NULL && strncmp(input, "TCPC", 4) == 0 && strncmp(output, "TCPC", 4) == 0)
            {
                parse_string(input, host, &port);

                parse_string(output, host2, &port2);
                handle_tcp_client(host, 0, port, 0, 'i', 'o', args);
            }

            //input with TCPC output with TCPS
            else if (output != NULL && input != NULL && strncmp(input, "TCPC", 4) == 0 && strncmp(output, "TCPS", 4) == 0 )
            {
                parse_string(input, host, &port);
                port2 = atoi(output + 4);
                handle_tcp_client(host, 0, port, port2, 'i', 's', args);
            }

            //input with TCPC output with UDPC
            else if (output != NULL && input != NULL && strncmp(output, "UDPC", 4) == 0 && strncmp(input, "TCPC", 4) == 0)
            {
                parse_string(input, host, &port);
                parse_string(output, host2, &port2);
                handle_tcp_client(host, host2, port,port2, 'i', 'd', args);
            }

            //input with TCPC output with UDPC           
            else if (input != NULL && output != NULL && strncmp(input, "TCPS", 4) == 0 && strncmp(output, "UDPC", 4) == 0)
            {
                port = atoi(input + 4);
                handle_tcp_server(port, 0, 'i', ' ', args);
                parse_string(output, host, &port);
                handle_udp_clinet(port, 0, host, args, 0,  ' ');
            }

            //input with TCPC output with USSCS
            else if (input != NULL && output != NULL && strncmp(input, "TCPC", 4) == 0 && strncmp(output, "UDSCS", 5) == 0)
            {
                socket_path = output + 5;
                parse_string(input, host, &port);
                
                handle_tcp_client(host, 0, port, 0, 'i', 'c', args); 
            }


            //input with TCPS  output with TCPS
            else if (output != NULL && input != NULL && strncmp(output, "TCPS", 4) == 0 && strncmp(input, "TCPS", 4) == 0)
            {
                handle_tcp_server(atoi(input + 4), atoi(output +4), 'i', 'o', args);
            }

            //input with TCPS output with TCPC
            else if (output != NULL && input != NULL && strncmp(output, "TCPC", 4) == 0 && strncmp(input, "TCPS", 4) == 0)
            {
                parse_string(output, host, &port);
                port2 = atoi(input + 4);
                handle_tcp_client(host, 0, port, port2, 'o', 's', args);
            }

            //input with TCPS output with USSCS
            else if (input != NULL && output != NULL && strncmp(input, "TCPS", 4) == 0 && strncmp(output, "UDSCS", 5) == 0)
            {
                socket_path = output + 5;
                parse_string(input, host, &port);
                
                handle_tcp_server(port, 0, 'i', 'u', args); 
            }

            
            //input with UDPS output with UDPS
            else if (input != NULL && output != NULL && strncmp(input, "UDPS", 4) == 0 && strncmp(output, "UDPC", 4) == 0){
                parse_string(output, host, &port);
                handle_udp_clinet(port, port2, host, args, atoi(seconds), 's');
            }

            //input with UDPS output with UDSCD
            else if (input != NULL && output != NULL && strncmp(input, "UDPS", 4) == 0 && strncmp(output, "UDSCD", 5) == 0)
            {
                socket_path = output + 5;
                parse_string(input, host, &port);
                handle_udp_server(port, 0, atoi(seconds), args, 'u'); 
            }

            //input with UDPS output with UDSSS
            else if (input != NULL && output != NULL && strncmp(input, "UDPS", 4) == 0 && strncmp(output, "UDSSS", 5) == 0)
            {
                socket_path = output + 5;
                parse_string(input, host, &port);
                handle_udp_server(port, 0, atoi(seconds), args, 's'); 
            }

            //input with UDPS output with UDSCS         
            else if (input != NULL && output != NULL && strncmp(input, "UDPS", 4) == 0 && strncmp(output, "UDSCS", 5) == 0)
            {
                socket_path = output + 5;
                parse_string(input, host, &port);
                handle_udp_server(port, 0, atoi(seconds), args, 'z'); 
            }

            //input with UDPS output wuth TCPS
            else if(input != NULL && output != NULL && strncmp(input, "UDPS", 4) == 0 && strncmp(output, "TCPS", 4) == 0)
            {
                handle_udp_server(atoi(input + 4), atoi(output +4), atoi(seconds), args, 't');

            }

            //input with UDPS output with TCPC
            else if (input != NULL && output != NULL && strncmp(input, "UDPS", 4) == 0 && strncmp(output, "TCPC", 4) == 0)
            {
                port = atoi(input + 4);
                parse_string(output, host, &port);
                handle_tcp_client(host, 0, port, port2, 'o', 'u', args);
            }


            //input UDSSD output with UDPC
            else if (input != NULL && output != NULL && strncmp(input, "UDSSD", 5) == 0 && strncmp(output, "UDPC", 4) == 0)
            {
                socket_path = input + 5;
                parse_string(output, host, &port);
                
                handle_udp_clinet(port, 0, host, args, 0,'u'); 
            }


            //input UDSSS ouput with TCPC
            else if (input != NULL && output != NULL && strncmp(input, "UDSSS", 5) == 0 && strncmp(output, "TCPC", 4) == 0)
            {
                socket_path = input + 5;
                parse_string(output, host, &port);
                
                handle_tcp_client(host, 0, port, 0, 'o', 's', args);

            }

            else
            {
                printf("else");
                print_usage(argv[0]);
                return EXIT_FAILURE;
            }
        }

        //without '-e'
        else
        {
            char *args[100];
            for (int i = 1; i < argc; ++i)
            {
                size_t length = strlen(argv[i]) + 1;
                memcpy(args[i - 1], argv[i], length);
            }

            args[argc - 1] = NULL;
            execv(args[0], args);
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
