/*
 * =====================================================================================
 * 
 *        Filename:  string_api.cpp
 * 
 *     Description:  api beyond string c++ 标准库
 * 
 *         Version:  1.0
 *         Created:  2015年05月03日 21时30分04秒 CST
 *        Revision:  none
 *        Compiler:  gcc
 * 
 *          Author:  yutaoli (), yutaoli@tencent.com
 *         Company:  Shenzhen Tencent Co.Ltd.
 * 
 * =====================================================================================
 */

#include "string_api.h"


namespace Athena
{

	void CStringApi::SplitString(const string &str, char c, vector<string> &vec)
	{
		if(str.empty())
		{
			vec.clear();
			return; 
		}

		string::size_type pos1, pos2;
		pos1 = 0;
		pos2 = 0;
		while(pos2 < str.length() && str[pos2] != c)
		{
			pos2++; 
		}
		//printf("pos2[%d], pos1[%d]\n", pos2, pos1);

		vec.clear();
		vec.push_back(str.substr(pos1, pos2 - pos1)); 

		while(pos2 < str.length())
		{
			pos1 = pos2 + 1;
			pos2 = pos1; 
			while(pos2 < str.length() && str[pos2] != c)
			{       
				pos2++; 
			}       
			//printf("pos2[%d], pos1[%d]\n", pos2, pos1);

			if(str.length() == pos1)
			{       
				vec.push_back(string(""));
			}       
			else    
			{       
				vec.push_back(str.substr(pos1, pos2 - pos1)); 
			}       
		}
	}

};//end of namespace Athena
