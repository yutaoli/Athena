#include "CMysql.h"
#include "person_manager.h"
#include <sstream>

using namespace std;
namespace PERSON{

	CPersonManager::CPersonManager()
	{
		memset(m_errmsg, 0, sizeof(m_errmsg));

		isConneted = false;
		int ret = Connect2DB();
		if(ret!=0)
		{
			return;
		}

		isConneted = true;
	}

	CPersonManager::~CPersonManager()
	{

	}

	char * CPersonManager::GetErrMsg()
	{
		return m_errmsg;
	}


	int CPersonManager::Connect2DB()
	{
		if(false == isConneted)
		{	
			m_ip = "10.213.121.21";
			m_port = 3306;
			m_user = "photo";
			m_password = "photo";
			m_dbname = "test";
			m_tablename = "persons";

			//init m_mysql
			int timeout = 2;
			int ret = 0;
			try     
			{
				m_mysql.Init(m_ip, m_user, m_password, m_port, "", timeout, timeout, timeout); 

				m_mysql.use(m_dbname);
			} 
			catch (const exception& e) 
			{
				ret = m_mysql.GetErrno();
				if(1130 == ret)//means: not allowed to connect to this MySQL server
				{
					snprintf(m_errmsg, sizeof(m_errmsg), "init mysql error, ret[%d], ip[%s], port[%d], user[%s], pass[%s], dbname[%s], tablename[%s], timeout[%d], errmsg[%s]\n", 
							ret, m_ip.c_str(), m_port, m_user.c_str(), m_password.c_str(), m_dbname.c_str(), m_tablename.c_str(), timeout, "not allowed to connect to this MySQL server");
				}
				else if(2013 == ret)//means: probably is ip:port wrong 
				{
					snprintf(m_errmsg, sizeof(m_errmsg), "init mysql error, ret[%d], ip[%s], port[%d], user[%s], pass[%s], dbname[%s], tablename[%s], timeout[%d], errmsg[%s]\n", 
							ret, m_ip.c_str(), m_port, m_user.c_str(), m_password.c_str(), m_dbname.c_str(), m_tablename.c_str(), timeout, (string(e.what()) + ", probably is ip:port wrong").c_str());
				}
				else
				{
					snprintf(m_errmsg, sizeof(m_errmsg), "init mysql error, ret[%d], ip[%s], port[%d], user[%s], pass[%s], dbname[%s], tablename[%s], timeout[%d], errmsg[%s]\n", 
							ret, m_ip.c_str(), m_port, m_user.c_str(), m_password.c_str(), m_dbname.c_str(), m_tablename.c_str(), timeout, e.what());
				}

				return -ret;
			}


		}
		return 0;
	}

	int CPersonManager::AddPerson(stPerson &person)
	{
		if(!isConneted)
		{
			snprintf(m_errmsg, sizeof(m_errmsg), "not conneted to mysql server");
			return -1;
		}

		ostringstream oss;
		oss << "insert into persons(lastname, firstname, address, telephone, city) values('" << m_mysql.escape_string(person.lastname) << "', '" << m_mysql.escape_string(person.firstname) << "', '" << m_mysql.escape_string(person.address) << "', '" << m_mysql.escape_string(person.telephone) << "', '" << m_mysql.escape_string(person.city) << "')"; 

		string sSql = oss.str();
		printf("sql[%s]\n", sSql.c_str());

		int ret = 0;
		try
		{
			MySqlData oData = m_mysql.query(sSql);

			if(oData.has_autoincrement_id())
			{
				person.personid = oData.get_insert_autoincrement_id();
				return person.personid;
			}
			else
			{
				snprintf(m_errmsg, sizeof(m_errmsg), "insert success, but not return autoincrement personid, ret[%d], ip[%s], port[%d], user[%s], password[%s], dbname[%s], tablename[%s]", ret, m_ip.c_str(), m_port, m_user.c_str(), m_password.c_str(), m_dbname.c_str(), m_tablename.c_str());
				return -1;
			}
		}
		catch(const exception &e)
		{
			ret = m_mysql.GetErrno();
			if(1049 == ret)//means: database not found
			{
				snprintf(m_errmsg, sizeof(m_errmsg), "mysql.query error, ret[%d], ip[%s], port[%d], user[%s], pass[%s], dbname[%s], errmsg[%s]", 
						ret, m_ip.c_str(), m_port, m_user.c_str(), m_password.c_str(), m_dbname.c_str(), "database not found");
			}
			else
			{
				snprintf(m_errmsg, sizeof(m_errmsg), "mysql.query error, ret[%d], ip[%s], port[%d], user[%s], pass[%s], dbname[%s], errmsg[%s]", 
						ret, m_ip.c_str(), m_port, m_user.c_str(), m_password.c_str(), m_dbname.c_str(), e.what());
			}

			return -ret;
		}

		return 0;

	}

