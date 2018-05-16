#include <iostream>
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<stdlib.h>
#include<errno.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<string.h>
#include<signal.h>
#include<sys/wait.h>
#include<poll.h>


#define Port 6666
#define Maxsize 1024




				

int main(int argc,char *argv[])
{
	int listenfd,connectfd, sockfd,maxfd,maxi;
	struct sockaddr_in servaddr, clientaddr;
	socklen_t client_len;
	int i,nready,n;
	client_len = sizeof(clientaddr);
	//fd_set allset,rset;
	char buf[Maxsize],clie_IP[Maxsize];
	struct pollfd client[Maxsize];


	if( (listenfd = socket(AF_INET,SOCK_STREAM,0)) == -1){
		std::cout << "error1:" << errno << std::endl;
	}
	
	
	memset(&servaddr,0,sizeof(servaddr));//
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(Port);
	
	int opt = 1;

	if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt)) < 0){
		perror("setsockopt error");	
	}
	
	if(bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) == -1){
		std::cout << "bind erron:" << errno << std::endl;
		exit(0);
	}
	
	if(listen(listenfd,10) == -1){
		std::cout << "listen erron:" << errno << std::endl;
		exit(0);
	}
	for (i = 0; i < FD_SETSIZE; ++i){
		client[i].fd = -1;
	}

	client[0].fd = listenfd;
	client[0].events = POLLIN;
	maxi = 0;	
	

	std::cout << "---------waiting client----------" << std::endl;
	while(1){
		nready = poll(client, maxi+1, -1);
		if (client[0].revents && POLLIN ){
			if ((connectfd = accept(listenfd,(struct sockaddr*)&clientaddr,&client_len)) == -1){
				std::cout << "accept errno" << errno << std::endl;
			}
			printf("IP : %s\t; port : %d\n", inet_ntop(AF_INET,&clientaddr.sin_addr.s_addr,clie_IP,sizeof(clie_IP)), ntohs(clientaddr.sin_port));
			for (i = 0; i < Maxsize; ++i){
				if(client[i].fd < 0){
					client[i].fd = connectfd;
					break;
				}
			}
			if (i == Maxsize){
				perror("too many client");
				exit(1);
			}
			client[i].events = POLLIN;
			if(i > maxi){
				maxi = i;
			}
			if(--nready==0){
				continue;
			}
		}
		for(i = 0; i <= maxi; ++i){
			if((sockfd = client[i].fd) < 0){
				continue;
			}
			if(client[i].revents && POLLIN){
				if((n = read(sockfd,buf,sizeof(buf)))==0){
					close(sockfd);
					client[i].fd = -1;
				} else if(n < 0){
					perror("read error");
				} else {
					for (int j = 0; j < n; ++j){
						buf[j] = toupper(buf[j]);
					}
					sleep(5);
					write(sockfd,buf,n);
				}
				if(--nready == 0){
					break;
				}

			}	
		}
	}
	close(listenfd);
	return 0;
}
