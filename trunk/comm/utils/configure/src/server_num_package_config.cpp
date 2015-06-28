#include <iostream>
#include <stdlib.h>
#include <stdio.h>

#include "server_num_package_config.h"

using namespace Athena;

CServerNumPackageConf::CServerNumPackageConf()
{
	InitServerNumPackageConf();
}

CServerNumPackageConf::CServerNumPackageConf(const char* szFileName)
{
	//set OK
	InitServerNumPackageConf();
	snprintf(m_ErrInfo.m_ErrMsg, sizeof(m_ErrInfo.m_ErrMsg), "configure file [%s] format check [OK].", szFileName);	

	m_ConfigFileName = szFileName;
	m_ConfigFile.open(szFileName);
	if(m_ConfigFile.fail())
	{
		snprintf(m_ErrInfo.m_ErrMsg, sizeof(m_ErrInfo.m_ErrMsg), "open configure file[%s] error.", szFileName);
		m_ErrInfo.SetErrorStatus();
		return ;
	}

	ParseFile();
}

CServerNumPackageConf:: ~CServerNumPackageConf()
{
	m_ConfigFile.close();
}

// delete pre- or -end white space
int CServerNumPackageConf:: StrimString(char* szLine)
{
	int i, j;

	// ignor -end comment
	char *p;
	p= strstr(szLine, "#");
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

	return 0;
}

int CServerNumPackageConf:: ParseFile(const char* szConfigFile)
{
	//set OK
	InitServerNumPackageConf();
	snprintf(m_ErrInfo.m_ErrMsg, sizeof(m_ErrInfo.m_ErrMsg), "configure file [%s] format check [OK].", szConfigFile);	

	m_ConfigFileName = szConfigFile;

	m_ConfigFile.open(szConfigFile);
	if (m_ConfigFile.fail()) {
		//cout << "--------->in ParseFile(const char* szConfigFile)" << endl;
		snprintf(m_ErrInfo.m_ErrMsg, sizeof(m_ErrInfo.m_ErrMsg), "open configure file[%s] error.", szConfigFile);
		m_ErrInfo.SetErrorStatus();
		return -1;
	}
	return ParseFile();
}

bool CServerNumPackageConf::IsExistNum(const string &szSection, unsigned int num)
{
	map<string, multimap<unsigned int, unsigned int> >::iterator mit;
	mit = m_ConfigMap.find(szSection); 
	if(mit==m_ConfigMap.end())
	{
		return false;
	}

	multimap<unsigned int, unsigned int>::iterator mmit = mit->second.begin();
	for(; mmit != mit->second.end(); mmit++)
	{
		unsigned int num_min = mmit->first, num_max = mmit->second;
		if(num_min <= num && num <= num_max)
		{
			//cout << "<" << num_min << "," << num_max << ">" << endl;
			return true;
		}
	}

	return false;
}

multimap<unsigned int, unsigned int> &CServerNumPackageConf::GetNumPackageRefFromSection(string szSection)
{
	return m_ConfigMap[szSection];
}

void CServerNumPackageConf::PrintAllSection()
{
	cout << "============in CServerNumPackageConf::PrintAllSection()============= "<< endl;
	map<string, multimap<unsigned int, unsigned int> >::iterator mit = m_ConfigMap.begin();
	for(; mit != m_ConfigMap.end(); mit++)
	{
		cout << "[" << mit->first << "]" << endl;

		multimap<unsigned int, unsigned int> &num_package = mit->second;
		multimap<unsigned int, unsigned int>::iterator mmit = num_package.begin();
		for(; mmit != num_package.end(); mmit++)
		{
			cout << "<" << mmit->first << "," << mmit->second << ">" << endl;
		}
	}
}

