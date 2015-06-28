/*********************************
  @file:EventHandlerBase.cpp
  @description:EventHandler家族，父类为CEventHandlerBase，子类为Concrete EventHandler，包括CAcceptor,CEventHandler
  所有的Concrete EventHandler都涉及4个要素：
  1)fd
  2)mask:fd感兴趣的事件，如EPOLLIN|EPOLLOUT，又如EPOLLIN|EPOLLET
  3)事件:如handle_input()，又如handle_output()
  4)读、写数据buf

  @author:yutaoli
  @date:2014年12月19日 17:34:39

 *********************************/

#include "EventHandlerBase.h"
#include "InitiationDispatcher.h"
#include "Tcphelper.h"
#include "Log.h"
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include "test_parser.h" //打解包用
#include "persons.h"
#include "PluginLoader.h"

/*
   class CAcceptor:public CEventHandlerBase
   {
   public:
   CAcceptor(int fd, int mask):CEventHandlerBase(fd, mask){}	
   virtual ~CAcceptor(){}

//hook methods
virtual int handle_accept();

virtual int handle_input(){return 0;}
virtual int handle_input(){return 0;}
virtual int handle_output(){return  0;}
virtual int handle_timeout(); 
virtual int handle_close(){return 0;}

virtual int handle_error();
virtual int handle_hangup(){return 0;}
};

class CEventHandler:public CEventHandlerBase
{
public:
CEventHandler(int fd, int mask):CEventHandlerBase(fd, mask){}
virtual ~CEventHandler(){}

//hook methods
virtual int handle_accept(){return 0;}

virtual int handle_input();
virtual int handle_output();
virtual int handle_timeout();
virtual int handle_close();

virtual int handle_error();
virtual int handle_hangup();

};
 */

CEventHandlerBase* CEventHandlerBase::Create(event_handler_type_t type, int fd, int mask, CInitiationDispatcher* initiation_dispatcher_ptr, string ip, unsigned short port, int listen_queue_size)
{
	if(eAcceptor == type)
	{
		return new CAcceptor(fd, mask, ip, port, listen_queue_size, initiation_dispatcher_ptr);
	}

	if(eEventHandler == type)
	{
		return new CEventHandler(fd, mask, initiation_dispatcher_ptr);
	}

	return NULL;//错误
}

//bind和listen放在构造函数
	CAcceptor::CAcceptor(int fd, int mask, string ip, unsigned short port, int listen_queue_size, CInitiationDispatcher *initiation_dispatcher_ptr)
	:CEventHandlerBase(fd, mask, eAcceptor)
	, _ip(ip)
	, _port(port)
	, _listen_queue_size(listen_queue_size)
, _initiation_dispatcher_ptr(initiation_dispatcher_ptr)
{
	TRACE_LOG("in CAcceptor::CAcceptor");


	struct sockaddr_in serveraddr;
	bzero(&serveraddr, sizeof(serveraddr));

	//set family, ip, port
	serveraddr.sin_family = AF_INET;	
	inet_aton(_ip.c_str(), &(serveraddr.sin_addr));
	serveraddr.sin_port = htons(_port);

	int ret = bind(_fd, (sockaddr *)&serveraddr, sizeof(serveraddr));
	if(ret != 0)
	{
		ERROR_LOG("fd[%d], bind error, ret[%d], errmsg[%s]", _fd, errno, strerror(errno));
		//exit(0);
	}

	ret = listen(_fd, _listen_queue_size);

	struct sockaddr_in serveraddr2;
	socklen_t serveraddr2_len;
	getsockname(_fd, (struct sockaddr*)&serveraddr2, &serveraddr2_len);
	DEBUG_LOG("fd[%d], serveraddr2.sin_port[%d]", _fd, serveraddr2.sin_port);

	if(ret != 0)
	{
		ERROR_LOG("fd[%d], listen error, ret[%d], errmsg[%s]", _fd, errno, strerror(errno));
		//exit(0);
	}

}

CAcceptor::~CAcceptor()
{
	TRACE_LOG("in CAcceptor::~CAcceptor");


	int ret = handle_close();
	if(ret != 0)
	{
		ERROR_LOG("acceptor handle_close error, ret[%d], fd[%d]", ret, _fd);
	}
}

int CAcceptor::handle_accept()
{
	struct sockaddr_in clientaddr;
	socklen_t clilen = sizeof(struct sockaddr);

	int conn_fd = accept(_fd, (sockaddr *)&clientaddr, &clilen);
	if(conn_fd<0)
	{
		ERROR_LOG("accept error, ret[%d], errmsg[%s]", conn_fd, strerror(errno));
		return -1;
	}

	DEBUG_LOG("new request coming, created fd[%d]", conn_fd);

	//register to InitiationDispatcher，new出来的内存由_initiation_dispatcher_ptr释放
	int mask = EPOLLIN|EPOLLHUP|EPOLLERR;

	//交给InitiationDispatcher生成eEventHandler类event_handler
	int ret = _initiation_dispatcher_ptr->register_handler(eEventHandler, conn_fd, mask);
	if(ret !=0)
	{
		ERROR_LOG(" _initiation_dispatcher_ptr->register_handler error, ret[%d], event_handler_type[%d], conn_fd[%d], mask[%d]", 
				ret, eEventHandler, conn_fd, mask);
		return -2;
	}

	return 0;
}

