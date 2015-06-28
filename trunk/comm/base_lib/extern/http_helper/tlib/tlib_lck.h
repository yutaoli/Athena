#ifndef _TLIB_LCK_H_
#define _TLIB_LCK_H_


#include <sys/ipc.h>
#include <sys/sem.h>


#define  C_LCK_TEMP_BUFFER_SIZE   1024


typedef struct _tagTlibLckHandle
{
	char  szLckName[C_LCK_TEMP_BUFFER_SIZE];
	key_t nSemKey;
	int   nSemID;
} tagTlibLckHandle;	
	

int  Tlib_Lck_Create( key_t nSemKey, tagTlibLckHandle **ppLckHandle );
int  Tlib_Lck_Destroy( tagTlibLckHandle *pLckHandle );

int  Tlib_Lck_EnterShare( tagTlibLckHandle *pLckHandle );
int  Tlib_Lck_LeaveShare( tagTlibLckHandle *pLckHandle );

int  Tlib_Lck_EnterExclusive( tagTlibLckHandle *pLckHandle );
int  Tlib_Lck_LeaveExclusive( tagTlibLckHandle *pLckHandle );

int  Tlib_Lck_Dump( tagTlibLckHandle *pLckHandle );

#endif

