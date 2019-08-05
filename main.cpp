#include "utility.h"

#define LISTEN_MAX 100

int init(const char* ip,int port);
void handle_request(int fd);

int main()
{
	int listenfd = init("0.0.0.0", 8080);	
	while(1)
	{
		int fd = accept(listenfd,NULL,NULL);
		printf("有客户端连接\n");
		handle_request(fd);
		close(fd);
	}
	close(listenfd);
	return 0;	
}

int init(const char* ip,int port)
{
	int listenfd = socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in serv_addr;
	bzero(&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(80);
	
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
	return listenfd;
}

void handle_request(int fd)
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