#ifndef _TLIB_IP_LIMIT_C_
#define _TLIB_IP_LIMIT_C_

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <string.h>

#include "tlib_ip_limit.h"

#define   Debug 	0

static int iMaxAccessCount=100;		//某一时间段内,一个IP最大访问数
static int iLimitTimeInterval=10;	//限制最大访问数的时间段,以分钟为单位
static int iRecalTimeInterval=10;	//访问计数到了最大值后,可以再次重新计算访问数的时间段,以分钟为单位
static int iShmID=-1;			//共享内存标识名
/*******-***********************************************************-****************************
Funciton    初始化IP访问限制共享内存区
input       iKey: 共享内存的标识
	    iLimitInterval:	//限制最大访问数的时间段,以分钟为单位 	
	    iRecalInterval:	//到了最大值后,可以重新计数的时间段,以分钟为单位
	    iMaxCount	:	//某一时间段内,一个IP最大访问数
output      sErrMsg		//错误信息	    
return:     0: success <0:Error
lastmodify  2000.06.29 //去掉了iRefresh参数
written by  Dennnis
*******-*****************************************************************************************/	
int  TLib_IP_Limit_Init(key_t iKey,int iLimitInterval,int iRecalInterval,int iMaxCount,char *sErrMsg)
{
	
	char *pShm;
	int iShmSize;
	int iFirst=1;
	
	//calculate the size of the shareMemory
	iShmSize=sizeof(TLIB_IP_ACCESS_ENTRY)*C_MAX_ENTRY_NUM*C_MAX_BLOCK_NUM;	
	if ((iShmID = shmget(iKey, iShmSize, IPC_CREAT | IPC_EXCL | 0666)) < 0)  //试图创建
	{
		if ((iShmID = shmget(iKey, iShmSize, 0666))<0)                   //有可能是已经存在同样的key_shm,则试图连接
		{
			snprintf(sErrMsg,1024,"Fail to shmget. ShmKey is %d", iKey);
			return -1;
		}
		iFirst=0;
	}
	
	//try to access shm 
	if ((pShm = shmat(iShmID, NULL ,0)) == (char *) -1) 
	{
		snprintf(sErrMsg,1024,"Fail to shmat. ShmID is %d", iShmID);
		//if access failed, try to remove the shareMemory
		shmctl(iShmID, IPC_RMID, NULL);
		return -2;
	}
	
	if (iFirst){
		memset(pShm,0,iShmSize);
	}	
	
	if ( shmdt(pShm) < 0 )
	{
		snprintf(sErrMsg,1024,"Fail to shmdt. ShmID is %d", iShmID);
		return -3;
	}
	
	iLimitTimeInterval=iLimitInterval;
	iRecalTimeInterval=iRecalInterval;
	iMaxAccessCount=iMaxCount;
	
	return 0;	
}	

int GetIPLastByte(char *pStr){
	char *pStrIndex;
	int i;
	
	for (pStrIndex=pStr+strlen(pStr)-1;pStrIndex>pStr;pStrIndex--){
		if (pStrIndex[0]=='.')
			return atoi(pStrIndex+1);
	}
	
	return 0;
	
}

/**********************************************************************************************************************
Funciton    取得指定IP的状态
input       sIP: 	//访问的IP 
output      sErrMsg	//错误信息	    
return:     0: 		//此IP还没有到达最大访问数
	    1: 		//此IP已经到达最大访问数, 并且给Block住了
	    <0:         //Error
lastmodify  2000.06.20
written by  Dennnis
处理逻辑：  1.根据IP最后一个byte定位内存访问块索引 
	    2.如果内存访问块中没有同样的IP记录，返回 0
	    3.如果内存访问块中有同样的IP记录，
	       。如果访问记录>最大数，并且没有超过重新计算时间段，则返回 1
	       。如果访问记录<=最大数，并且超过重新计算时间段内， 返回 0
***********************************************************************************************************************/	
int  TLib_IP_Limit_Get_Status(char *sIP,char *sErrMsg)
{
	char *pShm;
	TLIB_IP_ACCESS_ENTRY *pstEntry;
	TLIB_IP_ACCESS_ENTRY *pstFirstEntry;
	int iBlockIndex;
	time_t time_now;
	int iRetCode,i,iFound;	
	
	iBlockIndex=GetIPLastByte(sIP);
	if ((iBlockIndex==0) || (iBlockIndex>C_MAX_BLOCK_NUM)){
		snprintf(sErrMsg,1024,"Invalid IP %s",sIP);
		return -1;	
	}
	
	if (Debug)
		printf("ip=%s, iBlockIndex=%d\n",sIP,iBlockIndex);
		
	if (iShmID==-1){
		snprintf(sErrMsg,1024,"The ShareMemory hasn't beend Initiate");
		return -2;	
	}
	
	//try to access shm 
	if ((pShm = shmat(iShmID, NULL ,0)) == (char *) -1) 
	{
		snprintf(sErrMsg,1024,"Fail to shmat. Shmid is %d", iShmID);
		return -3;
	}
	
	pstFirstEntry=(TLIB_IP_ACCESS_ENTRY *)pShm;
	
	//首先定位在哪一个内存块中
	pstFirstEntry=(TLIB_IP_ACCESS_ENTRY *)(pstFirstEntry+(iBlockIndex-1)*C_MAX_ENTRY_NUM);	
	pstEntry=pstFirstEntry;
	
	iRetCode=0;
	for (i=0;i<C_MAX_ENTRY_NUM;i++){
		 if (strcmp(pstEntry->sIP,"")==0){//第一个为空的记录,内存块未满
			break;
		 }else if (strcmp(pstEntry->sIP,sIP)==0){ //判断是否在访问表中找到IP
			if (Debug) 
				printf("found at %d\n",i);
			//判断是否已经超过最大次数	
			if (pstEntry->iAccessCount>iMaxAccessCount){
				//如果已经超过,则判断是否超过可以重新计数的时间段
				time(&time_now);
				if ((time_now-pstEntry->iBeginTime)<iRecalTimeInterval*60){
					iRetCode=1; //还没有超过block的时间段,给Block住了
				}
			}	
			break;
		}	
		pstEntry++;
	}		
	
	
	shmdt(pShm);
	
	return iRetCode;	
}

