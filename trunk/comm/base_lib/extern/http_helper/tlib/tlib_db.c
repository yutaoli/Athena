#ifndef _TLIB_DB_C_
#define _TLIB_DB_C_

#include <stdlib.h>
#include "tlib_db.h"
#include "tlib_log.h"

void TLib_DB_Init(TLIB_DB_LINK *pstDBLink,int iMultiDBConn)
{
	memset(pstDBLink, 0, sizeof(TLIB_DB_LINK));
	
	//��ʼ��mysql������
	mysql_init(&(pstDBLink->stMysqlConn.stMysql));
	
	pstDBLink->iResNotNull = 0;
	pstDBLink->iMultiDBConn = iMultiDBConn;	
	pstDBLink->stMysqlConn.pstNext=NULL;
	pstDBLink->pstCurMysqlConn = &(pstDBLink->stMysqlConn); //���õ�ǰ����ָ��
	pstDBLink->pstCurMysqlConn->iDBConnect = 0;
}

int TLib_DB_CloseDatabase(TLIB_DB_LINK *pstDBLink)
{
	if (pstDBLink->iResNotNull == 1)
	{
		mysql_free_result(pstDBLink->pstRes);
		pstDBLink->iResNotNull=0;
	}		
	
	//����Ƕ�������رճ���һ����mysql���ӣ����ͷ������г���һ���ṹ��������Ŀ
	if (pstDBLink->iMultiDBConn!=0)
	{
		TLIB_MYSQL_CONN *pstMyConn,*pstMysqlConn;
		pstMysqlConn=pstDBLink->stMysqlConn.pstNext;
		while (pstMysqlConn!=NULL)
		{
			//�ر�mysql������
			if (pstMysqlConn->iDBConnect == 1)
			{
				mysql_close(&(pstMysqlConn->stMysql));
			}
			pstMyConn=pstMysqlConn;
			pstMysqlConn=pstMysqlConn->pstNext;
			free(pstMyConn);
		}
	}	
		
	//�ر������е�һ��mysql����
	if (pstDBLink->stMysqlConn.iDBConnect == 1)
		mysql_close(&(pstDBLink->stMysqlConn.stMysql));
	
		
	//���õ�ǰ����ָ��
	pstDBLink->stMysqlConn.iDBConnect = 0;
	pstDBLink->stMysqlConn.pstNext=NULL;

	pstDBLink->pstCurMysqlConn = &(pstDBLink->stMysqlConn);	

	pstDBLink->pstCurMysqlConn->sHostAddress[0] = '\0';
	
	return 0;
}

//--------------------------------------------------------------------
//
//			10.26  cyril modified this function.	
//
//       ʹ�������������֧�ֱȽϳ�ʱ������ӡ���������Ѿ���server
//   �Ͽ������Զ���������
//
//--------------------------------------------------------------------

