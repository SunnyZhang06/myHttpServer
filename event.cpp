#include "utility.h"
#include "event.h"



void CEvent::init()
{
	int listenfd = socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in serv_addr;
	bzero(&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	
	int option = 1;
	setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(void*)&option,sizeof(option));
	int ret = bind(listenfd,(struct sockaddr*)& serv_addr,sizeof(serv_addr));
	if(ret==-1)
	{
		perror("bind error");
		exit(1);
	}
	ret = listen(listenfd,LISTEN_MAX);
	if(ret == -1)
	{
		perror("listen error");
		exit(1);
	}	
	//return listenfd;
}


void CEvent::add_fd(int epfd,int fd)
{
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = fd;
	epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&ev);
	setnonblocking(fd);
}

void CEvent::handle_accept(int epfd,int listenfd)
{
	int client_fd;
	struct sockaddr_in client_addr;
	socklen_t cliaddrlen;
	client_fd = accept(listenfd,(struct sockaddr*)&client_addr,sizeof(cliaddrlen));
	if(client_fd<0)
	{
		perror("accept error");
		exit(1);
	}
	else
	{
		printf("accept a new client: %s:%d\n",inet_ntoa(client_addr.sin_addr),client_addr.sin_port);
	}
	add_fd(epfd,client_fd);
}



void CEvent::handle_request(int fd)
{
	char buffer[1024];
	int nread = read(fd,buffer,sizeof(buffer));
	printf("读到的请求是%s\n",buffer);
	
	char filename[10]={0};
	sscanf(buffer,"GET /%s",filename);
	printf("解析的文件名是：%s\n",filename);
	
	char* mine = NULL;         //解析的类型
	if(strstr(filename,".HTML"))
		mine = "text/html";
	if(strstr(filename,"."))
		mine = "image/jpg";
	
	//响应报文段
	char response[1024*1024];
	sprintf(response,"HTTP/1.1 200 OK\r\nContext-Type: %s\r\n\r\n",mine);
	int headlen = strlen(response);
	
	//打开客户请求的文件并写进响应报文中
	int filefd = open(filename,O_RDONLY);
	int filelen = read(filefd,response+headlen,sizeof(response)-headlen);
	
	//把响应报文段发送给客户端
	write(fd,response,headlen+filelen);
	close(filefd);
}

void CEvent::do_epoll()
{
	struct epoll_event ev[1000];
	epfd = epoll_create(EPOLL_CREATE);
	add_fd(epfd,listenfd);
	while(1)
	{
		int nready = epoll_wait(epfd,ev,MAX_EVENT,-1);
		for(int i=0;i<nready;++i)
		{
			int fd = ev[i].data.fd;
			if((fd  == listenfd) && (ev[i].events & EPOLLIN))
			{
				handle_accept(epfd,listenfd);
			}
			else if(ev[i].events & EPOLLIN)
			{
				handle_request(epfd,fd);
			}
			else if(ev[i].events & EPOLLOUT)
			{
				continue;
			}
		}
	}
	close(epfd);
}