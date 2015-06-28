/*
   @file: server_num_package_config.h

   @description:

   读取号码包配置文件，[cgi名字]，接着是一系列的<>，表示cgi对应的号码段.

 *****配置文件格式*****
 1.支持[section]文件名字
 如:
 [gmcc] # gmcc section
 [unicom] # unicom section
 2.支持<min, max>号码段:<min, max>表示号码段为:min到max, 包括min, max
 如：
 <123, 1000>，表示包含号码123,124,125...,999,1000
 3.支持单个号码('single num'):
 如：
 123, 表示号码123
 4.# 后面的内容为注释
 5.每行前后可以有多余空格
 6.可以有多余空行


 *****错误情形******
 1.格式错误：除了'[]', '<>', 'single num'之外的其他格式
 2.min,max为非数字
 3.<min, max>中,min>max


 *****错误排查*****
 举例1：
 PHOTOCONFIG::CServerNumPackageConf config("/usr/local/photo/conf/clear_num_package.ini");
 if(config.IsError())
 {
 cout << config.GetErrMsg()<<endl;
 }
 cout << config.GetCheckMsg()<<endl;
 如果有错误可以看到对应ErrMsg

 举例2:
 PHOTOCONFIG::CServerNumPackageConf config;
 config.ParseFile("/usr/local/photo/conf/clear_num_package.ini");
 if(config.IsError())
 {
 cout << config.GetErrMsg()<<endl;
 }
 cout << config.GetCheckMsg()<<endl;
 如果有错误可以看到对应ErrMsg

 @date:2014年12月10日 22:36:29

 @author:yutaoli

 */

#ifndef _H_SERVER_NUM_PACKAGE_CONFIG_H_
#define _H_SERVER_NUM_PACKAGE_CONFIG_H_

#include <fstream>
#include <map>
#include <string>

using namespace std;

namespace Athena
{

	class CErrInfo
	{
		friend class CServerNumPackageConf;

		public:
		CErrInfo()
		{
			InitAllErrorInfo();
		}
		virtual ~CErrInfo(){}

		void InitAllErrorInfo()
		{
			memset(m_ErrMsg, 0, sizeof(m_ErrMsg));
			m_Error = false;// by default: no error
		}

		char * GetErrMsg(){return m_ErrMsg;}

		bool GetErrorStatus(){return m_Error;}
		void SetErrorStatus(){m_Error = true;}
		void SetOkStatus(){m_Error = false;}

		private:
		char m_ErrMsg[1024];
		bool m_Error;
	};

	class CServerNumPackageConf {
		public:
			CServerNumPackageConf();
			CServerNumPackageConf(const char* szFileName);//need to clear all member data
			virtual ~CServerNumPackageConf();


		public:
			bool IsExistNum(const string &szSection, unsigned int);
			multimap<unsigned int, unsigned int> & GetNumPackageRefFromSection(string szSection);
			int ParseFile(const char* szConfigFile);//need to clear all member data

			void PrintAllSection();
			char * GetErrMsg(){return m_ErrInfo.GetErrMsg();}
			char * GetCheckMsg(){return m_ErrInfo.GetErrMsg();}

			bool IsError(){return m_ErrInfo.GetErrorStatus();}

		private:
			int IsNotNum(const char *ptr);
			static int StrimString(char* szLine);
			int ParseFile();
			void InitServerNumPackageConf();//clear all member data

		private:
			string m_ConfigFileName;
			ifstream m_ConfigFile;
			map<string, multimap<unsigned int, unsigned int> > m_ConfigMap;

			CErrInfo m_ErrInfo;
	};

};//end of namespace Athena
#endif // end of _H_SERVER_NUM_PACKAGE_CONFIG_H_

