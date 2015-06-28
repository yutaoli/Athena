#ifndef __TCPCONNECTOR__HH__
#define __TCPCONNECTOR__HH__

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include "sockaddr.h"

namespace Athena{

	class CTcpConnector
	{
		public:
			CTcpConnector();
			~CTcpConnector(void);


			int Connect(const CSocketAddr& addr);
			/*
			   功能:非阻塞connect
			   参数:ms_timeout --毫秒
			   返回值：成功0，失败<0
			 */
			int AsyncConnect(const CSocketAddr &addr, unsigned int ms_timeout=0);
			/*
			   功能:设置发送的超时时间
			   参数:sec --秒usec--微秒
			   返回值: 设置成功0
			   失败-1
			 */
			int SetSendTimeOut(const unsigned int sec,const unsigned int usec );
			/*
			   功能:设置接收的超时时间
			   参数:sec --秒usec--微秒
			   返回值: 设置成功0
			   失败-1
			 */
			int SetRecvTimeOut(const unsigned int sec,const unsigned int usec );
			int Connect_tm(const CSocketAddr& addr,int timeout=-1);

			int handle_ready(int fd,struct timeval& timeout,int read_ready,int write_ready,int exception_ready);

			int Read(void *pMsg, int iLen);

			//循环读数据至iLen，超时返回，用时钟来设置超时
			int Read_n(void *pMsg, int iLen,int timeout = -1) ;

			//循环读数据至iLen，超时返回，select实现ms级计时
			int Readn_ms(void *pMsg, int iLen,unsigned int timeout);

			//循环读数据至iLen，超时返回，用scoket option的SO_RCVTIMEO选项设置超时
			//返回值: 实际读到的字节数
			int Read_opt_n(void *pMsg,const int iLen);

			//
			int Write(const void *pMsg,int iLen);

			//循环写，用时钟来设置超时
			int Write_n(const void *pMsg,int iLen,int timeout=-1);

			//循环写，用scoket option的SO_SENDTIMEO选项设置超时
			//返回值: 发送的字节长度，错误返回<0
			int Write_opt_n(const void *pMsg,const int iLen);

			/*功能 :设置是否堵塞
			  参数:block true 为阻塞
			  false为非阻塞
			  返回值: 设置成功返回0,
			  失败返回-1
			 */
			int setBlock(bool block);

			/*int*/void Close(){ if(m_sockfd>0) close(m_sockfd); m_sockfd=-1;}

			//just for test
			int getFd(){return m_sockfd;};

			const char * GetErrMsg(){return m_szErrMsg;}

		private:
			bool   m_iBlock;
			string m_addr;
			int m_port;
			int m_sockfd;
			char m_szErrMsg[1024];
	};

}

#endif  //__TCPSOCKETCONNECTOR__HH__

