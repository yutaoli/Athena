/************************************************
@file:InitiationDispatcher.h
@description:
功能是对fd和hanlder的回调，涉及3个函数:
1)int handle_events():CInitiationDispatcher根据对fd感兴趣的event的返回，回调handler的对应接口，如根据EPOLLIN回调hanlder->handle_input()
2)int register_handler():1))把fd添加到多路复用IO器(如select，又如epoll);2))把handler添加到CInitiationDispatcher
3)int remove_handler():1))把fd从多路复用IO器(如select，又如epoll)删除;2))把handler从CInitiationDispatcher删除

注意：event_handler的new和delete由CInitiationDispatcher管理

@author:yutaoli
@date:2014年12月19日 17:16:01

************************************************/

#ifndef _INITIATION_DISPATCHER_H_
#define _INITIATION_DISPATCHER_H_

#include <map>
#include <string>
#include <sys/epoll.h>
#include <errno.h>
#include "CommDef.h"
#include "Log.h"
#include "EventHandlerBase.h"

using namespace std;

class CEventHandlerBase;

class CInitiationDispatcher
{
public:
	CInitiationDispatcher(int epoll_event_size):_ep_event_size(epoll_event_size)//spp框架取10W
	{
		TRACE_LOG("in CInitiationDispatcher::CInitiationDispatcher");


		_eevents = new struct epoll_event[_ep_event_size];	
		_map_event2handlers.clear();

		_epfd = epoll_create(epoll_event_size);//conn + accept
		if(_epfd < 0)
		{
			ERROR_LOG("epoll_create error, ret[%d], errmsg[%s]", errno, strerror(errno));
			return ;
		}
		
		DEBUG_LOG("epoll_create success, epfd[%d]", _epfd);
	}

	virtual ~CInitiationDispatcher()
	{
		TRACE_LOG("in CInitiationDispatcher::~CInitiationDispatcher");


		if(_eevents)
		{
			delete []_eevents;
			_eevents = NULL;
		}
		
		//delete event_handler
		map<int, CEventHandlerBase*>::iterator mit = _map_event2handlers.begin();	
		for(; mit!= _map_event2handlers.end(); mit++)
		{
			DEBUG_LOG("delete fd[%d], handler_address[%p]", mit->first, mit->second);

			if(mit->second)
			{
				delete (mit->second);//需要保证是new出来的
				mit->second = NULL;
			}
		}

		close(_epfd);
		DEBUG_LOG("close epoll fd [done], epfd[%d]", _epfd);
	}

	int handle_events();

	//最原始的实现分2步：
	//1.把fd加入epfd
	//2.new一个event_handler，把event_handler加入_map_event2handlers
	//实际的实现是：如果对应fd在_map_event2handlers没有分配event_handler，就new一个，如果有就复用event_handler
	//当eEventHandler==type时不用填ip, port, listen_queue_size
	int register_handler(event_handler_type_t event_handler_type, int fd, int mask, string ip="", unsigned short port=0, int listen_queue_size=-1);//new由CInitiationDispatcher管理

	//最原始的实现是分2步：
	//1.把fd从epfd中删除
	//2.把fd对应handler从_map_event2handlers中删除。
	//实际的实现并没有删除handler，只是把fd从epfd中remove，目的是为了复用event_handler
	int remove_handler(CEventHandlerBase *handler);

	//_fd感兴趣的事件
	int epoll_mod(int fd, int mask);

private:
	//CEventBase _eventBase;//CEventBase是多路复用IO的实现，可能用epoll实现也可能用select实现
	int _epfd;
	int _ep_event_size;//max open fd
	int _listen_queue_size;//listen
	struct epoll_event *_eevents;
	map<int, CEventHandlerBase*> _map_event2handlers;

};

#endif //end of _INITIATION_DISPATCHER_H_
