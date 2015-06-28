/***********************************************
*file: Athena_network_data_protocol.h
*description: 网络包头结构体。
有什么用？用来通过网络访问后端服务，所以需要命令字标识请求；
因为访问非本模块的后端服务，于是需要鉴权。
于是协议数据包含2部分：1）命令字，返回码；2）鉴权数据,具体鉴权数据结构体见Athena_base_pub_struct.h 

完整的网络数据包示意图：
|<----包头/Athena_head----->|<--变长数据-->|<-----------包体/Body--------->|
|<-----------------------------包/Pack------------------------------------>|

-----------------------------
*author: yutaoli
*date: 2014年11月22日 20:58:49
************************************************/
#ifndef ATHENA_NETWORK_DATA_PROTOCOL_H
#define ATHENA_NETWORK_DATA_PROTOCOL_H

namespace Athena
{
	typedef struct DETAIL_INFOtag
	{
		unsigned int _client_uin;	
		unsigned int _host_uin;
		char key[25];//skey
		unsigned short _ptlogin_state;//0xffff: 未验证，0: 验证成功, 1: 无权限

		unsigned short _auth_type;
		//client
		unsigned short _type_source;//前端来源，必须向后端申请备案，否则不能鉴权通过。
		unsigned int _req_ip;//前端ip即上一跳ip，发送以网络字节序发送，所以发送前要htonl
		unsigned int _client_ip;//客户端ip即最开始发起请求的ip，发送以网络字节序发送，所以发送前要htonl
		
		//server
		unsigned short _type_platform;//后端平台
		unsigned short _type_app;//后端应用，(后端平台,后端应用)二元组唯一确定一个应用
		unsigned int _server_ip//后端ip, 发送以网络字节序发送，所以发送前要htonl
		
		//reserve
		char _reserve_buf[17];
		char _reserve_buf2[15];
		
		//extend
		unsigned char _variable_extend_len;//变长数据长度，如果_variable_extend_len!=0，Athena_head后面的_variable_extend_len长度为变长数据

		DETAIL_INFOtag()
		{
			Clear();
		}
		
		void Clear()
		{
			memset(_key, 0, sizeof(_key));
			_ptlogin_state = 0xFFFF;	
		
			_type_auth_type = 0;
			_client_uin=0;
			_host_uin = 0;
			_type_source=0;
			_req_ip=0;
			_client_ip = 0;

			_type_platform = 0;
			_type_app = 0;
			_server_ip = 0;
	
			memset(_reserve_buf, 0, sizeof(_reserve_buf));
			memset(_reserve_buf2, 0, sizeof(_reserve_buf2));
			
			_variable_extend_len = 0;
		}
	}DETAIL_INFO;

	typedef struct ATHENA_HEADtag
	{
		unsigned short _version;//版本号，因为可能后续会新增字段，用版本号可以确定新增什么字段
		unsigned int _pack_len;//网络包总长度,最大支持2^32B即4GB
		unsigned short _req_cmd;
		short _rsp_code;
	
		DETAIL_INFO _detail_info;//鉴权数据

		ATHENA_HEADtag()
		{
			Clear();
		}

		void Clear()
		{
			_pack_len = 0;
			_req_cmd = 0;
			_rsp_code = 0;
			_detail_info.Clear();
		}

		int SetKey(string const & skey)
		{
			if(skey.length()>sizeof(_detail_info._key)-1)
			{
				return -1;
			}
		}
		
	}ATHENA_HEAD;


}//end of namespace Athena

#endif