/**********************************************************************************************************************
Funciton    计算IP访问数
input       sIP: 	//访问的IP 
output      sErrMsg	//错误信息	    
return:     0: 		//此IP还没有到达最大访问数 		    
	    1: 		//此IP已经到达最大访问数
	    <0:         //Error
lastmodify  2000.06.20
written by  Dennnis
处理逻辑：  1.根据IP最后一个byte定位内存访问块索引 
	    2.如果内存访问块中没有同样的IP记录，找出第一个空记录设置ip,iBeginTime=now()和访问计数=1 返回 0
	    3.如果内存访问块中没有同样的IP记录，而且内存块已满,找出现在内存块中的最小访问计数的记录，替换掉其记录 返回 0
	    4.如果内存访问块中有同样的IP记录，
	       。如果访问记录>最大数，并且没有超过重新计算时间段，且则返回 1
	       。如果访问记录>最大数，并且超过重新计算时间段内，则将此记录iBeginTime=now()和访问计数=1 返回 0
	       。如果++访问记录>最大数，并且超过访问限制时间段，则将此记录iBeginTime=now()和访问计数=1 返回 0
	       。如果++访问记录>最大数，并且没有访问限制时间段，则返回 1	       
***********************************************************************************************************************/	
int  TLib_IP_Limit_Access(char *sIP,char *sErrMsg)
{
	char *pShm;
	TLIB_IP_ACCESS_ENTRY *pstEntry;
	TLIB_IP_ACCESS_ENTRY *pstFirstEntry;
	int iBlockIndex;
	int iMinAccessCount;
	int iMinCountIndex;
	time_t time_now;
	int iRetCode,i,iFound;	
	
	
	
	iBlockIndex=GetIPLastByte(sIP);
	if ((iBlockIndex==0) || (iBlockIndex>C_MAX_BLOCK_NUM)){
		snprintf(sErrMsg,1024,"Invalid IP %s",sIP);
		return -1;	
	}
	
	if (Debug)
		printf("ip=%s, iBlockIndex=%d\n",sIP,iBlockIndex);
		
	if (iShmID==-1){
		snprintf(sErrMsg,1024,"The ShareMemory hasn't beend Initiate");
		return -2;	
	}
	
	//try to access shm 
	if ((pShm = shmat(iShmID, NULL ,0)) == (char *) -1) 
	{
		snprintf(sErrMsg,1024,"Fail to shmat. Shmid is %d", iShmID);
		return -3;
	}
	
	pstFirstEntry=(TLIB_IP_ACCESS_ENTRY *)pShm;
	
	//首先定位在哪一个内存块中
	pstFirstEntry=(TLIB_IP_ACCESS_ENTRY *)(pstFirstEntry+(iBlockIndex-1)*C_MAX_ENTRY_NUM);	
	pstEntry=pstFirstEntry;
	
	//计算访问次数
	iMinAccessCount=pstEntry->iAccessCount;
	iMinCountIndex=0;
	iFound=0;
	iRetCode=0;
		
	for (i=0;i<C_MAX_ENTRY_NUM;i++){
		 if (strcmp(pstEntry->sIP,"")==0){//第一个为空的记录,内存块未满
			strncpy(pstEntry->sIP,sIP,sizeof(pstEntry->sIP)-1);
			pstEntry->iAccessCount=1;	
			time(&time_now);				
			pstEntry->iBeginTime=time_now; //第一次访问的时间		
			break;
		 }else if (strcmp(pstEntry->sIP,sIP)==0){ //判断是否在访问表中找到IP
			iFound=1;
			if (Debug) 
				printf("found at %d\n",i);
			//判断是否已经超过最大次数	
			if (pstEntry->iAccessCount>iMaxAccessCount){
				//如果已经超过,则判断是否已经到了重新计数的时间
				time(&time_now);
				if ((time_now-pstEntry->iBeginTime)>iRecalTimeInterval*60){
					//重新计数
					pstEntry->iBeginTime=time_now;
					pstEntry->iAccessCount=1;
				}else{
					iRetCode=1;
				}
			}else if(++pstEntry->iAccessCount>iMaxAccessCount){//+1后超过最大次数
				//尽管超过最大的计数，如果也过了限制的时间段，就从头算起
				time(&time_now);
				if ((time_now-pstEntry->iBeginTime)>iLimitTimeInterval*60){ 
					//重新计数
					pstEntry->iBeginTime=time_now;
					pstEntry->iAccessCount=1;
				}else{ //否则就将计算时间设置为第一次超过最大数的时间
					pstEntry->iBeginTime=time_now; //第一次超过最大数的时间, 也是开始给block的时间
					iRetCode=1;
				}
			}
			break;
		 }else if (pstEntry->iAccessCount<iMinAccessCount){ //如果IP不为空,则找出最小访问数
			iMinAccessCount=pstEntry->iAccessCount;
			iMinCountIndex=i;
		 }
		 pstEntry++;
	}		
	
	//在访问表中没找到IP,且内存块已满, 则替换最小访问数的记录
	if ((!iFound) && (i>=C_MAX_ENTRY_NUM)){
		if (Debug)
			printf("replace MinCount=%d MinIndex=%d\n",iMinAccessCount,iMinCountIndex);
		pstEntry=pstFirstEntry+iMinCountIndex; //替换指针
		strncpy(pstEntry->sIP,sIP,sizeof(pstEntry->sIP)-1);
		pstEntry->iAccessCount=1;	
		time(&time_now);				
		pstEntry->iBeginTime=time_now; //第一次访问的时间		
	} 
	
	shmdt(pShm);
	
	return iRetCode;	
}




