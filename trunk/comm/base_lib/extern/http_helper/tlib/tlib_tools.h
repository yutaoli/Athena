#ifndef _TLIB_TOOLS_H_
#define _TLIB_TOOLS_H_

#include <time.h>

char *TLib_Tools_GetDateTimeStr(time_t *mytime);
char *TLib_Tools_GetDateTimeStrShort(time_t *mytime);
char *TLib_Tools_GetCurDateTimeStr(void);
char *TLib_Tools_GetCurDateTimeStrShort(void);

char *TLib_Tools_GetTimeStr(time_t *mytime);
char *TLib_Tools_GetTimeStrShort(time_t *mytime);
char *TLib_Tools_GetCurTimeStr(void);
char *TLib_Tools_GetCurTimeStrShort(void);

char *TLib_Tools_GetDateStr(time_t *mytime);
char *TLib_Tools_GetDateStrShort(time_t *mytime);
char *TLib_Tools_GetCurDateStr(void);
char *TLib_Tools_GetCurDateStrShort(void);

time_t TLib_Tools_GetDateTime(char* sDateTime);

char *TLib_Tools_GetToken(char *sToken, int iTokenSize, char *sString, char *sSeparator);

//可逆的字符串加密,解密 
void TLib_Tools_Encrypt_Reversible(char *sSrc,char *sDest); 
void TLib_Tools_Decrypt_Reversible(char *sSrc,char *sDest);       

/*
//不可逆的字符串加密,比较
char *TLib_Tools_Encrypt_UnReversible(char *sSrc); 
int  TLib_Tools_CheckPasswd(char *sSrcPasswd,char *sCryptPasswd);      
char *TLib_Tools_ExtractFileName(char *psPath);
*/    
#endif