int TLib_DB_SetDB(TLIB_DB_LINK *pstDBLink, char *sHostAddress, char *sUserName, char *sPassword, char *sDBName, char *sErrMsg)
{
	int iSelectDB;
	
	
	iSelectDB = 0;
	
	//�ж��Ƿ�������ֻҪһ��mysql������
	if (pstDBLink->iMultiDBConn==0)
	{
		//���Ҫ���ӵĵ�ַ�͵�ǰ�ĵ�ַ����ͬһ̨��������close��ǰ������,�����½�������
		if (strcmp(pstDBLink->pstCurMysqlConn->sHostAddress, sHostAddress) != 0)
		{
			if (pstDBLink->pstCurMysqlConn->iDBConnect==1)
			{
				TLib_DB_CloseDatabase(pstDBLink);
			}
		
			if (mysql_connect(&(pstDBLink->pstCurMysqlConn->stMysql), sHostAddress, sUserName, sPassword) == 0)
			{
				snprintf(sErrMsg,1024, "Fail To Connect To Mysql: %s", mysql_error(&(pstDBLink->pstCurMysqlConn->stMysql)));
				return -1;
			}	
			pstDBLink->pstCurMysqlConn->iDBConnect=1;
			iSelectDB = 1;
		}
		else  //�����ǰû�����ӣ�������mysql ?? cyril:  �����ǰ��ַ��Ҫ���ӵĵ�ַ��ͬһ̨����
		{
			if (pstDBLink->pstCurMysqlConn->iDBConnect==0){
				if (mysql_connect(&(pstDBLink->pstCurMysqlConn->stMysql), sHostAddress, sUserName, sPassword) == 0){
					snprintf(sErrMsg,1024, "Fail To Connect To Mysql: %s", mysql_error(&(pstDBLink->pstCurMysqlConn->stMysql)));
					return -1;
				}	
				pstDBLink->pstCurMysqlConn->iDBConnect=1;
			}
			else{
				if(mysql_ping(&(pstDBLink->pstCurMysqlConn->stMysql)) != 0){
					snprintf(sErrMsg,1024, "Fail To ping To Mysql: %s", mysql_error(&(pstDBLink->pstCurMysqlConn->stMysql)));
					return -1;
				}
			}	
		}	
	}
	//���mysql���ӵ�
	else
	{
		//���Ҫ���ӵĵ�ַ�͵�ǰ�ĵ�ַ��ͬ
		if (strcmp(pstDBLink->pstCurMysqlConn->sHostAddress, sHostAddress) != 0){
			
			TLIB_MYSQL_CONN *pstMysqlConn;
			TLIB_MYSQL_CONN *pstMysqlConnTail;//pstMysqlConnTail��βָ��
			
			//���ͷ����ڵ�Res
			if (pstDBLink->iResNotNull == 1){
				mysql_free_result(pstDBLink->pstRes);
				pstDBLink->iResNotNull=0;
			}
			
			//��Ѱ����û���Ѿ�������̨HOST�ϵ����ݿ�
			pstMysqlConn=&(pstDBLink->stMysqlConn);	
			pstMysqlConnTail=&(pstDBLink->stMysqlConn);
			
			while (pstMysqlConn!=NULL){
				if (strcmp(pstMysqlConn->sHostAddress,sHostAddress) == 0){
					pstDBLink -> pstCurMysqlConn = pstMysqlConn;
					if(mysql_ping(&(pstDBLink->pstCurMysqlConn->stMysql)) != 0){
						snprintf(sErrMsg,1024,"Fail To ping Mysql: %s",mysql_error(&(pstDBLink->pstCurMysqlConn->stMysql)));
						return -1;
					}
					break;
				}
				pstMysqlConnTail=pstMysqlConn;
				pstMysqlConn = pstMysqlConn->pstNext;
				iSelectDB = 1;
			}
			
			//û����ͬ���򴴽�һ���µ�����
			if(pstMysqlConn == NULL){
				pstMysqlConn = (TLIB_MYSQL_CONN *)malloc(sizeof(TLIB_MYSQL_CONN));
				memset(pstMysqlConn, 0, sizeof(TLIB_MYSQL_CONN));
				mysql_init(&(pstMysqlConn->stMysql));
				pstMysqlConnTail -> pstNext=pstMysqlConn;
				pstMysqlConn -> pstNext = NULL;
			}
			
			pstDBLink -> pstCurMysqlConn = pstMysqlConn;
					
			if (pstDBLink -> pstCurMysqlConn -> iDBConnect == 0){
				if (mysql_connect(&(pstDBLink->pstCurMysqlConn->stMysql), sHostAddress, sUserName, sPassword) == 0){
					snprintf(sErrMsg,1024, "Fail To Connect To Mysql: %s", mysql_error(&(pstDBLink->pstCurMysqlConn->stMysql)));
					return -1;
				}	
			
				//���õ�ǰ����ָ��
				strncpy(pstDBLink->pstCurMysqlConn->sHostAddress, sHostAddress,strlen(sHostAddress)+1);
				pstDBLink->pstCurMysqlConn->iDBConnect=1;
				iSelectDB = 1;
			}
		}
		else if (pstDBLink->pstCurMysqlConn->iDBConnect==0)
		{
			if (mysql_connect(&(pstDBLink->pstCurMysqlConn->stMysql), sHostAddress, sUserName, sPassword) == 0){
				snprintf(sErrMsg,1024, "Fail To Connect To Mysql: %s", mysql_error(&(pstDBLink->pstCurMysqlConn->stMysql)));
				return -1;
			}
			strncpy(pstDBLink->pstCurMysqlConn->sHostAddress, sHostAddress,strlen(sHostAddress)+1);	
			pstDBLink->pstCurMysqlConn->iDBConnect=1;
			iSelectDB = 1;
		}
	}

//	if ((iSelectDB != 0) || (strcmp(pstDBLink->pstCurMysqlConn->sDBName, sDBName) != 0))
//	{
	if (mysql_select_db(&(pstDBLink->pstCurMysqlConn->stMysql), sDBName) < 0){
		//TLib_DB_CloseDatabase(pstDBLink);
		snprintf(sErrMsg,1024, "Cannot Select Database %s: %s", sDBName, mysql_error(&(pstDBLink->pstCurMysqlConn->stMysql)));
		return -1;
	}
	strncpy(pstDBLink->pstCurMysqlConn->sDBName, sDBName,strlen(sDBName)+1);	
//	}	
	
	snprintf(sErrMsg,1024, "");
	
	return 0;
}

