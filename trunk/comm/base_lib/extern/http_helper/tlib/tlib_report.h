#ifndef _TLIB_REPORT_H_
#define _TLIB_REPORT_H_

#define REP_INT 0
#define REP_STR 1

int TLib_Report_Open(const char* sRootPath, const char* sSys, const char* sModule, const char* sHost, const char* sDate);
void TLib_Report_Close(int iReportHandle);
int TLib_Report_Go(int iReportHandle, int iHour, int iMinute);
int TLib_Report_Load(int iReportHandle);
int TLib_Report_Replace(int iReportHandle, const char* sName, const char* sValue);
int TLib_Report_ReplaceF(int iReportHandle, const char* sName, const char* sFormat, ...);
int TLib_Report_Commit(int iReportHandle);
int TLib_Report_Write(int iReportHandle, int iHour, int iMinute, ...);

#endif