/*******-**********************************************************-*****************************************************
Funciton    列出内存块中某一块的信息
input       iBlockIndex://内存索引
output      sErrMsg	//错误信息	    
return:     0: 		:success
            <0		:failed 
lastmodify  2000.06.20
written by  Dennnis
attention:  CGI一般不要调用,此函数是为了查看内存调试而设
*******-****************************************************************************************************************/	
int TLib_IP_Limit_ListBlockEntry(int iBlockIndex,char *sErrMsg)
{

	char *pShm;
	TLIB_IP_ACCESS_ENTRY *pstEntry;	
	int i;
	
	if ((iBlockIndex==0) || (iBlockIndex>C_MAX_BLOCK_NUM)){
		snprintf(sErrMsg,1024,"Invalid BlockIndex %d ",iBlockIndex);
		return -1;	
	}
	
	if (iShmID==-1){
		snprintf(sErrMsg,1024,"The ShareMemory hasn't beend Initiate");
		return -2;	
	}
	
	//try to access shm 
	if ((pShm = shmat(iShmID, NULL ,0)) == (char *) -1) 
	{
		snprintf(sErrMsg,1024,"Fail to shmat. Shmid is %d", iShmID);
		return -3;
	}
	
	pstEntry=(TLIB_IP_ACCESS_ENTRY *)pShm;
	
	//首先内存块中定位
	pstEntry=(TLIB_IP_ACCESS_ENTRY *)(pstEntry+(iBlockIndex-1)*C_MAX_ENTRY_NUM);
	
	
	printf("%d Block Table\n",iBlockIndex);
	for (i=0;i<C_MAX_ENTRY_NUM;i++){
		printf("Entry[%d].sIP=%s\n",i,pstEntry->sIP);
		printf("Entry[%d].iBeginTime=%s",i,ctime(&(pstEntry->iBeginTime)));
		printf("Entry[%d].iAccessCount=%d\n\n",i,pstEntry->iAccessCount);
		pstEntry++;
		
	}
	
	shmdt(pShm);	
	return 0;
}

/*******-**********************************************************-*****************************************************
Funciton    释放共享内存
input       
output      sErrMsg	//错误信息	    
return:     0: 		:success
            <0		:failed 
lastmodify  2000.06.20
written by  Dennnis
attention:  CGI一般不要调用
*******-****************************************************************************************************************/	
int  TLib_IP_Limit_Destroy(char *sErrMsg)
{

	if (iShmID==-1){
		snprintf(sErrMsg,1024,"The ShareMemory hasn't beend Initiate");
		return -1;	
	}
	
	if (shmctl(iShmID, IPC_RMID, NULL)<0){
		snprintf(sErrMsg,1024,"Faile to Remove The ShareMemory ,Sharem Key %d",iShmID);
		return -2;	
	}
	
	iShmID=-1;
	return 0;	
}


#endif
