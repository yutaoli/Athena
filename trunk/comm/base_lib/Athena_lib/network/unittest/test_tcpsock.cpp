#include <string>
#include "tcpcli.h"
#include "sockaddr.h"

using namespace std;

typedef struct PackageHeadtag
{
	unsigned int packetlen;//inclduing head and packet == packetlen
}PackageHead;

int main(int argc, char *argv[])
{
	if(argc !=3)
	{
		printf("%s ip port\n", argv[0]);
		return 0;
	}

	Athena::CTcpConnector m_tcpclient;

	string ip = argv[1];
	int port = atoi(argv[2]);
	Athena::CSocketAddr stSockAddr(ip, atoi(argv[2]));
	int m_iConnTimeoutMS = 10000;
	int ret = m_tcpclient.AsyncConnect(stSockAddr, m_iConnTimeoutMS);
	if(ret < 0)
	{
		printf("AsyncConnect error[%d], ip[%s], port[%d], errmsg[%s]\n", ret, ip.c_str(), port, m_tcpclient.GetErrMsg());
		m_tcpclient.Close();
		return 0;
	}
	else
	{
		//send "hello world" to svr
		char ch[100]="hello world";
		printf("%s, len[%d]\n", ch, strlen(ch));

		int sendTimeOutMs, recvTimeOutMs;
		sendTimeOutMs = recvTimeOutMs = 1000;
		m_tcpclient.SetSendTimeOut(sendTimeOutMs/1000, sendTimeOutMs%1000 * 1000);
		m_tcpclient.SetRecvTimeOut(recvTimeOutMs/1000, recvTimeOutMs%1000* 1000);

		ret = m_tcpclient.Write_opt_n(ch, strlen(ch));
		if(ret < 0)
		{
			printf("m_tcpclient.Write_opt_n error[%d]\n", ret);
			m_tcpclient.Close();
			return 0;
		}

		printf("send ok....buf[%s]\n", ch);

		
		//test svr echo demo begin
		memset(ch, 0, sizeof(ch));
		ret = m_tcpclient.Read_opt_n(ch, 1000);
		if(ret<0)
		{
			printf("m_tcpclient.Read_opt_n ret[%d]\n", ret);
			return 0;
		}

		printf("recv packet ok, packet[%s]\n", ch);
		return 0;
		//test svr echo demo end

		//read packethead
		memset(ch, 0, sizeof(ch));
		ret = m_tcpclient.Read_opt_n(ch, sizeof(PackageHead));
		if(ret != sizeof(PackageHead))
		{
			printf("m_tcpclient.Read_opt_n error[%d], packethead.size[%d]\n", ret, sizeof(PackageHead));
			m_tcpclient.Close();
			return 0;
		}
		PackageHead *head = (PackageHead *)ch;
		unsigned int total_len = ntohl(head->packetlen);
		printf("total_len[%d]\n", total_len);

		//read remain packet
		char ch_packet[ total_len - sizeof(PackageHead) +1];
		ret = m_tcpclient.Read_opt_n(ch_packet, total_len - sizeof(PackageHead));
		if(ret != int(total_len - sizeof(PackageHead)))
		{
			printf("m_tcpclient.Read_opt_n error[%d], total_len - sizeof(PackageHead)[%d]\n", ret, total_len - sizeof(PackageHead));
			m_tcpclient.Close();
			return 0;
		}
		ch_packet[total_len - sizeof(PackageHead)] = 0;

		printf("recv packet ok, packet[%s]\n", ch_packet);

	}

	return 0;
}
