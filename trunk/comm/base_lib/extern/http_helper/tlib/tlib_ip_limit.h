#ifndef _TLIB_IP_LIMIT_H_
#define _TLIB_IP_LIMIT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#define  C_MAX_BLOCK_NUM   255
#define  C_MAX_ENTRY_NUM   500

//#define  C_MAX_ACCESS_COUNT   100
//#define  C_MAX_LIMIT_TIME     10 //Minute
//#define  C_MAX_FreeBlockLIMIT_TIME     10 //Minute

#define  C_IP_LEN   20

typedef struct _TLIB_IP_ACCESS_ENTRY_
{
	char   sIP[C_IP_LEN];
	time_t iBeginTime;   //��ʼ�����ʱ��,�����Ѿ�����iAccessCount��������������,���Ϊ��ʼBlockס��ʱ��
	int    iAccessCount;
}TLIB_IP_ACCESS_ENTRY;


int  TLib_IP_Limit_Init(key_t iKey,int iLimitInterval,int iRecalInterval,int iMaxCount,char *sErrMsg);
int  TLib_IP_Limit_Get_Status(char *sIP,char *sErrMsg);
int  TLib_IP_Limit_Access(char *sIP,char *sErrMsg);
int  TLib_IP_Limit_ListBlockEntry(int iBlockIndex,char *sErrMsg);
int  TLib_IP_Limit_Destroy(char *sErrMsg);

/***************************************************************************************************
һ��Cgi����˳��:
    iRetCode=TLib_IP_Limit_Init(iKey,iLimitInterval,iRecalInterval,iMaxCount,0,sErrMsg);
    if (RetCode<0){
    	return -1;	
    }
    
    iRetCode= TLib_IP_Limit_Get_Status(sIP,sErrMsg);
    if (RetCode<0)
    	return -1	
    else if (RetCode==1)
       return 0; //blocked
    else if (RetCode==0){
    	iRetCode= TLib_IP_Limit_Access(sIP,sErrMsg);
    	if (RetCode<0){
           return -1;	
    	}
    }	
****************************************************************************************************/

#ifdef __cplusplus
}
#endif

#endif

