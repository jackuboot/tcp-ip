#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<string.h>
#include<netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>

#define MAXEVENTS 32

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
	int bind_err;
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
	};

	//设置状态
	tfl |= O_NONBLOCK;
	int new_tfl;
	new_tfl = fcntl(socketfd, F_SETFL, tfl);
	if (new_tfl == -1)
	{
		printf("tfcntl set err");
		close(socketfd);
		exit(1);
	};

	//listen
	int listen_err;
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
	int efd;
	efd = epoll_create1(0);
	if (efd == -1)
	{
		printf("epoll_create1 failed");
		close(socketfd);
		exit(1);
	};

	//设置触发模式
	struct epoll_event event;
	struct epoll_event *events;
	event.data.fd = socketfd;
	event.events = EPOLLIN | EPOLLET;
	int epoll_ctl_err;
	epoll_ctl_err = epoll_ctl(efd, EPOLL_CTL_ADD, socketfd, &event);
	if(epoll_ctl_err == -1)
	{
		printf("epoll_ctl_err --1");
		close(socketfd);
		exit(1);
	};

	//事件循环
	events = calloc(MAXEVENTS, sizeof event);

	while(1)
	{
		int n, i;
		n = epoll_wait(efd, events, MAXEVENTS, -1);
		for(i=0;i<n;i++)
		{
			if((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) ||(!(events[i].events & EPOLLIN)))
			{
				printf("event loop error!");
				close(events[i].data.fd);
				continue;
			}


			else if(socketfd == events[i].data.fd)
			{
				while(1)
				{
					struct sockaddr socket_addr;
					socklen_t socket_len;
					int cli_fd;
					char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
					socket_len  = sizeof socket_addr;
					
					cli_fd = accept(socketfd, &socket_addr, &socket_len);
					if(cli_fd == -1)
					{
						if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
						{
							break;
						}
						else
						{
							printf("error when accept");
							break;
						};
					};

                    			int nameinfo_error;
					nameinfo_error = getnameinfo(&socket_addr, socket_len, hbuf, sizeof hbuf,sbuf, sizeof sbuf, NI_NUMERICHOST|NI_NUMERICSERV);

					if (nameinfo_error == 0)
					{
						printf("ACCPETED CONNECTION ON DESCROPTOR %d""(host=%s,port=%s)\n",socketfd, hbuf,sbuf);
					};

                    //设置socket套接字 模式      阻塞  和  非阻塞
                    int tfl;
                    tfl = fcntl(cli_fd, F_GETFL, 0);
                    if (tfl == -1)
                    {
                        printf("while accpet tfcntl error");
                        close(cli_fd);
                        exit(1);
                    }
				}
                continue;
			}

            else
            {
                int done = 0;
                while(1)
                {
                    ssize_t count;
                    char buf[512];

                    //从连接文件描述符中读取内容
                    count = read(events[i].data.fd, buf, sizeof buf);
                    if (count == -1)
                    {
                        if (errno != EAGAIN)
                        {
                          printf ("read error");
                          done = 1;
                        }
                      break;
                    }
                    else if(count == 0)
                    {
                        done = 1;
                        break;
                    };

                    int write_err;
                    write_err = write (1, buf, count);
                    if (write_err == -1)
                    {
                      printf ("write error");
                      close(socketfd);
                      exit(1);
                    };

                    if (done)
                    {
                      printf ("Closed connection on descriptor %d\n",
                              events[i].data.fd);
                      close (events[i].data.fd);
                    }
                }
            }
		}
	}

    free (events);

	close(socketfd);

    return EXIT_SUCCESS;
}


