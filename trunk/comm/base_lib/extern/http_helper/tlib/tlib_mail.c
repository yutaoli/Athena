#ifndef _TLIB_MAIL_C_
#define _TLIB_MAIL_C_

#include "tlib_mail.h"
#include "tlib_sock.h"
#include "tlib_str.h"

	
int mail_device=TLIB_MAIL_DEVICE_NULL, mail_level=TLIB_MAIL_LEVEL_ERROR;
char mail_error[512]="";

/*	name: TLib_Mail_Printf
 *	function: log info 
 *	return: no return
 */
void TLib_Mail_Printf(int level, char *fmt, ...) {
	va_list ap;
	char buffer[256];
	char buffertime[16];
	time_t ltime;
	char * timeptr;

	va_start(ap, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);
	time(&ltime);
	timeptr=ctime(&ltime);
	memcpy(buffertime,timeptr+4,15);
	buffertime[15]='\0';

	if( level <= mail_level ) {
		switch(mail_device) {
			/* write to NULL */
			case TLIB_MAIL_DEVICE_NULL:
				break;

			/* write to stderr */
			case TLIB_MAIL_DEVICE_STDERR:
				fprintf(stderr, "[%s]mail-lib: %s\n", buffertime, buffer);
				break;

			/* write to logfile */
			case TLIB_MAIL_DEVICE_LOGFILE:
				break;

			default:
				break;
		}	/* end of switch */
	}	/* end of if */

	switch( level ) {
		case TLIB_MAIL_LEVEL_FATAL:
		case TLIB_MAIL_LEVEL_ERROR:
			snprintf(mail_error,512,"[%s]mail-lib: %s\n", buffertime, buffer);
			break;
		default:
			break;
	}

}

/*	name: TLib_Mail_Perror() 
 *	function: print error info
 */
void TLib_Mail_Perror() {
	printf("%s", mail_error);
}

void TLib_Mail_TrimSpace( char **s ) {
	register char *ptr;

	ptr = *s;
	while( *ptr == ' ' ) ptr++;
	*s = ptr;

	while( *ptr != '\0' ) ptr++;
	ptr--;

	while( *ptr == ' ' ) ptr--;
	ptr++;
	*ptr = '\0';
}

/*	name: TLib_Mail_CheckAddress() 
 *	function: check address string 
 *	return: 0 on success 
 * 	     1 for failure
 */
int TLib_Mail_CheckAddress(register char *string) {
	char buffer[TLIB_MAIL_ADDRESS_LENGTH];
	char *address;
	register char c;

	strncpy(buffer, string, TLIB_MAIL_ADDRESS_LENGTH);
	address=buffer;
	TLib_Mail_TrimSpace( &address );

	/* loop till end until invalid char is meet */
	while( (c=*address) ) {
		if( ((c>=48) && (c<=57)) || ((c>=65) && (c<=90)) || ((c>=97) && (c<=122)) || strchr(TLIB_MAIL_VALIDCHAR, c)) {
			address++;
			continue;
		}
		break;
	}

	/* is invalid char meet during loop? */
	if( *address != (int)NULL ) {
		TLib_Mail_Printf(TLIB_MAIL_LEVEL_ERROR,"Invalid char <%c> detected", (char )*address);
		return(1);
	}

	return(0);
}

/*	name: TLib_Mail_ParseAddress
 *	function: devide mail address into two parts with @ as delimiter
 */
int TLib_Mail_ParseAddress(register char *address, register char *account, register char *server) {
	register char c;
	int n;

	/* get mail account */
	n=0;
	while( (c=*address) && (c!='@') && (n<TLIB_MAIL_ACCOUNT_LENGTH) ) {
		*account++=c;
		n++;
	}
	account++;

	/* get mail server */
	n=0;
	while( (c=*address) && (n<TLIB_MAIL_SERVER_LENGTH) ) {
		*server=c;
		n++;
	}
}

/* name: TLib_Mail_CheckSmtpServer
 * function: authenticate the designated server 
 * return: 0 on success
 			  1 on invalid char included
 			  2 on server name can't be resolved
 			  3 on server can't be connected
 */
