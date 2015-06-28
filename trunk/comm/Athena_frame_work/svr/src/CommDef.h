#ifndef _COMM_DEF_H
#define _COMM_DEF_H

typedef enum
{
	eAcceptor=0,
	eEventHandler= 1
}event_handler_type_t;

#define LISTEN_QUEUE_SIZE 100
#define EPOLL_EVENT_SIZE 100000


#endif //end of _COMM_DEF_H
