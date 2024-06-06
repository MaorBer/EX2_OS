#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

int main(int argc, char * argv[])
{
	struct sockaddr_in myaddr;
	struct sockaddr_in theiraddr;
	socklen_t addrlen;
	int buflen = 100;
	char buf[buflen];
	int numbytes; 
	int ret;
	int sockfd=socket(AF_INET, SOCK_STREAM, 0);
	myaddr.sin_family=AF_INET;
	myaddr.sin_port=htons(12345);
	myaddr.sin_addr.s_addr=INADDR_ANY;
	printf ("socket(2) Sockfd = %d\n", sockfd); 
	ret = bind(sockfd, (struct sockaddr*)&myaddr, sizeof (myaddr));
	printf ("bind(2) ret = %d\n", ret);
	if (ret != 0) perror ("Bind");	
	ret = listen (sockfd,5);
	printf ("lisetn(2) ret = %d\n", ret);
	if (ret != 0) perror ("listen");	
	int newfd=accept (sockfd,(struct sockaddr *) &theiraddr, &addrlen);
	if (newfd > 0) {
		printf ("new fd = %d\n", newfd);
		numbytes = recv(newfd, buf, buflen-1,0);
		printf ("%d\n", numbytes);
		if (numbytes == 0) perror ("recv:");
		buf[numbytes]=0;
		printf("received :%s\n", buf);
		exit(0);
	} else perror ("Accept:");
}