int CServerNumPackageConf:: ParseFile()
{
	char szLine[1024];
	char szSection[64], szParam[128];
	char *pColon;
	int  iLen;
	int iLine_num=0;

	bzero(szSection, sizeof(szSection));
	bzero(szParam, sizeof(szParam));

	while (m_ConfigFile.getline(szLine, sizeof(szLine))) 
	{
		iLine_num++;

		int ret = 0;
		if ( (ret = StrimString(szLine)) != 0) 
		{
			snprintf(m_ErrInfo.m_ErrMsg, sizeof(m_ErrInfo.m_ErrMsg), "error in config file[%s]: line_num[%d], errmsg[StrimString error, ret[%d], szLine[%s]]", m_ConfigFileName.c_str(), iLine_num, ret, szLine);
			m_ErrInfo.SetErrorStatus();
			continue;
		}

		iLen = strlen(szLine);
		if(0==iLen)
		{
			continue;
		}

		/*
		   support what?
		   1.section
		   2.num section
		   3.single num
		   4.error format
		 */
		//1.section
		if (((szLine[0]=='[') && (szLine[iLen-1]==']')))
		{
			pColon = szLine+1;
			szLine[iLen-1] = 0;
			bzero(szSection, sizeof(szSection));
			strncpy(szSection, pColon, sizeof(szSection));

			// section name
			continue;
		}

		//2.num section
		if((szLine[0]=='<') &&(szLine[iLen-1]=='>'))
		{
			szLine[iLen-1]=0;

			if ((pColon= index(szLine, ',')) != NULL) //found
			{
				*pColon=0;
			}
			else
			{
				pColon = szLine;
			}
			pColon++;

			if ((ret = StrimString(pColon)) < 0) 
			{
				snprintf(m_ErrInfo.m_ErrMsg, sizeof(m_ErrInfo.m_ErrMsg), "error in config file[%s]: line_num[%d], errmsg[StrimString error, ret[%d], szLine[%s]]", m_ConfigFileName.c_str(), iLine_num, ret, pColon);
				m_ErrInfo.SetErrorStatus();
				continue;
			}
			if ((ret = StrimString(szLine+1)) < 0)
			{
				snprintf(m_ErrInfo.m_ErrMsg, sizeof(m_ErrInfo.m_ErrMsg), "error in config file[%s]: line_num[%d], errmsg[StrimString error, ret[%d], szLine[%s]]", m_ConfigFileName.c_str(), iLine_num, ret, szLine+1);
				m_ErrInfo.SetErrorStatus();
				continue;
			}

			if((ret=IsNotNum(pColon))<0) 
			{
				snprintf(m_ErrInfo.m_ErrMsg, sizeof(m_ErrInfo.m_ErrMsg), "error in config file[%s]: line_num[%d], errmsg[IsNotNum error, ret[%d], szLine[%s]]", m_ConfigFileName.c_str(), iLine_num, ret, pColon);
				m_ErrInfo.SetErrorStatus();
				continue;
			}

			if((ret = IsNotNum(szLine+1))<0)
			{
				snprintf(m_ErrInfo.m_ErrMsg, sizeof(m_ErrInfo.m_ErrMsg), "error in config file[%s]: line_num[%d], errmsg[IsNotNum error, ret[%d], szLine[%s]]", m_ConfigFileName.c_str(), iLine_num, ret, szLine+1);
				m_ErrInfo.SetErrorStatus();
				continue;
			}


			if (szSection[0] != 0)
			{
				snprintf(szParam, sizeof(szParam), "%s", szLine+1);

				if(atoll(szParam) > atoll(pColon))
				{
					snprintf(m_ErrInfo.m_ErrMsg, sizeof(m_ErrInfo.m_ErrMsg), "error in config file[%s]: line_num[%d], errmsg:[left value > right value, left_value[%s], right_value[%s]]", m_ConfigFileName.c_str(), iLine_num, szParam, pColon);
					m_ErrInfo.SetErrorStatus();
					continue;
				}
				m_ConfigMap[szSection].insert(pair<unsigned int, unsigned int>(atoll(szParam), atoll(pColon)));
			}
			else
			{
				snprintf(m_ErrInfo.m_ErrMsg, sizeof(m_ErrInfo.m_ErrMsg), "error in config file[%s]: line_num[%d], errmsg:[no find  [section] before , szLine[%s]]", m_ConfigFileName.c_str(), iLine_num, szLine);
				m_ErrInfo.SetErrorStatus();
			}
			continue;
		}

		//3.single num
		if((ret = IsNotNum(szLine))<0)
		{
			snprintf(m_ErrInfo.m_ErrMsg, sizeof(m_ErrInfo.m_ErrMsg), "error in config file[%s]: line_num[%d], errmsg[IsNotNum error, ret[%d], szLine[%s]]", m_ConfigFileName.c_str(), iLine_num, ret, szLine);
			m_ErrInfo.SetErrorStatus();
			continue;
		}
		else
		{
			if (szSection[0] != 0)
			{
				m_ConfigMap[szSection].insert(pair<unsigned int, unsigned int>(atoll(szLine), atoll(szLine)));
			}
			else
			{
				snprintf(m_ErrInfo.m_ErrMsg, sizeof(m_ErrInfo.m_ErrMsg), "error in config file[%s]: line_num[%d], errmsg:[no find  [section] before , szLine[%s]]", m_ConfigFileName.c_str(), iLine_num, szLine);
				m_ErrInfo.SetErrorStatus();
			}


			continue;
		}

		//4.error format
		snprintf(m_ErrInfo.m_ErrMsg, sizeof(m_ErrInfo.m_ErrMsg), "error in config file[%s]: line_num[%d], errmsg:[invalid format, only 3 valid format: '[]' or '<>' or 'single num', szLine[%s]]", m_ConfigFileName.c_str(), iLine_num, szLine);
		m_ErrInfo.SetErrorStatus();

	}
	return 0;
}

int CServerNumPackageConf::IsNotNum(const char *ptr)
{
	while(*ptr!=0)
	{
		if(*ptr<'0' || *ptr>'9')
		{
			return -1;
		}

		ptr++;
	}
	return 0;
}

void CServerNumPackageConf::InitServerNumPackageConf()
{
	m_ConfigFileName="";
	m_ConfigFile.close();
	m_ConfigMap.clear();
	m_ErrInfo.InitAllErrorInfo();
}

