#ifndef _TLIB_LOG_C_
#define _TLIB_LOG_C_

#include "tlib_com.h"
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include "tlib_log.h"

static int ShiftFiles(char *sLogBaseName, long lMaxLogSize, int iMaxLogNum, char *sErrMsg)
{
   struct stat stStat;
	char sLogFileName[300];
	char sNewLogFileName[300];
	int i;

   snprintf(sLogFileName,300,"%s.log", sLogBaseName);

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

	snprintf(sLogFileName,300,"%s%d.log", sLogBaseName, iMaxLogNum-1);
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
			snprintf(sLogFileName,300,"%s.log", sLogBaseName);
		else
			snprintf(sLogFileName,300,"%s%d.log", sLogBaseName, i);
			
		if (access(sLogFileName, F_OK) == 0)
		{
			snprintf(sNewLogFileName,300,"%s%d.log", sLogBaseName, i+1);
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

int TLib_Log_VWriteLog(char *sLogBaseName, long lMaxLogSize, int iMaxLogNum, char *sErrMsg, const char *sFormat, va_list ap)
{
	FILE  *pstFile;
	char sLogFileName[300];

   snprintf(sLogFileName,300,"%s.log", sLogBaseName);
	if ((pstFile = fopen(sLogFileName, "a+")) == NULL)
	{
		if (sErrMsg != NULL)
		{
			snprintf(sErrMsg,1024, "Fail to open log file");
		}
		return -1;
	}

	fprintf(pstFile, "[%s] ", TLib_Tools_GetCurDateTimeStr());
	
	vfprintf(pstFile, sFormat, ap);
	
	fprintf(pstFile, "\n");

	fclose(pstFile);

	return ShiftFiles(sLogBaseName, lMaxLogSize, iMaxLogNum, sErrMsg);
}

int TLib_Log_WriteLog(char *sLogBaseName, long lMaxLogSize, int iMaxLogNum, char *sErrMsg, const char *sFormat, ...)
{
	int iRetCode;
	va_list ap;
	
	va_start(ap, sFormat);
	iRetCode = TLib_Log_VWriteLog(sLogBaseName, lMaxLogSize, iMaxLogNum, sErrMsg, sFormat, ap);
	va_end(ap);

	return iRetCode;
}

static char sLogBaseName[200];
static long lMaxLogSize;
static int iMaxLogNum;
static int iLogInitialized = 0;
static int iIsShow;

void TLib_Log_LogInit(char *sPLogBaseName, long lPMaxLogSize, int iPMaxLogNum, int iShow)
{
	memset(sLogBaseName, 0, sizeof(sLogBaseName));
	strncpy(sLogBaseName, sPLogBaseName, sizeof(sLogBaseName)-1);
	lMaxLogSize = lPMaxLogSize;
	iMaxLogNum = iPMaxLogNum;
	iIsShow = iShow;
	iLogInitialized = 1;
}

void TLib_Log_LogMsgDef(int iShow, const char *sFormat, ...)
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

	if (iLogInitialized != 0)
	{
		va_start(ap, sFormat);
		TLib_Log_VWriteLog(sLogBaseName, lMaxLogSize, iMaxLogNum, NULL, sFormat, ap);
		va_end(ap);
	}
}

void TLib_Log_LogMsg(const char *sFormat, ...)
{
	va_list ap;
	
	if (iIsShow != 0)
	{
		printf("[%s] ", TLib_Tools_GetCurDateTimeStr());
	
		va_start(ap, sFormat);
		vprintf(sFormat, ap);
		va_end(ap);

		printf("\n");
	}

	if (iLogInitialized != 0)
	{
		va_start(ap, sFormat);
		TLib_Log_VWriteLog(sLogBaseName, lMaxLogSize, iMaxLogNum, NULL, sFormat, ap);
		va_end(ap);
	}
}

#endif

