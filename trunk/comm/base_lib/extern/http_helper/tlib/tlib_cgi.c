#ifndef _TLIB_CGI_C_
#define _TLIB_CGI_C_

#include "tlib_com.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "cgi-lib.h"
#include "cgi-llist.h"
#include "html-lib.h"
#include "string-lib.h"

#include "tlib_cgi.h"

static int iTLib_Cgi_Initialized = 0;
static llist stTLib_Cgi_Entries;
static llist stTLib_Cookie_Entries;
	
void TLib_Cgi_ReInit(void)
{
	iTLib_Cgi_Initialized = 0;
}

void TLib_Cgi_Init(void)
{
	if (iTLib_Cgi_Initialized != 0)
	{
		return;
	}
	
	list_create(&stTLib_Cgi_Entries);
	list_create(&stTLib_Cookie_Entries);

	iTLib_Cgi_Initialized = 1;

	read_cgi_input(&stTLib_Cgi_Entries);
	parse_cookies(&stTLib_Cookie_Entries);
}

char *TLib_Cgi_Value(char *sParam)
{
	char *p;

	TLib_Cgi_Init();
	
	p = cgi_val(stTLib_Cgi_Entries, sParam);
	if (p == NULL)
		return "";
	else
		return p;
}

char* TLib_Cgi_Cookie(char *key)
{
	char* p;
	
	TLib_Cgi_Init();

	p = cgi_val(stTLib_Cookie_Entries,key);
	if(p == NULL)
		return "";
	else
		return p;
}

char **TLib_Cgi_Value_Multi(char *sParam)
{
	char **p;

	TLib_Cgi_Init();
	
	p = cgi_val_multi(stTLib_Cgi_Entries, sParam);
	
	return p;
}

void TLib_Cgi_GetClientInfo(TLIB_CGI_CLIENT_INFO *pstHostInfo)
{
	memset(pstHostInfo, 0, sizeof(TLIB_CGI_CLIENT_INFO));
	if (getenv("REMOTE_ADDR") != NULL)
	{
		strncpy(pstHostInfo->sRemoteAddress, getenv("REMOTE_ADDR"),strlen(getenv("REMOTE_ADDR"))+1);
	}
	if (getenv("REMOTE_PORT") != NULL)
	{
		strncpy(pstHostInfo->sRemotePort, getenv("REMOTE_PORT"),strlen(getenv("REMOTE_ADDR"))+1);
	}
}

void TLib_Cgi_SetUploadOption(char *sPath, int iMaxSize)
{
	sUploadFilePath = sPath;
	iUploadFileMaxSize = iMaxSize;
}

int TLib_Cgi_IsUploadOverSize(void)
{
	return iUploadFileOverSize;
}

int TLib_Cgi_ResetOverSize()
{
	iUploadFileOverSize = 0;
	return 0;	
}

void TLib_Cgi_FreeEntries()
{
	list_clear(&stTLib_Cgi_Entries);
	list_clear(&stTLib_Cookie_Entries);
	iTLib_Cgi_Initialized = 0;
}

//获取包
unsigned char* TLib_Cgi_GetPostValue(int *iLen)
{
  char *buffer;
  if (CONTENT_LENGTH != NULL) {
  	
    *iLen = atoi(CONTENT_LENGTH);
    buffer = (unsigned char *)malloc(sizeof(char) * (*iLen) + 1);
    if (fread(buffer,sizeof(char),*iLen,stdin) != *iLen) {
    	free(buffer);
    	return NULL;
    }
    buffer[*iLen] = '\0';
  }
  return buffer;
}

//获取文件流
int TLib_Cgi_GetFormValue(unsigned char* sBuf, int *iLen, char *sName, char *sValue)
{
	long content_length;
	char *tempstr, *boundary;
	char *buffer = (char *)malloc(sizeof(char) * BUFSIZ + 1);
	short done;
	int i,j;
	int bytesread;
	int iTotalLen;
	if (CONTENT_LENGTH != NULL)
		content_length = atol(CONTENT_LENGTH);
	else
		return 0;
  /* get boundary */
	tempstr = newstr(CONTENT_TYPE);
	boundary = strstr(tempstr,"boundary=");
	boundary += (sizeof(char) * 9);
  /* ignore first boundary; this isn't so robust; improve it later */
	getline(buffer,BUFSIZ);
  /* now start parsing */
	if ((bytesread=getline(buffer,BUFSIZ)) == 0) return -1;
	buffer[bytesread] = '\0';
	tempstr = newstr(buffer);
	tempstr += (sizeof(char) * 38); /* 38 is header up to name */
	sName = tempstr;
	sValue = (char *)malloc(sizeof(char) * BUFSIZ + 1);
	sValue[0]=0;
	while (*tempstr != '"')
	tempstr++;
	*tempstr = '\0';
	if (strstr(buffer,"filename=\"") != NULL) {
		tempstr = newstr(buffer);
		tempstr =strstr(tempstr,"filename=\"");
		tempstr += (sizeof(char) * 10);
		if (strlen(tempstr) >= BUFSIZ)
			sValue = (char *) realloc(sValue, sizeof(char) * strlen(tempstr)+1);
		sValue = tempstr;
		while (*tempstr != '"')
			tempstr++;
		*tempstr = '\0';
	}
	while (getline(buffer, BUFSIZ) > 1) {
		/* DOS style blank line? */
			if ((buffer[0] == '\r') && (buffer[1] == '\n'))
			break;
	}
	iTotalLen = *iLen;
	*iLen = 0;
	j = 0;
	done = 0;
	while (!done) {
		bytesread = getline(buffer,BUFSIZ);
		buffer[bytesread] = '\0';
		if (bytesread && strstr(buffer,boundary) == NULL) 
		{
			i = 0;
			for (i=0;i < bytesread; i++)
			{
				if ((j>iTotalLen) && (iTotalLen!=0))
					break;
				sBuf[j] = buffer[i];
				j++;
			}
		}
		else
		{
			*iLen = j-2;
			done = 1;
		}
	}
	return 0;

}

#endif

