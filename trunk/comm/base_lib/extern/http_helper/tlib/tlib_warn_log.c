#ifndef _TLIB_WARN_LOG_C_
#define _TLIB_WARN_LOG_C_

#include "tlib_com.h"
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include "tlib_warn_log.h"

static char sWarn_LogBaseName[200];
static long lWarn_MaxLogSize;
static int iWarn_MaxLogNum;
static int iWarn_LogInitialized = 0;
static int iWarn_IsShow;
static char sHeadMessage[1024];

int ShiftFiles(char *sLogBaseName, long lMaxLogSize, int iMaxLogNum, char *sErrMsg)
{
   struct stat stStat;
	char sLogFileName[300];
	char sNewLogFileName[300];
	int i;

   snprintf(sLogFileName,sizeof(sLogFileName),"%s.log", sLogBaseName);

	if(stat(sLogFileName, &stStat) < 0)
	{
		if (sErrMsg != NULL)
		{
			snprintf(sErrMsg,1024, "Fail to get file status");
		}
		return -1;
	}

	if (stStat.st_size < lMaxLogSize)
	{
		return 0;
	}

	snprintf(sLogFileName,sizeof(sLogFileName),"%s%d.log", sLogBaseName, iMaxLogNum-1);
	if (access(sLogFileName, F_OK) == 0)
	{
		if (remove(sLogFileName) < 0 )
		{
			if (sErrMsg != NULL)
			{
				snprintf(sErrMsg,1024, "Fail to remove oldest log file");
			}
			return -1;
		}
	}

	for(i = iMaxLogNum-2; i >= 0; i--)
	{
		if (i == 0)
			snprintf(sLogFileName,sizeof(sLogFileName),"%s.log", sLogBaseName);
		else
			snprintf(sLogFileName,sizeof(sLogFileName),"%s%d.log", sLogBaseName, i);
			
		if (access(sLogFileName, F_OK) == 0)
		{
			snprintf(sNewLogFileName,128,"%s%d.log", sLogBaseName, i+1);
			if (rename(sLogFileName,sNewLogFileName) < 0 )
			{
				if (sErrMsg != NULL)
				{
					snprintf(sErrMsg,1024, "Fail to remove oldest log file");
				}
				return -1;
			}
		}
	}
	return 0;
}

int TLib_Warn_Log_VWriteLog(char *sLogBaseName, long lMaxLogSize, int iMaxLogNum, char *sErrMsg, const char *sFormat, va_list ap)
{

	FILE  *pstFile;
	char sLogFileName[300];

                  snprintf(sLogFileName,sizeof(sLogFileName),"%s.log", sLogBaseName);
	if ((pstFile = fopen(sLogFileName, "a+")) == NULL)
	{
		if (sErrMsg != NULL)
		{
			snprintf(sErrMsg,1024, "Fail to open log file");
		}
		return -1;
	}

	fprintf(pstFile, "%s\t", TLib_Tools_GetCurDateTimeStr());
	fprintf(pstFile,"%s\t",sHeadMessage);
	vfprintf(pstFile, sFormat, ap);
	
	fprintf(pstFile, "\n");

	fclose(pstFile);

	return ShiftFiles(sLogBaseName, lMaxLogSize, iMaxLogNum, sErrMsg);
}


int TLib_Warn_Log_WriteLog(char *sLogBaseName, long lMaxLogSize, int iMaxLogNum, char *sErrMsg, const char *sFormat, ...)
{
	int iRetCode;
	va_list ap;
	va_start(ap, sFormat);
	iRetCode = TLib_Warn_Log_VWriteLog(sLogBaseName, lMaxLogSize, iMaxLogNum, sErrMsg, sFormat, ap);
	va_end(ap);
	return iRetCode;
}



int TLib_Warn_Log_Init(const char* s_sLogPath,const char* sBaseFileName,char *sHostName,char *sSystemName,char *sModuleName)
{
	char sLogFileName[256];
	snprintf(sLogFileName,sizeof(sLogFileName), "%s/%s", s_sLogPath, sBaseFileName);
	TLib_Warn_Log_LogInit(sLogFileName, 10000000, 5, 0);
	snprintf(sHeadMessage,1024,"%s\t%s\t%s",sHostName,sSystemName,sModuleName);
	return 0;
}

void TLib_Warn_Log_LogInit(char *sPLogBaseName, long lPMaxLogSize, int iPMaxLogNum, int iShow)
{
	memset(sWarn_LogBaseName, 0, sizeof(sWarn_LogBaseName));
	strncpy(sWarn_LogBaseName, sPLogBaseName, sizeof(sWarn_LogBaseName)-1);
	lWarn_MaxLogSize = lPMaxLogSize;
	iWarn_MaxLogNum = iPMaxLogNum;
	iWarn_IsShow = iShow;
	iWarn_LogInitialized = 1;
}

void TLib_Warn_Log_LogMsgDef(int iShow, const char *sFormat, ...)
{
	va_list ap;
	
	if (iShow != 0)
	{
		printf("[%s] ", TLib_Tools_GetCurDateTimeStr());
	
		va_start(ap, sFormat);
		vprintf(sFormat, ap);
		va_end(ap);

		printf("\n");
	}

	if (iWarn_LogInitialized != 0)
	{
		va_start(ap, sFormat);
		TLib_Warn_Log_VWriteLog(sWarn_LogBaseName, lWarn_MaxLogSize, iWarn_MaxLogNum, NULL, sFormat, ap);
		va_end(ap);
	}
}

void TLib_Warn_Log_LogMsg(const char *sFormat, ...)
{
	va_list ap;
	
	if (iWarn_IsShow != 0)
	{
		printf("[%s] ", TLib_Tools_GetCurDateTimeStr());
	
		va_start(ap, sFormat);
		vprintf(sFormat, ap);
		va_end(ap);

		printf("\n");
	}

	if (iWarn_LogInitialized != 0)
	{
		va_start(ap, sFormat);
		TLib_Warn_Log_VWriteLog(sWarn_LogBaseName, lWarn_MaxLogSize, iWarn_MaxLogNum, NULL, sFormat, ap);
		va_end(ap);
	}
}

#endif

