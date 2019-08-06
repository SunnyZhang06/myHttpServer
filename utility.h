#ifndef _UTILITY_H_
#define _UTILITY_H_

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

//将fd设置称非阻塞
void setnonblocking(int fd)
{
	int old_option = fcntl(fd,F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd,F_SETFL,new_option);
}




#endif

