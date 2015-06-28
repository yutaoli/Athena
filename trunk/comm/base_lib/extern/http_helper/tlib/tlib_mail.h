#ifndef _TLIB_MAIL_H_
#define _TLIB_MAIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <syslog.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <setjmp.h>

/* for TLib_Mail_Printf() */
extern int mail_debug;
extern int mail_level;
extern char mail_error[512];
#define TLIB_MAIL_DEVICE_NULL    0
#define TLIB_MAIL_DEVICE_STDERR  1
#define TLIB_MAIL_DEVICE_LOGFILE 2
#define TLIB_MAIL_LEVEL_FATAL 	LOG_CRIT
#define TLIB_MAIL_LEVEL_ERROR 	LOG_ERR
#define TLIB_MAIL_LEVEL_WARNING LOG_WARNING
#define TLIB_MAIL_LEVEL_INFO 	LOG_INFO
#define TLIB_MAIL_LEVEL_DEBUG 	LOG_DEBUG

/* for TLib_Mail_ParseAddress() */
#define TLIB_MAIL_ADDRESS_LENGTH	128
#define TLIB_MAIL_ACCOUNT_LENGTH	64
#define TLIB_MAIL_SERVER_LENGTH		64

/* for TLib_Mail_CheckAddress() */
#define TLIB_MAIL_VALIDCHAR	"@_.-"

/* for TLib_Mail_CheckSmtpServer(),TLib_Mail_CheckPop3Server */
#define TLIB_MAIL_TCPCONNECTION_TIMEOUT 10

/* for TLib_Mail_SendMail() */
#define TLIB_MAIL_TOTAL_LENGTH	10240
#define TLIB_MAIL_CC_LENGTH	256
#define TLIB_MAIL_CC_DELIMITER ","
#define TLIB_MAIL_TCP_TIMEOUT   60  
#define TLIB_MAIL_SMTP_LINE_LENGTH	1024

/* for TLib_Mail_ModifyAlias() */
#define TLIB_MAIL_ALIAS_FILE	"/etc/aliases"
#define TLIB_MAIL_ALIAS_LOCKFILE	"/tmp/aliases.lock"
#define TLIB_MAIL_ALIAS_ADDACTION	"add"
#define TLIB_MAIL_ALIAS_DELACTION	"del"
#define TLIB_MAIL_ALIAS_ENTRYLENGTH	256
#define TLIB_MAIL_ALIAS_WAITTIME	8
#define TLIB_MAIL_ALIAS_TEMPFILE	"/tmp/aliases"

void TLib_Mail_Printf(int level, char *fmt, ...);
void TLib_Mail_Perror(void);
int  TLib_Mail_ParseAddress(register char *, register char *, register char *);
int  TLib_Mail_SendMail(char *, char *, char *, char *);
int  TLib_Mail_SendMail2(char *,char *,char *, char *,char *,char *);
int TLib_Mail_SendMail_Html(char *,char *,char *, char *,char *,char *);
void TLib_Mail_SetSmtpServer(char *,char *);
int  TLib_Mail_CheckAddress(register char *);
int  TLib_Mail_CheckPop3(char *, char *);
int  TLib_Mail_AddAlias(char *, char *, char *);
int TLib_Mail_SendMailDef(char *sFrom,char *sTo,char *sCc, char *sSubject,int iIsHTML, char *sContent,char *sErrMsg);

#endif

