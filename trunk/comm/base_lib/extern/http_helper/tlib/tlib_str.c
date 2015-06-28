#ifndef _TLIB_STR_C_
#define _TLIB_STR_C_

#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include	<stdlib.h>
#include	<ctype.h>
#include <stdarg.h>

#include "tlib_com.h"
#include	"tlib_str.h"

TLIB_STR_STRING_LIST *TLib_Str_StringListCreate(void)
{
	TLIB_STR_STRING_LIST *pstStringList;
	
	pstStringList = malloc(sizeof(TLIB_STR_STRING_LIST));
	memset(pstStringList, 0, sizeof(TLIB_STR_STRING_LIST));
	return pstStringList;
}

int TLib_Str_StringListAdd(TLIB_STR_STRING_LIST *pstStringList, char *sItem)
{
	int iLen;
	TLIB_STR_STRING_LIST_ITEM *pstStringListItem;
	
	if ((pstStringList == NULL) || (sItem == NULL))
	{
		return TLIB_STR_STRING_LIST_ERROR_INVALID_POINTER;
	}
	
	pstStringListItem = malloc(sizeof(TLIB_STR_STRING_LIST_ITEM));
	if (pstStringListItem == NULL)
	{
		return TLIB_STR_STRING_LIST_ERROR_FAIL_TO_ALLOC_MEM;
	}
	
	pstStringListItem->sItem = malloc(strlen(sItem)+1);
	if (pstStringListItem->sItem == NULL)
	{
		free(pstStringListItem);
		return TLIB_STR_STRING_LIST_ERROR_FAIL_TO_ALLOC_MEM;
	}
	strncpy(pstStringListItem->sItem, sItem,strlen(sItem)+1);
	
	if (pstStringList->pstItem == NULL)
	{
		pstStringList->pstItem = pstStringListItem;
	}
	if (pstStringList->pstItemTail != NULL)
	{
		pstStringList->pstItemTail->pstNext = pstStringListItem;
	}
	pstStringList->pstItemTail = pstStringListItem;
	pstStringListItem->pstNext = NULL;
	
	return 0;
}

void TLib_Str_StringListFree(TLIB_STR_STRING_LIST *pstStringList)
{
	TLIB_STR_STRING_LIST_ITEM *pstStringListItem;
	
	if (pstStringList == NULL)
	{
		return;
	}
	
	pstStringListItem = pstStringList->pstItem;
	while (pstStringListItem != NULL)
	{
		pstStringList->pstItem = pstStringList->pstItem->pstNext;
		free(pstStringListItem->sItem);
		free(pstStringListItem);
		pstStringListItem = pstStringList->pstItem;
	}
	free(pstStringList);
	return;
}

char *TLib_Str_StringListGetFirst(TLIB_STR_STRING_LIST *pstStringList)
{
	if (pstStringList == NULL)
	{
		return NULL;
	}
	
	pstStringList->pstItemCur = pstStringList->pstItem;
	if (pstStringList->pstItemCur == NULL)
	{
		return NULL;
	}
	
	return pstStringList->pstItemCur->sItem;
}
	
char *TLib_Str_StringListGetNext(TLIB_STR_STRING_LIST *pstStringList)
{
	if ((pstStringList == NULL) || (pstStringList->pstItemCur == NULL))
	{
		return NULL;
	}
	
	pstStringList->pstItemCur = pstStringList->pstItemCur->pstNext;
	if (pstStringList->pstItemCur == NULL)
	{
		return NULL;
	}
	
	return pstStringList->pstItemCur->sItem;
}

////////////////////////////////////////////////////////////////////

char *TLib_Str_Trim(char *s)
{
	char *pb;
	char *pe;
	char *ps;
	char *pd;

	if (strcmp(s, "") == 0) 
		return s;
	
	pb = s;
		 
	while (((*pb == ' ') || (*pb == '\t') || (*pb == '\n') || (*pb == '\r')) && (*pb != 0))
	{
		pb ++;
	}
	
	pe = s;
	while (*pe != 0)
	{
		pe ++;
	}
	pe --;
	while ((pe >= s) && ((*pe == ' ') || (*pe == '\t') || (*pe == '\n') || (*pe == '\r')))
	{
		pe --;
	}
	
	ps = pb;
	pd = s;
	while (ps <= pe)
	{
		*pd = *ps;
		ps ++;
		pd ++;
	}
	*pd = 0;
	
	return s;
}