	int CPersonManager::DelPersonById(stPerson &person)
	{
		if(!isConneted)
		{
			snprintf(m_errmsg, sizeof(m_errmsg), "not conneted to mysql server");
			return -1;
		}

		ostringstream oss;
		oss << "delete from " << m_tablename << " where personid = " << person.personid;

		string sSql = oss.str();
		printf("sql[%s]\n", sSql.c_str());

		int ret =0;
		try
		{
			MySqlData oData = m_mysql.query(sSql);//Êý¾Ý¿âµÄ·µ»ØÂëÔÚ³ö´íµÄÊ±ºò»ñÈ¡

			return 0;
		}
		catch(const exception &e)
		{
			ret = m_mysql.GetErrno();
			if(1049 == ret)//means: database not found
			{
				snprintf(m_errmsg, sizeof(m_errmsg), "mysql.query error, ret[%d], ip[%s], port[%d], user[%s], pass[%s], dbname[%s], tablename[%s], errmsg[%s]\n", 
						ret, m_ip.c_str(), m_port, m_user.c_str(), m_password.c_str(), m_dbname.c_str(), m_tablename.c_str(), "database not found");
			}
			else
			{
				snprintf(m_errmsg, sizeof(m_errmsg), "mysql.query error, ret[%d], ip[%s], port[%d], user[%s], pass[%s], dbname[%s], tablename[%s], errmsg[%s]\n", 
						ret, m_ip.c_str(), m_port, m_user.c_str(), m_password.c_str(), m_dbname.c_str(), m_tablename.c_str(), e.what());
			}

			return -ret;
		}

		return 0;
	}

	int CPersonManager::ModifyPerson(stPerson &person)
	{
		if(!isConneted)
		{
			snprintf(m_errmsg, sizeof(m_errmsg), "not conneted to mysql server");
			return -1;
		}

		ostringstream oss;
		oss << "update " << m_tablename << " set lastname = '" << m_mysql.escape_string(person.lastname) << "', firstname = '" << m_mysql.escape_string(person.firstname) << "', address = '" << m_mysql.escape_string(person.address) <<"', telephone = '" << m_mysql.escape_string(person.telephone) << "', city = '" << m_mysql.escape_string(person.city) << "' where personid = " << person.personid;

		string sSql = oss.str();
		printf("sql[%s]\n", sSql.c_str());

		int ret = 0;
		try
		{
			MySqlData oData = m_mysql.query(sSql);
			return 0;
		}
		catch(const exception &e)
		{
			ret = m_mysql.GetErrno();
			if(1049 == ret)//means: database not found
			{
				snprintf(m_errmsg, sizeof(m_errmsg), "mysql.query error, ret[%d], ip[%s], port[%d], user[%s], pass[%s], dbname[%s], tablename[%s], errmsg[%s]\n", 
						ret, m_ip.c_str(), m_port, m_user.c_str(), m_password.c_str(), m_dbname.c_str(), m_tablename.c_str(), "database not found");
			}
			else
			{
				snprintf(m_errmsg, sizeof(m_errmsg), "mysql.query error, ret[%d], ip[%s], port[%d], user[%s], pass[%s], dbname[%s], tablename[%s], errmsg[%s]", 
						ret, m_ip.c_str(), m_port, m_user.c_str(), m_password.c_str(), m_dbname.c_str(), m_tablename.c_str(), e.what());
			}

			return -ret;
		}

		return 0;
	}

