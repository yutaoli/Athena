/*
   @file: server_num_package_config.h

   @description:

   ��ȡ����������ļ���[cgi����]��������һϵ�е�<>����ʾcgi��Ӧ�ĺ����.

 *****�����ļ���ʽ*****
 1.֧��[section]�ļ�����
 ��:
 [gmcc] # gmcc section
 [unicom] # unicom section
 2.֧��<min, max>�����:<min, max>��ʾ�����Ϊ:min��max, ����min, max
 �磺
 <123, 1000>����ʾ��������123,124,125...,999,1000
 3.֧�ֵ�������('single num'):
 �磺
 123, ��ʾ����123
 4.# ���������Ϊע��
 5.ÿ��ǰ������ж���ո�
 6.�����ж������


 *****��������******
 1.��ʽ���󣺳���'[]', '<>', 'single num'֮���������ʽ
 2.min,maxΪ������
 3.<min, max>��,min>max


 *****�����Ų�*****
 ����1��
 PHOTOCONFIG::CServerNumPackageConf config("/usr/local/photo/conf/clear_num_package.ini");
 if(config.IsError())
 {
 cout << config.GetErrMsg()<<endl;
 }
 cout << config.GetCheckMsg()<<endl;
 ����д�����Կ�����ӦErrMsg

 ����2:
 PHOTOCONFIG::CServerNumPackageConf config;
 config.ParseFile("/usr/local/photo/conf/clear_num_package.ini");
 if(config.IsError())
 {
 cout << config.GetErrMsg()<<endl;
 }
 cout << config.GetCheckMsg()<<endl;
 ����д�����Կ�����ӦErrMsg

 @date:2014��12��10�� 22:36:29

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

