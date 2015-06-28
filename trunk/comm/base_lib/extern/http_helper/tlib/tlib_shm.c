#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>

#if defined __CYGWIN__
	#include <cygipc/sys/ipc.h>
	#include <cygipc/sys/shm.h>
#else
	#include <sys/ipc.h>
	#include <sys/shm.h>
#endif

#include <sys/sem.h>

#include "tlib_ini.h"
#include "tlib_shm.h"



void tlib_shm_log( char *szMsg, ... )
{
	char szTempBuf[C_SHM_TEMP_BUFFER_SIZE];
	va_list pArg;

	va_start(pArg, szMsg);
	vsnprintf(szTempBuf, C_SHM_TEMP_BUFFER_SIZE, szMsg, pArg);
	va_end(pArg);
   
	perror( szTempBuf );	
      
	return;
}


int  tlib_shm_GetSysParam( char *szCfgFile, int nMode, tagShmSysParam *pSysParam )
{
	int   nIniHandle;
	char  szTempBuf[C_SHM_TEMP_BUFFER_SIZE];
	
	int   i;	
	char  szDirNames[10][C_SHM_TEMP_BUFFER_SIZE];
	

	// use tlib_ini to get parameter from ini file 
	
	nIniHandle = TLib_Ini_Load(szCfgFile, TLIB_INI_OPEN_NORMAL);
	if (nIniHandle < 0)
	{
		tlib_shm_log("Fail To Load %s", szCfgFile);
		return -1;
	}
	
	TLib_Ini_ReadString(nIniHandle, "ShmParam", "ShmKey", "10000", szTempBuf, sizeof(szTempBuf));
	pSysParam->nShmKey = atoi( szTempBuf );
	
	TLib_Ini_ReadString(nIniHandle, "ShmParam", "ShmSize", "100000", szTempBuf, sizeof(szTempBuf));
	pSysParam->nShmSize = atoi( szTempBuf );
	
	TLib_Ini_ReadString(nIniHandle, "ShmParam", "ShmEntryNum", "100", szTempBuf, sizeof(szTempBuf));
	pSysParam->nShmEntryNum = atoi( szTempBuf );
	
	if ( nMode == C_SHM_GET_PARAM_CREATE )
	{
		for ( i=0; i<10; i++ )
		{
			snprintf( szTempBuf,C_SHM_TEMP_BUFFER_SIZE, "DIR%1d", i );
	
			TLib_Ini_ReadString(nIniHandle, "ShmParam", szTempBuf, "", szDirNames[i], sizeof(szDirNames[i]));
		}	
		
		TLib_Ini_ReadString(nIniHandle, "ShmParam", "FileNum", "0", szTempBuf, sizeof(szTempBuf));
		pSysParam->nFileNum = atoi( szTempBuf );
		
		if ((pSysParam->szFileNames = (char **)malloc(sizeof(char *)*(pSysParam->nFileNum))) == NULL)
		{
			tlib_shm_log("Fail to allocate memory for system parameter.\n");
			return -1;
		}
		
		for ( i=0; i<pSysParam->nFileNum; i++ )
		{
			if ((pSysParam->szFileNames[i] = (char *)malloc(C_SHM_TEMP_BUFFER_SIZE)) == NULL)
			{
				tlib_shm_log("Fail to allocate memory for system parameter.");
				return -1;
			}
				
			snprintf( szTempBuf,C_SHM_TEMP_BUFFER_SIZE, "FILE%d", i );
	
			TLib_Ini_ReadString(nIniHandle, "ShmParam", szTempBuf, "", pSysParam->szFileNames[i], C_SHM_TEMP_BUFFER_SIZE);
	
			if ( strlen(pSysParam->szFileNames[i]) > 6 && strncasecmp(pSysParam->szFileNames[i], "$DIR", 4) == 0 && isdigit( pSysParam->szFileNames[i][4] ) && pSysParam->szFileNames[i][5] == '$' )
			{
				snprintf( szTempBuf, C_SHM_TEMP_BUFFER_SIZE, "%s%s", szDirNames[pSysParam->szFileNames[i][4]-'0'], pSysParam->szFileNames[i]+6 );
				szTempBuf[C_SHM_TEMP_BUFFER_SIZE-1] = '\0';
				
				strncpy( pSysParam->szFileNames[i], szTempBuf ,strlen(szTempBuf)+1);			
			}
		}
	}
		
	TLib_Ini_Free(nIniHandle);
	
	return 0;
}

	
int tlib_shm_Create( char *szCfgFile )
{
   tagShmSysParam SysParam;

	int  nRc;
	
	int  nSizeWillshmget;
	
	int  nShmID;
	char *pShm;
		
	char *pContentArea;
	int  i;
	FILE *fp;
	long lFileLen;
	char *pContent;

	int  nLoadAll;


   // assume that all files will be load successfully
   
   nLoadAll = 1;
   
   
	// use tlib_ini to get parameter from ini file 
	
	nRc = tlib_shm_GetSysParam( szCfgFile, C_SHM_GET_PARAM_CREATE, &SysParam );
	if ( nRc < 0 )
	{
		tlib_shm_log( "Fail to get system parameter from %s.", szCfgFile );
		return -1;
	}
	
	
	// check the parameter get from ini file
	
	if ( SysParam.nShmSize <= 0 || SysParam.nShmEntryNum <= 0 )
	{
		tlib_shm_log( "Wrong parameter, ShmSize : %d, ShmEntryNum : %d.", SysParam.nShmSize, SysParam.nShmEntryNum );
		return -1;
	}
	
	
	// allocate the share memory block include ShmControlBlock, ShmContentEntry
	
	nSizeWillshmget = sizeof(tagShmControlBlock) + SysParam.nShmEntryNum * sizeof(tagShmContentEntry) + SysParam.nShmSize;
	
	
	if ((nShmID = shmget(SysParam.nShmKey, nSizeWillshmget, IPC_CREAT|IPC_EXCL|0666)) < 0)
	{
		tlib_shm_log("Fail to shmget. nShmKey is %d", SysParam.nShmKey);
		return -1;
	}
			
	if (shmctl(nShmID, SHM_LOCK, NULL) < 0) 
	{
		tlib_shm_log("Fail to shmctl with SHM_LOCK. nShmID is %d", nShmID);
		return -1;
	}
	
	if ((pShm = shmat(nShmID, NULL ,0)) == (char *) -1) 
	{
		tlib_shm_log("Fail to shmat. nShmID is %d", nShmID);
		return -1;
	}
	
	
	// init ShmControlBlock
	
	M_SHM_CTRL_BLK(pShm)->nShmContentAreaSize = SysParam.nShmSize;
	M_SHM_CTRL_BLK(pShm)->nShmContentAreaUsed = 0;
	M_SHM_CTRL_BLK(pShm)->nContentEntryNum = SysParam.nShmEntryNum;
	M_SHM_CTRL_BLK(pShm)->nContentEntryUsed = 0;
	
	
	// load files into share memory and init ShmContentEntry
	
	pContentArea = pShm + sizeof(tagShmControlBlock) + M_SHM_CTRL_BLK(pShm)->nContentEntryNum * sizeof(tagShmContentEntry);
	
	for ( i=0; i<SysParam.nFileNum; i++ )
	{
		if ( M_SHM_CTRL_BLK(pShm)->nContentEntryUsed >= M_SHM_CTRL_BLK(pShm)->nContentEntryNum )
		{
			tlib_shm_log( "ContentEntry table is full." );
			nLoadAll = 0;
			break;
		}
		
		if ( (fp = fopen(SysParam.szFileNames[i], "rb")) == NULL )
		{
			tlib_shm_log( "Can't open %s to load.", SysParam.szFileNames[i] );
			nLoadAll = 0;
			continue;
		}
		
		if ( fseek(fp, 0L, SEEK_END) != 0 )
		{
			fclose( fp );
			tlib_shm_log( "Can't seek end of %s", SysParam.szFileNames[i] );
			nLoadAll = 0;
			continue;
		}

		lFileLen = ftell( fp );
		if ( lFileLen < 0 )
		{
			fclose( fp );
			tlib_shm_log( "Can't obtain length of %s.", SysParam.szFileNames[i] );
			nLoadAll = 0;
			continue;
		}
		
		if ( fseek(fp, 0L, SEEK_SET) != 0 )
		{
			fclose( fp );
			tlib_shm_log( "Can't seek begin of %s", SysParam.szFileNames[i] );
			nLoadAll = 0;
			continue;
		}

		if ( lFileLen > M_SHM_CTRL_BLK(pShm)->nShmContentAreaSize - M_SHM_CTRL_BLK(pShm)->nShmContentAreaUsed )
		{
			fclose( fp );
			tlib_shm_log( "ContentArea is full." );
			nLoadAll = 0;
			break;
		}
			
		strncpy( M_SHM_CONTENT_ENTRY(pShm, M_SHM_CTRL_BLK(pShm)->nContentEntryUsed)->szEntryName, SysParam.szFileNames[i] ,strlen(SysParam.szFileNames[i])+1);
		
		M_SHM_CONTENT_ENTRY(pShm, M_SHM_CTRL_BLK(pShm)->nContentEntryUsed)->nEntrySize = lFileLen;
		
		M_SHM_CONTENT_ENTRY(pShm, M_SHM_CTRL_BLK(pShm)->nContentEntryUsed)->nOffsetInContentArea = M_SHM_CTRL_BLK(pShm)->nShmContentAreaUsed;

		pContent = pContentArea + M_SHM_CTRL_BLK(pShm)->nShmContentAreaUsed;
		
		nRc = fread(pContent, sizeof(char), lFileLen, fp);		
		if ( nRc != lFileLen )
		{
			fclose( fp );
			tlib_shm_log( "Can't read %s, fread : %ld, ftell : %ld", SysParam.szFileNames[i], nRc, lFileLen );
			nLoadAll = 0;
			continue;
		}
		
		fclose( fp );
		
		tlib_shm_log( "load %s successfully.", SysParam.szFileNames[i] );
		
		
		M_SHM_CTRL_BLK(pShm)->nContentEntryUsed++;
		M_SHM_CTRL_BLK(pShm)->nShmContentAreaUsed += lFileLen;
		
	}
	
	
	if ( shmdt( pShm ) < 0 )
	{
		tlib_shm_log("Fail to shmdt. nShmID is %d", nShmID);
		return -1;
	}
	
	
	if ( nLoadAll == 0 )
		return -2;
			
	return 0;	
	
}	


