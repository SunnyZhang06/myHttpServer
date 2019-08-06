#ifndef _EVENT_H_
#define _EVENT_H_

#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

using namespace std;

#define MAX_EVENT 1024  //epoll_events的最大个数
#define MAX_BUFFER 2048 //buffer的最大字节
#define EPOLL_CREATE 1024

enum EventType
{
	EIN = EPOLLIN,		  // 读事件
	EOUT = EPOLLOUT,	  // 写事件
	ECLOSE = EPOLLRDHUP,  // 对端关闭连接或者写半部
	EPRI = EPOLLPRI,	  // 紧急数据到达
	EERR = EPOLLERR,	  // 错误事件
	EET = EPOLLET, 		  // 边缘触发
	EDEFULT = EIN | ECLOSE | EERR | EET
};

class CEvent
{
public:
	CEvent(int m_port):port(m_port){}
	~CEvent();
	void init();                  //服务器初始化(建立套接字、绑定、监听)
	void add_fd(int epfd,int fd);
	void handle_accept(int epfd,int listenfd);
	void handle_request(int fd); //处理请求
	void do_epoll();             //epoll监听  
	
private:
	int epfd;       //Epoll的fd
	int listenfd;  //监听的fd
	int port;      //端口
	struct epoll_event events[MAX_EVENT];	
}



#endif