int TLib_Mail_CheckSmtpServer(char *server) {
	int n;
	struct in_addr inetaddr;
	struct sockaddr_in sockaddr;
	struct hostent *host;
	int fd;


	void alarm_handler(int signal) {};

	n=TLib_Mail_CheckAddress(server);
	if( n!= 0) {
		TLib_Mail_Printf(TLIB_MAIL_LEVEL_ERROR,"Invalid server address(unrecoginzed char included)");
		return(1);
	}

	inetaddr.s_addr=inet_addr(server);
	if( inetaddr.s_addr != -1 ) {
	} else {
		if( host=gethostbyname(server) ) {
			inetaddr.s_addr=((struct in_addr *)*host->h_addr_list)->s_addr;
		}
		else {
			TLib_Mail_Printf(TLIB_MAIL_LEVEL_ERROR,"Invalid server address(unable to resolve)");
			return(2);
		}
	}

	memset((char *)&sockaddr,0,sizeof(sockaddr));
	sockaddr.sin_family=AF_INET;
	sockaddr.sin_port=htons(25);
	sockaddr.sin_addr.s_addr=inetaddr.s_addr;

	fd=socket(AF_INET,SOCK_STREAM,0);
	signal(SIGALRM, alarm_handler);
	alarm(TLIB_MAIL_TCPCONNECTION_TIMEOUT);
	n=connect(fd,(struct sockaddr *)&sockaddr,sizeof(sockaddr));
	if( n < 0 ) {
		if ( errno = EINTR )
			TLib_Mail_Printf(TLIB_MAIL_LEVEL_ERROR,"Invalid server address(connection timeout)" );
		else
			TLib_Mail_Printf(TLIB_MAIL_LEVEL_ERROR,"Invalid server address(%s)", strerror(errno) );
		close(fd);
		return(3);
	}
	alarm(0);
	write(fd, "quit", sizeof("quit"));
	close(fd);

	return(0);
}

int TLib_Mail_ReadLine(int fd, char *buffer, int count) {
	int n,c=0;

	while( n=read(fd, buffer, 1) ) {
		if( (*buffer == 13) || (c == (count-1)) )  break;
		c++;
		buffer++;
	}
	*buffer='\0';
	if( n<0 ) {
		TLib_Mail_Printf(TLIB_MAIL_LEVEL_FATAL,"System call error(%s)",strerror(errno) );
		return(-1);
	}

	return(0);
} 

/* name: TLib_Mail_CheckPop3Server
 * function: authenticate the designated server 
 * return: 0 on success
 			  1 on invalid char included
 			  2 on server name can't be resolved
 			  3 on server can't be connected
 			  4 on POP3 session initialization error
 			  5 on user not exists on POP3 server
 			  6 on password error
 			  -1 on failed to read from remote server
 */