int  tlib_shm_Destroy( char *szCfgFile )
{
   tagShmSysParam SysParam;

	int  nRc;	

	int  nSizeWillshmget;	

	int  nShmID;
	

	// use tlib_ini to get parameter from ini file 
	
	nRc = tlib_shm_GetSysParam( szCfgFile, C_SHM_GET_PARAM_OTHER, &SysParam );

	if ( nRc < 0 )
	{
		tlib_shm_log( "Fail to get system parameter from %s.", szCfgFile );
		return -1;
	}
	
	
	// check the parameter get from ini file
	
	if ( SysParam.nShmSize <= 0 || SysParam.nShmEntryNum <= 0 )
	{
		tlib_shm_log( "Wrong parameter, ShmSize : %d, ShmEntryNum : %d.", SysParam.nShmSize, SysParam.nShmEntryNum );
		return -1;
	}
	
	
	// allocate the share memory block include ShmControlBlock, ShmContentEntry
	
	nSizeWillshmget = sizeof(tagShmControlBlock) + SysParam.nShmEntryNum * sizeof(tagShmContentEntry) + SysParam.nShmSize;
		
	if ((nShmID = shmget(SysParam.nShmKey, nSizeWillshmget, 0666)) < 0)
	{
		tlib_shm_log("Fail to shmget. nShmKey is %d", SysParam.nShmKey);
		return -1;
	}
			
	if (shmctl(nShmID, IPC_RMID, NULL) < 0) 
	{
		tlib_shm_log("Fail to shmctl with IPC_RMID. s_nShmID is %d", nShmID);
		return -1;
	}


	return 0;	
}


