#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

int main()
{
	int numbytes=100;
	char buf[numbytes];
	int newfile=open("output.txt", O_RDWR | O_CREAT | O_TRUNC);
	printf("newfile=%d\n",newfile);	
	close(1);
	dup(newfile);
	for(;;) {
		numbytes=read(0,buf, 100);
		buf[numbytes++]=0;
		write(1, buf, numbytes); 
	} 
}
