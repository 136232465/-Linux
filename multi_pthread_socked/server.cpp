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
#include <pthread.h>

#define Port 6666
#define Maxsize 1024



struct s_info {
  struct sockaddr_in clientaddr;
  int connectfd;
};


void *do_work(void *arg){

		int n,i;
		struct s_info* ts = (struct s_info*)arg;
		char buf[Maxsize],clie_IP[Maxsize];
		pthread_t tid = pthread_self();
		
		printf("my pid: %u\n",(unsigned int)tid);
	    //   	printf("IP : %s\t; port : %s\n",)	
		printf("IP : %s\t; port : %d\n", inet_ntop(AF_INET,&ts->clientaddr.sin_addr.s_addr,clie_IP,sizeof(clie_IP)), ntohs(ts->clientaddr.sin_port));
		printf("connected\n");
	//	std::cout << "my pid" << (unsigned int)tid << std::endl;
	//	std::cout << "IP "<< inet_ntop(AF_INET,&ts->clientaddr.sin_addr.s_addr,clie_IP,sizeof(clie_IP)) << "; port " << ntohs(ts->clientaddr.sin_port)  << std::endl;
	//	std::cout << "connect" << std::endl;
		while(1){
			memset(buf, 0,Maxsize);	
			n = read(ts->connectfd,buf,sizeof(buf));
			if (n == 0){
				printf("clie_IP : %s\t; port: %d is closed\n",clie_IP,ntohs(ts->clientaddr.sin_port));
			//	std::cout << clie_IP <<" " << ntohs(ts->clientaddr.sin_port)<< "  closed" << std::endl;
				break;
			}
			else if(n == -1){
				perror("read error");
				break;
			}
			else {
				for (int i = 0; i < n; ++i){
					buf[i] = toupper(buf[i]);
				}
				write(ts->connectfd,buf,n);
				std::cout << buf;
				}
			}
		close(ts->connectfd);
		return (void *)0;

}

int main(int argc,char *argv[])
{
	pthread_t pid;
	int listenfd,connectfd;
	struct sockaddr_in servaddr, clientaddr;
	socklen_t client_len;
	int i;
	struct s_info ts[Maxsize];

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
	
	if(listen(listenfd,120) == -1){
		std::cout << "listen erron:" << errno << std::endl;
		exit(0);
	}

	std::cout << "---------waiting client----------" << std::endl;
	while(1){	
		if ((connectfd = accept(listenfd,(struct sockaddr*)&clientaddr,&client_len)) == -1){
			std::cout << "accept errno" << errno << std::endl;
		}
		ts[i].clientaddr = clientaddr;
		ts[i].connectfd = connectfd;

		pthread_create(&pid,NULL,do_work,(void*)&ts[i]);
		pthread_detach(pid);
		i++;
	}
	close(listenfd);
	return 0;
}