int TLib_Mail_CheckPop3Server(char *server, char *account, char *passwd) {
	int n;
	struct in_addr inetaddr;
	struct sockaddr_in sockaddr;
	struct hostent *host;
	int fd;
	char pop3buf[128];

	void alarm_handler(int signal) {};

	n=TLib_Mail_CheckAddress(server);
	if( n!= 0) {
		TLib_Mail_Printf(TLIB_MAIL_LEVEL_ERROR,"Invalid server address(unrecoginzed char included)");
		return(1);
	}

	inetaddr.s_addr=inet_addr(server);
	if( inetaddr.s_addr != -1 ) {
	} else {
		if( host=gethostbyname(server) ) {
			inetaddr.s_addr=((struct in_addr *)*host->h_addr_list)->s_addr;
		}
		else {
			TLib_Mail_Printf(TLIB_MAIL_LEVEL_ERROR,"Invalid server address(unable to resolve)");
			return(2);
		}
	}

	memset((char *)&sockaddr,0,sizeof(sockaddr));
	sockaddr.sin_family=AF_INET;
	sockaddr.sin_port=htons(110);
	sockaddr.sin_addr.s_addr=inetaddr.s_addr;

	fd=socket(AF_INET,SOCK_STREAM,0);
	signal(SIGALRM, alarm_handler);
	alarm(TLIB_MAIL_TCPCONNECTION_TIMEOUT);
	n=connect(fd,(struct sockaddr *)&sockaddr,sizeof(sockaddr));
	if( n < 0 ) {
		if ( errno = EINTR )
			TLib_Mail_Printf(TLIB_MAIL_LEVEL_ERROR,"Invalid server address(connection timeout)" );
		else
			TLib_Mail_Printf(TLIB_MAIL_LEVEL_ERROR,"Invalid server address(%s)", strerror(errno) );
		close(fd);
		return(3);
	}
	alarm(0);

	n=TLib_Mail_ReadLine(fd, pop3buf, sizeof(pop3buf));
	if( n< 0 ) {
		TLib_Mail_Printf(TLIB_MAIL_LEVEL_FATAL,"Error in reading POP3 response");
		close(fd);
		return(-1);
	}
	if( strstr(pop3buf,"+OK") == NULL ) {
		TLib_Mail_Printf(TLIB_MAIL_LEVEL_FATAL,"Remote POP3 server error" );
		close(fd);
		return(4);
	}

	snprintf(pop3buf,128,"user %s\n", account);
	write(fd, pop3buf , strlen(pop3buf));
	n=TLib_Mail_ReadLine(fd, pop3buf, sizeof(pop3buf));
	if( n< 0 ) {
		TLib_Mail_Printf(TLIB_MAIL_LEVEL_FATAL,"Error in reading POP3 response");
		close(fd);
		return(-1);
	}
	if( strstr(pop3buf,"+OK") == NULL ) {
		TLib_Mail_Printf(TLIB_MAIL_LEVEL_FATAL,"Invalid user name" );
		close(fd);
		return(5);
	}

	snprintf(pop3buf,128,"pass %s\n", passwd);
	write(fd, pop3buf , strlen(pop3buf));
	n=TLib_Mail_ReadLine(fd, pop3buf, sizeof(pop3buf));
	if( n< 0 ) {
		TLib_Mail_Printf(TLIB_MAIL_LEVEL_FATAL,"Error in reading POP3 response" );
		close(fd);
		return(-1);
	}
	if( strstr(pop3buf,"+OK") == NULL ) {
		TLib_Mail_Printf(TLIB_MAIL_LEVEL_FATAL,"Invalid password" );
		close(fd);
		return(6);
	}

	close(fd);
	return(0);
}


/* name: TLib_Mail_SendMail()
 * function: send a mail to designated address
 * return: 0 on success
 			  1 if at least one mail failed to be sent
 			  -1 on failure to execute mail command
 */
