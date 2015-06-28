#ifndef _TLIB_DBF_H_
#define _TLIB_DBF_H_

#define	TLIB_DBF_MAX_OPENED_NUM					255
#define	TLIB_DBF_OPEN_MODE_READ					0
#define	TLIB_DBF_OPEN_MODE_WRITE				1

#define	TLIB_DBF_ERROR_FAIL_TO_ALLOC_MEM		(TLIB_DBF_ERROR_BASE-1)
#define	TLIB_DBF_ERROR_NO_FREE_HANDLE			(TLIB_DBF_ERROR_BASE-2)
#define	TLIB_DBF_ERROR_FAIL_TO_OPEN_FILE		(TLIB_DBF_ERROR_BASE-3)
#define	TLIB_DBF_ERROR_FAIL_TO_READ_HEADER	(TLIB_DBF_ERROR_BASE-4)
#define	TLIB_DBF_ERROR_INVALID_HEADER			(TLIB_DBF_ERROR_BASE-5)
#define	TLIB_DBF_ERROR_INVALID_HANDLE			(TLIB_DBF_ERROR_BASE-6)
#define	TLIB_DBF_ERROR_FAIL_TO_SEEK			(TLIB_DBF_ERROR_BASE-7)
#define	TLIB_DBF_ERROR_INVALID_RECNO			(TLIB_DBF_ERROR_BASE-8)
#define	TLIB_DBF_ERROR_FAIL_TO_READ_RECORD	(TLIB_DBF_ERROR_BASE-9)
#define	TLIB_DBF_ERROR_INVALID_FIELD_NAME	(TLIB_DBF_ERROR_BASE-10)
#define	TLIB_DBF_ERROR_FAIL_TO_WRITE_RECORD	(TLIB_DBF_ERROR_BASE-11)
#define	TLIB_DBF_ERROR_FAIL_TO_WRITE_HEADER	(TLIB_DBF_ERROR_BASE-12)

/*********************************************************************************/

int TLib_Dbf_Open(char *sFileName, int iMode);
void TLib_Dbf_Close(int iDbfHandle);

long TLib_Dbf_RecordCount(int iDbfHandle);
int TLib_Dbf_Go(int iDbfHandle, long iRecordNo);

int TLib_Dbf_LoadRecord(int iDbfHandle);
int TLib_Dbf_ReadField(int iDbfHandle, char *sFieldName, char *sValue, int iValueSize);

int TLib_Dbf_ReplaceField(int iDbfHandle, char *sFieldName, char *sValue);
int TLib_Dbf_ReplaceFieldF(int iDbfHandle, char *sFieldName, const char *sFormat, ...);
int TLib_Dbf_CommitRecord(int iDbfHandle);

int TLib_Dbf_ClearRecordBuffer(int iDbfHandle);
int TLib_Dbf_CommitAppend(int iDbfHandle);

int TLib_Dbf_Deleted(int iDbfHandle);
int TLib_Dbf_Delete(int iDbfHandle);
int TLib_Dbf_Undelete(int iDbfHandle);

int TLib_Dbf_Refresh(int iDbfHandle);
void TLib_Dbf_CloseAll(void);

#endif

