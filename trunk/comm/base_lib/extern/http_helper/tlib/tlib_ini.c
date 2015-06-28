#ifndef _TLIB_INI_C_
#define _TLIB_INI_C_

#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include	<stdlib.h>
#include	<ctype.h>

#include "tlib_com.h"
#include	"tlib_str.h"
#include	"tlib_ini.h"

#define TLIB_INI_MAX_LINE_LENGTH			1024

#define TLIB_INI_ANALYZE_SECTION	1
#define TLIB_INI_ANALYZE_VALUE	2
#define TLIB_INI_ANALYZE_COMMENT	3
#define TLIB_INI_ANALYZE_UNKNOWN	4

typedef struct _TLIB_INI_VALUE
{
	char *sIdent;
	char *sValue;
	int iRemoved;
	int iWritten;
	struct _TLIB_INI_VALUE *pstNext;
} TLIB_INI_VALUE;

typedef struct _TLIB_INI_SECTION
{
	char *sSection;
	TLIB_INI_VALUE *pstValue;
	TLIB_INI_VALUE *pstValueTail;
	int iRemoved;
	int iWritten;
	struct _TLIB_INI_SECTION *pstNext;
} TLIB_INI_SECTION;

typedef struct _TLIB_INI
{
	TLIB_INI_SECTION *pstSection;
	TLIB_INI_SECTION *pstSectionTail;
} TLIB_INI, *P_TLIB_INI;

static P_TLIB_INI apstIni[TLIB_INI_MAX_OPENED_NUM];

/*****************************************************************/

static int TLib_Ini_CheckInitiated(void)
{
	static int iInitiated = 0;
	int i;
	
	if (iInitiated == 0)
	{
		for (i = 0; i < TLIB_INI_MAX_OPENED_NUM; i++)
			apstIni[i] = NULL;
			
		iInitiated = 1;
	}
	
	return 0;
}

static int TLib_Ini_GetHandle(void)
{
	int i;
	
	for (i = 0; i < TLIB_INI_MAX_OPENED_NUM; i++)
	{
		if (apstIni[i] == NULL)
		{
			apstIni[i] = malloc(sizeof(TLIB_INI));
			if (apstIni[i] == NULL)
			{
				return TLIB_INI_ERROR_FAIL_TO_ALLOC_MEM;
			}
			memset(apstIni[i], 0, sizeof(TLIB_INI));
			return i;
		}
	}
	
	return TLIB_INI_ERROR_NO_FREE_HANDLE;
}

static int TLib_Ini_ReadLine(int iFileHandle, char *sLine, int iMaxLineLen)
{
	int i, iRetCode;
	
	for (i = 0; i < iMaxLineLen - 1; i++)
	{
		iRetCode = read(iFileHandle, sLine+i, 1);
		if (iRetCode < 0)
		{
			return TLIB_INI_ERROR_FAIL_TO_READ_FILE;
		}
		
		if ((iRetCode == 0) && (i == 0))
		{
			return 1;
		}
		
		if ((iRetCode == 0) || (sLine[i] == '\n'))
		{
			sLine[i] = 0;
			return 0;
		}
		
		if  (sLine[i] == '\r')
		{
			i --;
		}
	}
	
	sLine[i] = 0;
	return 0;
}

static int TLib_Ini_ShmReadLine(char *p, char *sLine, int iMaxLineLen, int* iPoint, int iShmLen)
{
	int i, iRetCode;
	
	for (i = 0; i < iMaxLineLen;i++)
	{
		sLine[i] = *p;
		p ++;
		//printf("%d:%d\n",i,sLine[i]);
		
		if (iShmLen == *iPoint)
		{
			return 1;
		}
		
		if (sLine[i] == '\n')
		{
			sLine[i] = 0;
			p ++;
			(*iPoint)++;
			//printf("%d:%s\n",*iPoint, sLine);
			return 0;
		}
		(*iPoint)++;
		
	}
	
	sLine[i] = 0;
	return 0;
}

