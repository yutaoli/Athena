#include <iostream>
#include <stdlib.h>
#include <stdio.h>

#include "server_config.h"

using namespace Athena;

CServerConf:: CServerConf()
{
}

CServerConf:: CServerConf(const char* szFileName)
{
	m_ConfigFile.open(szFileName);
	ParseFile();
}

CServerConf:: ~CServerConf()
{
	m_ConfigFile.close();
}

// delete pre- or -end white space
int CServerConf:: StrimString(char* szLine)
{
	int i, j;

	// ignor -end comment
	char *p;
	p= strstr(szLine, "//");
	if (p!=NULL) {
		*p=0;
	}

	// delete -end white space
	j= strlen(szLine)-1;
	while ((szLine[j]==' ')||(szLine[j]=='\t')||(szLine[j]=='\n')||(szLine[j]=='\r'))
	{
		if (j == 0) return -1;
		szLine[j]=0;
		j--;
	}
	// delete pre- white space
	i=0; j=0;
	while ((szLine[j] == ' ')||(szLine[j] == '\t')) {
		if (szLine[j] == 0) return -1;
		j++;
	}
	// shift string
	while (szLine[j] != 0) {
		szLine[i] = szLine[j];
		i++;
		j++;
	}
	szLine[i]=0;

	// whole comment line
	if ((szLine[0]=='/')&&(szLine[1]=='/'))
		return 1;

	return 0;
}

int CServerConf:: ParseFile(const char* szConfigFile)
{
	memset(m_ErrMsg, 0, sizeof(m_ErrMsg));
	if(access(szConfigFile, F_OK) < 0)
	{
		snprintf(m_ErrMsg, sizeof(m_ErrMsg), "file[%s] not exist", szConfigFile);
		return -1;
	}

	m_ConfigFile.open(szConfigFile);
	if (m_ConfigFile.fail()) {
		return -1;
	}
	return ParseFile();
}


int CServerConf:: ParseFile()
{
	char szLine[1024];
	char szSection[64], szParam[128];
	char *pColon;
	int  iLen;

	bzero(szSection, sizeof(szSection));
	bzero(szParam, sizeof(szParam));

	while (m_ConfigFile.getline(szLine, sizeof(szLine))) {
		if (StrimString(szLine) != 0) continue;
		iLen = strlen(szLine);
		if (((szLine[0]=='[') && (szLine[iLen-1]==']')) ||
			((szLine[0]=='<') && (szLine[iLen-1]=='>')) )
		{
			pColon = szLine+1;
			szLine[iLen-1] = 0;
			bzero(szSection, sizeof(szSection));
			strncpy(szSection, pColon, sizeof(szSection));
			// section name
			continue;
		}
		if ((pColon= index(szLine, '=')) == NULL) {
			pColon= index(szLine, ':');
		}
		if (pColon==NULL) continue;
		*pColon=0;
		pColon++;
		if (StrimString(pColon) < 0) continue;
		if (StrimString(szLine) < 0) continue;
		if (szSection[0] != 0) {
			snprintf(szParam, sizeof(szParam), "%s.%s", szSection, szLine);
		}
		else {
			snprintf(szParam, sizeof(szParam), "%s", szLine);
		}
		m_ConfigMap[szParam]= pColon;
	}
	return 0;
}

string& CServerConf:: operator[](const char* szParam)
{
	return m_ConfigMap[szParam];
}

string& CServerConf:: operator()(const char* szSection, const char* szName)
{
	char szParam[64];
	snprintf(szParam, sizeof(szParam), "%s.%s", szSection, szName);

	return m_ConfigMap[szParam];
}

