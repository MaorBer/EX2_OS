// exercise.h

#ifndef EXERCISE_H
#define EXERCISE_H

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

// Global variables
extern char *socket_path;
extern int udp_socket;
extern volatile sig_atomic_t time_up;

// Function declarationsW
void uds_client_stream(char *socket_path, char **args);
void uds_server_datagram(char *socket_path, char **args);
void uds_server_stream(char *socket_path, char **args);
void print_usage(const char *prog_name);
int s_socket_tcp(int port);
void handle_tcp_server(int port, int port2, char opt, char opt2, char **args);
void signal_handler(int signum);
void parse_string(const char *str, char *ip, int *port);
void handle_udp_client(int port, char *host, char **args, int seconds, char opt);
void handle_tcp_client(const char *host, const char *host2, int port, int port2, char opt, char opt2, char **args);
void handle_udp_server(int port, int seconds, char **args, char opt);
void uds_client_datagram(char *socket_path, char **args);

#endif // EXERCISE_H
