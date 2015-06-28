#include "tlib_com.h"
#include <string.h>

#include "tlib_mem.h"

static char MemHeap[TLIB_MEM_SIZE];
static int  iMemUse;
static char *sOffset;

int  TLib_Mem_Reset()
{
	iMemUse 	= 0;
	sOffset 	= MemHeap;
}

int  TLib_Mem_DebugInfo(char *sMsg)
{
	snprintf(sMsg,32, "iMemUse=%d", iMemUse);
}

char *TLib_Mem_Itoa( long i)
{
	char *s1;
	
	s1 = sOffset;
	snprintf( sOffset,16, "%ld\0", i);
	sOffset += 10;
	iMemUse += 10;
	return s1;
}


//获取字符串的一个摘要，从头开始计算
char *TLib_Mem_StrBrief( char *sStr1, int iLimitSize )
{
	char *s1;
	
	s1 = sOffset;
	if ((iLimitSize > TLIB_MEM_MAX_STR_BRIEF) || (iLimitSize<=0)) {
		iLimitSize = TLIB_MEM_MAX_STR_BRIEF;
	}
	
	memset(s1, 0, iLimitSize);
		
	strncpy( s1, sStr1, iLimitSize );
	sOffset += iLimitSize + 5;
	iMemUse += iLimitSize + 5;
	return s1;
}


char *TLib_Mem_SQL( char *sField)
{
	char *s1;
	int i,j;
	
	s1 = sOffset;
	
	if (sField==NULL) {
		snprintf(sOffset,16, "\0");
		sOffset +=2;
		iMemUse +=2;
		return s1; 	
	}

	j=0;	
	for (i=0; i<strlen(sField); i++)  {
		if (sField[i]==39)  { //单引号
			sOffset[j] = '\\';
			sOffset[j+1] = 39;
			iMemUse +=2;
			j +=2;
		}
		else {
			sOffset[j] = sField[i];
			iMemUse ++;
			j++;
		}			
	}
	sOffset[j] = '\0';
	sOffset+=j;
	return s1;
}




