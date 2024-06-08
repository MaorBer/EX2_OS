#include <arpa/inet.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

int inputFileDescriptor = STDIN_FILENO;
int outputFileDescriptor = STDOUT_FILENO;

void terminate(int exitCode) {
    if (inputFileDescriptor != STDIN_FILENO) {
        close(inputFileDescriptor);
    }
    if (outputFileDescriptor != STDOUT_FILENO && outputFileDescriptor != inputFileDescriptor) {
        close(outputFileDescriptor);
    }
    exit(exitCode);
}

void signalHandler(int signal) {
    terminate(EXIT_SUCCESS);
}

int createTCPServer(int port) {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Socket creation failed");
        terminate(EXIT_FAILURE);
    }

    int option = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1) {
        perror("setsockopt failed");
        terminate(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Binding failed");
        terminate(EXIT_FAILURE);
    }

    if (listen(serverSocket, 1) == -1) {
        perror("Listening failed");
        terminate(EXIT_FAILURE);
    }

    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);

    if (clientSocket == -1) {
        perror("Accepting connection failed");
        terminate(EXIT_FAILURE);
    }
    return clientSocket;
}

int connectToTCPServer(char *hostname, char *port) {
    struct addrinfo hints, *result, *resPtr;
    int status, clientSocket;

    memset(&hints, 0, sizeof hints);
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(hostname, port, &hints, &result)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        terminate(EXIT_FAILURE);
    }

    for (resPtr = result; resPtr != NULL; resPtr = resPtr->ai_next) {
        if ((clientSocket = socket(resPtr->ai_family, resPtr->ai_socktype, resPtr->ai_protocol)) == -1) {
            perror("Socket creation failed");
            continue;
        }

        if (connect(clientSocket, resPtr->ai_addr, resPtr->ai_addrlen) == -1) {
            close(clientSocket);
            perror("Connection to server failed");
            continue;
        }
        break;
    }

    if (resPtr == NULL) {
        fprintf(stderr, "Failed to connect to server\n");
        terminate(EXIT_FAILURE);
    }

    freeaddrinfo(result);
    return clientSocket;
}

int createUDPServer(int port) {
    int serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == -1) {
        perror("Socket creation failed");
        terminate(EXIT_FAILURE);
    }

    int option = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1) {
        perror("setsockopt failed");
        terminate(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Binding failed");
        terminate(EXIT_FAILURE);
    }

    char buffer[1024];
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    int receivedBytes = recvfrom(serverSocket, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (receivedBytes == -1) {
        perror("Receiving data failed");
        terminate(EXIT_FAILURE);
    }

    if (connect(serverSocket, (struct sockaddr *)&clientAddr, clientAddrLen) == -1) {
        perror("Connection to client failed");
        terminate(EXIT_FAILURE);
    }

    return serverSocket;
}

int connectToUDPServer(char *hostname, char *port) {
    struct addrinfo hints, *result, *resPtr;
    int status, clientSocket;

    memset(&hints, 0, sizeof hints);
    hints.ai_socktype = SOCK_DGRAM;

    if ((status = getaddrinfo(hostname, port, &hints, &result)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        terminate(EXIT_FAILURE);
    }

    for (resPtr = result; resPtr != NULL; resPtr = resPtr->ai_next) {
        if ((clientSocket = socket(resPtr->ai_family, resPtr->ai_socktype, resPtr->ai_protocol)) == -1) {
            perror("Socket creation failed");
            continue;
        }
        sendto(clientSocket, "Connection message\n", 19, 0, resPtr->ai_addr, resPtr->ai_addrlen);

        if (connect(clientSocket, resPtr->ai_addr, resPtr->ai_addrlen) == -1) {
            close(clientSocket);
            perror("Connection to server failed");
            continue;
        }
        break;
    }

    if (resPtr == NULL) {
        fprintf(stderr, "Failed to connect to server\n");
        terminate(EXIT_FAILURE);
    }

    freeaddrinfo(result);
    return clientSocket;
}

int createStreamUDSServer(char *socketPath) {
    int serverSocket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Socket creation failed");
        terminate(EXIT_FAILURE);
    }

    struct sockaddr_un serverAddr;
    serverAddr.sun_family = AF_UNIX;
    strcpy(serverAddr.sun_path, socketPath);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Binding failed");
        terminate(EXIT_FAILURE);
    }

    if (listen(serverSocket, 1) == -1) {
        perror("Listening failed");
        terminate(EXIT_FAILURE);
    }

    struct sockaddr_un clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);

    if (clientSocket == -1) {
        perror("Accepting connection failed");
        terminate(EXIT_FAILURE);
    }
    return clientSocket;
}

