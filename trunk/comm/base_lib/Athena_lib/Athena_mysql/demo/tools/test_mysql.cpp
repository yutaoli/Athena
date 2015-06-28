#include "person_manager.h"

using namespace PERSON;

int testAddPerson(string lastname, string firstname, string address, string telephone, string city)
{
	stPerson person;
	person.lastname = lastname;
	person.firstname = firstname;
	person.address = address;
	person.telephone = telephone;
	person.city = city;
	
	string errmsg;
	int ret = AddPerson(person, errmsg);
	if(ret <0)
	{
		printf("AddPerson error[%d], %s\n", ret, errmsg.c_str());
		return ret;
	}
	return 0;
}

int testDelPersonById()
{
	stPerson person;
	person.personid = 1;
	
	string errmsg;
	int ret = DelPersonById(person, errmsg);
	if(ret !=0)
	{
		printf("DelPersonById error[%d], personid[%d], %s\n", ret, person.personid, errmsg.c_str());
		return ret;
	}
	return 0;
}

int testModifyPerson(int personid, string lastname, string firstname, string address, string telephone, string city)
{
	stPerson person;
	person.personid = personid;
	person.lastname = lastname;
	person.firstname = firstname;
	person.address = address;
	person.telephone = telephone;
	person.city = city;

	string errmsg;
	int ret = ModifyPerson(person, errmsg);
	if(ret!=0)
	{
		printf("ModifyPerson error[%d], %s\n", ret, errmsg.c_str());
		return ret;
	}
	return 0;
}

int testListPerson()
{
	string errmsg;
	vector<stPerson> person_list;
	int ret = ListPerson(person_list, errmsg);
	if(ret!=0)
	{
		printf("%s\n", errmsg.c_str());
		return ret;
	}
	return 0;
}

int testGetPersonInfo()
{
	stPerson person;
	person.personid = 1;
	
	string errmsg;
	int ret = GetPersonInfo(person, person.personid, errmsg);
	if(ret !=0)
	{
		printf("%s\n", errmsg.c_str());
		return ret;
	}
	return 0;
}

int main()
{
	testListPerson();
	return 0;
}