int  tlib_shm_Open( char *szCfgFile, tagShmHandle *pShmHandle )
{
   tagShmSysParam SysParam;

	int  nRc;
	
	int  nSizeWillshmget;
	

	// use tlib_ini to get parameter from ini file 
	
	nRc = tlib_shm_GetSysParam( szCfgFile, C_SHM_GET_PARAM_OTHER, &SysParam );

	if ( nRc < 0 )
	{
		tlib_shm_log( "Fail to get system parameter from %s.", szCfgFile );
		return -1;
	}
	
	
	// check the parameter get from ini file
	
	if ( SysParam.nShmSize <= 0 || SysParam.nShmEntryNum <= 0 )
	{
		tlib_shm_log( "Wrong parameter, ShmSize : %d, ShmEntryNum : %d.", SysParam.nShmSize, SysParam.nShmEntryNum );
		return -1;
	}
	
	
	// allocate the share memory block include ShmControlBlock, ShmContentEntry
	
	nSizeWillshmget = sizeof(tagShmControlBlock) + SysParam.nShmEntryNum * sizeof(tagShmContentEntry) + SysParam.nShmSize;
	
	
	if ((pShmHandle->nShmID = shmget(SysParam.nShmKey, nSizeWillshmget, 0666)) < 0)
	{
		tlib_shm_log("Fail to shmget. nShmKey is %d", SysParam.nShmKey);
		return -1;
	}
			
	if ((pShmHandle->pShm = shmat(pShmHandle->nShmID, NULL ,0)) == (char *) -1) 
	{
		tlib_shm_log("Fail to shmat. nShmID is %d", pShmHandle->nShmID);
		return -1;
	}
	
	pShmHandle->nShmKey = SysParam.nShmKey;
	
	strncpy( pShmHandle->szShmName, szCfgFile, C_SHM_TEMP_BUFFER_SIZE );
	pShmHandle->szShmName[C_SHM_TEMP_BUFFER_SIZE-1] = '\0';
	
	
	return 0;
}


