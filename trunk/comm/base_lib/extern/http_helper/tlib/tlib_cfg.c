#ifndef _TLIB_CFG_C_
#define _TLIB_CFG_C_

#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#include "tlib_com.h"
#include "tlib_str.h"
#include "tlib_cfg.h"

#define MAX_CONFIG_LINE_LEN 255

static void InitDefault(va_list ap)
{
	char *sParam, *sVal, *sDefault;
	double *pdVal, dDefault;
	long *plVal, lDefault;
	int iType, *piVal, iDefault;
	long lSize;

	sParam = va_arg(ap, char *);
	while (sParam != NULL)
	{
		iType = va_arg(ap, int);
		switch(iType)
		{
			case CFG_STRING:
				sVal = va_arg(ap, char *);
				sDefault = va_arg(ap, char *);
				lSize = va_arg(ap, long);
				strncpy(sVal, sDefault, (int)lSize-1);
				sVal[lSize-1] = 0;
				break;
			case CFG_LONG:
				plVal = va_arg(ap, long *);
				lDefault = va_arg(ap, long);
				*plVal = lDefault;
				break;
			case CFG_INT:
				piVal = va_arg(ap, int *);
				iDefault = va_arg(ap, int);
				*piVal = iDefault;
				break;
			case CFG_DOUBLE:
				pdVal = va_arg(ap, double *);
				dDefault = va_arg(ap, double);
				*pdVal = dDefault;
				break;
		}
		sParam = va_arg(ap, char *);
	}
}

static void SetVal(va_list ap, char *sP, char *sV)
{
	char *sParam, *sVal, *sDefault;
	double *pdVal, dDefault;
	long *plVal, lDefault;
	int iType, *piVal, iDefault;
	long lSize;

	sParam = va_arg(ap, char *);
	while (sParam != NULL)
	{
		iType = va_arg(ap, int);
		switch(iType)
		{
			case CFG_STRING:
				sVal = va_arg(ap, char *);
				sDefault = va_arg(ap, char *);
				lSize = va_arg(ap, long);
				break;
			case CFG_LONG:
				plVal = va_arg(ap, long *);
				lDefault = va_arg(ap, long);
				if (strcmp(sP, sParam) == 0)
				{
					*plVal = atol(sV);
				}
				break;
			case CFG_INT:
				piVal = va_arg(ap, int *);
				iDefault = va_arg(ap, int);
				if (strcmp(sP, sParam) == 0)
				{
					*piVal = iDefault;
				}
				break;
			case CFG_DOUBLE:
				pdVal = va_arg(ap, double *);
				dDefault = va_arg(ap, double);
				*pdVal = dDefault;
				break;
		}

		if (strcmp(sP, sParam) == 0)
		{
			switch(iType)
			{
				case CFG_STRING:
					strncpy(sVal, sV, (int)lSize-1);
					sVal[lSize-1] = 0;
					break;
				case CFG_LONG:
					*plVal = atol(sV);
					break;
				case CFG_INT:
					*piVal = atoi(sV);
					break;
				case CFG_DOUBLE:
					*pdVal = atof(sV);
					break;
			}
			return;
		}

		sParam = va_arg(ap, char *);
	}
}

static int GetParamVal(char *sLine, char *sParam, char *sVal)
{
	char *p, *sP;
	
	p = sLine;
	while(*p != '\0')
	{
		if ((*p != ' ') && (*p != '\t') && (*p != '\n'))
			break;
		p++;
	}
	
	sP = sParam;
	while(*p != '\0')
	{
		if ((*p == ' ') || (*p == '\t') || (*p == '\n'))
			break;
			
		*sP = *p;
		p++;
		sP++;
	}
	*sP = '\0';
	
	strncpy(sVal, p,strlen(p)+1);
	TLib_Str_Trim(sVal);
	
	if (sParam[0] == '#')
		return 1;
		
	return 0;
}

void TLib_Cfg_GetConfig(char *sConfigFilePath, ...)
{
	FILE *pstFile;
	char sLine[MAX_CONFIG_LINE_LEN+1], sParam[MAX_CONFIG_LINE_LEN+1], sVal[MAX_CONFIG_LINE_LEN+1];
	va_list ap;
	
	va_start(ap, sConfigFilePath);
	InitDefault(ap);
	va_end(ap);

	if ((pstFile = fopen(sConfigFilePath, "r")) == NULL)
	{
		return;
	}

	while (1)
	{
		sLine[0]=0;
		
		fgets(sLine, sizeof(sLine), pstFile);
		if (strcmp(sLine, "") != 0)
		{
			if (GetParamVal(sLine, sParam, sVal) == 0)
			{
				va_start(ap, sConfigFilePath);
				SetVal(ap, sParam, sVal);
				va_end(ap);
			}
		}
		
		if (feof(pstFile))
		{
			break; 
		}
	}	
	fclose(pstFile);
	
}

#endif

