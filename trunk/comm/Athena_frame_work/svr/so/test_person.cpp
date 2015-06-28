#include <iostream>
#include <stdio.h>
#include "CommDef.h"
#include "InitiationDispatcher.h"
#include "EventHandlerBase.h"
#include "Tcphelper.h"
#include "Log.h"
#include "svrframe_incl.h"
#include "test_parser.h"
#include "persons.h"
#include "person_manager.h"

using namespace PERSON;

extern "C" int svrframe_check_complete(unsigned flow, char* buf, int len)
{

}

extern "C" int svrframe_handle_process(unsigned flow, void* argv1)
{	
	//最简单的跑通打解包逻辑，测试用
	CEventHandler *event_handler = (CEventHandler*) argv1;

	char m_errmsg[1024]={0};
	TestParser parser;
	//Decode Body
	PERSON::stGetPersonInfoReq stReq;
	char *in_buf = event_handler->GetSendBuf();
	unsigned int in_buf_len = event_handler->GetUsedBufLen();
	int ret = parser.Decode(in_buf, in_buf_len, stReq);
	if(ret !=0)
	{
		ERROR_LOG("m_parser.Decode error, ret[%d], errmsg[%s]", ret, parser.GetErrMsg());

		snprintf(m_errmsg, sizeof(m_errmsg), "m_parser.Decode error, ret[%d], errmsg[%s]", ret, parser.GetErrMsg());
		memcpy(in_buf, m_errmsg, strlen(m_errmsg));

		ret = event_handler->send_rsp(strlen(m_errmsg));
		if(ret!=0)
		{
			ERROR_LOG("CEventHandler::handle_process error, ret[%d]", ret);	
			return -1;
		}

		return 0;
	}

	DEBUG_LOG("decode body [done], recv buf address[%p], recv buf len[%d]", in_buf, in_buf_len);
	DEBUG_LOG("req: personid[%d]", stReq.personid);

	///////////////////////////////
	PERSON::Person person;
	string errmsg; 
	ret = GetPersonInfo(person, stReq.personid, errmsg);
	if(ret !=0)
	{
		ERROR_LOG("GetPersonInfo error, ret[%d], errmsg[%s]\n", ret, errmsg.c_str());
		return ret;
	}


	PERSON::stGetPersonInfoRsp stRsp;
	stRsp.base.retcode = ret;
	stRsp.base.errmsg = errmsg;
	stRsp.person_info = person;
	char *out_buf = NULL;	
	unsigned int out_buf_len = 0;
	int trylen = 1000;

	parser.MapBuffer(event_handler->GetSendBuf(), event_handler->GetMaxBufLen());//插件的buf是复用CEventHandler的_buf的，通过GetSendBuf()取得 
	ret = parser.Encode(stRsp, trylen, out_buf, out_buf_len);
	if(ret!=0)
	{       
		ERROR_LOG("m_parser.Encode error, ret[%d], errmsg[%s]", ret, parser.GetErrMsg());

		return ret;
	}       

	DEBUG_LOG("encode Head+Body [done], encoded buf[%s], encoded buf address[%p], len[%d]", 
			string(out_buf, out_buf_len).c_str(), out_buf, out_buf_len);

	ret = event_handler->send_rsp(out_buf_len);
	if(ret!=0)
	{
		ERROR_LOG("CEventHandler::handle_process error, ret[%d]", ret);
		return -1;
	}
	return 0;



}