int TLib_Mail_SendMail( char *address, char *subject, char *cc, char *content) {
	int n;
	FILE *file;
	char cmd[64];
	char mail[TLIB_MAIL_TOTAL_LENGTH];
	char ccbuffer[TLIB_MAIL_CC_LENGTH], *addrptr;
	char cccontent[TLIB_MAIL_CC_LENGTH];
	int flag=0, errorflag=0;
	char errorinfo[128]="";
	char errorbuf[1024]="";
	int erroroffset=0;
	char addressunit[128];

	addrptr=address;
	strncpy(cccontent, cc, TLIB_MAIL_CC_LENGTH);

	while( addrptr != NULL ) {
		strncpy( addressunit, addrptr, 128);
		addrptr=addressunit;
		TLib_Mail_TrimSpace( (char **)&addrptr );
		if ( strcmp( addrptr, "" ) == 0 ) {
			snprintf( errorinfo, 128, "Invalid mail address: %s(can't be NULL)\n", addrptr);
			erroroffset = erroroffset + strlen(errorinfo);
		   if ( erroroffset < 1024 ) 
		   	strncat( errorbuf, errorinfo,strlen(errorinfo));
			errorflag=1;
			goto next;
		}
	
		if ( TLib_Mail_CheckAddress(addrptr) != 0) {
			snprintf( errorinfo, 128, "Invalid mail address: %s(unrecognized char included)\n", addrptr);
			erroroffset = erroroffset + strlen(errorinfo);
		   if ( erroroffset < 1024 ) 
		   	strncat( errorbuf, errorinfo,strlen(errorinfo));
			errorflag=1;
			goto next;
		}

		snprintf(cmd,128,"sh -c '/usr/bin/sendmail %s' 1> /dev/null 2>&1", addrptr);
		if(cc == NULL) memset(ccbuffer,'\0',TLIB_MAIL_CC_LENGTH);
		else if(strcmp(cc, "") == 0) memset(ccbuffer,'\0',TLIB_MAIL_CC_LENGTH);
		else snprintf(ccbuffer,TLIB_MAIL_CC_LENGTH,"Cc: %s",cc);
		snprintf(mail, TLIB_MAIL_TOTAL_LENGTH, "To: %s\nSubject: %s\n%s\n\n%s\n", addrptr, subject, ccbuffer, content);
	
		if (( file=popen(cmd, "w")) == NULL )  {
			TLib_Mail_Printf( TLIB_MAIL_LEVEL_FATAL, "System call error(popen: %s)", strerror(errno));
			return(-1);
		}
		if ( fwrite(mail, strlen(mail), 1, file) != 1 ) {
			TLib_Mail_Printf( TLIB_MAIL_LEVEL_FATAL, "System error(fwrite: %s)", strerror(errno));
			return(-1);
		}
		if ( pclose(file) != 0) {
			snprintf( errorinfo, 128, "Invalid mail address: %s(user unknown)\n", addrptr);
			erroroffset = erroroffset + strlen(errorinfo);
		   if ( erroroffset < 1024 ) 
		   	strncat( errorbuf, errorinfo,strlen(errorinfo));
			errorflag=1;
			goto next;
			errorflag=1;
		}

next:	if( cc != NULL ) {
			if ( flag == 0 ) {
				addrptr=strtok( cccontent, TLIB_MAIL_CC_DELIMITER);
				flag = 1;
			} else {
				addrptr=strtok( NULL, TLIB_MAIL_CC_DELIMITER);
			}
		}

	}

	if ( errorflag == 1 ) {
		TLib_Mail_Printf( TLIB_MAIL_LEVEL_ERROR, "\n%s", errorbuf);
		return(1);
	}
	return(0);
}

int TLib_Mail_MatchEntry( char *user, char *entry) {
	register char * ptr;
	char owner[TLIB_MAIL_ALIAS_ENTRYLENGTH];
	char *optr=owner;

	ptr=entry;
	while( (*ptr != ':') && (*ptr != '\0') ) {
		*optr++ = *ptr++;
	}
	*optr='\0';

	optr=owner;
	TLib_Mail_TrimSpace((char **)&optr);
	if( strcmp(user, optr)==0 ) return(0);
	else {
		return(1);
	}
}

/* name: TLib_Mail_ModifyAlias
 *	function: modify alias
 *	return: 0 on success
 			  1 if the lockfile already exists
 			  2 if unable to create the lockfile
 			  3 if unable to open the alias file
 			  4 if failed to delete user entry before adding an entry
 			  5 if failed to remove alias file
 			  6 if failed to rename temp alias file
 			  7 if failed to remove lock file
 */
