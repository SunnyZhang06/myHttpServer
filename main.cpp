#include "event.h"

int main()
{
	CEvent *event = new CEvent(8080);
	event->init();
	event->do_epoll();
	delete event;
	return 0;	
}








