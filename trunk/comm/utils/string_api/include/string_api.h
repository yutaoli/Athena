/*
 * =====================================================================================
 * 
 *        Filename:  string_api.h
 * 
 *     Description:  api beyond string c++ 标准库
 * 
 *         Version:  1.0
 *         Created:  2015年05月03日 21时25分02秒 CST
 *        Revision:  none
 *        Compiler:  gcc
 * 
 *          Author:  yutaoli
 *         Company:  Shenzhen Tencent Co.Ltd.
 *           Email:  yutaoli@tencent.com
 * 
 * =====================================================================================
 */

#ifndef _STRING_API_H_
#define _STRING_API_H_

#include <string>
#include <vector>

using namespace std;

namespace Athena
{
	class CStringApi
	{
		public:
			CStringApi(){}
			virtual ~CStringApi(){}

		public:
			static void SplitString(const string &str, char c, vector<string> &vec);

	};

};//end of namespace Athena


#endif //end of _STRING_API_H_
