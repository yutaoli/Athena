/*
����һ������ṹ�����ڴ洢��̬���ݣ�����һ��ѭ������ͬʱ������������ݽ�������
ÿ�β���һ������������һ���µ�����.
   by Leo 2002 4 15
*/

#include <stdio.h>
#include <stdlib.h>
#include "List.h"
/*
����ͷָ��.iHead��ʾͷָ�룬iTrail��ʾβָ��
*/


 TList::TList()
 {
 	int *iNum=(int *)malloc(sizeof(int));
 	*iNum=0;
 	lTrail=lHead=(struct list *)malloc(sizeof(struct list));
 	if(lHead<=0)
 	{
 		snprintf(cErrorMsg,1024,"ϵͳ�ڴ�ռ䲻�㣡");
 	} 
 	lHead->data=iNum;	
 	lHead->next=NULL;
 	
}

 TList::~TList()
 {
 	
 	free(lHead->data);
 	free(lHead);
 	
 	
}

/*��������������һ*/
int TList::IncListCount()
{
	int iNum;
	if(lHead==NULL)
	{
		snprintf(cErrorMsg,1024,"û�г�ʼ��Head��");
		return -1;
	}
	iNum=*(int *)(lHead->data);
	iNum++;
	memcpy((void *)lHead->data,&iNum,sizeof(iNum));	
	return iNum;
}
/*��������������һDecListCount*/
int TList::DecListCount()
{
	int iNum;
	if(lHead==NULL)
	{
		snprintf(cErrorMsg,1024,"û�г�ʼ��Head��");
		return -1;
	}
	iNum=*(int *)(lHead->data);
	iNum--;
	memcpy((void *)lHead->data,&iNum,sizeof(iNum));	
	return iNum;
}

/*
���ܣ������������
*/
int TList::GetCount()
{
	int iNum;
	if(lHead==NULL)
	{
		snprintf(cErrorMsg,1024,"û�г�ʼ��Head��");
		return -1;
	}
	iNum=*(int *)(lHead->data);
	return iNum;
}


/*
���ܣ�����һ����,��һ������ӵ�����
����ֵ:0�ɹ� <0ʧ��
������Ҫ���ӵ��� ��
*/
int TList::AppendItem(void * item)
{
	struct list *mList;
	mList=(struct list *)malloc(sizeof(struct list));
	mList->data=item;
	mList->next=NULL;
 		
 	lTrail->next=mList;
 	lTrail=mList;
 	IncListCount();//������һ
 	return 0;
}
/*��һ��Ϊ0*/
void * TList::GetItem(int pos)
{
	
	struct list *mList;
	if(GetCount()<(pos+1))
	{
		return NULL;
	}
	
	mList=lHead;
	for(int i=0;i<=pos;i++)
	{		
		mList=mList->next;			
	}	
	//GetItemDelete(1);
	if(mList==NULL)
		return NULL;
	return mList->data;
}
/**/
int TList::getvalue()
{
	return 10;
}

/*���һ��ָ�룬��ɾ����*/
void * TList::GetItemDelete(int pos)
{
	struct list *mList,*mChange;
	if(GetCount()<(pos+1))//������Χ
	{
		return NULL;
	}
	
	mList=lHead;
	for(int i=0;i<=pos-1;i++)
	{		
		mList=mList->next;			
	}	
	if(mList==NULL)//����ΪNULL
		return NULL;
	
	//�ж�
	mChange=mList->next;
	mList->next=mChange->next;
	DecListCount();
	return mChange;
}

/*
ɾ����ָ������
*/

void * TList::DeleteItem(int pos)
{
	struct list *mList,*mChange;
	if(GetCount()<(pos+1))//������Χ
	{
		return NULL;
	}
	
	mList=lHead;
	for(int i=0;i<=pos-1;i++)
	{		
		mList=mList->next;			
	}	
	if(mList==NULL)//����ΪNULL
		return NULL;
	
	//�ж�
	mChange=mList->next;
	mList->next=mChange->next;
	DecListCount();
	return mChange->data;	
}
/*
��item����posλ�ã���������ɺ�item������posλ��
*/
int TList::InsertItem(void *item,int pos)
{
	struct list *mList,*mChange,*mCurrent;
	
	mList=(struct list *)malloc(sizeof(struct list));
	mList->data=item;
	mList->next=NULL;
	
	
	if(GetCount()<(pos+1))//������Χ
	{
		return -1;
	}
	
	mCurrent=lHead;
	for(int i=0;i<=pos-1;i++)
	{		
		mCurrent=mCurrent->next;			
	}	
	if(mCurrent==NULL)//����ΪNULL
	{
		snprintf(cErrorMsg,1024,"���������󣡣�");		
		return -1;
	}
	
	//�ж�
	mChange=mCurrent->next;
	mCurrent->next=mList;
	mList->next=mChange;
 	IncListCount();//������һ
	return 0;	
}
/*ɾ�����е���,���б���ɾ����ͬʱ�ͷ��ڴ���Դ*/
int TList::DeleteAll()
{
	struct list *mList;
		
	while(lHead->next!=NULL)
	{		
		mList=lHead->next;			
		lHead->next=mList->next;
		
		if(mList!=NULL)
		{
			
			free(mList);
		}
		printf("error\n");
		DecListCount();
	}		
	//�ж�
	return 0;	
}

 
