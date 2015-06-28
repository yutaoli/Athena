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

static int iMaxAccessCount=100;		//ĳһʱ�����,һ��IP��������
static int iLimitTimeInterval=10;	//��������������ʱ���,�Է���Ϊ��λ
static int iRecalTimeInterval=10;	//���ʼ����������ֵ��,�����ٴ����¼����������ʱ���,�Է���Ϊ��λ
static int iShmID=-1;			//�����ڴ��ʶ��
/*******-***********************************************************-****************************
Funciton    ��ʼ��IP�������ƹ����ڴ���
input       iKey: �����ڴ�ı�ʶ
	    iLimitInterval:	//��������������ʱ���,�Է���Ϊ��λ 	
	    iRecalInterval:	//�������ֵ��,�������¼�����ʱ���,�Է���Ϊ��λ
	    iMaxCount	:	//ĳһʱ�����,һ��IP��������
output      sErrMsg		//������Ϣ	    
return:     0: success <0:Error
lastmodify  2000.06.29 //ȥ����iRefresh����
written by  Dennnis
*******-*****************************************************************************************/	
int  TLib_IP_Limit_Init(key_t iKey,int iLimitInterval,int iRecalInterval,int iMaxCount,char *sErrMsg)
{
	
	char *pShm;
	int iShmSize;
	int iFirst=1;
	
	//calculate the size of the shareMemory
	iShmSize=sizeof(TLIB_IP_ACCESS_ENTRY)*C_MAX_ENTRY_NUM*C_MAX_BLOCK_NUM;	
	if ((iShmID = shmget(iKey, iShmSize, IPC_CREAT | IPC_EXCL | 0666)) < 0)  //��ͼ����
	{
		if ((iShmID = shmget(iKey, iShmSize, 0666))<0)                   //�п������Ѿ�����ͬ����key_shm,����ͼ����
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
Funciton    ȡ��ָ��IP��״̬
input       sIP: 	//���ʵ�IP 
output      sErrMsg	//������Ϣ	    
return:     0: 		//��IP��û�е�����������
	    1: 		//��IP�Ѿ�������������, ���Ҹ�Blockס��
	    <0:         //Error
lastmodify  2000.06.20
written by  Dennnis
�����߼���  1.����IP���һ��byte��λ�ڴ���ʿ����� 
	    2.����ڴ���ʿ���û��ͬ����IP��¼������ 0
	    3.����ڴ���ʿ�����ͬ����IP��¼��
	       ��������ʼ�¼>�����������û�г������¼���ʱ��Σ��򷵻� 1
	       ��������ʼ�¼<=����������ҳ������¼���ʱ����ڣ� ���� 0
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
	
	//���ȶ�λ����һ���ڴ����
	pstFirstEntry=(TLIB_IP_ACCESS_ENTRY *)(pstFirstEntry+(iBlockIndex-1)*C_MAX_ENTRY_NUM);	
	pstEntry=pstFirstEntry;
	
	iRetCode=0;
	for (i=0;i<C_MAX_ENTRY_NUM;i++){
		 if (strcmp(pstEntry->sIP,"")==0){//��һ��Ϊ�յļ�¼,�ڴ��δ��
			break;
		 }else if (strcmp(pstEntry->sIP,sIP)==0){ //�ж��Ƿ��ڷ��ʱ����ҵ�IP
			if (Debug) 
				printf("found at %d\n",i);
			//�ж��Ƿ��Ѿ�����������	
			if (pstEntry->iAccessCount>iMaxAccessCount){
				//����Ѿ�����,���ж��Ƿ񳬹��������¼�����ʱ���
				time(&time_now);
				if ((time_now-pstEntry->iBeginTime)<iRecalTimeInterval*60){
					iRetCode=1; //��û�г���block��ʱ���,��Blockס��
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
Funciton    ����IP������
input       sIP: 	//���ʵ�IP 
output      sErrMsg	//������Ϣ	    
return:     0: 		//��IP��û�е����������� 		    
	    1: 		//��IP�Ѿ�������������
	    <0:         //Error
lastmodify  2000.06.20
written by  Dennnis
�����߼���  1.����IP���һ��byte��λ�ڴ���ʿ����� 
	    2.����ڴ���ʿ���û��ͬ����IP��¼���ҳ���һ���ռ�¼����ip,iBeginTime=now()�ͷ��ʼ���=1 ���� 0
	    3.����ڴ���ʿ���û��ͬ����IP��¼�������ڴ������,�ҳ������ڴ���е���С���ʼ����ļ�¼���滻�����¼ ���� 0
	    4.����ڴ���ʿ�����ͬ����IP��¼��
	       ��������ʼ�¼>�����������û�г������¼���ʱ��Σ����򷵻� 1
	       ��������ʼ�¼>����������ҳ������¼���ʱ����ڣ��򽫴˼�¼iBeginTime=now()�ͷ��ʼ���=1 ���� 0
	       �����++���ʼ�¼>����������ҳ�����������ʱ��Σ��򽫴˼�¼iBeginTime=now()�ͷ��ʼ���=1 ���� 0
	       �����++���ʼ�¼>�����������û�з�������ʱ��Σ��򷵻� 1	       
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
	
	//���ȶ�λ����һ���ڴ����
	pstFirstEntry=(TLIB_IP_ACCESS_ENTRY *)(pstFirstEntry+(iBlockIndex-1)*C_MAX_ENTRY_NUM);	
	pstEntry=pstFirstEntry;
	
	//������ʴ���
	iMinAccessCount=pstEntry->iAccessCount;
	iMinCountIndex=0;
	iFound=0;
	iRetCode=0;
		
	for (i=0;i<C_MAX_ENTRY_NUM;i++){
		 if (strcmp(pstEntry->sIP,"")==0){//��һ��Ϊ�յļ�¼,�ڴ��δ��
			strncpy(pstEntry->sIP,sIP,sizeof(pstEntry->sIP)-1);
			pstEntry->iAccessCount=1;	
			time(&time_now);				
			pstEntry->iBeginTime=time_now; //��һ�η��ʵ�ʱ��		
			break;
		 }else if (strcmp(pstEntry->sIP,sIP)==0){ //�ж��Ƿ��ڷ��ʱ����ҵ�IP
			iFound=1;
			if (Debug) 
				printf("found at %d\n",i);
			//�ж��Ƿ��Ѿ�����������	
			if (pstEntry->iAccessCount>iMaxAccessCount){
				//����Ѿ�����,���ж��Ƿ��Ѿ��������¼�����ʱ��
				time(&time_now);
				if ((time_now-pstEntry->iBeginTime)>iRecalTimeInterval*60){
					//���¼���
					pstEntry->iBeginTime=time_now;
					pstEntry->iAccessCount=1;
				}else{
					iRetCode=1;
				}
			}else if(++pstEntry->iAccessCount>iMaxAccessCount){//+1�󳬹�������
				//���ܳ������ļ��������Ҳ�������Ƶ�ʱ��Σ��ʹ�ͷ����
				time(&time_now);
				if ((time_now-pstEntry->iBeginTime)>iLimitTimeInterval*60){ 
					//���¼���
					pstEntry->iBeginTime=time_now;
					pstEntry->iAccessCount=1;
				}else{ //����ͽ�����ʱ������Ϊ��һ�γ����������ʱ��
					pstEntry->iBeginTime=time_now; //��һ�γ����������ʱ��, Ҳ�ǿ�ʼ��block��ʱ��
					iRetCode=1;
				}
			}
			break;
		 }else if (pstEntry->iAccessCount<iMinAccessCount){ //���IP��Ϊ��,���ҳ���С������
			iMinAccessCount=pstEntry->iAccessCount;
			iMinCountIndex=i;
		 }
		 pstEntry++;
	}		
	
	//�ڷ��ʱ���û�ҵ�IP,���ڴ������, ���滻��С�������ļ�¼
	if ((!iFound) && (i>=C_MAX_ENTRY_NUM)){
		if (Debug)
			printf("replace MinCount=%d MinIndex=%d\n",iMinAccessCount,iMinCountIndex);
		pstEntry=pstFirstEntry+iMinCountIndex; //�滻ָ��
		strncpy(pstEntry->sIP,sIP,sizeof(pstEntry->sIP)-1);
		pstEntry->iAccessCount=1;	
		time(&time_now);				
		pstEntry->iBeginTime=time_now; //��һ�η��ʵ�ʱ��		
	} 
	
	shmdt(pShm);
	
	return iRetCode;	
}




/*******-**********************************************************-*****************************************************
Funciton    �г��ڴ����ĳһ�����Ϣ
input       iBlockIndex://�ڴ�����
output      sErrMsg	//������Ϣ	    
return:     0: 		:success
            <0		:failed 
lastmodify  2000.06.20
written by  Dennnis
attention:  CGIһ�㲻Ҫ����,�˺�����Ϊ�˲鿴�ڴ���Զ���
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
	
	//�����ڴ���ж�λ
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
Funciton    �ͷŹ����ڴ�
input       
output      sErrMsg	//������Ϣ	    
return:     0: 		:success
            <0		:failed 
lastmodify  2000.06.20
written by  Dennnis
attention:  CGIһ�㲻Ҫ����
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
