#include "utility.h"
#include "event.h"

#define LISTEN_MAX 100

int main()
{
	CEvent *event = new CEvent(8080);
	event->init();
	event->do_epoll();
	return 0;	
}








