#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "tlib_report.h"
#include "tlib_dbf.h"

#define COMMAND_LEN_LIMIT 1023
#define REPORT_FILE_SIZE_LIMIT 255
#define REPORT_NAME_SIZE_LIMIT 255
#define REPORT_VALUE_SIZE_LIMIT 255

int TLib_Report_Open(const char* sRootPath, const char* sSys, const char* sModule, const char* sHost, const char* sDate)
{
	char sReportFile[REPORT_FILE_SIZE_LIMIT+1], sTemplateFile[REPORT_FILE_SIZE_LIMIT+1];
	char sCommand[COMMAND_LEN_LIMIT+1];
	int iHandle, iRet, i;
	
	snprintf(sReportFile, REPORT_FILE_SIZE_LIMIT, "%s/%s/%s-%s-%s.dbf", sRootPath, sSys, sModule, sHost, sDate);
	sReportFile[REPORT_FILE_SIZE_LIMIT] = 0;
	snprintf(sTemplateFile, REPORT_FILE_SIZE_LIMIT, "%s/%s/%s-%s.dbf.emp", sRootPath, sSys, sModule, sHost);
	sTemplateFile[REPORT_FILE_SIZE_LIMIT] = 0;
	if (access(sReportFile, F_OK) < 0)
	{
		snprintf(sCommand, sizeof(sCommand), "/bin/cp %s %s", sTemplateFile, sReportFile);
		if (system(sCommand))
			return -100;
	}
	
	if ((iHandle = TLib_Dbf_Open(sReportFile, TLIB_DBF_OPEN_MODE_WRITE)) < 0)
		return iHandle;
		
	if (TLib_Dbf_RecordCount(iHandle) == 0)
	{
		for (i = 0; i < 60*24; i++)
		{
			if ((iRet = TLib_Dbf_Go(iHandle, i+1)) != 0)
				return iRet;
				
			if ((iRet = TLib_Dbf_ClearRecordBuffer(iHandle)) != 0)
				return iRet;
			
			if ((iRet = TLib_Dbf_ReplaceFieldF(iHandle, "time", "%d:%d", i / 60, i % 60)) != 0)
				return iRet;

			if ((iRet = TLib_Dbf_CommitRecord(iHandle)) != 0)
				return iRet;
		}
	}
	
	return iHandle;
}


void TLib_Report_Close(int iReportHandle)
{
	TLib_Dbf_Close(iReportHandle);
}


int TLib_Report_Go(int iReportHandle, int iHour, int iMinute)
{
	int iRecNo = iHour * 60 + iMinute + 1;
	
	return TLib_Dbf_Go(iReportHandle, iRecNo);
}


int TLib_Report_Load(int iReportHandle)
{
	return TLib_Dbf_LoadRecord(iReportHandle);
}


int TLib_Report_Replace(int iReportHandle, const char* sName, const char* sValue)
{
	return TLib_Dbf_ReplaceField(iReportHandle, (char*)sName, (char*)sValue);
}


int TLib_Report_ReplaceF(int iReportHandle, const char* sName, const char* sFormat, ...)
{
	va_list ap;
	char sValue[REPORT_VALUE_SIZE_LIMIT+1];
	
	va_start(ap, sFormat);
	vsnprintf(sValue, REPORT_VALUE_SIZE_LIMIT, sFormat, ap);
	va_end(ap);
	
	return TLib_Report_Replace(iReportHandle, sName, sValue);
}


int TLib_Report_Commit(int iReportHandle)
{
	return TLib_Dbf_CommitRecord(iReportHandle);
}


int TLib_Report_Write(int iReportHandle, int iHour, int iMinute, ...)
{
	va_list ap;
	char *sName, *sValue;
	int iType, iValue;
	int iRet;
	
	if ((iRet = TLib_Report_Go(iReportHandle, iHour, iMinute)) != 0)
		return iRet;
		
	if ((iRet = TLib_Report_Load(iReportHandle)) != 0)
		return iRet;
		
	va_start(ap, iMinute);
	sName = va_arg(ap, char*);
	while (sName)
	{
		iType = va_arg(ap, int);
		switch (iType)
		{
			case REP_INT:
				iValue = va_arg(ap, int);
				TLib_Report_ReplaceF(iReportHandle, sName, "%d", iValue);
				break;
				
			case REP_STR:
				sValue = va_arg(ap, char*);
				TLib_Report_Replace(iReportHandle, sName, sValue);
				break;
		}
		
		sName = va_arg(ap, char*);
	}
	
	va_end(ap);
	
	if ((iRet = TLib_Report_Go(iReportHandle, iHour, iMinute)) != 0)
		return iRet;
		
	return TLib_Report_Commit(iReportHandle);
}

