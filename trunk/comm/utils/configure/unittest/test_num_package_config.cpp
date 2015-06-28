#include <iostream>
#include "server_num_package_config.h"

using namespace std;

int main()
{
	Athena::CServerNumPackageConf config;
	config.ParseFile("./clear_num_package.ini");
	//config.ParseFile("./clear_num_package.ini");
	if(config.IsError())
	{
		cout << config.GetErrMsg()<<endl;
		return 0;
	}
	cout << config.GetCheckMsg()<<endl;

	multimap<unsigned int, unsigned int> m_map = config.GetNumPackageRefFromSection("clear_recycle");

	multimap<unsigned int, unsigned int>::iterator mit = m_map.begin();
	for(; mit!=m_map.end(); mit++)
	{
		cout << mit->first << " " << mit->second <<endl;
	}

	unsigned int uins[] = 
	{611966429U, 611966480U, 3374998180U, 578527422U, 611966481U, 611967078U};

	for(unsigned int i = 0; i < sizeof(uins)/sizeof(unsigned int); i++)
	{
		if(config.IsExistNum("clear_recycle", uins[i]))
		{
			cout << "-------->" << uins[i] << " found" << endl;
		}
		else
		{
			cout << "------------>" << uins[i] << " not found" << endl;
		}
	}

	return 0;
}
