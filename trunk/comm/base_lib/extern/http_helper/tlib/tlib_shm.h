#ifndef _TLIB_SHM_H_
#define _TLIB_SHM_H_


#include <sys/ipc.h>
#include <sys/shm.h>


#define  C_SHM_CONTENT_NAME_LEN   1024
#define  C_SHM_TEMP_BUFFER_SIZE   1024

#define  C_SHM_GET_PARAM_OTHER    0
#define  C_SHM_GET_PARAM_CREATE   1

#define  C_SHM_GET_OPTION_REF     0
#define  C_SHM_GET_OPTION_COPY    1

#define  M_SHM_CTRL_BLK(BaseAddr)   	  ((tagShmControlBlock *)BaseAddr)
#define  M_SHM_CONTENT_ENTRY(BaseAddr,i) ((tagShmContentEntry *)(BaseAddr+sizeof(tagShmControlBlock)+i*sizeof(tagShmContentEntry)))


typedef struct _tagShmContentEntry
{
	char  szEntryName[C_SHM_CONTENT_NAME_LEN];
	int   nEntryID;        //Reserved
	int   nEntrySize;
	int   nOffsetInContentArea;
	int   nEntryProperty;  //Reserved
} tagShmContentEntry;


typedef struct _tagShmControlBlock
{
	int   nShmContentAreaSize;
	int   nShmContentAreaUsed;
	int   nContentEntryNum;
	int   nContentEntryUsed;
} tagShmControlBlock;


typedef struct _tagShmSysParam 
{
	key_t nShmKey;
	int   nShmSize;
	int   nShmEntryNum;
	int   nFileNum;
	char  **szFileNames;	
} tagShmSysParam;
	

typedef struct _tagShmHandle
{
	char  szShmName[C_SHM_TEMP_BUFFER_SIZE];
	key_t nShmKey;
	int   nShmID;
	char  *pShm;
} tagShmHandle;	
	

int  tlib_shm_Create( char *szCfgFile );
int  tlib_shm_Destroy( char *szCfgFile );

int  tlib_shm_Open( char *szCfgFile, tagShmHandle *pShmHandle );
int  tlib_shm_Close( tagShmHandle *pShmHandle );

int  tlib_shm_Get( tagShmHandle *pShmHandle, char *szContentName, int nGetOption, char **ppContent, int *pnContentLen );
int  tlib_shm_Put( tagShmHandle *pShmHandle, char *szContentName, char *pContent, int nContentLen );
	
int  tlib_shm_Dump( tagShmHandle *pShmHandle );
int  tlib_shm_HealthTest( tagShmHandle *pShmHandle );

#endif

