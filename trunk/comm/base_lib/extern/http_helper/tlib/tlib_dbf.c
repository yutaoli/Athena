#define _TLIB_DBF_C_

#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<stdarg.h>

#include "tlib_com.h"
#include "tlib_str.h"
#include "tlib_dbf.h"

#define	TLIB_DBF_MARK_UNDELETED	32
#define	TLIB_DBF_MARK_DELETED		42
#define	TLIB_DBF_LOCK_POSITION		2000000000L

typedef struct _TLIB_DBF_FIELD
{
	char	sFieldName[13];
	char	cFieldType;
	int	iFieldLength;
	int	iFieldOffset;
} TLIB_DBF_FIELD;

typedef struct _TLIB_DBF
{
	int	iFileHandle;

	long	lRecordLength;
	long	lDataStartPosition;

	int	iFieldCount;
	TLIB_DBF_FIELD	*pstFields;

	long	lRecordCount;
	long	lRecordNo;
	long	lOffset;
	char	*pcRecordBuffer;
} TLIB_DBF, *P_TLIB_DBF;

static P_TLIB_DBF apstDbf[TLIB_DBF_MAX_OPENED_NUM];

/***************************************************************/

static int TLib_Dbf_CheckInitiated(void)
{
	static int iInitiated = 0;
	int i;
	
	if (iInitiated == 0)
	{
		for (i = 0; i < TLIB_DBF_MAX_OPENED_NUM; i++)
			apstDbf[i] = NULL;
			
		iInitiated = 1;
	}
	
	return 0;
}

static int TLib_Dbf_GetHandle(void)
{
	int i;
	
	for (i = 0; i < TLIB_DBF_MAX_OPENED_NUM; i++)
	{
		if (apstDbf[i] == NULL)
		{
			apstDbf[i] = malloc(sizeof(TLIB_DBF));
			if (apstDbf[i] == NULL)
			{
				return TLIB_DBF_ERROR_FAIL_TO_ALLOC_MEM;
			}
			memset(apstDbf[i], 0, sizeof(TLIB_DBF));
			apstDbf[i]->iFileHandle = -1;
			return i;
		}
	}
	
	return TLIB_DBF_ERROR_NO_FREE_HANDLE;
}

static void TLib_Dbf_FreeHandle(int iDbfHandle)
{
	if ((iDbfHandle < 0) || (iDbfHandle >= TLIB_DBF_MAX_OPENED_NUM))
		return;
		
	if (apstDbf[iDbfHandle] == NULL)
	{
		return;
	}
	
	if (apstDbf[iDbfHandle]->iFileHandle != -1)
	{
		close(apstDbf[iDbfHandle]->iFileHandle);
	}
	
	if (apstDbf[iDbfHandle]->pstFields != NULL)
	{
		free(apstDbf[iDbfHandle]->pstFields);
	}
		
	if (apstDbf[iDbfHandle]->pcRecordBuffer != NULL)
	{
		free(apstDbf[iDbfHandle]->pcRecordBuffer);
	}
		
	free(apstDbf[iDbfHandle]);
	apstDbf[iDbfHandle] = NULL;
	
	return;
}

static int TLib_Dbf_GetFieldNo(int iDbfHandle, char *sFieldName)
{
	int i, iFieldNo;
	
	iFieldNo = -1;
	for (i = 0; i < apstDbf[iDbfHandle]->iFieldCount; i ++)
	{
		if (TLib_Str_StrNCCmp(apstDbf[iDbfHandle]->pstFields[i].sFieldName, sFieldName) == 0)
		{
			iFieldNo = i;
			break;
		}
	}
	
	return iFieldNo;
}

