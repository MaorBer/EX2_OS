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

#define BUFFER_SIZE 1024

int udp_socket;
volatile sig_atomic_t time_up = 0;

void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s -e <command> [-i <input>] [-o <output>] [-b <both>]\n", prog_name);
}

int s_socket_tcp(int port){
    
    int server_socket = 0; 
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

    int optval = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        perror("setsockopt");
        exit(0);
    }

    return server_socket;
}

void handle_tcp_server(int port, int port2, char opt, char opt2, char** args) {
    int server_socket = s_socket_tcp(port);
    int server_socket2;
    int client_socket2, client_socket;

    struct sockaddr_in client_address;
    socklen_t client_address_length = sizeof(client_address);

    

    if (listen(server_socket, 5) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_length);
    if (client_socket == -1) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    if (port2 != 0) {
        server_socket2 = s_socket_tcp(port2);
        if (listen(server_socket2, 1) == -1) {
            perror("listen");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in client_address2;
        socklen_t client_address_length2 = sizeof(client_address2);

        client_socket2 = accept(server_socket2, (struct sockaddr *)&client_address2, &client_address_length2);
        if (client_socket2 == -1) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

    }

    if (opt == 'i' && opt2 == ' '){
        dup2(client_socket, STDIN_FILENO);
        execv(args[0], args);
    }

    else if(opt == 'o' && opt2 == ' '){
        dup2(client_socket, STDOUT_FILENO);
        execv(args[0], args);
    }

    else if(opt == 'b' && opt2 == ' '){
        dup2(client_socket, STDIN_FILENO);
        dup2(client_socket, STDOUT_FILENO);
        execv(args[0], args);
    }

    else if(opt == 'i' && opt2 == 'o')
    {
        dup2(client_socket, STDIN_FILENO);
        dup2(client_socket2, STDOUT_FILENO);
        execv(args[0], args);
    }

    else if(opt == 'o' && opt2 == 'i')
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

void parse_string(const char *str, char *ip, int *port) {
    // Find the position of the comma
    const char *comma_pos = strchr(str, ',');
    if (!comma_pos) {
        // Comma not found, unable to parse
        *port = -1;
        return;
    }
    
    // Calculate the length of the IP address or hostname
    int ip_length = comma_pos - (str + 4); // Length of "TCPC"
    if (ip_length <= 0) {
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

void handle_tcp_client(const char *host, const char *host2, int port, int port2, char opt, char opt2, char** args)
{

    int client_socket;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server;
    socklen_t socklen = sizeof(server);
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family =AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(host);

    if(connect(client_socket, (struct sockaddr *) &server, socklen)==-1)
    {
        perror("connect");   
        exit(1); 
    }
    
    if (opt == 'i' && opt2 == ' '){
        dup2(client_socket, STDIN_FILENO);
        execv(args[0], args);
    }
     
    else if(opt == 'o' && opt2 == ' '){
        dup2(client_socket, STDOUT_FILENO);
        execv(args[0], args);
    }

    else if(opt == 'b' && opt2 == ' '){
        dup2(client_socket, STDOUT_FILENO);
        dup2(client_socket, STDIN_FILENO);
        execv(args[0], args);
    }


    if(port2 != '0'){
        int client_socket2;
        client_socket2 = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in server2;
        socklen_t socklen2 = sizeof(server2);
        client_socket2 = socket(AF_INET, SOCK_STREAM, 0);

        server2.sin_family =AF_INET;
        server2.sin_port = htons(port2);
        server2.sin_addr.s_addr = inet_addr(host2);

        if(connect(client_socket2, (struct sockaddr *) &server2, socklen)==-1)
         {
             perror("connect");   
             exit(1); 
        }
             
        if(opt == 'i' && opt2 == 'o'){
            dup2(client_socket, STDIN_FILENO);
            dup2(client_socket2, STDOUT_FILENO);
            execv(args[0], args);
        }

        else if(opt == 'o' && opt2 == 'i'){
            dup2(client_socket2, STDIN_FILENO);
            dup2(client_socket, STDOUT_FILENO);
            execv(args[0], args);
        }
    }

    close(client_socket);
}
 
void signal_handler(int signum) {
    printf("Alarm triggered. Closing server.\n");
    close(udp_socket);
    exit(0);
}

void handle_udp_server(int port, int seconds, char** args){
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    int bytes_received;

    // Create UDP socket
    if ((udp_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Fill in server address details
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    // Bind socket to address
    if (bind(udp_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Socket bind failed");
        close(udp_socket);
        exit(EXIT_FAILURE);
    }

    // Set the signal handler for SIGALRM
    signal(SIGALRM, signal_handler);

    // Set the alarm
    alarm(seconds);

    printf("UDP server is running...\n");
    int count = 0;
    while (1) {
        // Receive data from client
        bytes_received = recvfrom(udp_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_len);
        
        if (bytes_received == -1) {
            perror("Receive failed");
            close(udp_socket);
            exit(EXIT_FAILURE);
        }
        count++;
        if(count> 4)
        {
            dup2(udp_socket, STDIN_FILENO);
            execv(args[0], args);
        }
    }
}

void handle_udp_clinet(int port, char* host, char** args, int seconds){
    struct sockaddr_in server_addr;
    int client_socket;
    char buffer[BUFFER_SIZE];
    int bytes_sent = 0;

    // Create UDP socket
    if ((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Fill in server address details
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(host); // Change this to the server's IP address

    for(int i = 0; i <3; i++){
        dup2(client_socket,STDOUT_FILENO);
        execv(args[0], args);
        bytes_sent = sendto(client_socket, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (bytes_sent == -1) {
            perror("Send failed");
            close(client_socket);
            exit(EXIT_FAILURE);
        }

        printf("Message sent successfully: %s\n", buffer);
        sleep(1);
    }
    
    printf("3\n");

    // Close socket
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
        if (command != NULL){
            char *token = strtok(command, " ");
            while (token != NULL){
                args[arg_count++] = token;
                token = strtok(NULL, " ");
            }
            args[arg_count] = NULL;
            // Child process
            if(input == NULL && output == NULL)
                execv(args[0], args);


            else if(both == 1 && (input, "TCPS", 4) == 0)
            {
                port = atoi(input + 4);
                handle_tcp_server(port, 0, 'b', ' ', args);
            }
    
            else if (input != NULL && output == NULL && strncmp(input, "TCPS", 4) == 0) //TCPS AND I 
            {
                port = atoi(input + 4);
                handle_tcp_server(port, 0, 'i', ' ', args);
            }

            else if(output != NULL  && input == NULL &&  strncmp(output, "TCPS", 4) == 0)
            {
                port = atoi(output + 4);
                handle_tcp_server(port, 0, 'o', ' ', args);
            }

            else if(output != NULL && input != NULL && strncmp(input, "TCPS", 4) == 0 && strncmp(output, "TCPS", 4) == 0)
            {
                port = atoi(input + 4);
                port2 = atoi(output + 4);
                handle_tcp_server(port, port2, 'i', 'o', args);
            }

            else if(input != NULL && output == NULL && strncmp(input, "TCPC", 4) == 0){
                parse_string(input, host, &port);
                handle_tcp_client(host, 0, port, 0, 'i', ' ',args);
            }

            else if(output != NULL && input == NULL && strncmp(output, "TCPC", 4) == 0){
                parse_string(output, host, &port);
                handle_tcp_client(host, 0, port, 0, 'o', ' ', args);
            }

            else if(both == 1 && strncmp(output, "TCPC", 4) == 0){
                parse_string(output, host, &port);
                handle_tcp_client(host, 0, port, 0, 'b', ' ',args);
            }

            else if(output != NULL && input != NULL && strncmp(input, "TCPC", 4) == 0 && strncmp(output, "TCPC", 4) == 0)
            {
                parse_string(input, host, &port);
                parse_string(output, host2, &port2);
                
                handle_tcp_client(host, host2, port, port2, 'i', 'o', args);
            }

            else if(input != NULL && output == NULL && strncmp(input, "UDPS", 4) == 0)
            {
                port = atoi(input +4);
                handle_udp_server(port, atoi(seconds), args);
            }

            else if(output != NULL && input == NULL && strncmp(output, "UDPC", 4) == 0){
                parse_string(output, host, &port);
                handle_udp_clinet(port, host, args, atoi(seconds));
            }
            
            else{
                print_usage(argv[0]);
                return EXIT_FAILURE;
            }
        }

        else{
            char *args[100];
            for(int i = 1; i < argc; ++i){
                size_t length = strlen(argv[i])+1;
                memcpy(args[i-1], argv[i], length);
             }
            
            args[argc-1] = NULL;
            execv(args[0], args);
        }
    }
    
    

    return EXIT_SUCCESS;
}