static int TLib_Ini_AnalyzeLine(char *sLine, char *sSection, char *sIdent, char *sValue)
{
	char s[TLIB_INI_MAX_LINE_LENGTH+1];
	int iLen;
	char *p;
	
	strncpy(s, sLine,strlen(sLine)+1);
	TLib_Str_Trim(s);
	iLen = strlen(s);
	
	if (iLen < 1)
	{
		return TLIB_INI_ANALYZE_UNKNOWN;
	}
	
	if ((s[0] == '[') && (s[iLen-1] == ']'))
	{
		strncpy(sSection, s+1, iLen-2);
		sSection[iLen-2] = 0;
		TLib_Str_Trim(sSection);
		return TLIB_INI_ANALYZE_SECTION;
	}
	
	if (s[0] == ';')
	{
		return TLIB_INI_ANALYZE_COMMENT;
	}
	
	p = strchr(s, '=');
	if (p != NULL)
	{
		strncpy(sIdent, s, p-s);
		sIdent[p-s] = 0;
		
		strncpy(sValue, p+1,strlen(p+1)+1);
		
		TLib_Str_Trim(sIdent);
		TLib_Str_Trim(sValue);
		return TLIB_INI_ANALYZE_VALUE;
	}
	
	return TLIB_INI_ANALYZE_UNKNOWN;
}

static int TLib_Ini_CheckHandle(int iIniHandle)
{
	if ((iIniHandle < 0) || (iIniHandle >= TLIB_INI_MAX_OPENED_NUM))
	{
		return TLIB_INI_ERROR_INVALID_HANDLE;
	}
		
	if (apstIni[iIniHandle] == NULL)
	{
		return TLIB_INI_ERROR_INVALID_HANDLE;
	}
	
	return 0;
}

/*****************************************************************/

void TLib_Ini_Free(int iIniHandle)
{
	TLIB_INI_SECTION *pstSection;
	TLIB_INI_VALUE *pstValue;
	
	TLib_Ini_CheckInitiated();
	
	if ((iIniHandle < 0) || (iIniHandle >= TLIB_INI_MAX_OPENED_NUM))
		return;
		
	if (apstIni[iIniHandle] == NULL)
	{
		return;
	}
	
	pstSection = apstIni[iIniHandle]->pstSection;
	while(pstSection != NULL)
	{
		pstValue = pstSection->pstValue;
		while (pstValue != NULL)
		{
			pstSection->pstValue = pstValue->pstNext;
			if (pstValue->sIdent != NULL)
				free(pstValue->sIdent);
			if (pstValue->sValue != NULL)
				free(pstValue->sValue);
			free(pstValue);
			pstValue = pstSection->pstValue;
		}
		
		apstIni[iIniHandle]->pstSection = pstSection->pstNext;
		if (pstSection->sSection != NULL)
			free(pstSection->sSection);
		free(pstSection);
		pstSection = apstIni[iIniHandle]->pstSection;
	}
	
	free(apstIni[iIniHandle]);
	apstIni[iIniHandle] = NULL;
	
	return;
}