char *TLib_Str_StrUppercase(char * s)
{
	char *p;
	
	p = s;
	while (*p != 0)
	{
		*p = toupper(*p);
		p++;
	}
		
	return s;
}

char *TLib_Str_StrLowercase(char * s)
{
	char *p;
	
	p = s;
	while (*p != 0)
	{
		*p = tolower(*p);
		p++;
	}
		
	return s;
}


int TLib_Str_StrCmp(char *s1, char *s2)
{
	char *p1;
	char *p2;
	
	if (s1==NULL) {
		if (s2==NULL)  return 0;
		if (*s2==0) 	return 0;
		return -1;
	}
	
	if (s2==NULL) {
		if (*s1==0) return 0;
		return 1;
	}
	
	
	p1 = s1;
	p2 = s2;
	
	while(1)
	{
		if (*p1 > *p2)
			return 1;
			
		if (*p1 < *p2)
			return -1;
			
		if (*p1 == 0)
			return 0;
			
		p1 ++;
		p2 ++;
	}
}

int TLib_Str_StrNCCmp(char *s1, char *s2)
{
	char *p1;
	char *p2;
	char c1, c2;

	if (s1==NULL) {
		if (s2==NULL)  return 0;
		if (*s2==0) 	return 0;
		return -1;
	}
	
	if (s2==NULL) {
		if (*s1==0) return 0;
		return 1;
	}
	
	p1 = s1;
	p2 = s2;
	
	while(1)
	{
		c1 = toupper(*p1);
		c2 = toupper(*p2);
		
		if (c1 > c2)
			return 1;
			
		if (c1 < c2)
			return -1;
			
		if (c1 == 0)
			return 0;
			
		p1 ++;
		p2 ++;
	}
}

char *TLib_Str_StrReplaceChar(char *pStrSrc, char cOld, char cNew)
{
	char *pChar;

	pChar = pStrSrc;
	while (*pChar)
	{
		if (*pChar == cOld)
		{
			*pChar = cNew;
		}
		pChar ++;
	}
	return pStrSrc;
}

char *TLib_Str_StrCat(char *s, const char *sFormat, ...)
{
	va_list ap;

	va_start(ap, sFormat);
	(void) vsnprintf(s+strlen(s),1024, sFormat, ap);
	va_end(ap);
	return (s);
}


char *TLib_Str_Strcpy( char *sOBJ, char *sSRC)
{
	if (sSRC==NULL) {
		*sOBJ = 0;
		return sOBJ;
	}
	return strncpy( sOBJ, sSRC,strlen(sSRC)+1);
}


char *TLib_Str_Strncpy( char *sOBJ, char *sSRC, int iMaxChar)
{
	if (sSRC==NULL) {
		*sOBJ = 0;
		return sOBJ;
	}
	return strncpy( sOBJ, sSRC, iMaxChar );
}

//add by pilot 2000-11-17
//return: 0正常 1含有非法字符 2没有@和. 3长度太短
int TLib_Str_Check_Email(char* sEmail)
{
  	int i;
  	char *p;
  	char *sCharSet="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890@_.";
  	
  	//cyril modified here for safe overlap.    2000-10-12
  	
  	p = (char*)malloc(strlen(sEmail) + 1);
  	strncpy(p,sEmail,strlen(sEmail)+1);

	i = 0;
	while (p[i]) {
		if (strchr(sCharSet,p[i])) i++;
		else {free(p);return 1;}
	}

	free(p);
	
	if (strchr(sEmail,'@')==NULL || strchr(sEmail,'.')==NULL) return 2;
	
	if (strlen(sEmail)<9) return 3;
	
	return 0;
}


char* TLib_Str_QuoteDB(char* dest,char* src)
{
  	int i, j;
  	char *p;
  	
  	//cyril modified here for safe overlap.    2000-10-12
  	
  	p = (char*)malloc(strlen(src) + 1);
  	strncpy(p,src,strlen(src)+1);

	i = j = 0;
	while (p[i]) {
		switch((unsigned char)p[i]) {
			case '\'':
				dest[j++] = '\\';
				dest[j++] = '\'';
				break;
			default:
				dest[j++] = p[i];
		}
		i++;
	}
	dest[j]=0;
	free(p);
	return dest;
}

