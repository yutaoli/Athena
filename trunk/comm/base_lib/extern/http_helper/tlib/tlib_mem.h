#ifndef _TLIB_MEM_H_
#define _TLIB_MEN_H_

/*有小量内存分配的函数，公用一个静态堆*/
/*
  通常用于供在SPrintf中之类一个语句中有多个字符串分配，
  Sprintf()执行后，分配了的内存就不再使用了
*/  

#define TLIB_MEM_SIZE					10240       //10K应该已经足够
#define TLIB_MEM_MAX_STR_BRIEF		300			//最大的字符串摘要

int  TLib_Mem_Reset();
int  TLib_Mem_DebugInfo(char *ErrMsg);
char *TLib_Mem_Itoa( long i);
char *TLib_Mem_SQL( char *sField);
char *TLib_Mem_StrBrief( char *sStr1, int iLimitSize );
#endif



