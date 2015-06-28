/************************************************
  @file:InitiationDispatcher.cpp
  @description:
  功能是对fd和event_hanlder的回调，是对所有涉及网络fd的管理和调度，区别与对单个fd的管理(对单个fd的管理放在CEventHandlerBase及其子类实现，即event_handler)，CInitiationDispatcher涉及3个函数:
  1)int handle_events():CInitiationDispatcher根据对fd感兴趣的event的返回，回调event_handler的对应接口，如根据EPOLLIN回调event_hanlder->handle_input()
  2)int register_handler():1))把fd添加到多路复用IO器(如select，又如epoll);2))把event_handler添加到CInitiationDispatcher
  3)int remove_handler():1))把fd从多路复用IO器(如select，又如epoll)删除;2))把event_handler从CInitiationDispatcher删除

  注意：event_handler的new和delete由CInitiationDispatcher管理

  @author:yutaoli
  @date:2014年12月19日 17:16:01

 ************************************************/


#include "InitiationDispatcher.h"
#include "EventHandlerBase.h"
#include "Tcphelper.h"
#include <errno.h>

/*
   class CInitiationDispatcher
   {
   public:
   CInitiationDispatcher(int epoll_event_size):_ep_event_size(epoll_event_size)
   {
   _eevents = new struct epoll_event[_ep_event_size];  
   _map_event2handlers.clear();

   _epfd = epoll_create(epoll_event_size);//conn + accept
   }

   virtual ~CInitiationDispatcher()
   {
   if(_eevents)
   {       
   delete []_eevents;
   _eevents = NULL; 
   }       
   if(_ehandlers)
   {       
   delete []_ehandlers;
   _ehandlers = NULL; 
   }       
   }

   int handle_events();
   int register_handler();
   int remove_handler(); 

   private:
//CEventBase _eventBase;//CEventBase是多路复用IO的实现，可能用epoll实现也可能用select实现
int _epfd;
int _ep_event_size;//max open fd
int _listen_queue_size;//listen
struct epoll_event *_eevents;
map<int, CEventHandlerBase*> _map_event2handlers;
};
 */

int CInitiationDispatcher::handle_events()
{
	int nfds = 0, epoll_wait_timeout_ms = 500, fd = 0;

	//每次只做一轮
	//wait for epoll event occur
	nfds = epoll_wait(_epfd, _eevents, _ep_event_size, epoll_wait_timeout_ms);	
	if(nfds>0)//当没有请求时，epoll_wait()返回0
	{
		DEBUG_LOG("epoll_wait return nfds[%d]", nfds);
	}

	for(int i = 0; i< nfds; i++)
	{
		fd = _eevents[i].data.fd;
		if(_eevents[i].events & EPOLLIN)
		{
			TRACE_LOG("fd[%d], EPOLLIN", fd);
		}
		if(_eevents[i].events & EPOLLOUT)
		{
			TRACE_LOG("fd[%d], EPOLLIN", fd);
		}

		map<int, CEventHandlerBase*>::iterator mit = _map_event2handlers.find(fd);
		if(mit == _map_event2handlers.end())
		{
			ERROR_LOG("IMPOSSIBLE: _map_event2handlers cannot find fd[%d]", fd);
			continue;
		}

		CEventHandlerBase* handler = mit->second;
		if(handler->GetMask() & _eevents[i].events & EPOLLIN)
		{
			DEBUG_LOG("EPOLLIN event coming, fd[%d], fd interested events[%d]", fd, _eevents[i].events);

			event_handler_type_t type = handler->GetEventHandlerType();

			//eAcceptor
			if(eAcceptor == type)
			{
				int ret = handler->handle_accept();//
				if(ret != 0)
				{
					ERROR_LOG("handler->handle_accept error, ret[%d], fd[%d], event_handler_type[%d]", ret, fd, type);
					continue;
				}
			}
			//eEventHandler
			else if(eEventHandler == type)
			{
				int ret = handler->handle_input();//
				if(ret != 0)
				{
					ERROR_LOG("handler->handle_input error, ret[%d], fd[%d], event_handler_type[%d]", ret, fd, type);
					continue;
				}
			}

			//add your code here

			continue;
		}

		if(handler->GetMask() & _eevents[i].events & EPOLLOUT)
		{
			DEBUG_LOG("EPOLLOUT event coming, fd[%d], fd interested events[%d]", fd, _eevents[i].events);
			int ret = handler->handle_output();//
			if(ret != 0)
			{
				ERROR_LOG("handler->handle_input error, ret[%d], fd[%d]", ret, fd);
				continue;
			}

			//add your code here

			continue;
		}


		if(handler->GetMask() & _eevents[i].events & EPOLLERR)
		{
			DEBUG_LOG("EPOLLERR event coming, fd[%d], fd interested events[%d]", fd, _eevents[i].events);
			int ret = handler->handle_error();//
			if(ret != 0)
			{
				ERROR_LOG("handler->handle_error, ret[%d], fd[%d]", ret, fd);
				continue;
			}

			//add your code here

			continue;
		}

		if(handler->GetMask() & _eevents[i].events & EPOLLHUP)
		{
			DEBUG_LOG("EPOLLHUP event coming, fd[%d], fd interested events[%d]", fd, _eevents[i].events);
			int ret = handler->handle_hangup();//
			if(ret !=0)
			{
				ERROR_LOG("handler->handle_hangup, ret[%d], fd[%d]", ret, fd);
				continue;
			}

			//add your code here

			continue;
		}

	}

	return 0;
}