int  tlib_shm_Close( tagShmHandle *pShmHandle )
{
	if ( shmdt( pShmHandle->pShm ) < 0 )
	{
		tlib_shm_log("Fail to shmdt. nShmID is %d", pShmHandle->nShmID);
		return -1;
	}
		
	return 0;	
}


int  tlib_shm_Get( tagShmHandle *pShmHandle, char *szContentName, int nGetOption, char **ppContent, int *pnContentLen )
{
	int  i;
	char *pContentArea;
	

	pContentArea = pShmHandle->pShm + sizeof(tagShmControlBlock) + M_SHM_CTRL_BLK(pShmHandle->pShm)->nContentEntryNum * sizeof(tagShmContentEntry);

	for ( i = 0; i < M_SHM_CTRL_BLK(pShmHandle->pShm)->nContentEntryUsed; i++ )
	{
		if ( strcmp( M_SHM_CONTENT_ENTRY(pShmHandle->pShm, i)->szEntryName, szContentName ) == 0 )
			break;
	}
	
	if ( i == M_SHM_CTRL_BLK(pShmHandle->pShm)->nContentEntryUsed )
		return -1;
	
	if ( nGetOption == C_SHM_GET_OPTION_COPY )
	{	
		if ((*ppContent = (char *)malloc( M_SHM_CONTENT_ENTRY(pShmHandle->pShm, i)->nEntrySize + 1 )) == NULL)
		{
			tlib_shm_log( "Can't allocate memory for tlib_shm_get." );
			return -1;
		}		
		
		memcpy( *ppContent, pContentArea + M_SHM_CONTENT_ENTRY(pShmHandle->pShm, i)->nOffsetInContentArea, M_SHM_CONTENT_ENTRY(pShmHandle->pShm, i)->nEntrySize );
		
		*( *ppContent + M_SHM_CONTENT_ENTRY(pShmHandle->pShm, i)->nEntrySize ) = '\0';
	}
	else
	{
		*ppContent = pContentArea + M_SHM_CONTENT_ENTRY(pShmHandle->pShm, i)->nOffsetInContentArea;
	}
	
	*pnContentLen = M_SHM_CONTENT_ENTRY(pShmHandle->pShm, i)->nEntrySize;
		
	return 0;		
}


int  tlib_shm_Put( tagShmHandle *pShmHandle, char *szContentName, char *pContent, int nContentLen )
{
	return 0;		
}


int  tlib_shm_Dump( tagShmHandle *pShmHandle )
{
	int i;
	
	printf( "\nShmControlBlock dump: %d, %d, %d ,%d\n", 
	         M_SHM_CTRL_BLK(pShmHandle->pShm)->nShmContentAreaSize,
				M_SHM_CTRL_BLK(pShmHandle->pShm)->nShmContentAreaUsed,
				M_SHM_CTRL_BLK(pShmHandle->pShm)->nContentEntryNum,
				M_SHM_CTRL_BLK(pShmHandle->pShm)->nContentEntryUsed );

	printf( "\nShmContentEntry list:\n" );
	
	for ( i = 0; i < M_SHM_CTRL_BLK(pShmHandle->pShm)->nContentEntryUsed; i++ )
	{
		printf( "%s, %d, %d\n", 
		         M_SHM_CONTENT_ENTRY(pShmHandle->pShm, i)->szEntryName,
					M_SHM_CONTENT_ENTRY(pShmHandle->pShm, i)->nEntrySize,
					M_SHM_CONTENT_ENTRY(pShmHandle->pShm, i)->nOffsetInContentArea );
	}
	
	return 0;		
}


int  tlib_shm_HealthTest( tagShmHandle *pShmHandle )
{
	return 0;	
}


