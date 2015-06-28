#ifndef _TLIB_CGI_H_
#define _TLIB_CGI_H_

typedef struct _TLIB_CGI_CLIENT_INFO
{
	char sRemoteAddress[50];
	char sRemotePort[10];
} TLIB_CGI_CLIENT_INFO;


void TLib_Cgi_ReInit(void);
void TLib_Cgi_Init(void);
char *TLib_Cgi_Value(char *sParam);
char* TLib_Cgi_Cookie(char *key);
char **TLib_Cgi_Value_Multi(char *sParam);

void TLib_Cgi_GetClientInfo(TLIB_CGI_CLIENT_INFO *pstHostInfo);
void TLib_Cgi_SetUploadOption(char *sPath, int iMaxSize);
int TLib_Cgi_IsUploadOverSize(void);
int TLib_Cgi_ResetOverSize();

void TLib_Cgi_FreeEntries();

int TLib_Cgi_GetFormValue(unsigned char* sBuf, int *iLen, char *sName, char *sValue);
unsigned char* TLib_Cgi_GetPostValue(int *iLen);
#endif