	int CPersonManager::ListPerson(vector<stPerson> &person_list)
	{
		if(!isConneted)
		{
			snprintf(m_errmsg, sizeof(m_errmsg), "not conneted to mysql server");
			return -1;
		}

		ostringstream oss;
		oss << "select personid, lastname, firstname, address, telephone, city from "
			<< m_tablename << " order by personid asc";

		string sSql = oss.str();
		printf("sql[%s]\n", sSql.c_str());

		person_list.clear();
		int ret =0;
		try
		{
			MySqlData oData = m_mysql.query(sSql);//

			for (uint32_t i = 0; i < oData.num_rows(); i++) 
			{
				MySqlRowData oRow = oData[i];

				stPerson person;
				person.personid = (unsigned int)atoll(oRow["personid"].c_str());
				person.lastname = oRow["lastname"];
				person.firstname = oRow["firstname"];
				person.address = oRow["address"];
				person.telephone = oRow["telephone"];
				person.city = oRow["city"];

				person_list.push_back(person);
				printf("[%d]:personid[%u], lastname[%s], firstname[%s], address[%s], telephoto[%s], city[%s]\r\n",
						i, person.personid, person.lastname.c_str(), person.firstname.c_str(), person.address.c_str(),person.telephone.c_str(), person.city.c_str());
			}
		}
		catch(const exception &e)
		{
			ret = m_mysql.GetErrno();
			if(1049 == ret)//means: database not found
			{
				snprintf(m_errmsg, sizeof(m_errmsg), "mysql.query error, ret[%d], ip[%s], port[%d], user[%s], pass[%s], dbname[%s], tablename[%s], errmsg[%s]\n", 
						ret, m_ip.c_str(), m_port, m_user.c_str(), m_password.c_str(), m_dbname.c_str(), m_tablename.c_str(), "database not found");
			}
			else
			{
				snprintf(m_errmsg,sizeof(m_errmsg), "mysql.query error, ret[%d], ip[%s], port[%d], user[%s], pass[%s], dbname[%s], tablename[%s], errmsg[%s]\n", 
						ret, m_ip.c_str(), m_port, m_user.c_str(), m_password.c_str(), m_dbname.c_str(), m_tablename.c_str(), e.what());
			}

			return -ret;
		}


		return 0;

	}

	int CPersonManager::GetPersonInfo(stPerson &person, int personid)
	{
		if(!isConneted)
		{
			snprintf(m_errmsg, sizeof(m_errmsg), "not conneted to mysql server");
			return -1;
		}

		ostringstream oss;
		oss << "select personid, lastname, firstname, address, telephone, city from "
			<< m_tablename << " where personid = " << personid << " order by lastname asc";

		string sSql = oss.str();
		printf("sql[%s]\r\n", sSql.c_str());

		int ret =0;
		try
		{
			MySqlData oData = m_mysql.query(sSql);//

			for (uint32_t i = 0; i < oData.num_rows(); i++) 
			{
				MySqlRowData oRow = oData[i];

				person.personid = (unsigned int)atoll(oRow["personid"].c_str());
				person.lastname = oRow["lastname"];
				person.firstname = oRow["firstname"];
				person.address = oRow["address"];
				person.telephone = oRow["telephone"];
				person.city = oRow["city"];

				printf("[%d]:personid[%u], lastname[%s], firstname[%s], address[%s], telephoto[%s], city[%s]\r\n",
						i, person.personid, person.lastname.c_str(), person.firstname.c_str(), person.address.c_str(),person.telephone.c_str(), person.city.c_str());
			}
		}
		catch(const exception &e)
		{
			ret = m_mysql.GetErrno();
			if(1049 == ret)//means: database not found
			{
				snprintf(m_errmsg, sizeof(m_errmsg), "mysql.query error, ret[%d], ip[%s], port[%d], user[%s], pass[%s], dbname[%s], errmsg[%s]\n", 
						ret, m_ip.c_str(), m_port, m_user.c_str(), m_password.c_str(), m_dbname.c_str(), "database not found");
			}
			else
			{
				snprintf(m_errmsg,sizeof(m_errmsg), "mysql.query error, ret[%d], ip[%s], port[%d], user[%s], pass[%s], dbname[%s], errmsg[%s]\n", 
						ret, m_ip.c_str(), m_port, m_user.c_str(), m_password.c_str(), m_dbname.c_str(), e.what());
			}

			return -ret;
		}


		return 0;
	}

	//return personid
	int AddPerson(stPerson &person, string &errmsg)
	{
		CPersonManager manager;
		int ret = manager.AddPerson(person);
		if(ret <0)
		{
			errmsg = manager.GetErrMsg();
			return ret;
		}
		return ret;
	}

	int DelPersonById(stPerson &person, string &errmsg)
	{
		CPersonManager manager;
		int ret = manager.DelPersonById(person);
		if(ret !=0)
		{
			errmsg = manager.GetErrMsg();
			return ret;
		}
		return 0;

	}

	int ModifyPerson(stPerson &person, string &errmsg)
	{
		CPersonManager manager;
		int ret = manager.ModifyPerson(person);
		if(ret !=0)
		{
			errmsg = manager.GetErrMsg();
			return ret;
		}
		return 0;

	}

	int ListPerson(vector<stPerson> &person_list, string &errmsg)
	{
		CPersonManager manager;
		int ret = manager.ListPerson(person_list);
		if(ret !=0)
		{
			errmsg = manager.GetErrMsg();
			return ret;
		}
		return 0;
	}

	int GetPersonInfo(stPerson &person, int personid, string &errmsg)
	{
		CPersonManager manager;
		int ret = manager.GetPersonInfo(person, personid);
		if(ret !=0)
		{
			errmsg = manager.GetErrMsg();
			return ret;
		}
		return 0;
	}
}