int CInitiationDispatcher::register_handler(event_handler_type_t event_handler_type, int fd, int mask, string ip, unsigned short port, int listen_queue_size)
{
	//register epoll event
	if(fd > _ep_event_size)
	{
		ERROR_LOG("register_handler error, fd > _ep_event_size, fd[%d], _ep_event_size[%d]", fd, _ep_event_size);
		return -1;
	}

	//mask:表示acceptor/connector感兴趣的事件，因为这里不能区分acceptor还是connector，于是向上归纳了一层，把acceptor/connector感兴趣的时间统一叫mask

	int ret = CTcphelper::SetNonBlocking(fd);//这样才能多路复用
	if(ret!=0)
	{
		ERROR_LOG("CTcphelper::SetNonBlocking error[%d], fd[%d], mask[%d], event_handler_type[%d]", 
				ret, fd, mask, event_handler_type);
		return -2;
	}

	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = mask;

	if(epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &ev)<0)
	{
		ERROR_LOG("epoll_ctl ADD error[%s], fd[%d]", strerror(errno), fd);
		return -3;
	}

	//register handler if necessary
	map<int, CEventHandlerBase*>::iterator mit = _map_event2handlers.find(fd);
	if(mit == _map_event2handlers.end())
	{
		CEventHandlerBase *event_handler_ptr = CEventHandlerBase::Create(event_handler_type, fd, mask, this, ip, port, listen_queue_size);//保证了CEventHandlerBase->GetFd() == fd
		if(NULL == event_handler_ptr)//其他错误情况
		{
			ERROR_LOG("CEventHandlerBase::Create error, return[NULL], event_handler_type[%d], fd[%d], mask[%d], InitiationDispatcher_ptr[%p], ip[%s], port[%d], listen_queue_size[%d]",
					event_handler_type, fd, mask, this, ip.c_str(), port, listen_queue_size);
			return -4;
		}

		_map_event2handlers[fd] = event_handler_ptr;

		DEBUG_LOG("CInitiationDispatcher::register_handler success, create a new handler[%p], fd[%d]", event_handler_ptr, fd);
		return 0;
	}
	else//复用event_handler
	{
		mit->second->Init(mask);//初始化成员变量
		if(fd != mit->second->GetFd())
		{
			ERROR_LOG("IMPOSSBILE. why epfd.fd != CEventHandlerBase->GetFd(), epfd.fd[%d], CEventHandlerBase->GetFd()[%d]", 
					fd, mit->second->GetFd());
			return -5;
		}

		DEBUG_LOG("CInitiationDispatcher::register_handler success, handler reused, handler[%p], fd[%d]", mit->second, fd);

	}


	return 0;
}

int CInitiationDispatcher::remove_handler(CEventHandlerBase *handler)
{
	//remove fd in epoll epfd
	int fd = handler->GetFd();

	struct epoll_event ev;//In kernel versions before 2.6.9, the EPOLL_CTL_DEL operation required a non-NULL pointer in event, even though this argument is ignored.  Since  kernel  2.6.9, event can be specified as NULL when using EPOLL_CTL_DEL.
	if(epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, &ev)<0)
	{
		ERROR_LOG("epoll_ctl DEL error:%s, fd[%d]", strerror(errno), fd);
		return -1;
	}

	//remove event_handler放在CInitiationDispatcher析构函数中做，目的是复用event_handler

	DEBUG_LOG("CInitiationDispatcher::remove_handler success, fd[%d]", fd);
	return 0;
}

int CInitiationDispatcher::epoll_mod(int fd, int mask)
{
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = mask;

	DEBUG_LOG("CInitiationDispatcher::epoll_mod, fd[%d], changed to events[%d]", fd, mask);

	if(epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &ev)<0)
	{
		ERROR_LOG("epoll_ctl MOD error:%s, fd[%d]", strerror(errno), fd);
		return -1;
	}

	return 0;
}
