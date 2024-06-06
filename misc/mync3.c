#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int createTCPClient()
{
	int sockfd;
	struct sockaddr_in theiraddr;
	socklen_t socklen=sizeof(theiraddr);
	sockfd = socket (AF_INET, SOCK_STREAM, 0);
	theiraddr.sin_family = AF_INET;
	theiraddr.sin_port = htons (12345);
	theiraddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	int ret =connect(sockfd, (struct sockaddr *) &theiraddr, socklen);
	return sockfd; 
} 

int createTCPClient2()
{
	int sockfd;
	struct sockaddr_in theiraddr;
	socklen_t socklen=sizeof(theiraddr);
	sockfd = socket (AF_INET, SOCK_STREAM, 0);
	theiraddr.sin_family = AF_INET;
	theiraddr.sin_port = htons (12346);
	theiraddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	int ret =connect(sockfd, (struct sockaddr *) &theiraddr, socklen);
	return sockfd; 
} 



int main()
{
	int numbytes=	101;
	close(0); // close stdin
	int sockfd=createTCPClient();
	dup(sockfd);
	close(1); // close stdout
	sockfd=createTCPClient2();
	dup(sockfd);
	char buf[numbytes];
	for(;;) {
		numbytes=read(0,buf, 100);
		buf[numbytes++]=0;
		write(1, buf, numbytes); 
	} 
}
