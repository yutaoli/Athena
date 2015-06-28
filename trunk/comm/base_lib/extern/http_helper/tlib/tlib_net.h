#ifndef _TLIB_NET_H_
#define _TLIB_NET_H_

#include "tlib_com.h"
#include <stdlib.h>
#include <stdarg.h>
#include <syslog.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

#include <netinet/in_systm.h>	/* required for ip.h */
#include <netinet/in.h>
#include <netinet/ip.h>
//#include <netinet/ip_var.h>
#include <netinet/udp.h>
//#include <netinet/udp_var.h>
#include <net/if.h>
#include <netinet/if_ether.h>

int TLib_Net_GetUDPSocket(char *sErrMsg);
int TLib_Net_WriteUDPSocketDef(int iUDPSocket, 
				struct sockaddr_in *pstSrcAddr,
				struct sockaddr_in *pstDestAddr,
				char *sBuf, int iLen, char *sErrMsg);
int TLib_Net_WriteUDPSocket(int iUDPSocket, 
				char *sSrcAddr, char *sSrcPort, 
				char *sDestAddr, char *sDestPort, 
				char *sBuf, int iLen, char *sErrMsg);
int TLib_Net_CloseUDPSocket(int iUDPSocket);


#endif

