/*
 * =====================================================================================
 * 
 *        Filename:  file_api.h
 * 
 *     Description:  unix file api wraper
 * 
 *         Version:  1.0
 *         Created:  2015年05月03日 21时54分27秒 CST
 *        Revision:  none
 *        Compiler:  gcc
 * 
 *          Author:  yutaoli ()
 *         Company:  Shenzhen Tencent Co.Ltd.
 *           Email:  yutaoli@tencent.com
 * 
 * =====================================================================================
 */

#ifndef _FILE_API_H_
#define _FILE_API_H_

#include <dirent.h>
#include <set>

using namespace std;

namespace Athena
{
	class CFileApi
	{
		public:
			CFileApi(){ClearErrMsg();};
			virtual ~CFileApi(){}


		public:
			//增

			//删
			static int RemoveFile(const char *filename);

			//改

			//查
			static bool IsFileExist(const char *filename);

			static bool IsDirExist(const char *dirname);

			static bool CanReadFile(const char *filename);

			static bool CanWriteFile(const char *filename);


			//从filename中读出内容到set中，一行一个string
			//输入参数：filename
			//输出参数：m_finish_filename_set
			int ListFile(char *filename, set<string> &m_finish_filename_set);


			//列出目录下的所有文件和目录，包括隐藏文件和目录
			//输入参数dirname
			//输出参数m_filename_set
			int ListDir(const char *dirname, set<string> &m_filename_set);

		public:
			char *GetErrMsg(){return m_ErrMsg;}
			void ClearErrMsg(){memset(m_ErrMsg, 0, sizeof(m_ErrMsg));}
		private:
			char m_ErrMsg[1024];
	};

};//end of namespace Athena

#endif //end of _FILE_API_H_
