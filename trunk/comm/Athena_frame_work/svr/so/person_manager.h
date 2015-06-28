#ifndef _PERSON_MANAGER_H
#define _PERSON_MANAGER_H

#include <string>
#include "CMysql.h"
#include "persons.h"
using namespace std;

namespace PERSON{

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

			int AddPerson(Person &person);
			int DelPersonById(Person &person);
			int ModifyPerson(Person &person);
			int LiPerson(vector<Person> &person_list);
			int GetPersonInfo(Person &person, int personid);

	};

	int AddPerson(Person &person, string &errmsg);
	int DelPersonById(Person &person, string &errmsg);
	int ModifyPerson(Person &person, string &errmsg);
	int LiPerson(vector<Person> &person_list, string &errmsg);
	int GetPersonInfo(Person &person, int personid, string &errmsg);

}
#endif