int TLib_Mail_ModifyAlias(char *user, char *action, char *args) {
	int fd, newfd;
	struct stat st;
	int n, k;
	FILE *file,*newfile;
	char filename[64];
	char entry[TLIB_MAIL_ALIAS_ENTRYLENGTH];

	while( !(((n = stat(TLIB_MAIL_ALIAS_LOCKFILE, &st))<0) && (errno == 2)) ) {
		sleep(1);
		if( k++ < TLIB_MAIL_ALIAS_WAITTIME ) {
			TLib_Mail_Printf( TLIB_MAIL_LEVEL_ERROR, "Unable to create lockfile" );
			return(1);
		}
	}

	/* create the lockfile */
	fd = open(TLIB_MAIL_ALIAS_LOCKFILE, O_RDWR|O_CREAT);
	if( fd < 0) {
		TLib_Mail_Printf( TLIB_MAIL_LEVEL_FATAL, "System error(open %s: %s)", TLIB_MAIL_ALIAS_LOCKFILE, strerror(errno));
		return(2);
	}
	close( fd );

	/* open /etc/aliases file */
	file=fopen(TLIB_MAIL_ALIAS_FILE, "r");
	if( file == NULL) {
		TLib_Mail_Printf( TLIB_MAIL_LEVEL_FATAL, "System error(fopen: %s)", strerror(errno));
		unlink( TLIB_MAIL_ALIAS_LOCKFILE );
		return(3);
	}

	/* open aliases temp file */
	snprintf(filename,64, "%s.%d", TLIB_MAIL_ALIAS_TEMPFILE, getpid());
	fd=open(filename, O_RDWR|O_CREAT);
	if( fd<0) {
		TLib_Mail_Printf( TLIB_MAIL_LEVEL_FATAL, "System error(open: %s)", strerror(errno));
		unlink( TLIB_MAIL_ALIAS_LOCKFILE );
		return(2);
	}

	if( strcmp( action, TLIB_MAIL_ALIAS_DELACTION ) == 0 ) {
		while( fgets(entry, TLIB_MAIL_ALIAS_ENTRYLENGTH, file) ) {
			if( TLib_Mail_MatchEntry(user,entry)!=0) {
				write(fd, entry, strlen(entry));
			}
		}
	}
	if( strcmp( action, TLIB_MAIL_ALIAS_ADDACTION ) == 0 ) {
		while( fgets(entry, TLIB_MAIL_ALIAS_ENTRYLENGTH, file) ) {
			if( TLib_Mail_MatchEntry(user,entry)!=0) {
				write(fd, entry, strlen(entry));
			}
		}
		snprintf(entry,TLIB_MAIL_ALIAS_ENTRYLENGTH, "%s: %s\n", user, args);
		write(fd, entry, strlen(entry));
	}

	close(fd);
	fclose(file);

	if( unlink( TLIB_MAIL_ALIAS_FILE)<0) {
		TLib_Mail_Printf( TLIB_MAIL_LEVEL_FATAL, "System error(unlink: %s)", strerror(errno));
		return(5);
	}

	if( rename( filename, TLIB_MAIL_ALIAS_FILE) < 0)  {
		TLib_Mail_Printf( TLIB_MAIL_LEVEL_FATAL, "System error(unlink: %s)", strerror(errno));
		return(6);
	}

	if( unlink( TLIB_MAIL_ALIAS_LOCKFILE )<0) {
		TLib_Mail_Printf( TLIB_MAIL_LEVEL_FATAL, "System error(unlink: %s)", strerror(errno));
		return(7);
	}

	return(0);
}

#define Smtp_Debug 0

static char sSmtpServer[20]="127.0.0.1"; 
static char sSmtpPort[5]="25"; 

/**********************************************************************************
Function:    :设置smtp网关的地址
input:        sHostAddress
 	      sPort  	   
return        none
lastmodify:   2000.07.11
written by    Dennis
**********************************************************************************/
void TLib_Mail_SetSmtpServer(char *sHostAddress,char *sPort)
{
	strncpy(sSmtpServer,sHostAddress,sizeof(sSmtpServer)-1);
	strncpy(sSmtpPort,sPort,sizeof(sSmtpPort)-1);
	
} 

/**********************************************************************************
Function:    :分析smtp的应答码
input:        sReply 	      
output        sErrMsg
return        code //220.250,500...etc
lastmodify:   2000.07.11
written by    Dennis
**********************************************************************************/
int GetSmtpReplyCode(char *sReply,char *sErrMsg)
{
     char sCode[10];
     int  iCode;
     
     if (sReply==NULL)
     {
     	 snprintf(sErrMsg,1024,"the Reply is null");
     	 return 550;
     }	
     
     strncpy(sCode,sReply,3);
     strncpy(sErrMsg,sReply+3,sizeof(sErrMsg)-1);
     iCode= atoi(sCode);
     return  iCode;
     
}

/**********************************************************************************
Function:    :发送smtp的命令
input:        sReply 	      
output        sErrMsg
return        code //220.250,500...etc
lastmodify:   2000.07.11
written by    Dennis
**********************************************************************************/
int OutSmtpCommand(int iSocket,char *sLine,char *sErrMsg)
{
	int iCode;

	if (Smtp_Debug)
		printf(sLine);
	
	TLib_Sock_Puts(iSocket,sLine);
	TLib_Sock_Gets(iSocket,sLine,TLIB_MAIL_SMTP_LINE_LENGTH);
	
	if (Smtp_Debug)
		printf("%s\n",sLine);

  	iCode = GetSmtpReplyCode(sLine,sErrMsg);
  	if (iCode>=400)   
  		return -1;
  		
  	return 0;	
}