int TLib_Ini_Load(char *sIniFileName, int iMode)
{
	int iIniHandle;
	int iFileHandle, iOpenMode;
	char sLine[TLIB_INI_MAX_LINE_LENGTH+1];
	char sSection[TLIB_INI_MAX_LINE_LENGTH+1];
	char sIdent[TLIB_INI_MAX_LINE_LENGTH+1];
	char sValue[TLIB_INI_MAX_LINE_LENGTH+1];
	int i, iPos, iRetCode;
	
	TLib_Ini_CheckInitiated();
	
	iIniHandle = TLib_Ini_GetHandle();
	if (iIniHandle < 0)
	{
		return iIniHandle;
	}
	
	if ((sIniFileName == NULL) || (sIniFileName[0] == 0))
	{
		return iIniHandle;
	}
	
	iOpenMode = O_BINARY | O_DENYNONE | O_RDONLY;
		
	iFileHandle = open(sIniFileName, iOpenMode);
	if (iFileHandle == -1)
	{
		if (iMode != TLIB_INI_OPEN_FAIL_IGNORE)
		{
			TLib_Ini_Free(iIniHandle);
			return TLIB_INI_ERROR_FAIL_TO_OPEN_FILE;
		}
		else
		{
			return iIniHandle;
		}
	}
	
	sSection[0]=0;
	while(1)
	{
		iRetCode = TLib_Ini_ReadLine(iFileHandle, sLine, sizeof(sLine));
		if (iRetCode < 0)
		{
			close(iFileHandle);
			TLib_Ini_Free(iIniHandle);
			return iRetCode;
		}
		else if (iRetCode > 0)
		{
			break;
		}
		
		iRetCode = TLib_Ini_AnalyzeLine(sLine, sSection, sIdent, sValue);
		if (iRetCode == TLIB_INI_ANALYZE_VALUE)
		{
			iRetCode = TLib_Ini_WriteString(iIniHandle, sSection, sIdent, sValue);
			if (iRetCode < 0)
			{
				close(iFileHandle);
				TLib_Ini_Free(iIniHandle);
				return iRetCode;
			}
		}
	}
	
	close(iFileHandle);
	return 0;
}

int TLib_Ini_ShmLoad(char *sShm, int iMode)
{
	int iIniHandle;
	char sLine[TLIB_INI_MAX_LINE_LENGTH+1];
	char sSection[TLIB_INI_MAX_LINE_LENGTH+1];
	char sIdent[TLIB_INI_MAX_LINE_LENGTH+1];
	char sValue[TLIB_INI_MAX_LINE_LENGTH+1];
	int i, j, iShmLen, iPos, iRetCode;
	char *p;
	
	TLib_Ini_CheckInitiated();
	
	iIniHandle = TLib_Ini_GetHandle();
	if (iIniHandle < 0)
	{
		return iIniHandle;
	}
	
	p = sShm+2;
	iShmLen=sShm[0]*256+sShm[1];
	i = 0;

	sSection[0]=0;
	while(1)
	//for (j=1;j<10;j++)
	{
		iRetCode = TLib_Ini_ShmReadLine(p, sLine, sizeof(sLine), &i, iShmLen);
		p = sShm+i+2;
		//printf("ii:%d\n",i);
		if (iRetCode < 0)
		{
			TLib_Ini_Free(iIniHandle);
			return iRetCode;
		}
		else if (iRetCode > 0)
		{
			break;
		}

		iRetCode = TLib_Ini_AnalyzeLine(sLine, sSection, sIdent, sValue);
		
		if (iRetCode == TLIB_INI_ANALYZE_VALUE)
		{
			iRetCode = TLib_Ini_WriteString(iIniHandle, sSection, sIdent, sValue);
			if (iRetCode < 0)
			{
				TLib_Ini_Free(iIniHandle);
				return iRetCode;
			}
		}
	}

	
	return 0;
}

