#ifndef _TLIB_MEM_H_
#define _TLIB_MEN_H_

/*��С���ڴ����ĺ���������һ����̬��*/
/*
  ͨ�����ڹ���SPrintf��֮��һ��������ж���ַ������䣬
  Sprintf()ִ�к󣬷����˵��ڴ�Ͳ���ʹ����
*/  

#define TLIB_MEM_SIZE					10240       //10KӦ���Ѿ��㹻
#define TLIB_MEM_MAX_STR_BRIEF		300			//�����ַ���ժҪ

int  TLib_Mem_Reset();
int  TLib_Mem_DebugInfo(char *ErrMsg);
char *TLib_Mem_Itoa( long i);
char *TLib_Mem_SQL( char *sField);
char *TLib_Mem_StrBrief( char *sStr1, int iLimitSize );
#endif