/**********************************************************************************
Function:    :从字符流中取得一行
input:        buf
output:       sLine
return        sbuf
lastmodify:   1999.11.17
written by    Dennis
**********************************************************************************/
char *GetLineFromBuf(char *sBuf,char *sLine,int iMaxCount){ 
	
    int i=0;
    char *pStr;
    
    while ((sBuf[i]!='\n') && (sBuf[i]!='\0') && (i<iMaxCount-3)){
    	   sLine[i] = sBuf[i++];
    }
    
    pStr= NULL;
    if (sBuf[i]=='\n')
    	pStr=(char*)(sBuf+i+1);
       
    if (sLine[i-1]!='\r')
    	sLine[i++]='\r';
    sLine[i++] = '\n';  //make sure that the sline is end with '\r\n';
    sLine[i] = '\0';
    
    return pStr;
    
}

/**********************************************************************************
Function:     输出邮件内容
input:        iSocket
	      sContenet--邮件内容	
output:       sLine
return        sbuf
lastmodify:   1999.11.17
written by    Dennis
**********************************************************************************/
void OutMailContent(int iSocket,char *sFrom,char* sTo,char *sCc,int iIsHTML,char *sSubject,char *sContent)
{
  char *psContent;
  char sLine[TLIB_MAIL_SMTP_LINE_LENGTH];
  
  psContent=sContent;
  snprintf(sLine,TLIB_MAIL_SMTP_LINE_LENGTH-1,"%s%s\r\n","Subject:",sSubject);
  TLib_Sock_Puts(iSocket,sLine);
  if (Smtp_Debug)
	printf(sLine);

  if (sFrom)
  {
  	snprintf(sLine,TLIB_MAIL_SMTP_LINE_LENGTH-1,"%s%s\r\n","From:",sFrom);
  	TLib_Sock_Puts(iSocket,sLine);
  	if (Smtp_Debug)
		printf(sLine);
  }

  
  if (sTo)
  {
  	snprintf(sLine,TLIB_MAIL_SMTP_LINE_LENGTH-1,"%s%s\r\n","To:",sTo);
  	TLib_Sock_Puts(iSocket,sLine);
  	if (Smtp_Debug)
		printf(sLine);
  }
   	
  if (sCc)
  {
  	snprintf(sLine,TLIB_MAIL_SMTP_LINE_LENGTH-1,"%s%s\r\n","Cc:",sCc);
  	TLib_Sock_Puts(iSocket,sLine);
 	if (Smtp_Debug)
	 	printf(sLine);
  }

  if (iIsHTML)
  {
  	snprintf(sLine,TLIB_MAIL_SMTP_LINE_LENGTH-1,"Content-Type: text/html; charset=gb2312\r\n");
  	TLib_Sock_Puts(iSocket,sLine);
  	snprintf(sLine,TLIB_MAIL_SMTP_LINE_LENGTH-1,"Content-Transfer-Encoding: base64\r\n");
  	TLib_Sock_Puts(iSocket,sLine);
  	if (Smtp_Debug)
		printf(sLine);
  }
    	  	  
  
  TLib_Sock_Puts(iSocket,"\n");	
  while ((psContent!=NULL) && strcmp(psContent,""))
  {
  	psContent=GetLineFromBuf(psContent,sLine,1024);
  	if (strncmp(sLine,".\r\n",3)==0)
  		TLib_Sock_Puts(iSocket,".");
  	else	
  		TLib_Sock_Puts(iSocket,sLine);
  		
  	if (Smtp_Debug)
		printf(sLine);
  }
  
  snprintf(sLine,TLIB_MAIL_SMTP_LINE_LENGTH,".\r\n");
  TLib_Sock_Puts(iSocket,sLine);
  if (Smtp_Debug)
	printf(sLine);
  
  TLib_Sock_Gets(iSocket,sLine,TLIB_MAIL_SMTP_LINE_LENGTH);
  if (Smtp_Debug)
	printf("%s\n",sLine);
  
  snprintf(sLine,TLIB_MAIL_SMTP_LINE_LENGTH,"QUIT\r\n");
  TLib_Sock_Puts(iSocket,sLine);
  if (Smtp_Debug)
	printf(sLine);
}

