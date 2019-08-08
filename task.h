#ifndef _TASK_H_
#define _TASK_H_

#include <sstream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/wait.h>
#include <errno.h>

#include "threadpool.h"


const int BUFFER_SIZE = 1024;

class CTask
{
private:
	int accept_fd;
	int epfd;
	
public:
	CTask(){}
	CTask(int fd,int epoll_fd):accept_fd(fd),epfd(epoll_fd){}
	~CTask();
	
	void doit();//执行任务
	
	void removefd(int epfd,int fd);
	
	//发送文件
	int send_file(const string &filename,const char *type,int start,
					const int num=200,const char *info = "OK");
					
	int deal_get(const string &uri, int start=0); //处理GET请求
	int deal_post(const string &uri,char *buf);   //处理POST请求
	int get_size(const string &filename);         //获取文件大小
};


#endif