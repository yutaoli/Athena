/*********************************
  @file:EventHandlerBase.h
  @description:EventHandler家族，父类为CEventHandlerBase，子类为Concrete EventHandler，包括CAcceptor,CEventHandler
  所有的Concrete EventHandler都涉及4个要素：
  1)fd
  2)mask:fd感兴趣的事件，如EPOLLIN|EPOLLOUT，又如EPOLLIN|EPOLLET
  3)事件:如handle_input()，又如handle_output()
  4)读、写数据buf

  @author:yutaoli
  @date:2014年12月19日 17:34:39

 *********************************/
#ifndef _EVENT_HANDLER_BASE_H_
#define _EVENT_HANDLER_BASE_H_

#include <string>
#include "CommDef.h"
#include "Log.h"

using namespace std;

class CInitiationDispatcher;

class CEventHandlerBase
{
	public:
		CEventHandlerBase(int fd, int mask, event_handler_type_t type):_fd(fd), _mask(mask), _event_handler_type(type){}
		virtual ~CEventHandlerBase(){TRACE_LOG("in CEventHandlerBase::~CEventHandlerBase");}

		inline int GetFd(){return _fd;}
		inline int GetMask(){return _mask;}
		inline void SetMask(int mask){_mask = mask;}
		inline event_handler_type_t GetEventHandlerType(){return _event_handler_type;}

		//工厂模式
		//当eEventHandler==type时不用填ip, port, listen_queue_size
		static CEventHandlerBase * Create(event_handler_type_t type, int fd, int mask, CInitiationDispatcher* initiation_dispatcher_ptr, string ip="", unsigned short port=0, int listen_queue_size=-1);

		//初始化成员变量，event_handler复用时调用
		virtual void Init(int mask)=0;

		//hook methods that are called back by
		//the InitiationDispatcher to handle
		//particular type of events.
		virtual int handle_accept() = 0;

		virtual int handle_input() = 0;
		virtual int handle_output() = 0;
		virtual int handle_timeout() = 0;
		virtual int handle_close() = 0;

		virtual int handle_error() = 0;
		virtual int handle_hangup() = 0;

	protected:
		int _fd;
		int _mask;//表示_fd感兴趣的事件，如EPOLLIN|EPOLLET
		//CEventHandlerBase是否加入event_handler_type_t _event_handler_type;成员变量？（当EPOLLIN事件触发时，CInitiationDispatcher根据_event_handler_type决定调用handle_accept()还是handle_input()）方案1：不用加入，eAcceptor和eEventHandler统一调用handle_input()，好处：实现比方案2简单；坏处：代码可读性不如方案2；方案2：加入，目的是当EPOLLIN事件触发时，eAcceptor调用handle_accept(), eEventHandler调用handle_input()，好处：直观，可读性好，因为eAcceptor调用handle_accept()， eEventHandler调用handle_input()；坏处：实现比方案1复杂。这里为了考虑到以后代码的维护，选择了可读性好，即方案2。
		event_handler_type_t _event_handler_type;//CInitiationDispatcher根据_event_handler_type决定调用handle_accept()还是handle_input()
};

class CAcceptor:public CEventHandlerBase
{
	public:
		//bind和listen放在构造函数
		CAcceptor(int fd, int mask, string ip, unsigned short port, int listen_queue_size, CInitiationDispatcher* initiation_dispatcher_ptr);

		virtual ~CAcceptor();

		//初始化成员变量，event_handler复用时调用
		virtual void Init(int mask){}//不用实现


		//hook methods
		virtual int handle_accept();

		virtual int handle_input(){return 0;}
		virtual int handle_output(){return  0;}
		virtual int handle_timeout(){return 0;}//暂时不做 
		virtual int handle_close();

		virtual int handle_error(){return 0;}//暂时不做
		virtual int handle_hangup(){return 0;}//暂时不做

	private:
		string _ip;
		unsigned short _port;
		int _listen_queue_size;
		CInitiationDispatcher* _initiation_dispatcher_ptr;
};

class CEventHandler:public CEventHandlerBase
{
	public:
		CEventHandler(int fd, int mask, CInitiationDispatcher* initiation_dispatcher_ptr):CEventHandlerBase(fd, mask, eEventHandler), _initiation_dispatcher_ptr(initiation_dispatcher_ptr), _buf_send_len(0), _buf_len(0){}
		virtual ~CEventHandler(){TRACE_LOG("in CEventHandler::~CEventHandler");}


		//hook methods
		virtual int handle_accept(){return 0;}

		virtual int handle_input();
		virtual int handle_output();
		virtual int handle_timeout(){return 0;}//暂时不做
		virtual int handle_close();

		virtual int handle_error(){return 0;}//暂时不做
		virtual int handle_hangup(){return 0;}//暂时不做

		virtual int send_rsp(int send_buf_len);//设置epoll状态为可写, 目的是为了支持handle_process()中而不是handle_process()结束后才回包，由插件调用

	
		//初始化成员变量，event_handler复用时调用
		virtual void Init(int mask)
		{
			SetMask(mask);
			_buf_send_len = _buf_len = 0;
		}
		
		//插件调用：取得_buf，目的是复用_buf，无需将插件要发送的数据copy一份到_buf
		char * GetSendBuf(){return _buf;}
		//插件调用：取得_buf使用的长度，目的是复用_buf，无需将插件要发送的数据copy一份到_buf
		int GetUsedBufLen(){return _buf_len;}
		//插件调用：取得_buf的最大内存长度，目的是复用_buf，无需将插件要发送的数据copy一份到_buf
		int GetMaxBufLen(){return sizeof(_buf)/sizeof(char);}

	protected:
		//检查是否收包结束
		//return 0 => go on recving;
		//return >0 => means recv complete, package len is returned;
		//return <0 => error occur
		virtual int check_complete(char *buf, int len);//插件实现

		virtual int handle_process();//插件实现

	private:
		char _buf[10000];//10KB
		int _buf_len;//buf的实际使用长度
		int _buf_send_len;//已经发出去的buf长度
		CInitiationDispatcher* _initiation_dispatcher_ptr;
};

#endif //end of _EVENT_HANDLER_BASE_H_
