/*
 * =====================================================================================
 * 
 *        Filename:  Daemonize.cpp
 * 
 *     Description:  
 * 
 *         Version:  1.0
 *         Created:  2015年05月19日 21时35分59秒 CST
 *        Revision:  none
 *        Compiler:  gcc
 * 
 *          Author:  yutaoli (), yutaoli@tencent.com
 *         Company:  Shenzhen Tencent Co.Ltd.
 * 
 * =====================================================================================
 */

#include <fcntl.h>
#include <sys/resource.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include "Log.h"

void daemonize()
{
	int i, fd0, fd1, fd2;
	pid_t pid;
	struct rlimit rl;
	struct sigaction sa;


	umask(0);

	if(getrlimit(RLIMIT_NOFILE, &rl) < 0)
	{
		printf("cannot get file limit\n");
		return;
	}

	if((pid = fork()) < 0)
	{
		printf("cannot fork\n");
		return;
	}
	else if(pid != 0)//父进程
	{
		exit(0);
	}

	setsid();


	//
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	if(sigaction(SIGHUP, &sa, NULL) < 0)
	{
		printf("cannot ignore SIGHUP\n");
		return;
	}
	if((pid = fork()) < 0)
	{
		printf("cannot fork\n");
		return;
	}
	else if(pid != 0)
	{
		exit(0);
	}


	/*
	if(chdir("/") < 0)
	{
		printf("cannot change directory to /\n");
		return;
	}
	*/


	//把rlim_max描述符关掉，这里并没有改变进程的rl.rlim_max数，因为没有set
	if(rl.rlim_max == RLIM_INFINITY)
	{
		rl.rlim_max = 1024;
	}
	for(int i = 0; i < rl.rlim_max; i++)
	{
		close(i);
	}


	//
	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);

	//

}
/*

void child_handler(int sig)
{
	printf("sig[%d]\n", sig);
}

int main(int argc, char **argv)
{
	//daemonize(argv[0]);
	signal(SIGTERM, child_handler);
	signal( SIGINT,  child_handler);
	signal( SIGHUP,  child_handler);
	signal( SIGQUIT, child_handler);
	signal( SIGPIPE, child_handler);
	signal( SIGTTOU, child_handler);
	signal( SIGTTIN, child_handler);
	signal( SIGCHLD, child_handler);
	signal( SIGTERM, child_handler);
	printf("SIGTERM[%d]\n", SIGTERM);
	printf("SIGINT[%d]\n", SIGINT);
	printf("SIGHUP[%d]\n", SIGHUP);
	printf("SIGQUIT[%d]\n", SIGQUIT);
	printf("SIGPIPE[%d]\n", SIGPIPE);
	printf("SIGTTOU[%d]\n", SIGTTOU);
	printf("SIGTTIN[%d]\n", SIGTTIN);
	printf("SIGCHLD[%d]\n", SIGCHLD);

	while(1)
	{
		sleep(60);
	}
	return 0;
}
*/