static int TLib_Dbf_CheckHandle(int iDbfHandle)
{
	if ((iDbfHandle < 0) || (iDbfHandle >= TLIB_DBF_MAX_OPENED_NUM))
	{
		return TLIB_DBF_ERROR_INVALID_HANDLE;
	}
		
	if (apstDbf[iDbfHandle] == NULL)
	{
		return TLIB_DBF_ERROR_INVALID_HANDLE;
	}
	
	if (apstDbf[iDbfHandle]->iFileHandle == -1)
	{
		return TLIB_DBF_ERROR_INVALID_HANDLE;
	}
	
	return 0;
}

/***************************************************************/

int TLib_Dbf_Open(char *sFileName, int iMode)
{
	int iDbfHandle;
	int iOpenMode;
	unsigned char caHeader[32];
	int i, iPos;
	
	TLib_Dbf_CheckInitiated();
	
	iDbfHandle = TLib_Dbf_GetHandle();
	if (iDbfHandle < 0)
	{
		return iDbfHandle;
	}
	
	iOpenMode = O_BINARY | O_DENYNONE;
	if (iMode & TLIB_DBF_OPEN_MODE_WRITE)
		iOpenMode = iOpenMode | O_RDWR;
	else
		iOpenMode = iOpenMode | O_RDONLY;
		
	apstDbf[iDbfHandle]->iFileHandle = open(sFileName, iOpenMode);
	if (apstDbf[iDbfHandle]->iFileHandle == -1)
	{
		TLib_Dbf_FreeHandle(iDbfHandle);
		return TLIB_DBF_ERROR_FAIL_TO_OPEN_FILE;
	}
	
	if (read(apstDbf[iDbfHandle]->iFileHandle, caHeader, 32) != 32)
	{
		TLib_Dbf_FreeHandle(iDbfHandle);
		return TLIB_DBF_ERROR_FAIL_TO_READ_HEADER;
	}
	
	apstDbf[iDbfHandle]->lDataStartPosition = caHeader[9]*256 + caHeader[8];
	apstDbf[iDbfHandle]->lRecordLength = caHeader[11]*256 + caHeader[10];
	apstDbf[iDbfHandle]->iFieldCount = (apstDbf[iDbfHandle]->lDataStartPosition / 32) - 1;

	if ((apstDbf[iDbfHandle]->lRecordLength <= 0)
		|| (apstDbf[iDbfHandle]->iFieldCount <= 0)
		|| (apstDbf[iDbfHandle]->lDataStartPosition <= 0))
	{
		TLib_Dbf_FreeHandle(iDbfHandle);
		return TLIB_DBF_ERROR_INVALID_HEADER;
	}

	apstDbf[iDbfHandle]->pcRecordBuffer = malloc(apstDbf[iDbfHandle]->lRecordLength);
	if (apstDbf[iDbfHandle]->pcRecordBuffer == NULL)
	{
		TLib_Dbf_FreeHandle(iDbfHandle);
		return TLIB_DBF_ERROR_FAIL_TO_ALLOC_MEM;
	}
	
	apstDbf[iDbfHandle]->pstFields = malloc(apstDbf[iDbfHandle]->iFieldCount*sizeof(TLIB_DBF_FIELD));
	if (apstDbf[iDbfHandle]->pstFields == NULL)
	{
		TLib_Dbf_FreeHandle(iDbfHandle);
		return TLIB_DBF_ERROR_FAIL_TO_ALLOC_MEM;
	}
	
	memset(apstDbf[iDbfHandle]->pstFields, 0, apstDbf[iDbfHandle]->iFieldCount*sizeof(TLIB_DBF_FIELD));
	iPos = 1;
	for (i = 0; i < apstDbf[iDbfHandle]->iFieldCount; i++)
	{
		if (read(apstDbf[iDbfHandle]->iFileHandle, caHeader, 32) != 32)
		{
			TLib_Dbf_FreeHandle(iDbfHandle);
			return TLIB_DBF_ERROR_FAIL_TO_READ_HEADER;
		}
	
		strncpy(apstDbf[iDbfHandle]->pstFields[i].sFieldName, caHeader, sizeof(apstDbf[iDbfHandle]->pstFields[i].sFieldName)-1);
		TLib_Str_StrUppercase(TLib_Str_Trim(apstDbf[iDbfHandle]->pstFields[i].sFieldName));
		apstDbf[iDbfHandle]->pstFields[i].cFieldType = caHeader[11];
		apstDbf[iDbfHandle]->pstFields[i].iFieldLength = caHeader[16];
		apstDbf[iDbfHandle]->pstFields[i].iFieldOffset = iPos;
		iPos += apstDbf[iDbfHandle]->pstFields[i].iFieldLength;
	}
	
	return iDbfHandle;
}

