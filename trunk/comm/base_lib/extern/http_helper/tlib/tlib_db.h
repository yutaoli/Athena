#ifndef _TLIB_DB_H_
#define _TLIB_DB_H_

#include "mysql.h"

typedef struct _TLIB_MYSQL_CONN_
{
	struct _TLIB_MYSQL_CONN_ *pstNext;
	
	char			sHostAddress[50];	//DB Server �ĵ�ַ
	char			sUserName[50];		//�û���
	char			sPassword[50];		//����
	char			sDBName[50];		//Database ����
	
	MYSQL			stMysql;		//��ǰ�򿪵�Mysql����
	
	int			iDBConnect;		//�Ƿ��Ѿ������϶�Ӧ��Database, 0=�Ͽ���1=������	
	
} TLIB_MYSQL_CONN;

typedef struct _TLIB_DB_LINK_
{
	TLIB_MYSQL_CONN		stMysqlConn;		//Mysql��������ĵ�һ��
	TLIB_MYSQL_CONN		*pstCurMysqlConn;	//��ǰ�򿪵�Mysql����ָ��

	MYSQL_RES		*pstRes;		//��ǰ������RecordSet
	MYSQL_ROW		stRow;			//��ǰ������һ��
	  	
	int			iResNotNull;		//��ǰ������RecordSet�Ƿ�Ϊ��,0=�գ�1=�ǿ�
	int			iResNum;		//��ǰ������RecordSet�ļ�¼��Ŀ
	char			sQuery[80000];		//��ǰ������SQL���
	int			iQueryType;		//��ǰ������SQL����Ƿ񷵻�Recordset, 0=��Ҫ��1=select
	int			iMultiDBConn;		//�Ƿ� 0=ֻ��һ��mysql���ӣ�1=���mysql����
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
