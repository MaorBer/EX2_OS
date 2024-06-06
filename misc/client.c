#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char * argv[])
{
	int sockfd;
	struct sockaddr_in theiraddr;
	socklen_t socklen=sizeof(theiraddr);
	sockfd = socket (AF_INET, SOCK_STREAM, 0);
	theiraddr.sin_family = AF_INET;
	theiraddr.sin_port = htons (12345);
	theiraddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	int ret =	connect(sockfd, (struct sockaddr *) &theiraddr, socklen);
	printf("Ret = %d\n", ret);
	if (ret == -1) perror("connect"); 
	close(1);
	dup(sockfd);
	printf("hello world\n");  
	exit(0);
} 

