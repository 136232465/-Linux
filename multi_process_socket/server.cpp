#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>


#define Port 6666
#define Maxsize 1024



void wait_child(int signo){
	while(waitpid(0,NULL,WNOHANG) > 0);
	return;
}


int main(int argc,char *argv[])
{
	pid_t pid;
	int listenfd,connectfd;
	struct sockaddr_in servaddr, clientaddr;
	socklen_t client_len;
	char buf[Maxsize],clie_IP[Maxsize];
	int n;

	client_len = sizeof(clientaddr);

	if( (listenfd = socket(AF_INET,SOCK_STREAM,0)) == -1){
		std::cout << "error1:" << errno << std::endl;
	}
	
	
	memset(&servaddr,0,sizeof(servaddr));//
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(Port);
	
	
	if(bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) == -1){
		std::cout << "bind erron:" << errno << std::endl;
		exit(0);
	}
	
	if(listen(listenfd,10) == -1){
		std::cout << "listen erron:" << errno << std::endl;
		exit(0);
	}

	std::cout << "---------waiting client----------" << std::endl;
	while(1){	
		if ((connectfd = accept(listenfd,(struct sockaddr*)&clientaddr,&client_len)) == -1){
				std::cout << "accept errno" << errno << std::endl;
			}
		pid = fork();
		if(pid < 0){
			perror("fork error");
			exit(1);
		} else if(pid == 0){
			std::cout << "IP "<< inet_ntop(AF_INET,&clientaddr.sin_addr.s_addr,clie_IP,sizeof(clie_IP)) << "; port " << ntohs(clientaddr.sin_port)  << std::endl;
			close(listenfd);
			std::cout << "connect" << std::endl;
			while(1){
				memset(buf, 0,Maxsize);	
				n = read(connectfd,buf,sizeof(buf));
				if (n == 0){
					close(connectfd);
					std::cout << clie_IP <<" " << ntohs(clientaddr.sin_port)<< "  closed" << std::endl;
					return 0;
				}
				else if(n == -1){
					perror("read error");
					exit(1);
				}
				else {
					for (int i = 0; i < n; ++i){
						buf[i] = toupper(buf[i]);
					}
					write(connectfd,buf,n);
					//write(STDOUT_FILNO,buf,n);
					std::cout << buf;
				}
			}
		} else {
			close(connectfd);
			signal(SIGCHLD,wait_child);
		}
	}
}