int TLib_Ini_WriteString(int iIniHandle, char *sSection, char *sIdent, char *sValue)
{
	int iRetCode;
	TLIB_INI_SECTION *pstSection;
	TLIB_INI_VALUE	*pstValue;
	
	TLib_Ini_CheckInitiated();
	
	iRetCode = TLib_Ini_CheckHandle(iIniHandle);
	if (iRetCode < 0)
	{
		return iRetCode;
	}
	
	pstSection = apstIni[iIniHandle]->pstSection;
	while(pstSection != NULL)
	{
		if (TLib_Str_StrCmp(sSection, pstSection->sSection) == 0)
		{
			break;
		}
		pstSection = pstSection->pstNext;
	}
	
	if (pstSection == NULL)
	{
		pstSection = malloc(sizeof(TLIB_INI_SECTION));
		if (pstSection == NULL)
		{
			return TLIB_INI_ERROR_FAIL_TO_ALLOC_MEM;
		}
		memset(pstSection, 0, sizeof(TLIB_INI_SECTION));
		
		pstSection->sSection = malloc(strlen(sSection)+1);
		if (pstSection->sSection == NULL)
		{
			free(pstSection);
			return TLIB_INI_ERROR_FAIL_TO_ALLOC_MEM;
		}
		strncpy(pstSection->sSection, sSection,strlen(sSection)+1);
		
		pstSection->pstNext = NULL;
		if (apstIni[iIniHandle]->pstSectionTail != NULL)
		{
			apstIni[iIniHandle]->pstSectionTail->pstNext = pstSection;
		}
		apstIni[iIniHandle]->pstSectionTail = pstSection;
		if (apstIni[iIniHandle]->pstSection == NULL)
		{
			apstIni[iIniHandle]->pstSection = pstSection;
		}
	}
	
	pstValue = pstSection->pstValue;
	while(pstValue != NULL)
	{
		if (TLib_Str_StrCmp(sIdent, pstValue->sIdent) == 0)
		{
			break;
		}
		pstValue = pstValue->pstNext;
	}
	
	if (pstValue != NULL)
	{
		free(pstValue->sValue);
		pstValue->sValue = NULL;
	}
	else
	{
		pstValue = malloc(sizeof(TLIB_INI_VALUE));
		if (pstValue == NULL)
		{
			return TLIB_INI_ERROR_FAIL_TO_ALLOC_MEM;
		}
		memset(pstValue, 0, sizeof(TLIB_INI_VALUE));
		
		pstValue->sIdent = malloc(strlen(sIdent)+1);
		if (pstValue->sIdent == NULL)
		{
			return TLIB_INI_ERROR_FAIL_TO_ALLOC_MEM;
		}
		strncpy(pstValue->sIdent, sIdent,strlen(sIdent)+1);

		pstValue->pstNext = NULL;
		if (pstSection->pstValueTail != NULL)
		{
			pstSection->pstValueTail->pstNext = pstValue;
		}
		pstSection->pstValueTail = pstValue;
		if (pstSection->pstValue == NULL)
		{
			pstSection->pstValue = pstValue;
		}
	}
	
	pstValue->sValue = malloc(strlen(sValue)+1);
	if (pstValue->sValue == NULL)
	{
		return TLIB_INI_ERROR_FAIL_TO_ALLOC_MEM;
	}
	strncpy(pstValue->sValue, sValue,strlen(sValue)+1);
	pstValue->iRemoved = 0;

	return 0;
}

int TLib_Ini_Dump(int iIniHandle)
{
	int iRetCode;
	TLIB_INI_SECTION *pstSection;
	TLIB_INI_VALUE	*pstValue;
	
	TLib_Ini_CheckInitiated();
	
	iRetCode = TLib_Ini_CheckHandle(iIniHandle);
	if (iRetCode < 0)
	{
		return iRetCode;
	}
	
	pstSection = apstIni[iIniHandle]->pstSection;
	while(pstSection != NULL)
	{
		printf("\n[%s]\n", pstSection->sSection);
		pstValue = pstSection->pstValue;
		while(pstValue != NULL)
		{
			printf("%s=%s\n", pstValue->sIdent, pstValue->sValue);
			pstValue = pstValue->pstNext;
		}
		
		pstSection = pstSection->pstNext;
	}
	
	return 0;
}

int TLib_Ini_ReadString(int iIniHandle, char *sSection, char *sIdent, char *sDefault, char *sValue, int iSize)
{
	int iRetCode;
	TLIB_INI_SECTION *pstSection;
	TLIB_INI_VALUE	*pstValue;
	char *p;
	
	TLib_Ini_CheckInitiated();
	
	iRetCode = TLib_Ini_CheckHandle(iIniHandle);
	if (iRetCode < 0)
	{
		return iRetCode;
	}
	
	p = sDefault;
	pstSection = apstIni[iIniHandle]->pstSection;
	while(pstSection != NULL)
	{
		if (TLib_Str_StrCmp(pstSection->sSection, sSection) == 0)
		{
			pstValue = pstSection->pstValue;
			while(pstValue != NULL)
			{
				if (TLib_Str_StrCmp(pstValue->sIdent, sIdent) == 0)
				{
					if (pstValue->iRemoved == 0)
					{
						p = pstValue->sValue;
					}
					break;
				}
				pstValue = pstValue->pstNext;
			}
			break;
		}
		
		pstSection = pstSection->pstNext;
	}
	
	strncpy(sValue, p, iSize-1);
	sValue[iSize-1] = 0;
	
	return 0;
}

