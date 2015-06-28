#include "tlib_com.h"
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "tlib_lck.h"



void Tlib_Lck_Log( char *szMsg, ... )
{
	char szTempBuf[C_LCK_TEMP_BUFFER_SIZE];
	va_list pArg;

	va_start(pArg, szMsg);
	vsnprintf(szTempBuf, C_LCK_TEMP_BUFFER_SIZE, szMsg, pArg);
	va_end(pArg);
   
	perror( szTempBuf );	
      
	return;
}


int  Tlib_Lck_Create( key_t nSemKey, tagTlibLckHandle **ppLckHandle )
{
#ifndef SLACKWARE
	union semun {
		int val;
   	struct semid_ds *buf;
   	ushort *array;
	};	
#endif

	union semun arg;
	
	int  nSemID;
	
	u_short array[2] = { 0, 0 };
	
	
	if ( (*ppLckHandle = (tagTlibLckHandle *)malloc(sizeof(tagTlibLckHandle))) == NULL )
	{
		Tlib_Lck_Log( "Can't allocate memory for tagTlibLckHandle." );
		return -1;
	}		

	if ( (nSemID = semget( nSemKey, 2, IPC_CREAT | IPC_EXCL )) != -1 )
	{
		arg.array = &array[0];
		
		if ( semctl( nSemID, 0, SETALL, arg ) == -1 )
		{
			free( *ppLckHandle );
			Tlib_Lck_Log( "Can't initialize the semaphore value." );
			return -1;
		}
	}
	else
	{
		if ( errno != EEXIST )
		{
			free( *ppLckHandle );
			Tlib_Lck_Log( "Fail to semget." );
			return -1;
		}

		if ( (nSemID = semget( nSemKey, 2, 0 )) == -1 )
		{
			free( *ppLckHandle );
			Tlib_Lck_Log( "Fail to semget." );
			return -1;
		}		
		
	}		

	(*ppLckHandle)->nSemKey = nSemKey;
	(*ppLckHandle)->nSemID = nSemID;

	return 0;				
}


int  Tlib_Lck_Destroy( tagTlibLckHandle *pLckHandle )
{
/*	
#ifndef SLACKWARE
	union semun {
		int val;
		struct semid_ds *buf;
		ushort *array;
	};	
#endif

	if ( semctl( pLckHandle->nSemID, 0, IPC_RMID, (union semun)0 ) == -1 )
		return -1;
*/
	free( pLckHandle );
		
	return 0;
}


int  Tlib_Lck_EnterShare( tagTlibLckHandle *pLckHandle )
{
	struct sembuf sops[2] = { {0, 0, 0}, {1, 1, 0} };
	size_t nsops = 2;

	if ( semop( pLckHandle->nSemID, &sops[0], nsops ) == -1 )
		return -1;
		
	return 0;
}


int  Tlib_Lck_LeaveShare( tagTlibLckHandle *pLckHandle )
{
	struct sembuf sops[1] = { {1, -1, 0} };
	size_t nsops = 1;

	if ( semop( pLckHandle->nSemID, &sops[0], nsops ) == -1 )
		return -1;
		
	return 0;	
}


int  Tlib_Lck_EnterExclusive( tagTlibLckHandle *pLckHandle )
{
	struct sembuf sops[3] = { {0, 0, 0}, {1, 0, 0}, {0, 1, 0} };
	size_t nsops = 3;

	if ( semop( pLckHandle->nSemID, &sops[0], nsops ) == -1 )
		return -1;
		
	return 0;		
}


int  Tlib_Lck_LeaveExclusive( tagTlibLckHandle *pLckHandle )
{
	struct sembuf sops[1] = { {0, -1, 0} };
	size_t nsops = 1;

	if ( semop( pLckHandle->nSemID, &sops[0], nsops ) == -1 )
		return -1;
		
	return 0;			
}


int  Tlib_Lck_Dump( tagTlibLckHandle *pLckHandle )
{
#ifndef SLACKWARE
	union semun {
		int val;
   	struct semid_ds *buf;
   	ushort *array;
	};	
#endif

	union  semun  arg;
	
	struct semid_ds  l_semid_ds;

	int    i;
	int    nRc;
	

	arg.buf = &l_semid_ds;
	
	if ( semctl( pLckHandle->nSemID, 0, IPC_STAT, arg ) == -1 )
	{
		Tlib_Lck_Log( "Can't execute semctl with IPC_STAT." );
		return -1;
	}

	fprintf( stdout, "Dump struct semid_ds:\n" );
	
	fprintf( stdout, "sem_perm.uid = %d\n", (arg.buf)->sem_perm.uid );
	fprintf( stdout, "sem_perm.gid = %d\n", (arg.buf)->sem_perm.gid );
	fprintf( stdout, "sem_perm.mode = %o\n", (arg.buf)->sem_perm.mode );
	
	fprintf( stdout, "sem_nsems = %d\n", (arg.buf)->sem_nsems );
		
	for ( i = 0; i < (arg.buf)->sem_nsems; i++ )
	{		
		if ( (nRc = semctl( pLckHandle->nSemID, i, GETNCNT, arg )) == -1 )
		{
			Tlib_Lck_Log( "Can't execute semctl with GETNCNT." );
			return -1;
		}
		fprintf( stdout, "ncnt = %d " , nRc );

		if ( (nRc = semctl( pLckHandle->nSemID, i, GETZCNT, arg )) == -1 )
		{
			Tlib_Lck_Log( "Can't execute semctl with GETZCNT." );
			return -1;
		}		
		fprintf( stdout, "zcnt = %d " , nRc );

		if ( (nRc = semctl( pLckHandle->nSemID, i, GETVAL, arg )) == -1 )
		{
			Tlib_Lck_Log( "Can't execute semctl with GETVAL." );
			return -1;
		}		
		fprintf( stdout, "val = %d " , nRc );

		if ( (nRc = semctl( pLckHandle->nSemID, i, GETPID, arg )) == -1 )
		{
			Tlib_Lck_Log( "Can't execute semctl with GETPID." );
			return -1;
		}
		fprintf( stdout, "pid = %d\n" , nRc );
	}

	return 0;
}