int createDatagramUDSServer(char *socketPath) {
    int serverSocket = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (serverSocket == -1) {
        perror("Socket creation failed");
        terminate(EXIT_FAILURE);
    }

    struct sockaddr_un serverAddr;
    serverAddr.sun_family = AF_UNIX;
    strcpy(serverAddr.sun_path, socketPath);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Binding failed");
        terminate(EXIT_FAILURE);
    }

    char buffer[1024];
    struct sockaddr_un clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    int receivedBytes = recvfrom(serverSocket, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (receivedBytes == -1) {
        perror("Receiving data failed");
        terminate(EXIT_FAILURE);
    }

    return serverSocket;
}

int connectToStreamUDSServer(char *socketPath) {
    int clientSocket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Socket creation failed");
        terminate(EXIT_FAILURE);
    }

    struct sockaddr_un serverAddr;
    serverAddr.sun_family = AF_UNIX;
    strcpy(serverAddr.sun_path, socketPath);

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Connection to server failed");
        terminate(EXIT_FAILURE);
    }

    return clientSocket;
}

int connectToDatagramUDSServer(char *socketPath) {
    int clientSocket = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (clientSocket == -1) {
        perror("Socket creation failed");
        terminate(EXIT_FAILURE);
    }

    struct sockaddr_un serverAddr;
    serverAddr.sun_family = AF_UNIX;
    strcpy(serverAddr.sun_path, socketPath);

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Connection to server failed");
        terminate(EXIT_FAILURE);
    }

    return clientSocket;
}

void parseServerDetails(char *input, char **hostname, char **port) {
    *hostname = strtok(input, ",");
    if (*hostname == NULL) {
        fprintf(stderr, "Invalid hostname\n");
        terminate(EXIT_FAILURE);
    }

    *port = strtok(NULL, ",");
    if (*port == NULL) {
        fprintf(stderr, "Invalid port\n");
        terminate(EXIT_FAILURE);
    }
}

void printUsage(char *programName) {
    fprintf(stderr, "Usage: %s [-c] [-p protocol] [-s source] [-d destination]\n", programName);
    fprintf(stderr, "  -c: Create server\n");
    fprintf(stderr, "  -p protocol: Protocol to use (tcp, udp, stream-uds, dgram-uds)\n");
    fprintf(stderr, "  -s source: Source address (hostname,port) or socket path\n");
    fprintf(stderr, "  -d destination: Destination address (hostname,port) or socket path\n");
    exit(EXIT_FAILURE);
}

void redirectData() {
    char buffer[1024];
    ssize_t bytesRead;

    while ((bytesRead = read(inputFileDescriptor, buffer, sizeof(buffer))) > 0) {
        if (write(outputFileDescriptor, buffer, bytesRead) != bytesRead) {
            perror("Writing to output failed");
            terminate(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[]) {
    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);

    int option;
    int isServer = 0;
    char *protocol = NULL;
    char *source = NULL;
    char *destination = NULL;

    while ((option = getopt(argc, argv, "cp:s:d:")) != -1) {
        switch (option) {
        case 'c':
            isServer = 1;
            break;
        case 'p':
            protocol = optarg;
            break;
        case 's':
            source = optarg;
            break;
        case 'd':
            destination = optarg;
            break;
        default:
            printUsage(argv[0]);
        }
    }

    if (!protocol || (!source && !isServer) || !destination) {
        printUsage(argv[0]);
    }

    if (strcmp(protocol, "tcp") == 0) {
        if (isServer) {
            int port = atoi(destination);
            if (port == 0) {
                fprintf(stderr, "Invalid port\n");
                terminate(EXIT_FAILURE);
            }
            outputFileDescriptor = createTCPServer(port);
        } else {
            char *hostname, *port;
            parseServerDetails(destination, &hostname, &port);
            outputFileDescriptor = connectToTCPServer(hostname, port);
        }
    } else if (strcmp(protocol, "udp") == 0) {
        if (isServer) {
            int port = atoi(destination);
            if (port == 0) {
                fprintf(stderr, "Invalid port\n");
                terminate(EXIT_FAILURE);
            }
            outputFileDescriptor = createUDPServer(port);
        } else {
            char *hostname, *port;
            parseServerDetails(destination, &hostname, &port);
            outputFileDescriptor = connectToUDPServer(hostname, port);
        }
    } else if (strcmp(protocol, "stream-uds") == 0) {
        if (isServer) {
            outputFileDescriptor = createStreamUDSServer(destination);
        } else {
            outputFileDescriptor = connectToStreamUDSServer(destination);
        }
    } else if (strcmp(protocol, "dgram-uds") == 0) {
        if (isServer) {
            outputFileDescriptor = createDatagramUDSServer(destination);
        } else {
            outputFileDescriptor = connectToDatagramUDSServer(destination);
        }
    } else {
        fprintf(stderr, "Unsupported protocol\n");
        terminate(EXIT_FAILURE);
    }

    if (source) {
        inputFileDescriptor = open(source, O_RDONLY);
        if (inputFileDescriptor == -1) {
            perror("Opening source file failed");
            terminate(EXIT_FAILURE);
        }
    }

    redirectData();
    terminate(EXIT_SUCCESS);

    return 0;
}