int TLib_Ini_RemoveSection(int iIniHandle, char *sSection)
{
	int iRetCode;
	TLIB_INI_SECTION *pstSection;
	TLIB_INI_VALUE	*pstValue;
	
	TLib_Ini_CheckInitiated();
	
	iRetCode = TLib_Ini_CheckHandle(iIniHandle);
	if (iRetCode < 0)
	{
		return iRetCode;
	}
	
	pstSection = apstIni[iIniHandle]->pstSection;
	while(pstSection != NULL)
	{
		if (TLib_Str_StrCmp(pstSection->sSection, sSection) == 0)
		{
			pstSection->iRemoved = 1;
			pstValue = pstSection->pstValue;
			while(pstValue != NULL)
			{
				pstValue->iRemoved = 1;
				pstValue = pstValue->pstNext;
			}
			break;
		}
		
		pstSection = pstSection->pstNext;
	}
	
	return 0;
}

int TLib_Ini_RemoveKey(int iIniHandle, char *sSection, char *sIdent)
{
	int iRetCode;
	TLIB_INI_SECTION *pstSection;
	TLIB_INI_VALUE	*pstValue;
	
	TLib_Ini_CheckInitiated();
	
	iRetCode = TLib_Ini_CheckHandle(iIniHandle);
	if (iRetCode < 0)
	{
		return iRetCode;
	}
	
	pstSection = apstIni[iIniHandle]->pstSection;
	while(pstSection != NULL)
	{
		if (TLib_Str_StrCmp(pstSection->sSection, sSection) == 0)
		{
			pstValue = pstSection->pstValue;
			while(pstValue != NULL)
			{
				if (TLib_Str_StrCmp(pstValue->sIdent, sIdent) == 0)
				{
					pstValue->iRemoved = 1;
					break;
				}
				pstValue = pstValue->pstNext;
			}
			break;
		}
		
		pstSection = pstSection->pstNext;
	}
	
	return 0;
}