char* TLib_Str_Quote(char* dest, char* src)
{
  	register int i, j;
  	int n;
  	char *p;
  	
  	//cyril modified here for safe overlap.    2000-10-12
  	
  	p = (char*)malloc(strlen(src) + 1);
  	strncpy(p,src,strlen(src)+1);

	i = j = 0;
	while (p[i]) {
		switch((unsigned char)p[i]) {
			case '\n':
				dest[j++] = '\\';
				dest[j++] = 'n';
				break;
			case '\t':
				dest[j++] = '\\';
				dest[j++] = 't';
				break;
			case '\r':
				dest[j++] = '\\';
				dest[j++] = 'r';
				break;
			case '\b':
				dest[j++] = '\\';
				dest[j++] = 'b';
				break;
			case '\'':
				dest[j++] = '\\';
				dest[j++] = '\'';
				break;
			case '\"':
				dest[j++] = '\\';
				dest[j++] = '\"';
				break;
			case '\\':
				dest[j++] = '\\';
				dest[j++] = '\\';
				break;
			default:
				if ((unsigned char)p[i]>=32) dest[j++] = p[i];
				else dest[j++] =32;
		}
		i++;
	}
	dest[j]=0;
	free(p);
	return dest;
}

char* TLib_Str_QuoteWild(char* dest, char* src)
{
  	register int i, j;
  	int n;

	char *p;
  	
  	//cyril modified here for safe overlap.    2000-10-12
  	
  	p = (char*)malloc(strlen(src) + 1);
  	strncpy(p,src,strlen(src)+1);
  	
	i = j = 0;
	while (src[i]) {
		switch((unsigned char)src[i]) {
			case '\n':
				dest[j++] = '\\';
				dest[j++] = 'n';
				break;
			case '\t':
				dest[j++] = '\\';
				dest[j++] = 't';
				break;
			case '\r':
				dest[j++] = '\\';
				dest[j++] = 'r';
				break;
			case '\b':
				dest[j++] = '\\';
				dest[j++] = 'b';
				break;
			case '\'':
				dest[j++] = '\\';
				dest[j++] = '\'';
				break;
			case '\"':
				dest[j++] = '\\';
				dest[j++] = '\"';
				break;
			case '\\':
				dest[j++] = '\\';
				dest[j++] = '\\';
				break;
			case '%':
				dest[j++] = '\\';
				dest[j++] = '%';
				break;
			case '_':
				dest[j++] = '\\';
				dest[j++] = '_';
				break;
			default:
				dest[j++] = p[i];
		}
		i++;
	}
	dest[j]=0;
	
	free(p);
	return dest;
}

char* TLib_Str_CutHZString(char* sSrc)
{
	int iLength;
	int i = 1;
	unsigned char* pcSrc = (unsigned char*)sSrc;
	iLength = strlen(sSrc);
	
	while (*pcSrc)
	{
    		if (*pcSrc > 127 && i<iLength)
    			{
    				pcSrc+=2; i+=2;
    			}
    		else if (*pcSrc <= 127)
    			{
    				pcSrc++; i++;
    			}
    		else
    			{
    				*pcSrc = 0;
    			}
    	}
	return sSrc;
}

static int min(int x, int y)
{
	if(x > y)
		return y;
	else 
		return x;
} 

//------------------------------------------------------------
//
//				字符串全程替换，返回替换次数
//
//
//------------------------------------------------------------

int TLib_Str_StrReplace(char* szDesc,int len,char* szSrc,char* szOld,char* szNew)
{
	char *p;
	int count = 0;
	char *pDesc,*pSrc;
	int num = 0;
	int space = len;
	char* buffer;
	
	buffer = (char*) malloc(strlen(szSrc) + 1);
	strncpy(buffer,szSrc,strlen(szSrc)+1);
	
	szDesc[0]=0;
	pDesc = szDesc;
	pSrc = buffer;
	
	while((p = strstr(pSrc,szOld)) != NULL
		&& (space = len - strlen(szDesc)) != 0){
		num = min(space,p - pSrc);	
		strncpy(pDesc,pSrc,num);
		pDesc += num;
		*pDesc = 0;
		pSrc += num;
		space = len - strlen(szDesc);
		num = min(space,strlen(szNew));
		strncpy(pDesc,szNew,num);
		pDesc += num;
		*pDesc = 0;
		pSrc = pSrc + strlen(szOld);
		count ++;
	}
	
	space = len - strlen(szDesc);
	strncpy(pDesc,pSrc,space);
	szDesc[len - 1] = 0;
	return count;
}

#endif

