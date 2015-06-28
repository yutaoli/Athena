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

#include "tlib_warn.h"

/* data type define */
#define DWORD_LEN 4
typedef char DWORD[DWORD_LEN];
#define ADDR_SIZE 200

typedef struct _WarnBody
{
	DWORD m_dwTime;
	char  m_sHost[21];
	char  m_sApp[21];
	char  m_cLevel;
	DWORD m_dwCode;
	char  m_sMsg[201];
} WarnBody;

#define WP_MAJOR_VERSION 0x01
#define WP_MINOR_VERSION 0x00
#define WP_COMMAND       0x01
typedef struct _WarnHead
{
	char  m_cMajorVer;
	char  m_cMinorVer;
	char  m_cCmd;
	DWORD m_dwBodyLen;
} WarnHead;

#define WP_STX 0x02
#define WP_ETX 0x03
typedef struct _WarnPkg
{
	char  m_cStx;
	WarnHead m_stWarnHead;
	WarnBody m_stWarnBody;
	char  m_cEtx;
} WarnPkg;

/* the default warn server address and port */
#define DEFAULT_SERVER_IP "127.0.0.1"
#define DEFAULT_SERVER_PATH "/tmp/warn_server"
#define DEFAULT_SERVER_PORT 8000

/* the global server address area */ 
static char caWarnServerAddr[50];
static int iWarnServerPort;

#define DEFAULT_TIME_OUT 15

static int iSocket;
static void *psAddr;
static int iAddrSize;
static int iSocketDomain;
static int iSocketType;
static int iKeepConnectFlag;
static int iConTimeOut;

static unsigned char cWarnInitFlag = 0;
static unsigned char cConnectFlag;

static void _SigalrmHandler(int signo)
{
	return;
}

static int _InitSocket()
{
	close(iSocket);
	iSocket = socket(iSocketDomain, iSocketType, 0);
	if(iSocket < 0)
	{
		return -1;
	}
	
	return 0;
}

/* the warn server connect routine */
static int _WSConnect()
{
	struct sigaction act, oact;
	sigset_t set, oset;
	
	if(!cConnectFlag&&iSocketType == CON_TCP)
	{	
		act.sa_handler = _SigalrmHandler;
		sigemptyset(&act.sa_mask);
		act.sa_flags = 0;
		if(sigaction(SIGALRM, &act, &oact) < 0) return -1;
		
		sigemptyset(&set);
		sigaddset(&set, SIGALRM);
		sigprocmask(SIG_UNBLOCK, &set, &oset);

		alarm(iConTimeOut);
		if(connect(iSocket, (struct sockaddr *)psAddr, iAddrSize) < 0) 
		{
			alarm(0);
			if(errno == EINTR) 
			{
				return -3;
			}
			else
			{
				if(_InitSocket()) return -4;
				alarm(iConTimeOut);
				if(connect(iSocket, (struct sockaddr *)psAddr, iAddrSize) < 0)
				{
					alarm(0);
					if(errno == EINTR) return -3;
					else
					{
						if(_InitSocket()) return -4;
						return -2;
					}
				}
			}	
				
		}
		
		sigaction(SIGALRM, &oact, NULL);
		sigprocmask(SIG_SETMASK, &oset, NULL);
	}
	
	cConnectFlag = 1;
	
	return 0;
}
	

