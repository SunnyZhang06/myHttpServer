#include "event.h"
#include "utility.h"


CEvent::CEvent(int m_port):port(m_port)
{
	
}

CEvent::~CEvent()
{
	//delete pool;在哪里释放？
}

void CEvent::init()
{
	listenfd = socket(AF_INET,SOCK_STREAM,0);
	if(listenfd<0)
	{
		perror("bind error");
		exit(1);
	}
	struct sockaddr_in serv_addr;
	bzero(&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
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
	
	pool = CThreadPoolProxy::instance();//初始化线程池CThreadPoolProxy *pool;
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
	socklen_t cliaddrlen = sizeof(client_addr);
	client_fd = accept(listenfd,(struct sockaddr*)&client_addr,&cliaddrlen);
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

void CEvent::handle_request(int epfd,int fd)
{
	char buffer[1024*1024];
	int nread = read(fd,buffer,sizeof(buffer));
	printf("读到的请求是%s\n",buffer);
 
	char filename[10] = {0};
	sscanf(buffer,"GET /%s",filename);
	printf("解析的文件名是：%s\n",filename);
 
	char* mime = NULL;
	if(strstr(filename,".html"))
		mime = "text/html";
	else if(strstr(filename,".jpg"))
		mime = "image/jpg";
	
	//响应报文段
	char response[1024*1024];
	sprintf(response,"HTTP/1.1 200 OK\r\nContext-Type: %s\r\n\r\n",mime);
	int headlen = strlen(response);
	
	//打开客户请求的文件并写进响应报文中
	int filefd = open(filename,O_RDONLY);
	int filelen = read(filefd,response+headlen,sizeof(response)-headlen);
	
	//把响应报文段发送给客户端
	write(fd,response,headlen+filelen);
	epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL);
	close(fd);
	close(filefd); 
}
 

void CEvent::do_epoll()
{
	epfd = epoll_create(EPOLL_CREATE);
	add_fd(epfd,listenfd);
	while(1)
	{
		int nready = epoll_wait(epfd,ev,MAX_EVENT,-1);
		if( nready < 0 ) {
            perror( "epoll_wait error" );
            exit(1);
        }
		for(int i=0;i<nready;++i)
		{
			int fd = ev[i].data.fd;
			if((fd  == listenfd) && (ev[i].events & EPOLLIN))//新连接
			{
				handle_accept(epfd,listenfd);
			}
			else if(ev[i].events & EPOLLIN)//有数据写入
			{
				CTask *task = new CTask(fd,epfd);//该fd在处理完一次请求后关闭
				pool->add_task(task);            //添加到任务队列中
				//printf("add a task, %d\n",fd);
			}
			else if(ev[i].events & EPOLLOUT)
			{
				continue;
			}
		}
	}
	close(listenfd);//关闭listenfd和epfd
	close(epfd);

}