/**********************************************************************************
Function:    :从一个字符串中取得以指定字符分隔分开的一个单词, 
input:        sLine,sWord
output:       sWord
return        sLine; 
lastmodify:   1999.11.17
written by    dy
**********************************************************************************/
static char *GetOneWordFromLine(char *sWord,char *sLine,char cFilter,int iMaxCount){  
    
    int i=0;        
    
    while ((sLine[i]!=cFilter) && (sLine[i]!='\0') && (i<iMaxCount)){
    	   sWord[i] = sLine[i++];
    }
    
    sWord[i] = '\0';
    
    if (sLine[i]==cFilter)      
    	return (char*)(sLine+i+sizeof(char));
    	
    return NULL;
}


#define  cSplidStr ';'

/**********************************************************************************
Function:      分隔收件人的地址
input:         sRepStr   :收件人地址;	       	       
	       sSplidStr :多个分隔符以分号隔开sContenet: 内容
output:        sLine
return         sbuf
lastmodify:    1999.11.17
written by     Dennis
**********************************************************************************/
void  GetRepList(char *sRepStr,TLIB_STR_STRING_LIST * sRepList,int *piCount)
{

	char *psStr;
	int  iCount=0;
	char sRep[TLIB_MAIL_ADDRESS_LENGTH];
	int  iLen;
	
	*piCount=0;
	if ((sRepStr==NULL) || !strcmp(sRepStr,""))
		return;
	
	psStr=sRepStr;
	while (psStr)
	{
		psStr=GetOneWordFromLine(sRep,psStr,cSplidStr,sizeof(sRep)-1);	
		if (strcmp(sRep,""))
		{
			TLib_Str_StringListAdd(sRepList,sRep);
			iCount++;
		}
	}
	
	*piCount=iCount;
}

static int iTimeOut=0;
sigjmp_buf env;

static void AlarmHandler()

{
	iTimeOut = 1;
	signal(SIGALRM, AlarmHandler);	
	siglongjmp(env, 1);

}

