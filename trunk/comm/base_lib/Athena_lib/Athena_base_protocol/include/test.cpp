#include "test_protocol.h"
#include "persons.h"
#include "test_tcpclient_base.h"
#include <iostream>

#include "commlog.h" //log做debug用，后期稳定后做成一个基础库，把log去掉，错误信息通过GetErrMsg()返回调用方

using namespace Athena;

using namespace std;
using namespace PERSON;

int main()
{
	_INIT_LOG("/usr/local/photo/log/test_tcpclient_base", 10);
	CTcpClientBase oClient("10.12.198.53", 10086);
	oClient.SetCmd(1);

	stGetPersonInfoReq stReq;
	stGetPersonInfoRsp stRsp;
	stReq.personid = 2;
	int ret = oClient.SendAndRecv(stReq, stRsp, 1000);
	if(ret!=0)
	{
		printf("SendAndRecv error[%d], personid[%d], errmsg[%s]\n", ret, stReq.personid, oClient.GetErrMsg());
		return 0;
	}
	
	printf("retcode[%d], errmsg[%s], personid[%d], lastname[%s], firstname[%s], address[%s], telephone[%s], city[%s]\n",
		stRsp.base.retcode, stRsp.base.errmsg.c_str(), stRsp.person_info.personid, stRsp.person_info.lastname.c_str(), stRsp.person_info.firstname.c_str(), stRsp.person_info.address.c_str(), stRsp.person_info.telephone.c_str(), stRsp.person_info.city.c_str());

	printf("SendAndRecv [done]\n");

	return 0;
	
}

