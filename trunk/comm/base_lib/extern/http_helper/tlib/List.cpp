/*
产生一个链表结构，用于存储动态数据，生成一个循环连表，同时对于连表的内容进行排序，
每次插入一个新链表，产生一个新的内容.
   by Leo 2002 4 15
*/

#include <stdio.h>
#include <stdlib.h>
#include "List.h"
/*
生成头指针.iHead表示头指针，iTrail表示尾指针
*/


 TList::TList()
 {
 	int *iNum=(int *)malloc(sizeof(int));
 	*iNum=0;
 	lTrail=lHead=(struct list *)malloc(sizeof(struct list));
 	if(lHead<=0)
 	{
 		snprintf(cErrorMsg,1024,"系统内存空间不足！");
 	} 
 	lHead->data=iNum;	
 	lHead->next=NULL;
 	
}

 TList::~TList()
 {
 	
 	free(lHead->data);
 	free(lHead);
 	
 	
}

/*设置链表总数加一*/
int TList::IncListCount()
{
	int iNum;
	if(lHead==NULL)
	{
		snprintf(cErrorMsg,1024,"没有初始化Head！");
		return -1;
	}
	iNum=*(int *)(lHead->data);
	iNum++;
	memcpy((void *)lHead->data,&iNum,sizeof(iNum));	
	return iNum;
}
/*设置链表总数加一DecListCount*/
int TList::DecListCount()
{
	int iNum;
	if(lHead==NULL)
	{
		snprintf(cErrorMsg,1024,"没有初始化Head！");
		return -1;
	}
	iNum=*(int *)(lHead->data);
	iNum--;
	memcpy((void *)lHead->data,&iNum,sizeof(iNum));	
	return iNum;
}

/*
功能：获得链表总数
*/
int TList::GetCount()
{
	int iNum;
	if(lHead==NULL)
	{
		snprintf(cErrorMsg,1024,"没有初始化Head！");
		return -1;
	}
	iNum=*(int *)(lHead->data);
	return iNum;
}


/*
功能：增加一个项,将一个项添加到后面
返回值:0成功 <0失败
参数：要增加的项 ！
*/
int TList::AppendItem(void * item)
{
	struct list *mList;
	mList=(struct list *)malloc(sizeof(struct list));
	mList->data=item;
	mList->next=NULL;
 		
 	lTrail->next=mList;
 	lTrail=mList;
 	IncListCount();//总数加一
 	return 0;
}
/*第一个为0*/
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

/*获得一个指针，并删除她*/
void * TList::GetItemDelete(int pos)
{
	struct list *mList,*mChange;
	if(GetCount()<(pos+1))//超出范围
	{
		return NULL;
	}
	
	mList=lHead;
	for(int i=0;i<=pos-1;i++)
	{		
		mList=mList->next;			
	}	
	if(mList==NULL)//假如为NULL
		return NULL;
	
	//判断
	mChange=mList->next;
	mList->next=mChange->next;
	DecListCount();
	return mChange;
}

/*
删除制指定的项
*/

void * TList::DeleteItem(int pos)
{
	struct list *mList,*mChange;
	if(GetCount()<(pos+1))//超出范围
	{
		return NULL;
	}
	
	mList=lHead;
	for(int i=0;i<=pos-1;i++)
	{		
		mList=mList->next;			
	}	
	if(mList==NULL)//假如为NULL
		return NULL;
	
	//判断
	mChange=mList->next;
	mList->next=mChange->next;
	DecListCount();
	return mChange->data;	
}
/*
将item插在pos位置，即插入完成后item在链的pos位置
*/
int TList::InsertItem(void *item,int pos)
{
	struct list *mList,*mChange,*mCurrent;
	
	mList=(struct list *)malloc(sizeof(struct list));
	mList->data=item;
	mList->next=NULL;
	
	
	if(GetCount()<(pos+1))//超出范围
	{
		return -1;
	}
	
	mCurrent=lHead;
	for(int i=0;i<=pos-1;i++)
	{		
		mCurrent=mCurrent->next;			
	}	
	if(mCurrent==NULL)//假如为NULL
	{
		snprintf(cErrorMsg,1024,"链表发生错误！！");		
		return -1;
	}
	
	//判断
	mChange=mCurrent->next;
	mCurrent->next=mList;
	mList->next=mChange;
 	IncListCount();//总数加一
	return 0;	
}
/*删除所有的项,从列表中删除，同时释放内存资源*/
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
	//判断
	return 0;	
}

 