void TLib_Dbf_Close(int iDbfHandle)
{
	TLib_Dbf_FreeHandle(iDbfHandle);
}


long TLib_Dbf_RecordCount(int iDbfHandle)
{
	long l;
	int iRetCode;
	
	TLib_Dbf_CheckInitiated();
	iRetCode = TLib_Dbf_CheckHandle(iDbfHandle);
	if (iRetCode < 0)
	{
		return iRetCode;
	}

   l = lseek(apstDbf[iDbfHandle]->iFileHandle, 0, 2);
   if (l < 0)
   {
   	return TLIB_DBF_ERROR_FAIL_TO_SEEK;
   }
   
   if (l < apstDbf[iDbfHandle]->lDataStartPosition)
   {
   	return TLIB_DBF_ERROR_INVALID_HEADER;
   }
   	
   return (l - apstDbf[iDbfHandle]->lDataStartPosition) / (apstDbf[iDbfHandle]->lRecordLength);
}


int TLib_Dbf_Go(int iDbfHandle, long lRecordNo)
{
	long l;
	int iRetCode;
	
	TLib_Dbf_CheckInitiated();
	iRetCode = TLib_Dbf_CheckHandle(iDbfHandle);
	if (iRetCode < 0)
	{
		return iRetCode;
	}

	if (lRecordNo < 1)
	{
		return TLIB_DBF_ERROR_INVALID_RECNO;
	}
	
	l = (lRecordNo - 1) * apstDbf[iDbfHandle]->lRecordLength + apstDbf[iDbfHandle]->lDataStartPosition;
   if (lseek(apstDbf[iDbfHandle]->iFileHandle, l, 0) != l)
   {
   	return TLIB_DBF_ERROR_FAIL_TO_SEEK;
   }
   
   return 0;
}

int TLib_Dbf_LoadRecord(int iDbfHandle)
{
	int iRetCode;
	
	TLib_Dbf_CheckInitiated();
	iRetCode = TLib_Dbf_CheckHandle(iDbfHandle);
	if (iRetCode < 0)
	{
		return iRetCode;
	}

	if (read(apstDbf[iDbfHandle]->iFileHandle, 
				apstDbf[iDbfHandle]->pcRecordBuffer, 
				apstDbf[iDbfHandle]->lRecordLength) != apstDbf[iDbfHandle]->lRecordLength)
	{
		return TLIB_DBF_ERROR_FAIL_TO_READ_RECORD;
	}
	
	return 0;
}

int TLib_Dbf_ReadField(int iDbfHandle, char *sFieldName, char *sValue, int iValueSize)
{
	int iFieldNo, iLen;
	int iRetCode;
	
	TLib_Dbf_CheckInitiated();
	iRetCode = TLib_Dbf_CheckHandle(iDbfHandle);
	if (iRetCode < 0)
	{
		return iRetCode;
	}

	iFieldNo = TLib_Dbf_GetFieldNo(iDbfHandle, sFieldName);
	if (iFieldNo < 0)
	{
		return TLIB_DBF_ERROR_INVALID_FIELD_NAME;
	}
	
	iLen = apstDbf[iDbfHandle]->pstFields[iFieldNo].iFieldLength;
	if (iLen > iValueSize-1)
	{
		iLen = iValueSize - 1;
	}
	
	strncpy(sValue, 
				apstDbf[iDbfHandle]->pcRecordBuffer
					+apstDbf[iDbfHandle]->pstFields[iFieldNo].iFieldOffset,
				iLen);
	sValue[iLen] = 0;
	TLib_Str_Trim(sValue);
	
	return 0;
}