/* the warn socket disconnect routine */
static void _WSDisconnect()
{
	if(cConnectFlag&&iSocketType == CON_TCP)
	{
		shutdown(iSocket, SHUT_RDWR);
		cConnectFlag = 0;
	}
}

 	
/* the warn log init routine */
int TLib_Warn_Init(int iDomain, int iConType, char *psServerAddr, int iServerPort, int iKeepConnect, int iTimeOut)
{
	int iLen, iRet;
	
	if(cWarnInitFlag) return -1;
	
	memset(caWarnServerAddr, 0, 50);
	if(psServerAddr != NULL)
	{
		if((iLen = strlen(psServerAddr)) > 49) iLen = 49;
		memcpy(caWarnServerAddr, psServerAddr, iLen);
	}
	else
	{
		if(iDomain == LOG_NET)
		{
			memcpy(caWarnServerAddr, DEFAULT_SERVER_IP, strlen(DEFAULT_SERVER_IP));
		}
		else if(iDomain == LOG_LOCAL)
		{
			memcpy(caWarnServerAddr, DEFAULT_SERVER_PATH, strlen(DEFAULT_SERVER_PATH));
		}
		else
		{
			return -1;
		}
	}
	
	if(iServerPort > 0) iWarnServerPort = iServerPort;
	else iWarnServerPort = DEFAULT_SERVER_PORT;

	/* init the address structure */
	psAddr = malloc(ADDR_SIZE);
	if(psAddr == NULL)
	{
		return -1;
	}
	memset(psAddr, 0, ADDR_SIZE);
	if(iDomain == LOG_NET)
	{
		((struct sockaddr_in *)psAddr)->sin_family = iDomain;
		((struct sockaddr_in *)psAddr)->sin_addr.s_addr = inet_addr(caWarnServerAddr);   /* receiver address */
		((struct sockaddr_in *)psAddr)->sin_port = htons(iWarnServerPort);  /* receiver port    */
		iAddrSize = sizeof(struct sockaddr_in);
	}
	else if(iDomain == LOG_LOCAL)
	{
		((struct sockaddr_un *)psAddr)->sun_family = iDomain;
		memcpy(((struct sockaddr_un *)psAddr)->sun_path, caWarnServerAddr, strlen(caWarnServerAddr));
		iAddrSize = sizeof(struct sockaddr_un);
	}
	else
	{
		return -1;
	}
		
	/* create the socket */
	iSocket = socket(iDomain, iConType, 0);
	if(iSocket < 0)
	{
		return -2;
	}
	
	iSocketDomain = iDomain;
	iSocketType = iConType;
	
	iKeepConnectFlag = iKeepConnect;
	if(iTimeOut > 0) iConTimeOut = iTimeOut;
	else iConTimeOut = DEFAULT_TIME_OUT;
	
	cConnectFlag = 0;
	if(iKeepConnectFlag)
	{
		if(_WSConnect())
		{
			close(iSocket);
			return -3;
		}
		else 
		{
			cConnectFlag = 1;
		}
	}
	
	cWarnInitFlag = 1;
	
	return 0;
}


/* the warn log un-init routine */
void TLib_Warn_UnInit()
{
	if(cWarnInitFlag)
	{
		_WSDisconnect();
		close(iSocket);
		cWarnInitFlag = 0;
	}
}


/* the warn message log routine */
int TLib_Warn_Log(WarnMessage *pstWarnMessage)
{
	WarnPkg stWarnPkg;
	
	int iLen, iRet;
	
	if(!cWarnInitFlag) return -1;
	
	/* transfer the byte order */
	pstWarnMessage->iTime = htonl(pstWarnMessage->iTime);
	pstWarnMessage->iCode = htonl(pstWarnMessage->iCode);
	/* make the warn pkg */
	memset(&stWarnPkg, 0, sizeof(WarnPkg));
	stWarnPkg.m_cStx = WP_STX;
	stWarnPkg.m_cEtx = WP_ETX;
	stWarnPkg.m_stWarnHead.m_cMajorVer = WP_MAJOR_VERSION;
	stWarnPkg.m_stWarnHead.m_cMinorVer = WP_MINOR_VERSION;
	stWarnPkg.m_stWarnHead.m_cCmd = WP_COMMAND;
	iLen = sizeof(WarnBody);
	iLen = htonl(iLen);
	memcpy(stWarnPkg.m_stWarnHead.m_dwBodyLen, &iLen, DWORD_LEN);
	memcpy(stWarnPkg.m_stWarnBody.m_dwTime, &(pstWarnMessage->iTime), DWORD_LEN);
	memcpy(stWarnPkg.m_stWarnBody.m_dwCode, &(pstWarnMessage->iCode), DWORD_LEN);
	memcpy(stWarnPkg.m_stWarnBody.m_sHost, pstWarnMessage->sHost, 20);
	memcpy(stWarnPkg.m_stWarnBody.m_sApp, pstWarnMessage->sApp, 20);
	stWarnPkg.m_stWarnBody.m_cLevel = pstWarnMessage->cLevel;
	memcpy(stWarnPkg.m_stWarnBody.m_sMsg, pstWarnMessage->sMessage, 200);
	
	if(_WSConnect())
	{
		return -2;
	}
	
	if(iSocketType == CON_TCP)
	{
		if(send(iSocket, &stWarnPkg, sizeof(WarnPkg), 0) != sizeof(WarnPkg))
		{
			_WSDisconnect();
			return -3;
		}
	}
	else
	{
		if(sendto(iSocket, &stWarnPkg, sizeof(WarnPkg), 0, (struct sockaddr *)psAddr, iAddrSize) != sizeof(WarnPkg))
		{
			_WSDisconnect();
			return -3;
		}
	}
	
	if(!iKeepConnectFlag) _WSDisconnect();
	
	return 0;
}

	
