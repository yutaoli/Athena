#include <iostream>
#include <stdio.h>
#include "CommDef.h"
#include "InitiationDispatcher.h"
#include "EventHandlerBase.h"
#include "Tcphelper.h"
#include "Log.h"
#include "PluginLoader.h"
#include "Daemonize.h"
#include<dlfcn.h>

#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>

using namespace std;
using namespace SvrFrame;

#define RUN_FLAG_QUIT		0x01
#define RUN_FLAG_RELOAD		0x02

static unsigned int _flag = 0;
static bool reload()
{
	if(_flag & RUN_FLAG_RELOAD)
	{
		_flag &= ~RUN_FLAG_RELOAD;
		return true;
	}
	return false;
}

static bool quit()
{
	if(_flag & RUN_FLAG_QUIT)
	{
		return true;
	}
	return false;
}

static void sigusr1_handler(int signo)
{
	DEBUG_LOG("catch signo[%d], reload...", signo);
	_flag |= RUN_FLAG_RELOAD;
}

static void sigusr2_handler(int signo)
{
	DEBUG_LOG("catch signo[%d], quit...", signo);
	_flag |= RUN_FLAG_QUIT;
}



int main(int argc, char *argv[])
{
	_INIT_LOG("/usr/local/photo/log/svr", 10);

	if(argc<3)
	{
		printf("usage:%s serverip port\n", argv[0]);
		return 0;
	}

	string ip = argv[1];
	unsigned short port = atoi(argv[2]);

	//设定可以打开的文件个数
	struct rlimit rlim;
	if (0 == getrlimit(RLIMIT_NOFILE, &rlim))
	{
		rlim.rlim_cur = rlim.rlim_max;//至少要把rlim_cur变成rlim_max
		setrlimit(RLIMIT_NOFILE, &rlim);
		rlim.rlim_cur = EPOLL_EVENT_SIZE;
		rlim.rlim_max = EPOLL_EVENT_SIZE;
		setrlimit(RLIMIT_NOFILE, &rlim);//如果可以把rlim_cur，rlim_max设置成EPOLL_EVENT_SIZE是最好
	}

	//信号处理
	signal( SIGINT,  SIG_IGN);
	signal( SIGHUP,  SIG_IGN);
	signal( SIGQUIT, SIG_IGN);
	signal( SIGPIPE, SIG_IGN);
	signal( SIGTTOU, SIG_IGN);
	signal( SIGTTIN, SIG_IGN);
	signal( SIGCHLD, SIG_IGN);
	signal( SIGTERM, SIG_IGN);

	//后台运行 
	daemonize();


	//设置reload，quit对应信号
	_flag = 0;
	signal( SIGUSR1, sigusr1_handler);//reload, kill -s SIGUSR1 pid触发
	signal( SIGUSR2, sigusr2_handler);//quit, kill -s SIGUSR2 pid触发


	//载入插件
	load_plugin("../so/libperson_manager.so", RTLD_NOW);

	CInitiationDispatcher *initiation_dispatcher_ptr =  new CInitiationDispatcher(EPOLL_EVENT_SIZE);

	int acceptor_fd = socket(AF_INET, SOCK_STREAM, 0);
	int mask = EPOLLIN;
	int ret = initiation_dispatcher_ptr->register_handler(eAcceptor, acceptor_fd, mask, ip, port, LISTEN_QUEUE_SIZE);
	if(ret!=0)
	{
		ERROR_LOG("initiation_dispatcher_ptr->register_handler error, ret[%d], event_handler_type[%d], fd[%d], mask[%d], ip[%s], port[%d], listen_queue_size[%d]", 
				ret, eAcceptor, acceptor_fd, mask, ip.c_str(), port, LISTEN_QUEUE_SIZE);
		return 0;
	}	



	while(true)
	{
		initiation_dispatcher_ptr->handle_events();

		if(reload())
		{
			DEBUG_LOG("reloading...");

			//reload
			//载入插件
			load_plugin("../so/libperson_manager.so", RTLD_NOW);

		}
		else if(quit())
		{
			DEBUG_LOG("quiting...");
			if(initiation_dispatcher_ptr)
			{
				DEBUG_LOG("delete initiation_dispatcher_ptr");
				delete initiation_dispatcher_ptr;

			}

			return 0;
		}


	}


	return 0;
}
