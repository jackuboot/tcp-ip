#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<string.h>
#include<netinet/in.h>

typedef union epoll_data
{
	void *ptr;
	int  fd;
	__uint32_t u32;
	__uint4_t  u64;

}epoll_data_t;

struct epoll_event
{
	__uint32_t events;
	epoll_data_t data;
}

int main()
{	

	//create socket
	int socketfd;
	socketfd = socket(AF_INET,SOCK_STREAM,0);
	
	//success check
	if(socketfd == -1)
	{
		printf("this socket error");
		exit(1);	
	};

	//blind
	struct sockaddr_in my_addr;
	my_addr.sin_family = AF_INET;//2字节
	my_addr.sin_port   = htons(6666);//固定了要用网络字节顺序，大端模式  高位在低地址//2字节
	my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");//4字节
	bzero(&(my_addr.sin_zero),8);//8字节
	printf("the size of my_addr %d",sizeof(my_addr));
	int bind_err
	bind_err = bind(socketfd,(struct sockaddr*)&my_addr, sizeof(struct sockaddr));
	if (bind_err == -1)
	{
		printf("bind faild");
		close(socketfd);
		exit(1);
	};
	

	//设置socket套接字 模式      阻塞  和  非阻塞
	int tfl;
	tfl = fcntl(socketfd, F_GETFL, 0);
	if (tfl == -1)
	{
		printf("tfcntl error");
		close(socketfd);
		exit(1);
	}
	
	//设置状态
	tfl |= O_NONBLOCK;
	int new_tfl;
	new_tfl = ftctl(socketfd, F_SETFL, tfl);
	if (new_tfl == -1)
	{
		print("tfcntl set err");
		close(socketfd);
		exit(1);
	}

	//listen
	int listen_err 
	listen_err= listen(socketfd,5);
	if (listen_err == -1)
	{
		printf("listen faild");
		close(socketfd);
		exit(1);
	};
	
	//循环阻塞，accept client发过来的连接
	/*
	while(1)
	{
		//accpet
		//int accept(int sock, struct sockaddr *addr, socklen_t *addrlen);  //Linux
		int new_client_fd = accept(socketfd, my_addr, sizeof(my_addr));
		if (new_client_fd == -1)
		{
			printf("accpet faild");
			close(new_client_fd);
			close(socketfd);
			exit(1);
		};
	
		//close
		close(new_client_fd);
	}
	*/
	
	//采用epoll模式 处理socket事件
	//epoll实例创建 参数为0表示  无fd个数限制
	int efd 
	efd = epoll_create1(0);
	if (efd == -1)
	{
		printf("epoll_create1 failed");
		close(socketfd);
		exit(1);
	}

	//设置触发模式
	struct epoll_event event;
	struct epoll_event *events;
	event.data.fd = sfd;
	event.events = EPOLLIN | EPOLLET;
	int epoll_ctl_err
	epoll_ctl_err = epoll_ctl(efd, EPOLL_CTL_ADD, socketfd, &event);		
	if(epoll_ctl_err == -1)
	{
		printf("epoll_ctl_err --1");
		close(socketfd);
		exit(1);
	}

	//事件循环
	events = calloc(MAXEVENTS, sizeof event);
	while(1)
	{
		int n, i;
		n = epoll_wait(efd, events, MAXEVENTS, -1);
		for(i=0;i<n;i++)
		{
			if((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) ||(!(EVENTS[i].events & EPOLLIN)))
			{
				printf("event loop error!");
				close(events[i].data.fd);
				continue;
			};


			elseif(socketfd == events[i].data.fd)
			{
				while(1)
				{
					struct sockaddr sock_addr;
					socklen_t sock_len;
					int sock_fd;
					char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
					in_len = sizeof in_addr;
					sock_fd = accept(socketfd, &socket_addr, &socket_len);
					if(sock_fd == -1)
					{
						if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
						{
							break;
						}
						else
						{
							printf("error when accept");
							break;
						}	
					}
					
					s = getnameinfo(&socket_addr, socket_len, hbuf, sizeof hbuf,sbuf, sizeof sbuf, NI_NUMERICHOST|NI_NUMBERICSERV);
					if (s == 0)
					{
						printf("ACCPETED CONNECTION ON DESCROPTOR %d""(host=%s,port=%s)\n",socketfd, hbuf,subf);
					}
	
					
				
				}
			}
		}
	}

	close(socketfd);
}

	
