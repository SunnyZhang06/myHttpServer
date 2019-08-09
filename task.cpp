#include <iostream>

#include "task.h"

using namespace std;

void CTask::removefd(int epfd,int fd)
{
	epoll_ctl(epfd,EPOLL_CTL_DEL,fd,0);
	close(fd);
}

// void CTask::doit()
// {
	// char buf[BUFFER_SIZE] = {0};
	
	// /*从fd上循环读数据*/
	// while(int ret = recv( accept_fd, buf, 1024, 0 ))
	// {
		// if(!ret)
		// {
			// cout<<"browser exit.\n"<<endl;
			// break;
		// }
		// else if(ret<0)
		// {
			// continue;//如果接收出错则继续接收
		// }
		
		// int start = 0;
		// char method[5],uri[100],version[10];
		// sscanf(buf,"%s %s %s",method,uri,version);
		
		// if( char *tmp = strstr( buf, "Range:" ) ) {
            // tmp += 13;
            // sscanf( tmp, "%d", &start );
        // }
		
		// if(!strcmp(method,"GET"))        //为GET
			// deal_get(uri,start);
		// else if(!strcmp(method,"POST"))  //为POST
			// deal_post(uri,buf);
		// else
		// {
			// const char *header = "HTTP/1.1 501 Not Implemented\r\nContent-Type: text/plain;charset=utf-8\r\n\r\n";
			// send(accept_fd,header,strlen(header),0);
		// }
		// break;  // 只要处理完就退出循环，避免浏览器一直处于pending状态
	// }
	// close( accept_fd );  // 任务完成直接close
// }

void CTask::doit()
{
	handle_request(epfd,accept_fd);
}

void CTask::handle_request(int epfd,int fd)
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

int CTask::deal_get(const string &uri,int start)
{
	string filename = uri.substr(1);
	
	if( uri == "/" || uri == "/index.html" ) {
        send_file( "index.html", "text/html", start );
    } else if( uri.find( ".jpg" ) != string::npos || uri.find( ".png" ) != string::npos ) {
        send_file( filename, "image/jpg", start );
    } else if( uri.find( ".html" ) != string::npos ) {
        send_file( filename, "text/html", start );
    } else if( uri.find( ".ico" ) != string::npos ) {
        send_file( filename, "image/x-icon", start );
    } else if( uri.find( ".js" ) != string::npos ) {
        send_file( filename, "yexy/javascript", start );
    } else if( uri.find( ".css" ) != string::npos ) {
        send_file( filename, "text/css", start );
    } else if( uri.find( ".mp3" ) != string::npos ) {
        send_file( filename, "audio/mp3", start );
    } else if( uri.find( ".mp4" ) != string::npos ) {
        send_file( filename, "audio/mp4", start );
    } else {
        send_file( filename, "text/plain", start );
    }
	
}

int CTask::deal_post(const string &uri,char *buf)
{
	string filename = uri.substr(1);
	
	if(uri.find("adder")!=string::npos)//使用CGI服务器，进行加法运算
	{
		char *tmp = buf;
		int len,a,b;
		char *l = strstr(tmp,"Content-Length:");//获取请求报文主体大小
		sscanf(l,"Content-Length: %d", &len);
		len = strlen(tmp)-len;
		tmp += len;
		sscanf(tmp,"a=%d&b=%d",&a,&b);
		sprintf(tmp,"%d+%d,%d",a,b,accept_fd);// tmp存储发送到CGI服务器的参数
		
		// fork产生子进程，执行CGI服务器进行计算（webServer只进行解析、发送数据，不进行相关计算）
		if(fork()==0)
		{
			execl(filename.c_str(),tmp,NULL);
		}
		wait(NULL);//等待子进程结束
	}
	else
	{
		send_file("html/404.html", "text/html", 0, 404, "Not Found");
	}
}

// type对应response的Content-Type，num为状态码，info为状态描述
int CTask::send_file(const string & filename, const char *type, 
                    int start, const int num, const char *info )
{
	struct stat filestate;//这个结构体是用来描述一个linux系统文件系统中的文件属性的结构
	int ret = stat(filename.c_str(),&filestate);
	if(ret<0 || !S_ISREG(filestate.st_mode))// 打开文件出错或没有该文件
	{
		cout << "file not found : " << filename << endl;
        send_file( "html/404.html", "text/html", 0, 404, "Not Found" );
        return -1;
	}
	/*以上可参考：https://blog.csdn.net/acs713/article/details/8569962 */
	
	char header[1024];
	sprintf(header, "HTTP/1.1 %d %s\r\nServer: niliushall\r\nContent-Length: %d\r\nContent-Type: %s;charset:utf-8\r\n\r\n", num, info, int(filestate.st_size - start), type );
	
	// send第二个参数只能是c类型字符串，不能使用string
    send( accept_fd, header, strlen(header), 0 );
	
	int fd = open( filename.c_str(), O_RDONLY );
    int sum = start;

    while( sum < filestate.st_size )
	{
		off_t t = sum;

        int r = sendfile( accept_fd, fd, &t, filestate.st_size );

        if( r < 0 ) 
		{
            printf("errno = %d, r = %d\n", errno, r);
            // perror("sendfile : ");
            if( errno == EAGAIN ) 
			{
                printf("errno is EAGAIN\n");
                // reset_oneshot( epoll_fd, accept_fd );
                continue;
			} 
			else {
                perror( "sendfile " );
                close( fd );
                break;
            }
		}
		else
		{
			sum += r;
		}            
	}
	close(fd);
	close(accept_fd);
	
	return 0;
}


int CTask::get_size(const string &filename)
{
	struct stat filestate;
	int ret = stat(filename.c_str(),&filestate);

	if( ret < 0 ) 
	{
        cout << "file not found : " << filename << endl;
        return 0;
    }
	
	return filestate.st_size;
}

































