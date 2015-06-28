/*
 * =====================================================================================
 * 
 *        Filename:  test_string_api.cpp
 * 
 *     Description:  test
 * 
 *         Version:  1.0
 *         Created:  2015年05月03日 21时35分38秒 CST
 *        Revision:  none
 *        Compiler:  gcc
 * 
 *          Author:  yutaoli (), yutaoli@tencent.com
 *         Company:  Shenzhen Tencent Co.Ltd.
 * 
 * =====================================================================================
 */


#include "string_api.h"
using namespace Athena;

int main()
{
	string str="|abcd|yutaoli||";
	vector<string> vec_res;

	CStringApi::SplitString(str, '|', vec_res);

	printf("str[%s]\n", str.c_str());
	for(unsigned int i = 0; i < vec_res.size(); i++)
	{
		printf("%d:vec_res[%s]\n", i, vec_res[i].c_str());
	}

	return 0;
}
