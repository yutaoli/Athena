/********************************
*file: Athena_base_pub_struct.h
*description: 网络数据包鉴权结构体及数据

----------------------------
*author: yutaoli
*date: 2014年11月22日 20:28:55
*********************************/

#ifndef ATHENA_BASE_PUB_STRUCT_H
#define ATHENA_BASE_PUB_STRUCT_H

namespace Athena
{
	enum ENUM_AUTH_TYPE
	{
		ENUM_AUTH_TYPE_NULL = 0,
		ENUM_AUTH_TYPE_WEB = 1,//skey
		ENUM_AUTH_TYPE_SVR = 2,
		ENUM_AUTH_TYPE_3G = 3
	};

	typedef struct AUTH_INFOtag
	{
		//client
		unsigned _uin;//用户号码, 用来确定用户
		unsigned _enum_auth_type;//取值来自ENUM_AUTH_TYPE
		string _key;//skey
		string _extend_key;
		unsigned _ptlogin_id;//0: check success, 1: check error, permission deny

		unsigned _type_source;//前端来源，必须向后端申请，不然无法访问
		unsigned _client_ip;//发送时以网络字节序发送

		//server
		unsigned _type_platform;//后端平台
		unsigned _type_app;//后端服务，后端平台-后端服务唯一确定一个后端服务
		
		unsigned _server_ip;//发送时以网络字节序发送
	}AUTH_INFO;


}//end of namespace Athena

#endif
