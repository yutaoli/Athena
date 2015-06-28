#ifndef _TLIB_INI_H_
#define _TLIB_INI_H_

#define	TLIB_INI_MAX_OPENED_NUM					255

#define	TLIB_INI_OPEN_NORMAL						0
#define	TLIB_INI_OPEN_FAIL_IGNORE				1

#define	TLIB_INI_ERROR_FAIL_TO_ALLOC_MEM		(TLIB_INI_ERROR_BASE-1)
#define	TLIB_INI_ERROR_NO_FREE_HANDLE			(TLIB_INI_ERROR_BASE-2)
#define	TLIB_INI_ERROR_FAIL_TO_OPEN_FILE		(TLIB_INI_ERROR_BASE-3)
#define	TLIB_INI_ERROR_INVALID_HANDLE			(TLIB_INI_ERROR_BASE-4)
#define	TLIB_INI_ERROR_FAIL_TO_READ_FILE		(TLIB_INI_ERROR_BASE-5)
#define	TLIB_INI_ERROR_FAIL_TO_WRITE			(TLIB_INI_ERROR_BASE-6)
#define	TLIB_INI_ERROR_FAIL_TO_SEEK			(TLIB_INI_ERROR_BASE-7)

int TLib_Ini_Load(char *sIniFileName, int iMode);
void TLib_Ini_Free(int iIniHandle);
int TLib_Ini_Dump(int iIniHandle);

int TLib_Ini_ReadString(int iIniHandle, char *sSection, char *sIdent, char *sDefault, char *sValue, int iSize);
int TLib_Ini_WriteString(int iIniHandle, char *sSection, char *sIdent, char *sValue);
int TLib_Ini_RemoveSection(int iIniHandle, char *sSection);
int TLib_Ini_RemoveKey(int iIniHandle, char *sSection, char *sIdent);

int TLib_Ini_Save(int iIniHandle, char *sIniFileName);
void TLib_Ini_FreeAll(void);

int TLib_Ini_ShmLoad(char *sShm, int iMode);

#endif

