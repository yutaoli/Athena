/*
 * =====================================================================================
 * 
 *        Filename:  test_file_api.cpp
 * 
 *     Description:  for test
 * 
 *         Version:  1.0
 *         Created:  2015年05月03日 22时53分57秒 CST
 *        Revision:  none
 *        Compiler:  gcc
 * 
 *          Author:  yutaoli (), yutaoli@tencent.com
 *         Company:  Shenzhen Tencent Co.Ltd.
 * 
 * =====================================================================================
 */

#include "file_api.h"
#include <string>

using namespace std;

using namespace Athena;

int main()
{
	//判断目录是否存在
	char dirname[]="/data/";
	
	if(CFileApi::IsDirExist(dirname))
	{
		printf("dirname[%s] exist\n", dirname);
	}
	else
	{
		printf("dirname[%s] not exist\n", dirname);
	}


	//ls
	CFileApi file_api;
	set<string> file_set;
	int ret = file_api.ListDir(dirname, file_set);
	if(ret != 0)
	{
		printf("ListDir error, ret[%d], dirname[%s], errmsg[%s]\n", ret, dirname, file_api.GetErrMsg());
		return 0;
	}
	
	
	set<string>::iterator s_it = file_set.begin();
	for(; s_it != file_set.end(); s_it++)
	{
		printf("file_name[%s]\n", (*s_it).c_str());
	}

	return 0;
}