int CAcceptor::handle_close()
{
	//最原始的实现是分2步：
	//1.从epfd中删除_fd
	//2.从_initiation_dispatcher_ptr的_map_event2handlers中删除_fd对应的event_handler。
	//实际的实现是：只从epfd中删除_fd，没有删除_fd对应event_handler，目的是让handler复用
	int ret = _initiation_dispatcher_ptr->remove_handler(this);
	if(ret!=0)
	{
		ERROR_LOG("_initiation_dispatcher_ptr->remove_handler error, ret[%d], fd[%d]", ret, _fd);
		return -1;
	}

	DEBUG_LOG("remove_acceptor [done], fd[%d]", _fd);

	//close(fd)必须放在_initiation_dispatcher_ptr->remove_handler后实现，因为remove_handler()会从epfd中删除_fd，删除_fd，会检查_fd的合法性，如果已经调用close(fd)，会返回Bad file descriptor
	ret = close(_fd);
	if(ret!=0)
	{
		ERROR_LOG("handle_close close(fd) error, ret[%d], fd[%d]", ret, _fd);
		return -2;
	}

	DEBUG_LOG("close fd[done], fd[%d]", _fd);

	return 0;
}


int CEventHandler::handle_input()
{
	int nread = CTcphelper::Read_n(_fd, _buf + _buf_len, 1000);//每次至多读1K数据，目的是读太多怕别的fd饿死

	if(nread < 0)
	{
		ERROR_LOG("CTcphelper::Read_n error");
		return -1;
	}
	else
	{
		_buf_len += nread;
		DEBUG_LOG("after CTcphelper::Read_n fd[%d], buf[%p], read_len[%d], have_read_len[%d]", _fd, _buf, nread, _buf_len);

		if(_buf_len > sizeof(_buf))
		{
			ERROR_LOG("read buf not enough, max_read_buf_len[%d], now read_buf_len[%d]", sizeof(_buf), _buf_len);
			return -2;
		}

		int ret = check_complete(_buf, _buf_len);
		if(ret<0)
		{
			ERROR_LOG("check_complete error, ret[%d]", ret);
			return -3;
		}
		else if(0==ret)//下次继续收
		{
			return 0;
		}
		else
		{
			//success
			DEBUG_LOG("get request finish, request package[%s], fd[%d]", string(_buf, _buf_len).c_str(), _fd);
			ret = handle_process();//插件实现
			if(ret!=0)
			{
				ERROR_LOG("handle_process error, ret[%d]", ret);	
				return -4;
			}

			return 0;
		}
	}

	return 0;
}

int CEventHandler::handle_output()
{
	int send_len = _buf_len - _buf_send_len > 1000? 1000: _buf_len - _buf_send_len;
	int nwrite = CTcphelper::Write_n(_fd, _buf+_buf_send_len, send_len);//每次至多写1K数据，目的是写太多怕其他fd饿死
	if(nwrite<0)
	{
		ERROR_LOG("Tcphelper::Write_n, ret[%d], fd[%d], _buf_len[%d]", nwrite, _fd, _buf_len);
		return -1;
	}
	else
	{
		_buf_send_len += nwrite;	

		if(_buf_send_len == _buf_len)//发完
		{
			DEBUG_LOG("send rsp finish, rsp package[%s], buf address[%p], len[%d], fd[%d]", string(_buf, _buf_len).c_str(), _buf, _buf_len, _fd);
			handle_close();
		}
		else //下次继续发
		{
			return 0;
		}
	}

	return 0;
}

int CEventHandler::handle_close()
{
	//最原始的实现是分2步：
	//1.从epfd中删除_fd
	//2.从_initiation_dispatcher_ptr的_map_event2handlers中删除_fd对应的event_handler。
	//实际的实现是：只从epfd中删除_fd，没有删除_fd对应event_handler，目的是让handler复用
	int ret = _initiation_dispatcher_ptr->remove_handler(this);
	if(ret!=0)
	{
		ERROR_LOG("_initiation_dispatcher_ptr->remove_handler error, ret[%d], fd[%d]", ret, _fd);
		return -1;
	}

	DEBUG_LOG("remove_handler [done]");

	//close(fd)必须放在_initiation_dispatcher_ptr->remove_handler后实现，因为remove_handler()会从epfd中删除_fd，删除_fd，会检查_fd的合法性，如果已经调用close(fd)，会返回Bad file descriptor
	ret = close(_fd);
	if(ret!=0)
	{
		ERROR_LOG("handle_close close(fd) error, ret[%d], fd[%d]", ret, _fd);
		return -2;
	}

	DEBUG_LOG("close fd[done], fd[%d]", _fd);

	return 0;
}