int TLib_Dbf_ReplaceField(int iDbfHandle, char *sFieldName, char *sValue)
{
	int i, iFieldNo, iLen;
	int iRetCode;
	char *p;
	
	TLib_Dbf_CheckInitiated();
	iRetCode = TLib_Dbf_CheckHandle(iDbfHandle);
	if (iRetCode < 0)
	{
		return iRetCode;
	}

	iFieldNo = TLib_Dbf_GetFieldNo(iDbfHandle, sFieldName);
	if (iFieldNo < 0)
	{
		return TLIB_DBF_ERROR_INVALID_FIELD_NAME;
	}

	iLen = strlen(sValue);
	if (iLen > apstDbf[iDbfHandle]->pstFields[iFieldNo].iFieldLength)
	{
		iLen = apstDbf[iDbfHandle]->pstFields[iFieldNo].iFieldLength;
	}
	
	p = apstDbf[iDbfHandle]->pcRecordBuffer
		+apstDbf[iDbfHandle]->pstFields[iFieldNo].iFieldOffset;
	for (i = 0; i < iLen; i++)
	{
		*(p+i) = *(sValue+i);
	}
	
	for (i = iLen; i < apstDbf[iDbfHandle]->pstFields[iFieldNo].iFieldLength; i++)
	{
		*(p+i) = ' ';
	}
	
	return 0;
}

int TLib_Dbf_ReplaceFieldF(int iDbfHandle, char *sFieldName, const char *sFormat, ...)
{
        int i, iFieldNo, iLen;
        int iRetCode;
        char *p;
        va_list ap;

        TLib_Dbf_CheckInitiated();
        iRetCode = TLib_Dbf_CheckHandle(iDbfHandle);
        if (iRetCode < 0)
        {
                return iRetCode;
        }

        iFieldNo = TLib_Dbf_GetFieldNo(iDbfHandle, sFieldName);
        if (iFieldNo < 0)
        {
                return TLIB_DBF_ERROR_INVALID_FIELD_NAME;
        }

                iLen = apstDbf[iDbfHandle]->pstFields[iFieldNo].iFieldLength;

        p = apstDbf[iDbfHandle]->pcRecordBuffer
                +apstDbf[iDbfHandle]->pstFields[iFieldNo].iFieldOffset;

        va_start(ap, sFormat);
        iLen = vsnprintf(p, apstDbf[iDbfHandle]->pstFields[iFieldNo].iFieldLength, sFormat, ap);
        va_end(ap);

	if (iLen > 0)
	{
		for (i = iLen; i < apstDbf[iDbfHandle]->pstFields[iFieldNo].iFieldLength; i++)
        	{
                	*(p+i) = ' ';
		}
        }

        return 0;
}

int TLib_Dbf_CommitRecord(int iDbfHandle)
{
	int iRetCode;
	
	TLib_Dbf_CheckInitiated();
	iRetCode = TLib_Dbf_CheckHandle(iDbfHandle);
	if (iRetCode < 0)
	{
		return iRetCode;
	}

	if (write(apstDbf[iDbfHandle]->iFileHandle, 
				apstDbf[iDbfHandle]->pcRecordBuffer, 
				apstDbf[iDbfHandle]->lRecordLength) != apstDbf[iDbfHandle]->lRecordLength)
	{
		return TLIB_DBF_ERROR_FAIL_TO_WRITE_RECORD;
	}
	
	return 0;
}

