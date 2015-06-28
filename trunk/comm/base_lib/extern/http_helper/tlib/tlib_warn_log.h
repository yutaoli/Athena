#ifndef _TLIB_WARN_LOG_H_
#define _TLIB_WARN_LOG_H_

#include <stdarg.h>
int ShiftFiles(char *sLogBaseName, long lMaxLogSize, int iMaxLogNum, char *sErrMsg);
int TLib_Warn_Log_Init(const char* s_sLogPath,const char* sBaseFileName,char *sHostName,char *sSystemName,char *sModuleName);
int TLib_Warn_Log_VWriteLog(char *sLogBaseName, long lMaxLogSize, int iMaxLogNum, char *sErrMsg, const char *sFormat, va_list ap);
int TLib_Warn_Log_WriteLog(char *sLogBaseName, long lMaxLogSize, int iMaxLogNum, char *sErrMsg, const char *Format, ...);
void TLib_Warn_Log_LogMsg(const char *sFormat, ...);
void TLib_Warn_Log_LogMsgDef(int iShow, const char *sFormat, ...);
void TLib_Warn_Log_LogInit(char *sPLogBaseName, long lPMaxLogSize, int iPMaxLogNum, int iShow);

#endif

