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
#include<sys/epoll.h>
#include<sys/wait.h>
#include <fcntl.h>


#define Port 6666
#define Maxsize 1024



				

int main(int argc,char *argv[])
{
	signal(SIGPIPE, SIG_IGN);
	int listenfd,connectfd, sockfd,maxfd,maxi,epfd;
	struct sockaddr_in servaddr, clientaddr;
	socklen_t client_len;
	int i,nready,n,res;
	client_len = sizeof(clientaddr);
	char buf[Maxsize],clie_IP[Maxsize];
	struct epoll_event ev, events[20];
	

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

	epfd = epoll_create(20);
	ev.events = EPOLLIN;
	ev.data.fd = listenfd;	
	res = epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);
	if (res == -1){
		perror("epoll_creat error");
		exit(1);
	}

	std::cout << "---------waiting client----------" << std::endl;
	while(1){
		
		nready = epoll_wait(epfd,events,20,-1);
		if (nready < 0){
			perror("select error");
			exit(1);
		}
		if (nready == 0){
			continue;
		}
		for(i = 0; i < nready; ++i){
			if(!events[i].events & EPOLLIN){
				continue;
			}
			if (events[i].data.fd == listenfd){
			if ((connectfd = accept(listenfd,(struct sockaddr*)&clientaddr,&client_len)) == -1){
				std::cout << "accept errno" << errno << std::endl;
			}
		printf("IP : %s\t; port : %d\n", inet_ntop(AF_INET,&clientaddr.sin_addr.s_addr,clie_IP,sizeof(clie_IP)), ntohs(clientaddr.sin_port));
			
		fcntl(connectfd,F_SETFL, O_NONBLOCK);	
		
			ev.events = EPOLLIN | EPOLLET;
			ev.data.fd = connectfd;
			res = epoll_ctl(epfd,EPOLL_CTL_ADD,connectfd,&ev);
			if(res == -1){
				perror("epoll_creat error");
				exit(1);
			}
		} else {		
			if((sockfd = events[i].data.fd) < 0){
				continue;
			}
				while((n = read(sockfd,buf,2)) > 0 ){
					for (int j = 0; j < n; ++j){
						buf[j] = toupper(buf[j]);
					}
					write(sockfd,buf,n);
				}
				//sleep(10);
				//res = epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,NULL);
				//close(sockfd);
				if(--nready == 0){
					break;
				}

				}	
		}
	
	}
		
	close(listenfd);
	return 0;
}
