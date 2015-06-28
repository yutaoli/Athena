/*
 *  This file is provided for use with the unix-socket-faq.  It is public
 *  domain, and may be copied freely.  There is no copyright on it.  The
 *  original work was by Vic Metcalfe (vic@brutus.tlug.org), and any
 *  modifications made to that work were made with the understanding that
 *  the finished work would be in the public domain.
 *
 *  If you have found a bug, please pass it on to me at the above address
 *  acknowledging that there will be no copyright on your work.
 *
 *  The most recent version of this file, and the unix-socket-faq can be
 *  found at http://www.interlog.com/~vic/sock-faq/.
 */

#ifndef _TLIB_SOCK_H_
#define _TLIB_SOCK_H_

#include "tlib_com.h"
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <limits.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>

/* On some crazy systems, these aren't defined. */
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

int TLib_Sock_Atoport(/* char *service, char *proto */);
struct in_addr *TLib_Sock_Atoaddr(/* char *address */);
int TLib_Sock_Get_Connection(/* int socket_type, u_short port, int *listener */);
int TLib_Sock_Make_Connection(/* char *service, int type, char *netaddress */);
int TLib_Sock_Read(/* int sockfd, char *buf, size_t count */);
int TLib_Sock_Write(/* int sockfd, const char *buf, size_t count */);
int TLib_Sock_Gets(/* int sockfd, char *str, size_t count */);
int TLib_Sock_Puts(/* int sockfd, const char *str */);
void TLib_Sock_Ignore_Pipe(/*void*/);

#endif

