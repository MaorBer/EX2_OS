#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <sys/time.h>

#define MAXDATASIZE 100
#define SERVERPORT "4950"   // the port users will be connecting to
#define MYPORT "5050"
#define BACKLOG 10          // how many pending connections queue will hold

int createUdpServer(const char *port)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int yes = 1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;          // IPv4
    hints.ai_socktype = SOCK_DGRAM;     // UDP
    hints.ai_flags = AI_PASSIVE;        // use my IP

    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    // Loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        return -1;
    }

    freeaddrinfo(servinfo);

    return sockfd;
}

int createUdpClient(const char *host, const char *port)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;          // IPv4
    hints.ai_socktype = SOCK_DGRAM;     // UDP

    if ((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    // Loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return -1;
    }

    freeaddrinfo(servinfo);

    return sockfd;
}

void sigalarm_handler(int signum) {
    // Handle alarm signal
    exit(0);
}

int main(int argc, char *argv[])
{
    if (argc < 5) {
        fprintf(stderr, "Usage: %s -e \"message\" -i UDPS<PORT> -o UDPC<IP, PORT/hostname, port> -t <timeout>\n", argv[0]);
        exit(1);
    }

    char *message = NULL;
    char *udpServerPort = NULL;
    char *udpClientArgs = NULL;
    int timeout = 0;

    // Parsing command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-e") == 0) {
            message = argv[i+1];
        } else if (strncmp(argv[i], "-i", 2) == 0) {
            udpServerPort = strtok(argv[i+1], "S");
        } else if (strncmp(argv[i], "-o", 2) == 0) {
            udpClientArgs = strtok(argv[i+1], "C");
        } else if (strcmp(argv[i], "-t") == 0) {
            timeout = atoi(argv[i+1]);
        }
    }

    if (message == NULL || udpServerPort == NULL || udpClientArgs == NULL || timeout < 0) {
        fprintf(stderr, "Invalid arguments\n");
        exit(1);
    }

    int serverSockfd, clientSockfd;
    char buf[MAXDATASIZE];
    struct sigaction sa;
    struct itimerval timer;

    // Set up the SIGALRM handler
    sa.sa_handler = sigalarm_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    // Set the alarm for timeout seconds
    if (timeout > 0) {
        timer.it_value.tv_sec = timeout;
        timer.it_value.tv_usec = 0;
        timer.it_interval = timer.it_value;
        setitimer(ITIMER_REAL, &timer, NULL);
    }

    // Start UDP server
    serverSockfd = createUdpServer(udpServerPort);

    // Start UDP client
    char *host = strtok(udpClientArgs, ",");
    char *port = strtok(NULL, ",");
    clientSockfd = createUdpClient(host, port);

    // Read from UDP server and write to UDP client
    for (;;) {
        ssize_t numbytes = recvfrom(serverSockfd, buf, MAXDATASIZE - 1, 0, NULL, NULL);
        if (numbytes == -1) {
            perror("recvfrom");
            exit(1);
        }
        buf[numbytes] = '\0';
        send(clientSockfd, buf, strlen(buf), 0);
    }

    close(serverSockfd);
    close(clientSockfd);

    return 0;
}
