/*#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
*/
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
#include<sys/epoll.h>
#include<fcntl.h>


#define Port 6666
#define Maxsize 1024

void recvdata(int fd, int events, void *arg);
void senddata(int fd,int events, void *arg);




struct myevent_s{
	int fd;
	int events;
	void *arg;
	void(*call_back)(int fd,int events,void *arg);
	int status;
	char buf[Maxsize];
	int len;
	long last_active;
};

struct myevent_s g_events[Maxsize+1];
int g_efd;


void eventdel(int efd,struct myevent_s *ev){
	struct epoll_event epv = {0,{0}};
	if (ev->status != 1){
		return;
	}
	epv.data.ptr = ev;
	ev->status = 0;
	epoll_ctl(efd,EPOLL_CTL_DEL,ev->fd,&epv);
	return;

}



void eventset(struct myevent_s *ev,int fd, void(*call_back)(int,int,void *),void *arg){
	struct myevent_s *events = (struct myevent_s *)arg;
	ev->fd = fd;
	ev->call_back = call_back;
	ev->events = 0;
	ev->arg = arg;
	ev->status = 0;
	if (ev->len <= 0){
		memset(ev->buf,0,sizeof(ev->buf));
		ev->len = 0;
	}
	ev->last_active = time(NULL);
	return ;
}

void eventadd(int efd,int events,struct myevent_s *ev){
	struct epoll_event epv = {0,{0}};
	int op;
	epv.data.ptr = ev;

	epv.events = ev->events = events;

	if(ev->status == 1){
		op=EPOLL_CTL_MOD;
	} else {
		op=EPOLL_CTL_ADD;
		ev->status = 1;
	}
	if(epoll_ctl(efd,op,ev->fd,&epv) < 0){
		printf("failed");
	} else {
		printf("OK,[fd=%d],op = %d,events[%0X]\n",ev->fd,op,events);
	}
	return;
}

void acception(int lfd,int events,void *arg){
	struct sockaddr_in cin;
	socklen_t len = sizeof(cin);
	int cfd,i;
	if((cfd = accept(lfd,(struct sockaddr *)&cin,&len)) == -1){
		printf("accpet error");
		return;
	}
	do{
		for(i = 0; i < Maxsize;++i){
			if(g_events[i].status == 0){
				break;
			}
		}
		if(i == Maxsize){
			printf("client too many");
			break;
		}
		int flag;
		if((flag = fcntl(cfd,F_SETFL,O_NONBLOCK))){
			printf("%s:fcntl nonblocking failed, %s\n",__func__,strerror(errno));
			break;
		}
	
	}while(0);
	printf("new connect [%s:%d] [time:%ld],[pos:%d]",inet_ntoa(cin.sin_addr),ntohs(cin.sin_port),g_events[i].last_active,i);
	
	eventset(&g_events[i],cfd,recvdata,&g_events[i]);
	eventadd(g_efd,EPOLLIN,&g_events[i]);
	return;

}

				
void recvdata(int cfd,int events,void * arg){
	struct myevent_s *ev = (struct myevent_s *)arg;
	int len;
	len = recv(cfd,ev->buf,sizeof(ev->buf),0);
	eventdel(g_efd,ev);
	if(len > 0){
		ev->len = len;
		for(int i = 0; i < len; ++i){
			ev->buf[i] = toupper(ev->buf[i]);
		}
		printf("%d\n",ev->status);			
		eventset(ev,cfd,senddata,ev);
		eventadd(g_efd,EPOLLOUT,ev);
	}
	else if (len == 0){
		close(ev->fd);
		printf("[fd=%d] pos[%ld]:close\n",cfd,ev-g_events);
	} else {
		close(ev->fd);
		printf("recv[fd=%d] error[%d]:%s\n",cfd,errno,strerror(errno));
	}
	return;

}

void senddata(int cfd,int events,void *arg){
	struct myevent_s *ev = (struct myevent_s *)arg;
	int len;
	len = send(cfd,ev->buf,ev->len,0);
	if(len > 0){
		printf("send[fd=%d],[%d]%s\n",cfd,len,ev->buf);
		eventdel(g_efd,ev);
		eventset(ev,cfd,recvdata,ev);
		eventadd(g_efd,EPOLLIN,ev);

	} else {
		close(ev->fd);
		eventdel(g_efd,ev);
		printf("send[fd=%d] error %s\n",cfd,strerror(errno));
	}
	return;
}





int main(int argc,char *argv[])
{
	//isignal(SIGPIPE, SIG_IGN);
	int listenfd;
	struct sockaddr_in servaddr;
	int i,nready,n;
	char buf[Maxsize],clie_IP[Maxsize];
	struct epoll_event events[Maxsize+1];
	int checkpos = 0;

	g_efd = epoll_create(Maxsize+1);
	if(g_efd <= 0){
		printf("creat efd in %s err %s\n",__func__,strerror(errno));

	}
	fcntl(listenfd,F_SETFL,O_NONBLOCK);

	if( (listenfd = socket(AF_INET,SOCK_STREAM,0)) == -1){
		std::cout << "error1:" << errno << std::endl;
	}
	
	eventset(&g_events[Maxsize],listenfd,acception,&g_events[Maxsize]);
	eventadd(g_efd,EPOLLIN,&g_events[Maxsize]);
	
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
	
	
	

	std::cout << "---------waiting client----------" << std::endl;
	while(1){

		long now = time(NULL);
		for(i = 0; i < 100; ++i,checkpos++){
			if(checkpos == Maxsize){
				checkpos = 0;
			}
			if(g_events[checkpos].status != 1){
				continue;
			}

			long duration = now-g_events[checkpos].last_active;

			if(duration >= 60){
				close(g_events[checkpos].fd);
				printf("[fd=%d] timeout\n",g_events[checkpos].fd);
				eventdel(g_efd,&g_events[checkpos]);
			}
		}

		nready = epoll_wait(g_efd, events,20,-1);
		if (nready < 0){
			perror("select error");
		}
		if (nready == 0){
			continue;
		}
		for (i = 0; i < nready; ++i){
			struct myevent_s *ev = (struct myevent_s *)events[i].data.ptr;
		
		if((events[i].events & EPOLLIN)&&(ev->events & EPOLLIN)){
			ev->call_back(ev->fd,events[i].events,ev->arg);
		}
		
		if((events[i].events & EPOLLOUT)&&(ev->events & EPOLLOUT)){
			ev->call_back(ev->fd,events[i].events,ev->arg);
		}

		}

	}
	close(listenfd);
	return 0;
}
