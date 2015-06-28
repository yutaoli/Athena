#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include "json_value.h"
#include "json_writer.h"
#include "commlog.h" //log做debug用，后期稳定后做成一个基础库，把log去掉，错误信息通过GetErrMsg()返回调用方
#include "test_tcpclient_base.h"
#include "persons.h"

using namespace PERSON;

extern "C"{
#include "tlib_cgi.h"
}

enum charsetCheck {
	NO_CHECK = 0,
	UTF8_CHECK = 1,
	GBK_CHECK = 2
};

void genOutputData(Json::Value &data, const stGetPersonInfoRsp &stRsp)
{
	Json::Value person;
	person["personid"] = stRsp.person_info.personid;
	person["lastname"] = stRsp.person_info.lastname;
	person["firstname"] = stRsp.person_info.firstname;
	person["address"] = stRsp.person_info.address;
	person["telephone"] = stRsp.person_info.telephone;
	person["city"] = stRsp.person_info.city;

	data.append(person);

	person["personid"] = 11;
	person["lastname"] = "keller";
	person["firstname"] = "Hellen"; 
	data.append(person);

}

using namespace std;

int main(int argc, char *argv[])
{
	TLib_Cgi_Init();//处理QUERY_STRING和COOKIE和CONTENT

	string filename("/usr/local/photo/log/echo.log"); 
	ofstream ofile(filename.c_str());
	printf("\r\n\r\nsend by yutaoli from ip:10.12.198.53\r\n");  

	unsigned int hostuin = atoll(TLib_Cgi_Value("hostUin"));
	string skey = TLib_Cgi_Cookie("skey");//COOKIE
	DEBUG_LOG("hostuin[%u]", hostuin);   
	DEBUG_LOG("skey[%s]", skey.c_str());   
	DEBUG_LOG("g_tk[%s]", TLib_Cgi_Value("g_tk"));   
	DEBUG_LOG("personid[%s]", TLib_Cgi_Value("personid"));   


	unsigned int personid = atoll(TLib_Cgi_Value("personid"));//QUERY_STRING
	printf("personid[%u]\r\n", personid);

	TLib_Cgi_FreeEntries();//释放处理过的QUERY_STRING和COOKIE和CONTENT的内存

	_INIT_LOG("/usr/local/photo/log/test_fcg", 10);
	CTcpClientBase oClient("10.12.198.53", 10087); 
	oClient.SetCmd(1);

	stGetPersonInfoReq stReq;
	stGetPersonInfoRsp stRsp;
	stReq.personid = personid;
	int ret = oClient.SendAndRecv(stReq, stRsp, 1000);
	if(ret!=0)
	{
		ERROR_LOG("SendAndRecv error[%d], personid[%d], errmsg[%s]\n", ret, stReq.personid, oClient.GetErrMsg());
		printf("SendAndRecv error[%d], personid[%d], errmsg[%s]\r\n", ret, stReq.personid, oClient.GetErrMsg());
		return 0;
	}

	printf("retcode[%d], errmsg[%s], personid[%d], lastname[%s], firstname[%s], address[%s], telephone[%s], city[%s]\r\n",
			stRsp.base.retcode, stRsp.base.errmsg.c_str(), stRsp.person_info.personid, stRsp.person_info.lastname.c_str(), stRsp.person_info.firstname.c_str(), stRsp.person_info.address.c_str(), stRsp.person_info.telephone.c_str(), stRsp.person_info.city.c_str());

	Json::Value cgi_output_data;
	genOutputData(cgi_output_data, stRsp);

	Json::StyledWriter writer(true, GBK_CHECK);
	string output_data;
	output_data = writer.write(cgi_output_data);
	printf("%s", output_data.c_str());

	char *var = getenv ("QUERY_STRING");
	if(NULL != var)
	{
		printf("QUERY_STRING: %s\r\n", var);
	}
	var = NULL;

	var = getenv("HTTP_COOKIE");
	if(NULL != var)
	{   
		printf("HTTP_COOKIE: %s\r\n", var);
	}   
	var = NULL;


	var = getenv("HTTP_HEADER");
	if(NULL != var)
	{
		printf("HTTP_HEADER: %s\r\n", var);
	}
	var = NULL;

	var = getenv("HTTP_HEADER_LEN");
	if(NULL != var)
	{
		printf("HTTP_HEADER_LEN: %s\r\n", var);
	}
	var = NULL;

	var = getenv("CONTENT_LENGTH");
	if(NULL != var)
	{
		printf("CONTENT_LENGTH: %s\r\n", var);
		ofile << "CONTENT_LENGTH :" << var << "\r\n";
	}
	int post_len=atoi(var);

	if(0>=post_len)
	{
		printf("content body is empty!\r\n");
		return 0;
	}

	char *postvalue=(char*)malloc(post_len+1);
	if(NULL==postvalue)
	{
		printf("read post data failed, alloc memory failed!\r\n");
		return 0;       
	}

	int ret_len = fread(postvalue, 1, post_len,stdin);
	if(ret_len != post_len)
	{
		printf("read post data failed, length not match!\r\n");
		free(postvalue);
		return 0;
	}

	postvalue[post_len]='\0';

	printf("post_len:%d, postvalue:%s\r\n",post_len,postvalue);


	printf("printf will output to the web browser.\r\n");
	free(postvalue);

	return 0;
}





