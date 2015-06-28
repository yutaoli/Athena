#ifndef _TLIB_TOOLS_C_
#define _TLIB_TOOLS_C_

#include "tlib_com.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "tlib_tools.h"

#define _XOPEN_SOURCE
#include <unistd.h>


//char *crypt(const char *key, const char *salt);

char *TLib_Tools_GetDateTimeStr(time_t *mytime)
{
	static char s[50];
	struct tm curr;
	
	curr = *localtime(mytime);

	if (curr.tm_year > 50)
	{
		snprintf(s,50, "%04d-%02d-%02d %02d:%02d:%02d", 
					curr.tm_year+1900, curr.tm_mon+1, curr.tm_mday,
					curr.tm_hour, curr.tm_min, curr.tm_sec);
	}
	else
	{
		snprintf(s,50, "%04d-%02d-%02d %02d:%02d:%02d", 
					curr.tm_year+2000, curr.tm_mon+1, curr.tm_mday,
					curr.tm_hour, curr.tm_min, curr.tm_sec);
	}
				
	return s;
}

char *TLib_Tools_GetDateTimeStrShort(time_t *mytime)
{
	static char s[50];
	struct tm curr;
	
	curr = *localtime(mytime);

	if (curr.tm_year > 50)
	{
		snprintf(s,50, "%04d%02d%02d%02d%02d%02d", 
					curr.tm_year+1900, curr.tm_mon+1, curr.tm_mday,
					curr.tm_hour, curr.tm_min, curr.tm_sec);
	}
	else
	{
		snprintf(s,50, "%04d%02d%02d%02d%02d%02d", 
					curr.tm_year+2000, curr.tm_mon+1, curr.tm_mday,
					curr.tm_hour, curr.tm_min, curr.tm_sec);
	}
				
	return s;
}

char *TLib_Tools_GetCurDateTimeStr(void)
{
	time_t	iCurTime;

	time(&iCurTime);
	return TLib_Tools_GetDateTimeStr(&iCurTime);
}

char *TLib_Tools_GetCurDateTimeStrShort(void)
{
	time_t	iCurTime;

	time(&iCurTime);
	return TLib_Tools_GetDateTimeStrShort(&iCurTime);
}

///////////////////////////////////////////////////////////////////

char *TLib_Tools_GetTimeStr(time_t *mytime)
{
	static char s[50];
	struct tm curr;
	
	curr = *localtime(mytime);

	snprintf(s,50, "%02d:%02d:%02d", 
				curr.tm_hour, curr.tm_min, curr.tm_sec);
				
	return s;
}

char *TLib_Tools_GetTimeStrShort(time_t *mytime)
{
	static char s[50];
	struct tm curr;
	
	curr = *localtime(mytime);

	snprintf(s,50, "%02d%02d%02d", 
				curr.tm_hour, curr.tm_min, curr.tm_sec);
				
	return s;
}

char *TLib_Tools_GetCurTimeStr(void)
{
	time_t	iCurTime;

	time(&iCurTime);
	return TLib_Tools_GetTimeStr(&iCurTime);
}

char *TLib_Tools_GetCurTimeStrShort(void)
{
	time_t	iCurTime;

	time(&iCurTime);
	return TLib_Tools_GetTimeStrShort(&iCurTime);
}

//////////////////////////////////////////////////////////////

char *TLib_Tools_GetDateStr(time_t *mytime)
{
	static char s[50];
	struct tm curr;
	
	curr = *localtime(mytime);

	if (curr.tm_year > 50)
	{
		snprintf(s,50, "%04d-%02d-%02d", 
					curr.tm_year+1900, curr.tm_mon+1, curr.tm_mday);
	}
	else
	{
		snprintf(s,50, "%04d-%02d-%02d", 
					curr.tm_year+2000, curr.tm_mon+1, curr.tm_mday);
	}		
	return s;
}

char *TLib_Tools_GetDateStrShort(time_t *mytime)
{
	static char s[50];
	struct tm curr;
	
	curr = *localtime(mytime);

	if (curr.tm_year > 50)
	{
		snprintf(s,50, "%04d%02d%02d", 
					curr.tm_year+1900, curr.tm_mon+1, curr.tm_mday);
	}
	else
	{
		snprintf(s,50, "%04d%02d%02d", 
					curr.tm_year+2000, curr.tm_mon+1, curr.tm_mday);
	}
				
	return s;
}

char *TLib_Tools_GetCurDateStr(void)
{
	time_t	iCurTime;

	time(&iCurTime);
	return TLib_Tools_GetDateStr(&iCurTime);
}

