#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define Port 6666
#define IP "127.0.0.1"
#define Maxsize 1024

int main(){
	int  cfd;
	struct sockaddr_in serv_addr;
	socklen_t serv_len;
	char buf[Maxsize];
	int n;

	if((cfd = socket(AF_INET,SOCK_STREAM,0)) == -1){
		std::cout << "errno:" << errno << std::endl;
	}


	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(Port);
	//serv_addr.sin_addr.s_addr = htonl()
	inet_pton(AF_INET,IP,&serv_addr.sin_addr.s_addr);


	if (connect(cfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr)) == -1){
	std::cout << "errno :" << errno << std::endl;
	}
	while (1){
		fgets(buf,sizeof(buf),stdin);
		write(cfd,buf,strlen(buf));
		n = read(cfd, buf, sizeof(buf));
		write(STDOUT_FILENO,buf,n);
	}

	close(cfd);

	
	return 0;
}