/*Add By jingle 2001-08-24 */
int TLib_DB_ExecSQL_New(TLIB_DB_LINK *pstDBLink, char *sErrMsg) 
{
	int   iRetCode;
	
	char sTemp[80010];
	char sUserinfo[20]="userinfo";
	char sPasswd[20]="passwd";
	// �������Ƿ���ȷ
	if (pstDBLink->iQueryType != 0)
	{
		if ((pstDBLink->sQuery[0]!='s') && (pstDBLink->sQuery[0]!='S'))
		{
			snprintf(sErrMsg,1024, "QueryType=1, But SQL is not select");
			return -1;
		}
	}
	
	strncpy(sTemp,TLib_Str_StrLowercase(pstDBLink->sQuery),80000);
                  if ((strstr(sTemp,sUserinfo)!=NULL)&&(strstr(sTemp,sPasswd)!=NULL))
                  {
                  	
                  	pstDBLink->iResNum = 0;
		pstDBLink->iResNotNull = 1;
                  	return 0;
                                   
                  }
	// �Ƿ���Ҫ�ر�ԭ��RecordSet
	if (pstDBLink->iResNotNull==1)
	{
		mysql_free_result(pstDBLink->pstRes);
		pstDBLink->iResNotNull=0;
	}		
	
	if ((pstDBLink->pstCurMysqlConn ==NULL) || (pstDBLink->pstCurMysqlConn->iDBConnect == 0))
	{
		snprintf(sErrMsg,1024, "Has Not Connect To DB Server Yet");
		return -1;
	}

	// ִ����Ӧ��SQL���
	iRetCode =mysql_query(&(pstDBLink->pstCurMysqlConn->stMysql), pstDBLink->sQuery);
	if (iRetCode != 0)
	{
		snprintf(sErrMsg,1024, "Fail To Execute SQL: %s", mysql_error(&(pstDBLink->pstCurMysqlConn->stMysql)));
		return -1;
	}
	
	// ������
	if (pstDBLink->iQueryType == 1)
	{
		pstDBLink->pstRes = mysql_store_result(&(pstDBLink->pstCurMysqlConn->stMysql));
		pstDBLink->iResNum = mysql_num_rows(pstDBLink->pstRes);
		pstDBLink->iResNotNull = 1;
	}

	return 0;
}