char *TLib_Tools_GetCurDateStrShort(void)
{
	time_t	iCurTime;

	time(&iCurTime);
	return TLib_Tools_GetDateStrShort(&iCurTime);
}

time_t TLib_Tools_GetDateTime(char* sDateTime)
{
	struct tm stDateTime;
	char str[5];
  
	if (strlen(sDateTime) != 14)
	  return 0;

  strncpy(str, &sDateTime[0], 4);
	str[4] = 0;
	stDateTime.tm_year = atoi(str) - 1900;

  strncpy(str, &sDateTime[4], 2);
	str[2] = 0;
	stDateTime.tm_mon = atoi(str) - 1;

  strncpy(str, &sDateTime[6], 2);
	str[2] = 0;
	stDateTime.tm_mday = atoi(str);

  strncpy(str, &sDateTime[8], 2);
	str[2] = 0;
	stDateTime.tm_hour = atoi(str);

  strncpy(str, &sDateTime[10], 2);
	str[2] = 0;
	stDateTime.tm_min = atoi(str);

  strncpy(str, &sDateTime[12], 2);
	str[2] = 0;
	stDateTime.tm_sec = atoi(str);

	return mktime(&stDateTime);
}

char *TLib_Tools_GetToken(char *sToken, int iTokenSize, char *sString, char *sSeparator)
{
	char *pe;
	int iBytesToCopy;
	int i;

	pe = strstr(sString, sSeparator);
	if (pe == NULL)
	{
		if (sToken != NULL)
		{
			strncpy(sToken, sString,iTokenSize-1);
		}
		return sString + strlen(sString);
	}
	else
	{
		if (sToken != NULL)
		{
			iBytesToCopy = pe - sString;
			if (iBytesToCopy > iTokenSize-1)
			{
				iBytesToCopy = iTokenSize-1;
			}
		
			for (i=0; i<iBytesToCopy; i++)
			{
				sToken[i] = sString[i];
			}
			sToken[iBytesToCopy] = 0;
		}
		
		return pe + strlen(sSeparator);
	}
}

/*******-****************************************************
Funciton    将俩个16#的字符组合成一个字符
input       first,second,
ouput       none
return      一个字符
lastmodify  1999.11.21
written by  Dennis
************************************************************/
static char x2c(char first,char second)
{
  register char digit;

  digit = (first >= 'A' ? (first - 'A')+10 : (first - '0'));
  digit *= 16;
  digit += (second >= 'A' ? (second  - 'A')+10 : (second - '0'));
  return(digit);
}

/*******-****************************************************
Funciton    将一个字符拆分俩个16#的字符表示
input       what :待拆分的字符
ouput       sDest;字符数组 
lastmodify  1999.11.21
written by  Dennis
************************************************************/
static void c2x(unsigned char what,char *sDest)
{
  char sHex[20]="0123456789ABCDEF";
  sDest[0]=sHex[what / 16];
  sDest[1]=sHex[what % 16];
}

#define  C1  52845
#define  C2  22719
#define  KEY_VALUE 12345

/*******-****************************************************
Funciton    加密字符串函数
input       sSrc :源字符串
ouput       sDest 加密后的字符串
lastmodify  1999.11.21
written by  Dennis
************************************************************/
void TLib_Tools_Encrypt_Reversible(char *  sSrc,char *sDest){
  int i;
  unsigned short key;
  char cStr;
  key=KEY_VALUE;		

  for (i=0; i<strlen(sSrc);i++){
     cStr = (char)(sSrc[i] ^ (key >> 8));
     key = (unsigned short)(cStr + key) * C1 + C2;
     c2x(cStr,&sDest[i*2]);
  }
  sDest[i*2+1]='\0';
}

/***********************************************************
Funciton    解密字符串函数
input       sSrc :源字符串
ouput       sDest 解密后的字符串
lastmodify  1999.11.21
written by  Dennis
************************************************************/
void TLib_Tools_Decrypt_Reversible(char * sSrc,char *sDest){
  int i,ilen;
  unsigned short key;
  char *psStr;
  psStr=malloc(sizeof(char)*strlen(sSrc) / 2 +1);
  for (i =0;i<strlen(sSrc) / 2;i++){
      psStr[i] = x2c(sSrc[i*2],sSrc[i*2+1]);
  }
  psStr[i]='\0';
  ilen=i;
  key=KEY_VALUE;
  for (i =0;i<ilen;i++){
      sDest[i] = (char)(psStr[i] ^ (key >> 8));
      key = (unsigned short)(psStr[i] + key) * C1 + C2;
  }
  sDest[i]='\0';
  free(psStr);
}


#endif

