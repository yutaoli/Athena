#ifndef _TLIB_STR_H_
#define _TLIB_STR_H_

#define TLIB_STR_STRING_LIST_ERROR_INVALID_POINTER 	(TLIB_STR_ERROR_BASE-1)
#define TLIB_STR_STRING_LIST_ERROR_FAIL_TO_ALLOC_MEM	(TLIB_STR_ERROR_BASE-2)

typedef struct _TLIB_STR_STRING_LIST_ITEM
{
	char *sItem;
	struct _TLIB_STR_STRING_LIST_ITEM *pstNext;
} TLIB_STR_STRING_LIST_ITEM;

typedef struct _TLIB_STR_STRING_LIST
{
	TLIB_STR_STRING_LIST_ITEM *pstItem;
	TLIB_STR_STRING_LIST_ITEM *pstItemTail;
	TLIB_STR_STRING_LIST_ITEM *pstItemCur;
} TLIB_STR_STRING_LIST;

TLIB_STR_STRING_LIST *TLib_Str_StringListCreate(void);
int TLib_Str_StringListAdd(TLIB_STR_STRING_LIST *pstStringList, char *sItem);
void TLib_Str_StringListFree(TLIB_STR_STRING_LIST *pstStringList);
char *TLib_Str_StringListGetFirst(TLIB_STR_STRING_LIST *pstStringList);
char *TLib_Str_StringListGetNext(TLIB_STR_STRING_LIST *pstStringList);
////////////////////////////////////////////////////////////////////

char *TLib_Str_Trim(char *s);
char *TLib_Str_StrUppercase(char * s);
char *TLib_Str_StrLowercase(char * s);
int TLib_Str_StrCmp(char *s1, char *s2);
int TLib_Str_StrNCCmp(char *s1, char *s2);
char *TLib_Str_StrReplaceChar(char *pStrSrc, char cOld, char cNew);
char *TLib_Str_StrCat(char *s, const char *sFormat, ...);

char *TLib_Str_Strcpy( char *sOBJ, char *sSrc);
char *TLib_Str_Strncpy( char *sOBJ, char *sSrc, int iMaxChar);

//added by Mark 2000.04.28
char* TLib_Str_Quote(char* dest, char* src);
char* TLib_Str_QuoteWild(char* dest, char* src);
//added by Pilot 2000.11.17
char* TLib_Str_QuoteDB(char* dest,char* src);
int TLib_Str_Check_Email(char* sEmail);
//added by Mark 2000.05.11
char* TLib_Str_CutHZString(char* sSrc);
int TLib_Str_StrReplace(char* szDesc,int len,char* szSrc,char* szOld,char* szNew);

#endif

