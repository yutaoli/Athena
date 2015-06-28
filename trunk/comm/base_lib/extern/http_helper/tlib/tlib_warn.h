/*************************************************************/
/* The System Warning API                                    */
/*                                                           */
/* Company: Tencent                                          */
/* Author: Kenix                                             */
/* Date: 2001/04/06                                          */
/* Version: 1.0a                                             */
/* Description:                                              */
/*                                                           */
/*************************************************************/

#ifndef _TLIB_WARN_H
#define _TLIB_WARN_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define LOG_NET	  AF_INET
#define LOG_LOCAL AF_UNIX
#define CON_TCP   SOCK_STREAM
#define CON_UDP   SOCK_DGRAM

/* warn message structure */
typedef struct _WarnMessage
{
	int    iTime;      /* the log time        */
	char   sHost[21];  /* the log host        */
	char   sApp[21];   /* the log application */
	char   cLevel;     /* the message level   */
	int    iCode;      /* the warning code    */
	char   sMessage[201]; /* the log message  */
} WarnMessage;

int TLib_Warn_Init(int iDomain, int iConType, char *psServerAddr, int iServerPort, int iKeepConnect, int iTimeOut);
void TLib_Warn_UnInit();
int TLib_Warn_Log(WarnMessage *pstWarnMessage);

#endif
