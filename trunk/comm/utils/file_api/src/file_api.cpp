/*
 * =====================================================================================
 * 
 *        Filename:  file_api.cpp
 * 
 *     Description:  unix file api wraper
 * 
 *         Version:  1.0
 *         Created:  2015年05月03日 22时40分55秒 CST
 *        Revision:  none
 *        Compiler:  gcc
 * 
 *          Author:  yutaoli (), yutaoli@tencent.com
 *         Company:  Shenzhen Tencent Co.Ltd.
 * 
 * =====================================================================================
 */

#include "file_api.h"
#include <unistd.h>
#include <errno.h>
#include <fstream>
#include <set>
#include <vector>

using namespace std;

namespace Athena
{

	int CFileApi::RemoveFile(const char *filename)
	{
		if(unlink(filename) < 0)
		{
			return -1;
		}

		return 0;
	}

	//改

	//查
	bool CFileApi::IsFileExist(const char *filename)
	{
		if(access(filename, F_OK) < 0)
		{
			return false;
		}

		return true;
	}

	bool CFileApi::IsDirExist(const char *dirname)
	{
		return IsFileExist(dirname);
	}

	bool CFileApi::CanReadFile(const char *filename)
	{
		if(access(filename, R_OK) < 0)
		{
			return false;
		}

		return true;
	}

	bool CFileApi::CanWriteFile(const char *filename)
	{
		if(access(filename, W_OK) < 0)
		{
			return false;
		}

		return true;
	}



	//从filename中读出内容到set中，一行一个string
	//输入参数：filename
	//输出参数：m_finish_filename_set
	int CFileApi::ListFile(char *filename, set<string> &m_finish_filename_set)
	{
		m_finish_filename_set.clear();

		if(!IsFileExist(filename))
		{
			ClearErrMsg();
			snprintf(m_ErrMsg, sizeof(m_ErrMsg), "file[%s] is not exist", filename);
			return -1;
		}

		ifstream ifile(filename);
		string finish_file_name;
		while(getline(ifile, finish_file_name))
		{
			m_finish_filename_set.insert(finish_file_name);
		}

		return 0;
	}




	//列出目录下的所有文件和目录，包括隐藏文件和目录
	//输入参数dirname
	//输出参数m_filename_set
	int CFileApi::ListDir(const char *dirname, set<string> &m_filename_set)
	{
		m_filename_set.clear();

		DIR *dp;
		struct dirent *dirp;

		if ((dp = opendir(dirname)) == NULL)
		{
			ClearErrMsg();
			snprintf(m_ErrMsg, sizeof(m_ErrMsg), "opendir error[%d], errmsg[%s]", errno, strerror(errno));
			return -1;
		}

		while((dirp = readdir(dp)) != NULL)
		{
			m_filename_set.insert(dirp->d_name);
		}

		closedir(dp);

		return 0;
	}

};//end of namespace Athena
