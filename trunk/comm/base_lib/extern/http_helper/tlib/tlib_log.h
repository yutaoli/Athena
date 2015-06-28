#ifndef _TLIB_LOG_H_
#define _TLIB_LOG_H_

#include <stdarg.h>

int TLib_Log_VWriteLog(char *sLogBaseName, long lMaxLogSize, int iMaxLogNum, char *sErrMsg, const char *sFormat, va_list ap);
int TLib_Log_WriteLog(char *sLogBaseName, long lMaxLogSize, int iMaxLogNum, char *sErrMsg, const char *Format, ...);

void TLib_Log_LogMsg(const char *sFormat, ...);
void TLib_Log_LogMsgDef(int iShow, const char *sFormat, ...);
void TLib_Log_LogInit(char *sPLogBaseName, long lPMaxLogSize, int iPMaxLogNum, int iShow);

#endif