/**********************************************************************************
Function:      通过smtp端口发送邮件
input:         sFrom :发件人地址 :
	       sTo   :收件人地址,多个以分号隔开; 
	       sCC   :抄送人地址,多个以分号隔开;
	       sSubject: 主题
	       sContenet: 内容
output:        sErrMsg
return         0: success
	       <0 error
lastmodify:    2000.07.11
written by     Dennis
注意   	      :sTo,sFrom,sCc必须以分号;结束
**********************************************************************************/
int TLib_Mail_SendMailDef(char *sFrom,char *sTo,char *sCc, char *sSubject,int iIsHTML, char *sContent,char *sErrMsg)
{
	
	int iToCount=0;
	int iCcCount=0;
	int i;
	TLIB_STR_STRING_LIST *sToList;
	TLIB_STR_STRING_LIST *sCcList;
	
	char *sAddr;
	int iSocket=-1;
	int iRetCode;
	char sLine[TLIB_MAIL_SMTP_LINE_LENGTH];
	
	iRetCode=-1;
	
	sToList=TLib_Str_StringListCreate(); //收件人列表
	sCcList=TLib_Str_StringListCreate(); //抄送人列表
	
	//检查收件人地址
	if (sTo)
	{
		GetRepList(sTo,sToList,&iToCount);
		sAddr=TLib_Str_StringListGetFirst(sToList);
		for (i=0;i<iToCount;i++)
		{
			if (TLib_Mail_CheckAddress(sAddr) != 0)
			{
				snprintf(sErrMsg,1024,"Invalid mail address: %s(unrecognized char included)\n", sAddr);
				iRetCode=-2;
				goto leave;
			}
			sAddr=TLib_Str_StringListGetNext(sToList);
		}
	}
	
	if (sCc)
	{
		//检查抄送人列表
		GetRepList(sCc,sCcList,&iCcCount);
		sAddr=TLib_Str_StringListGetFirst(sCcList);	
		for (i=0;i<iCcCount;i++)
		{
			if (TLib_Mail_CheckAddress(sAddr) != 0)
			{
				snprintf(sErrMsg,1024,"Invalid mail address: %s(unrecognized char included)\n", sAddr);
				iRetCode=-2;
				goto leave;
			}
			sAddr=TLib_Str_StringListGetNext(sCcList);
		}
	}
	
	/*try to connect to smtp server */
	iTimeOut = 0;
	signal(SIGALRM, AlarmHandler);
	alarm(TLIB_MAIL_TCP_TIMEOUT);
	
	if (!sigsetjmp(env, 1))
	{
		
		iSocket = TLib_Sock_Make_Connection(sSmtpPort, SOCK_STREAM, sSmtpServer);
		if (iSocket==-1)
			goto leave;
			
		TLib_Sock_Gets(iSocket,sLine,sizeof(sLine)-1);
		if (Smtp_Debug)
			printf("%s\n",sLine);
			
		snprintf(sLine,TLIB_MAIL_SMTP_LINE_LENGTH,"%s%s","HELO ","tencent.com\r\n");
		iRetCode=OutSmtpCommand(iSocket,sLine,sErrMsg);
		if (iRetCode<0)
			goto leave;
			
  		snprintf(sLine,TLIB_MAIL_SMTP_LINE_LENGTH,"%s%s%s","MAIL FROM:<",sFrom,">\r\n");
  		iRetCode=OutSmtpCommand(iSocket,sLine,sErrMsg);
		if (iRetCode<0)
			goto leave;
		
  		
  		if (iToCount>0)
  			sAddr=TLib_Str_StringListGetFirst(sToList);
  		for (i=0;i<iToCount;i++)
		{
			snprintf(sLine,TLIB_MAIL_SMTP_LINE_LENGTH,"%s%s%s","RCPT TO:<",sAddr,">\r\n");
			iRetCode=OutSmtpCommand(iSocket,sLine,sErrMsg);
			if (iRetCode<0)
				goto leave;
			sAddr=TLib_Str_StringListGetNext(sToList);
		} 	
		
  		if (iCcCount>0)
  		   	sAddr=TLib_Str_StringListGetFirst(sCcList);
  		for (i=0;i<iCcCount;i++)
		{
			snprintf(sLine,TLIB_MAIL_SMTP_LINE_LENGTH,"%s%s%s","RCPT TO:<",sAddr,">\r\n");
			iRetCode=OutSmtpCommand(iSocket,sLine,sErrMsg);
			if (iRetCode<0)
				goto leave;
			sAddr=TLib_Str_StringListGetNext(sCcList);
		}
    	
       		
       		snprintf(sLine,TLIB_MAIL_SMTP_LINE_LENGTH,"DATA\r\n");
       		iRetCode=OutSmtpCommand(iSocket,sLine,sErrMsg);
       		if (iRetCode<0)
			goto leave;
	
		alarm(0);
		//out mail data
       		alarm(TLIB_MAIL_TCP_TIMEOUT);
       		OutMailContent(iSocket,sFrom,sTo,sCc,iIsHTML,sSubject,sContent);
       		iRetCode=0;
       	}
       	
leave:	
       alarm(0);
       TLib_Str_StringListFree(sToList);
       TLib_Str_StringListFree(sCcList);	       
       if (iSocket!=-1)
       		close(iSocket);
       		
       return iRetCode;
}

int TLib_Mail_SendMail2(char *sFrom,char *sTo,char *sCc, char *sSubject,char *sContent,char *sErrMsg)
{
	return TLib_Mail_SendMailDef(sFrom,sTo,sCc, sSubject,0,sContent,sErrMsg);
}

int TLib_Mail_SendMail_Html(char *sFrom,char *sTo,char *sCc, char *sSubject,char *sContent,char *sErrMsg)
{
	return TLib_Mail_SendMailDef(sFrom,sTo,sCc, sSubject,1,sContent,sErrMsg);
}

#endif