int TLib_Ini_Save(int iIniHandle, char *sIniFileName)
{
	int iFileHandle, iOpenMode;
	char sLine[TLIB_INI_MAX_LINE_LENGTH+1];
	char sSection[TLIB_INI_MAX_LINE_LENGTH+1];
	char sLastSection[TLIB_INI_MAX_LINE_LENGTH+1];
	char sIdent[TLIB_INI_MAX_LINE_LENGTH+1];
	char sValue[TLIB_INI_MAX_LINE_LENGTH+1];
	TLIB_STR_STRING_LIST *pstStringList;
	TLIB_INI_SECTION *pstCurSection;
	TLIB_INI_VALUE	*pstValue;
	
	int i, iLen, iPos, iRetCode;
	int iRemoved, iValueRemoved;
	char *p;
	
	TLib_Ini_CheckInitiated();
	
	iRetCode = TLib_Ini_CheckHandle(iIniHandle);
	if (iRetCode < 0)
	{
		return iRetCode;
	}
	
	if ((sIniFileName == NULL) || (sIniFileName[0] == 0))
	{
		return iIniHandle;
	}
	
	pstStringList = TLib_Str_StringListCreate();
	if (pstStringList == NULL)
	{
		return TLIB_INI_ERROR_FAIL_TO_ALLOC_MEM;
	}

	iOpenMode = O_BINARY | O_DENYNONE | O_RDWR | O_CREAT;
		
	iFileHandle = open(sIniFileName, iOpenMode);
	if (iFileHandle == -1)
	{
		TLib_Str_StringListFree(pstStringList);
		return TLIB_INI_ERROR_FAIL_TO_OPEN_FILE;
	}

	pstCurSection = apstIni[iIniHandle]->pstSection;
	while(pstCurSection != NULL)
	{
		pstValue = pstCurSection->pstValue;
		while(pstValue != NULL)
		{
			pstValue->iWritten = 0;
			pstValue = pstValue->pstNext;
		}
		pstCurSection->iWritten = 0;
		pstCurSection = pstCurSection->pstNext;
	}

	pstCurSection = NULL;
	while(1)
	{
		iRetCode = TLib_Ini_ReadLine(iFileHandle, sLine, sizeof(sLine));
		if (iRetCode < 0)
		{
			TLib_Str_StringListFree(pstStringList);
			close(iFileHandle);
			return iRetCode;
		}
		else if (iRetCode > 0)
		{
			break;
		}
		
		iRetCode = TLib_Ini_AnalyzeLine(sLine, sSection, sIdent, sValue);
		if (iRetCode == TLIB_INI_ANALYZE_SECTION)
		{
			if (pstCurSection != NULL)
			{
				if	(pstCurSection->iRemoved == 0)
				{
					pstValue = pstCurSection->pstValue;
					while(pstValue != NULL)
					{
						if ((pstValue->iRemoved == 0) && (pstValue->iWritten == 0))
						{
							snprintf(sLine,sizeof(sLine), "%s=%s", pstValue->sIdent, pstValue->sValue);
							iRetCode = TLib_Str_StringListAdd(pstStringList, sLine);
							if (iRetCode < 0)
							{
								TLib_Str_StringListFree(pstStringList);
								close(iFileHandle);
								return iRetCode;
							}
							pstValue->iWritten = 1;
						}
						pstValue = pstValue->pstNext;
					}
				}
			}

			pstCurSection = apstIni[iIniHandle]->pstSection;
			while(pstCurSection != NULL)
			{
				if (TLib_Str_StrCmp(pstCurSection->sSection, sSection) == 0)
				{
					break;
				}
				pstCurSection = pstCurSection->pstNext;
			}

			if ((pstCurSection == NULL) 
					|| ((pstCurSection != NULL) && (pstCurSection->iRemoved == 0)))
			{
				snprintf(sLine,sizeof(sLine), "[%s]", sSection);
				iRetCode = TLib_Str_StringListAdd(pstStringList, sLine);
				if (iRetCode < 0)
				{
					TLib_Str_StringListFree(pstStringList);
					close(iFileHandle);
					return iRetCode;
				}
				if (pstCurSection != NULL)
				{
					pstCurSection->iWritten = 1;
				}
			}
		}
		else if (iRetCode == TLIB_INI_ANALYZE_VALUE)
		{
			iValueRemoved = 0;
			if (pstCurSection != NULL)
			{
				if (pstCurSection->iRemoved != 0)
				{
					iValueRemoved = 1;
				}
				else
				{
					pstValue = pstCurSection->pstValue;
					while(pstValue != NULL)
					{
						if (TLib_Str_StrCmp(pstValue->sIdent, sIdent) == 0)
						{
							if (pstValue->iRemoved != 0)
							{
								iValueRemoved = 1;
							}
							else
							{
								snprintf(sLine,sizeof(sLine), "%s=%s", pstValue->sIdent, pstValue->sValue);
								pstValue->iWritten = 1;
							}
							break;
						}
						pstValue = pstValue->pstNext;
					}
				}
			}
			
			if (iValueRemoved == 0)
			{
				iRetCode = TLib_Str_StringListAdd(pstStringList, sLine);
				if (iRetCode < 0)
				{
					TLib_Str_StringListFree(pstStringList);
					close(iFileHandle);
					return iRetCode;
				}
			}
		}
		else
		{
			if ((pstCurSection == NULL) 
					|| ((pstCurSection != NULL) && (pstCurSection->iRemoved == 0)))
			{
				iRetCode = TLib_Str_StringListAdd(pstStringList, sLine);
				if (iRetCode < 0)
				{
					TLib_Str_StringListFree(pstStringList);
					close(iFileHandle);
					return iRetCode;
				}
			}
		}
	}

	if (pstCurSection != NULL)
	{
		if	(pstCurSection->iRemoved == 0)
		{
			pstValue = pstCurSection->pstValue;
			while(pstValue != NULL)
			{
				if ((pstValue->iRemoved == 0) && (pstValue->iWritten == 0))
				{
					snprintf(sLine,sizeof(sLine), "%s=%s", pstValue->sIdent, pstValue->sValue);
					iRetCode = TLib_Str_StringListAdd(pstStringList, sLine);
					if (iRetCode < 0)
					{
						TLib_Str_StringListFree(pstStringList);
						close(iFileHandle);
						return iRetCode;
					}
					pstValue->iWritten = 1;
				}
				pstValue = pstValue->pstNext;
			}
		}
	}

	pstCurSection = apstIni[iIniHandle]->pstSection;
	while(pstCurSection != NULL)
	{
		if	((pstCurSection->iRemoved == 0) && (pstCurSection->iWritten == 0))
		{
			snprintf(sLine,sizeof(sLine), "[%s]", pstCurSection->sSection);
			iRetCode = TLib_Str_StringListAdd(pstStringList, sLine);
			if (iRetCode < 0)
			{
				TLib_Str_StringListFree(pstStringList);
				close(iFileHandle);
				return iRetCode;
			}
			pstCurSection->iWritten = 1;
		}
		
		pstValue = pstCurSection->pstValue;
		while(pstValue != NULL)
		{
			if ((pstValue->iRemoved == 0) && (pstValue->iWritten == 0))
			{
				if	(pstCurSection->iWritten == 0)
				{
					snprintf(sLine,sizeof(sLine), "[%s]", pstCurSection->sSection);
					iRetCode = TLib_Str_StringListAdd(pstStringList, sLine);
					if (iRetCode < 0)
					{
						TLib_Str_StringListFree(pstStringList);
						close(iFileHandle);
						return iRetCode;
					}
					pstCurSection->iWritten = 1;
				}

				snprintf(sLine,sizeof(sLine), "%s=%s", pstValue->sIdent, pstValue->sValue);
				iRetCode = TLib_Str_StringListAdd(pstStringList, sLine);
				if (iRetCode < 0)
				{
					TLib_Str_StringListFree(pstStringList);
					close(iFileHandle);
					return iRetCode;
				}
				pstValue->iWritten = 1;
			}
			pstValue = pstValue->pstNext;
		}
		pstCurSection = pstCurSection->pstNext;
	}

#ifndef T_LIB_UNIX
	if (chsize(iFileHandle, 0L) != 0)
#else
	if (ftruncate(iFileHandle, 0L) != 0)
#endif
	{
		TLib_Str_StringListFree(pstStringList);
		close(iFileHandle);
		return TLIB_INI_ERROR_FAIL_TO_WRITE;
	}
   if (lseek(iFileHandle, 0, 0) < 0)
   {
   	return TLIB_INI_ERROR_FAIL_TO_SEEK;
   }
	
	
	p = TLib_Str_StringListGetFirst(pstStringList);
	while (p != NULL)
	{
		iLen = strlen(p);
		if (iLen > 0)
		{
			if (write(iFileHandle, p, iLen) != iLen)
			{
				return TLIB_INI_ERROR_FAIL_TO_WRITE;
			}
		}

		if (write(iFileHandle, "\r\n", 2) != 2)
		{
			return TLIB_INI_ERROR_FAIL_TO_WRITE;
		}
		p = TLib_Str_StringListGetNext(pstStringList);
	}
			
	TLib_Str_StringListFree(pstStringList);
	close(iFileHandle);
	return 0;
}

void TLib_Ini_FreeAll(void)
{
	int i;
	
	for (i = 0; i < TLIB_INI_MAX_OPENED_NUM; i ++)
	{
		TLib_Ini_Free(i);
	}
	
	return;
}

#endif
