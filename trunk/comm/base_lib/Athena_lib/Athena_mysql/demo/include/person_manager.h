#ifndef _PERSON_MANAGER_H
#define _PERSON_MANAGER_H

#include <string>
#include "CMysql.h"
using namespace std;

namespace PERSON{
	struct stPerson
	{
		int personid;
		string lastname;
		string firstname;
		string address;
		string telephone;
		string city;
	};

	class CPersonManager
	{
		private:
			CMySql m_mysql;
			string m_ip;
			int m_port;
			string m_user;
			string m_password;
			string m_dbname;
			string m_tablename;
			bool isConneted;

			char m_errmsg[1024];

		private:
			int Connect2DB();

		public:
			CPersonManager();
			~CPersonManager();

			char * GetErrMsg();

			int AddPerson(stPerson &person);
			int DelPersonById(stPerson &person);
			int ModifyPerson(stPerson &person);
			int ListPerson(vector<stPerson> &person_list);
			int GetPersonInfo(stPerson &person, int personid);

	};

	int AddPerson(stPerson &person, string &errmsg);
	int DelPersonById(stPerson &person, string &errmsg);
	int ModifyPerson(stPerson &person, string &errmsg);
	int ListPerson(vector<stPerson> &person_list, string &errmsg);
	int GetPersonInfo(stPerson &person, int personid, string &errmsg);

}
#endif
