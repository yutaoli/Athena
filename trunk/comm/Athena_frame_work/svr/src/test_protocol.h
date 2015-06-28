/*************************************************

-----------------
*author:yutaoli
*date:2014年11月23日 23:55:51

**************************************************/
#ifndef TEST_PROTOCOL_H
#define TEST_PROTOCOL_H

#pragma pack(1)

typedef struct TEST_HEADtag
{
	int _pack_len;
	unsigned short _req_cmd;
	int _rsp_code;		

	void SetCmd(int cmd)
	{
		_req_cmd = cmd;
	}
	
	//整个包长度
	void SetPackLen(int len)
	{
		_pack_len = len;
	}
	
	int GetPackLen()
	{
		return _pack_len;
	}

	//包头长度
	int GetHeadLen()
	{
		return sizeof(TEST_HEADtag);
	}

	//包体
	char * GetBody()
	{
		return (char*)this + GetHeadLen();
	}
	
	int GetRspCode()
	{
		return _rsp_code;
	}

}TEST_HEAD;

#pragma pack()
#endif
