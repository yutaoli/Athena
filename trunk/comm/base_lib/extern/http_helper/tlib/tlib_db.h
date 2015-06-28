#ifndef _TLIB_DB_H_
#define _TLIB_DB_H_

#include "mysql.h"

typedef struct _TLIB_MYSQL_CONN_
{
	struct _TLIB_MYSQL_CONN_ *pstNext;
	
	char			sHostAddress[50];	//DB Server 的地址
	char			sUserName[50];		//用户名
	char			sPassword[50];		//密码
	char			sDBName[50];		//Database 名字
	
	MYSQL			stMysql;		//当前打开的Mysql连接
	
	int			iDBConnect;		//是否已经连接上对应的Database, 0=断开，1=连接上	
	
} TLIB_MYSQL_CONN;

typedef struct _TLIB_DB_LINK_
{
	TLIB_MYSQL_CONN		stMysqlConn;		//Mysql连接链表的第一项
	TLIB_MYSQL_CONN		*pstCurMysqlConn;	//当前打开的Mysql连接指针

	MYSQL_RES		*pstRes;		//当前操作的RecordSet
	MYSQL_ROW		stRow;			//当前操作的一行
	  	
	int			iResNotNull;		//当前操作的RecordSet是否为空,0=空，1=非空
	int			iResNum;		//当前操作的RecordSet的记录数目
	char			sQuery[80000];		//当前操作的SQL语句
	int			iQueryType;		//当前操作的SQL语句是否返回Recordset, 0=不要，1=select
	int			iMultiDBConn;		//是否 0=只有一个mysql连接，1=多个mysql连接
}TLIB_DB_LINK;

void TLib_DB_Init(TLIB_DB_LINK *pstDBLink,int iMultiDBConn);
int  TLib_DB_SetDB(TLIB_DB_LINK *pstDBLink, char *sHostAddress, char *sUserName, char *sPassword, char *sDBName, char *sErrMsg);

int  TLib_DB_CloseDatabase(TLIB_DB_LINK *pstDBLink);
int  TLib_DB_ExecSQL(TLIB_DB_LINK *pstDBLink, char *sErrMsg);
int TLib_DB_RealExecSQL(TLIB_DB_LINK *pstDBLink,unsigned int len,char *sErrMsg);
int  TLib_DB_FreeResult(TLIB_DB_LINK  *pstDBLink);
int  TLib_DB_FetchRow(TLIB_DB_LINK  *pstDBLink, char *sErrMsg);
int TLib_DB_AffectedRow(TLIB_DB_LINK  *pstDBLink, char *sErrMsg);
int TLib_DB_InsertID(TLIB_DB_LINK  *pstDBLink);
int TLib_DB_ExecSQL_New(TLIB_DB_LINK *pstDBLink, char *sErrMsg) ;
#endif