//插件调用
//这里不会立马发送回包，而是交给CInitiationDispatcher通过epoll去调度
int CEventHandler::send_rsp(int send_buf_len)//插件的buf是复用CEventHandler的_buf的，通过GetSendBuf()取得
{
	_mask = EPOLLOUT;//感兴趣的是写事件
	if(_initiation_dispatcher_ptr->epoll_mod(_fd, _mask)<0)
	{       
		ERROR_LOG("_initiation_dispatcher_ptr->epoll_mod error, fd[%d], mask[%d]", _fd, _mask); 
		return -1;
	}       


	//设置发送buf长度
	if(send_buf_len > sizeof(_buf))
	{
		ERROR_LOG("send_buf_len[%d] > max buf size[%d]", send_buf_len, sizeof(_buf));
		return -2;
	}

	_buf_len = send_buf_len;

	return 0;
}

//插件实现
int CEventHandler::check_complete(char *buf, int len)
{
	int flow = 1;
	int ret = svrframe_dll.svrframe_check_complete(flow, buf, len);
	if(ret<0)
	{
		ERROR_LOG("svrframe_dll.svrframe_check_complete error[%d], flow[%d], buf address[%p], len[%d]",
				ret, flow, buf, len);

		return ret;
	}
	return ret;

	//最简单的跑通打解包逻辑，测试用
	return TestParser::CheckAsyComplete((char*)buf, len);

	//最简单的echo回显，测试用
	return len;

	// add your code here

	return 0;
}

//插件实现
int CEventHandler::handle_process()
{
	int flow = 1;
	int ret = svrframe_dll.svrframe_handle_process(flow, this);
	if(ret!=0)
	{
		ERROR_LOG("svrframe_dll.svrframe_handle_process error[%d], flow[%d], this[%p]", ret, flow, this);
		return ret;
	}
	return 0;
	/*
	//最简单的跑通打解包逻辑，测试用
	char m_errmsg[1024]={0};
	TestParser parser;
	//Decode Body
	PERSON::stGetPersonInfoReq stReq;
	char *in_buf = _buf;
	unsigned int in_buf_len = _buf_len;
	ret = parser.Decode(in_buf, in_buf_len, stReq);
	if(ret !=0)
	{
	ERROR_LOG("m_parser.Decode error, ret[%d], errmsg[%s]", ret, parser.GetErrMsg());

	snprintf(m_errmsg, sizeof(m_errmsg), "m_parser.Decode error, ret[%d], errmsg[%s]", ret, parser.GetErrMsg());
	memcpy(_buf, m_errmsg, strlen(m_errmsg));

	ret = send_rsp(strlen(m_errmsg));
	if(ret!=0)
	{
	ERROR_LOG("CEventHandler::handle_process error, ret[%d]", ret);	
	return -1;
	}

	return 0;
	}

	DEBUG_LOG("decode body [done], recv buf address[%p], recv buf len[%d]", in_buf, in_buf_len);
	DEBUG_LOG("req: personid[%d]", stReq.personid);

	///////////////////////////////
	PERSON::Person person;
	string errmsg; 
	ret = GetPersonInfo(person, stReq.personid, errmsg);
	if(ret !=0)
	{
	ERROR_LOG("GetPersonInfo error, ret[%d], errmsg[%s]\n", ret, errmsg.c_str());
	return ret;
	}


	PERSON::stGetPersonInfoRsp stRsp;
	stRsp.base.retcode = ret;
	stRsp.base.errmsg = errmsg;
	stRsp.person_info = person;
	char *out_buf = NULL;	
	unsigned int out_buf_len = 0;
	int trylen = 1000;

	parser.MapBuffer((char *)GetSendBuf(), sizeof(_buf));//插件的buf是复用CEventHandler的_buf的，通过GetSendBuf()取得 
	ret = parser.Encode(stRsp, trylen, out_buf, out_buf_len);
	if(ret!=0)
	{       
	ERROR_LOG("m_parser.Encode error, ret[%d], errmsg[%s]", ret, parser.GetErrMsg());

	return ret;
	}       

	DEBUG_LOG("encode Head+Body [done], encoded buf[%s], encoded buf address[%p], len[%d]", 
	string(out_buf, out_buf_len).c_str(), out_buf, out_buf_len);

	ret = send_rsp(out_buf_len);
	if(ret!=0)
	{
	ERROR_LOG("CEventHandler::handle_process error, ret[%d]", ret);
	return -1;
	}
	return 0;

	//最简单的echo回显，测试用
	ret = send_rsp(_buf_len);
	if(ret!=0)
	{
	ERROR_LOG("CEventHandler::handle_process error, ret[%d]", ret);
	return -1;
}
*/
//add your code here

return 0;
}
