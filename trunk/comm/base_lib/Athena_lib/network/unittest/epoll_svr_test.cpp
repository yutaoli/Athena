#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

using namespace std;

#define MAXLINE 50
#define OPEN_MAX 100
#define LISTENQ 20
#define INFTIM 1000

/*
quetion:
1.为什么epoll的EPOLLET要和nonblock配套用?
2.发送数据后没有把对应sockfd EPOLL_CTL_DEL && close掉为什么会使程序终止?
3.accept的第三个参数为啥一定要初始化? 即：clilen = sizeof(struct sockaddr);
connfd = accept(listenfd,(sockaddr *)&clientaddr, &clilen);
4.要把int数据发送到网络为什么一定要htonl()

ET模式和LT模式的区别：
ET模式仅当状态发生变化的时候才获得通知,这里所谓的状态的变化并不包括缓冲区中还有未处理的数据,也就是说,如果要采用ET模式,需要一直read/write直到出错为止,很多人反映为什么采用ET模式只接收了一部分数据就再也得不到通知了,大多因为这样;而 LT 模式是只要有数据没有处理就会一直通知下去的.
所以要ET模式得到正确的数据必须一次性读完，否则数据就读不全，因为下次内核不会通知你。
 */
void setnonblocking(int sock)
{
	int opts;
	opts=fcntl(sock,F_GETFL);
	if(opts<0)
	{
		perror("fcntl(sock,GETFL)");
		exit(1);
	}
	opts = opts|O_NONBLOCK;
	if(fcntl(sock,F_SETFL,opts)<0)
	{
		perror("fcntl(sock,SETFL,opts)");
		exit(1);
	}   
}

int main(int argc ,char *argv[])
{
	if(argc !=3)
	{
		printf("%s bind_ip bind_port\n", argv[0]);
		return 0;
	}
	int i, maxi, listenfd, connfd, sockfd,epfd,nfds;
	ssize_t n;
	char line[MAXLINE];
	socklen_t clilen;
	//声明epoll_event结构体的变量,ev用于注册事件,数组用于回传要处理的事件
	struct epoll_event ev,events[20];
	//生成用于处理accept的epoll专用的文件描述符
	epfd=epoll_create(256);
	struct sockaddr_in clientaddr;
	struct sockaddr_in serveraddr;
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	//把socket设置为非阻塞方式
	setnonblocking(listenfd);
	//设置与要处理的事件相关的文件描述符
	ev.data.fd=listenfd;
	//设置要处理的事件类型
	ev.events=EPOLLIN|EPOLLET;
	//ev.events=EPOLLIN;
	//注册epoll事件
	epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	//char *local_addr="127.0.0.1";
	char *local_addr=argv[1];
	inet_aton(local_addr,&(serveraddr.sin_addr));//htons(SERV_PORT);
	serveraddr.sin_port=htons(atoi(argv[2]));
	bind(listenfd,(sockaddr *)&serveraddr, sizeof(serveraddr));
	listen(listenfd, LISTENQ);
	maxi = 0;
	for ( ; ; ) {
		//等待epoll事件的发生
		nfds=epoll_wait(epfd,events,20,500);
		cout << nfds << endl;
		//处理所发生的所有事件     
		for(i=0;i<nfds;++i)
		{
			if(events[i].data.fd==listenfd)
			{
				clilen = sizeof(struct sockaddr);
				connfd = accept(listenfd,(sockaddr *)&clientaddr, &clilen);
				if(connfd<0){
					perror("connfd<0");
					exit(1);
				}
				setnonblocking(connfd);
				char *str = inet_ntoa(clientaddr.sin_addr);
				cout << "accapt a connection from " << str << endl;
				//设置用于读操作的文件描述符
				ev.data.fd=connfd;
				//设置用于注测的读操作事件
				ev.events=EPOLLIN|EPOLLET;
				//ev.events=EPOLLIN;
				//注册ev
				epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev);
			}
			else if(events[i].events&EPOLLIN)
			{
				cout << "EPOLLIN" << endl;
				if ( (sockfd = events[i].data.fd) < 0) 
					continue;
				if ( (n = read(sockfd, line, MAXLINE)) < 0) {
					if (errno == ECONNRESET) {
						close(sockfd);
						events[i].data.fd = -1;
					} else
						std::cout<<"readline error"<<std::endl;
				} else if (n == 0) {
					close(sockfd);
					events[i].data.fd = -1;
				}
				line[n] = '\0';
				cout << "read " << line << endl;
				//设置用于写操作的文件描述符
				ev.data.fd=sockfd;
				//设置用于注测的写操作事件
				ev.events=EPOLLOUT|EPOLLET;
				//修改sockfd上要处理的事件为EPOLLOUT
				epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
			}
			else if(events[i].events&EPOLLOUT)
			{   
				sockfd = events[i].data.fd;

				int len = htonl(strlen(line) + 4);
				cout << "strlen(line) = " << len << endl;
				cout << "line = " << line << endl; 
				char *ptr = (char*)&len;
				write(sockfd, ptr, 4);
				write(sockfd, line, n);
				//设置用于读操作的文件描述符
				ev.data.fd=sockfd;
				//设置用于注测的读操作事件
				ev.events=EPOLLIN|EPOLLET;
				//修改sockfd上要处理的事件为EPOLIN
				epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,&ev);
				close(sockfd);
			}
		}
	}
	return 0;
}