int TLib_DB_ExecSQL(TLIB_DB_LINK *pstDBLink, char *sErrMsg) 
{
	int   iRetCode;
	
	// �������Ƿ���ȷ
	if (pstDBLink->iQueryType != 0)
	{
		if ((pstDBLink->sQuery[0]!='s') && (pstDBLink->sQuery[0]!='S'))
		{
			snprintf(sErrMsg,1024, "QueryType=1, But SQL is not select");
			return -1;
		}
	}
                  
	// �Ƿ���Ҫ�ر�ԭ��RecordSet
	if (pstDBLink->iResNotNull==1)
	{
		mysql_free_result(pstDBLink->pstRes);
		pstDBLink->iResNotNull=0;
	}		
	
	if ((pstDBLink->pstCurMysqlConn ==NULL) || (pstDBLink->pstCurMysqlConn->iDBConnect == 0))
	{
		snprintf(sErrMsg,1024, "Has Not Connect To DB Server Yet");
		return -1;
	}

	// ִ����Ӧ��SQL���
	iRetCode =mysql_query(&(pstDBLink->pstCurMysqlConn->stMysql), pstDBLink->sQuery);
	if (iRetCode != 0)
	{
		snprintf(sErrMsg,1024, "Fail To Execute SQL: %s", mysql_error(&(pstDBLink->pstCurMysqlConn->stMysql)));
		return -1;
	}
	
	// ������
	if (pstDBLink->iQueryType == 1)
	{
		pstDBLink->pstRes = mysql_store_result(&(pstDBLink->pstCurMysqlConn->stMysql));
		pstDBLink->iResNum = mysql_num_rows(pstDBLink->pstRes);
		pstDBLink->iResNotNull = 1;
	}

	return 0;
}

int TLib_DB_RealExecSQL(TLIB_DB_LINK *pstDBLink,unsigned int len,char *sErrMsg)
{
	int   iRetCode;
	
	// �������Ƿ���ȷ
	if (pstDBLink->iQueryType != 0)
	{
		if ((pstDBLink->sQuery[0]!='s') && (pstDBLink->sQuery[0]!='S'))
		{
			snprintf(sErrMsg,1024, "QueryType=1, But SQL is not select");
			return -1;
		}
	}

	// �Ƿ���Ҫ�ر�ԭ��RecordSet
	if (pstDBLink->iResNotNull==1)
	{
		mysql_free_result(pstDBLink->pstRes);
		pstDBLink->iResNotNull=0;
	}		
	
	if ((pstDBLink->pstCurMysqlConn ==NULL) || (pstDBLink->pstCurMysqlConn->iDBConnect == 0))
	{
		snprintf(sErrMsg,1024, "Has Not Connect To DB Server Yet");
		return -1;
	}

	// ִ����Ӧ��SQL���
	iRetCode =mysql_real_query(&(pstDBLink->pstCurMysqlConn->stMysql), pstDBLink->sQuery,len);
	if (iRetCode != 0)
	{
		snprintf(sErrMsg,1024, "Fail To Execute SQL: %s", mysql_error(&(pstDBLink->pstCurMysqlConn->stMysql)));
		return -1;
	}
	
	// ������
	if (pstDBLink->iQueryType == 1)
	{
		pstDBLink->pstRes = mysql_store_result(&(pstDBLink->pstCurMysqlConn->stMysql));
		pstDBLink->iResNum = mysql_num_rows(pstDBLink->pstRes);
		pstDBLink->iResNotNull = 1;
	}

	return 0;
}

int TLib_DB_FetchRow(TLIB_DB_LINK  *pstDBLink, char *sErrMsg)
{
	if (pstDBLink->iResNotNull == 0)
	{
		snprintf(sErrMsg,1024, "Recordset is Null");
		return -1;	
	}		
	
	if (pstDBLink->iResNum == 0)
	{
		snprintf( sErrMsg,1024, "Recordset count=0");
		return -1;	
	}		

	pstDBLink->stRow = mysql_fetch_row(pstDBLink->pstRes);
	return 0;
}

int TLib_DB_FreeResult(TLIB_DB_LINK  *pstDBLink)
{
	if (pstDBLink->iResNotNull==1)
	{
		mysql_free_result(pstDBLink->pstRes);
		pstDBLink->iResNotNull=0;
	}		
	
	return 0;
}

int TLib_DB_AffectedRow(TLIB_DB_LINK  *pstDBLink, char *sErrMsg)
{
	if ((pstDBLink->pstCurMysqlConn ==NULL) || (pstDBLink->pstCurMysqlConn->iDBConnect == 0))
	{
		snprintf(sErrMsg,1024, "Has Not Connect To DB Server Yet");
		return -1;
	}

	return mysql_affected_rows(&(pstDBLink->pstCurMysqlConn->stMysql));
}

int TLib_DB_InsertID(TLIB_DB_LINK  *pstDBLink)
{
	return mysql_insert_id(&(pstDBLink -> pstCurMysqlConn -> stMysql));
}
#endif