int TLib_Dbf_ClearRecordBuffer(int iDbfHandle)
{
	int iRetCode;
	
	TLib_Dbf_CheckInitiated();
	iRetCode = TLib_Dbf_CheckHandle(iDbfHandle);
	if (iRetCode < 0)
	{
		return iRetCode;
	}

	memset(apstDbf[iDbfHandle]->pcRecordBuffer, ' ', apstDbf[iDbfHandle]->lRecordLength);
	return 0;
}

int TLib_Dbf_CommitAppend(int iDbfHandle)
{
	long l, lRecordCount;
	int iRetCode;
	unsigned char caRecordCount[4];
	
	lRecordCount = TLib_Dbf_RecordCount(iDbfHandle);
	if (lRecordCount < 0)
	{
		return lRecordCount;
	}

   lRecordCount++;
   iRetCode = TLib_Dbf_Go(iDbfHandle, lRecordCount);
   if (iRetCode < 0)
   {
   	return iRetCode;
   }
   
   iRetCode = TLib_Dbf_CommitRecord(iDbfHandle);
   if (iRetCode < 0)
   {
   	return iRetCode;
   }

	
   l = lseek(apstDbf[iDbfHandle]->iFileHandle, 4, 0);
   if (l < 0)
   {
   	return TLIB_DBF_ERROR_FAIL_TO_SEEK;
   }

	caRecordCount[0] = lRecordCount % 256;
	lRecordCount = lRecordCount / 256;
	caRecordCount[1] = lRecordCount % 256;
	lRecordCount = lRecordCount / 256;
	caRecordCount[2] = lRecordCount % 256;
	lRecordCount = lRecordCount / 256;
	caRecordCount[3] = lRecordCount % 256;
	
	if (write(apstDbf[iDbfHandle]->iFileHandle, caRecordCount, 4) != 4)
	{
		return TLIB_DBF_ERROR_FAIL_TO_WRITE_HEADER;
	}
	
	return 0;
}

int TLib_Dbf_Deleted(int iDbfHandle)
{
	int iRetCode;
	
	TLib_Dbf_CheckInitiated();
	iRetCode = TLib_Dbf_CheckHandle(iDbfHandle);
	if (iRetCode < 0)
	{
		return iRetCode;
	}

	if (apstDbf[iDbfHandle]->pcRecordBuffer[0] != TLIB_DBF_MARK_DELETED)
		return 0;
	else
		return 1;
}

int TLib_Dbf_Delete(int iDbfHandle)
{
	int iRetCode;
	
	TLib_Dbf_CheckInitiated();
	iRetCode = TLib_Dbf_CheckHandle(iDbfHandle);
	if (iRetCode < 0)
	{
		return iRetCode;
	}

	apstDbf[iDbfHandle]->pcRecordBuffer[0] = TLIB_DBF_MARK_DELETED;
	return 0;
}

int TLib_Dbf_Undelete(int iDbfHandle)
{
	int iRetCode;
	
	TLib_Dbf_CheckInitiated();
	iRetCode = TLib_Dbf_CheckHandle(iDbfHandle);
	if (iRetCode < 0)
	{
		return iRetCode;
	}

	apstDbf[iDbfHandle]->pcRecordBuffer[0] = TLIB_DBF_MARK_UNDELETED;
	return 0;
}

int TLib_Dbf_Refresh(int iDbfHandle)
{
#ifndef T_LIB_UNIX

	int iRetCode;
	
	TLib_Dbf_CheckInitiated();
	iRetCode = TLib_Dbf_CheckHandle(iDbfHandle);
	if (iRetCode < 0)
	{
		return iRetCode;
	}

	lock(apstDbf[iDbfHandle]->iFileHandle, TLIB_DBF_LOCK_POSITION, 1);
	unlock(apstDbf[iDbfHandle]->iFileHandle, TLIB_DBF_LOCK_POSITION, 1);
	
#endif

	return 0;
}

void TLib_Dbf_CloseAll(void)
{
	int i;
	
	for (i = 0; i < TLIB_DBF_MAX_OPENED_NUM; i++)
	{
		TLib_Dbf_Close(i);
	}
}

