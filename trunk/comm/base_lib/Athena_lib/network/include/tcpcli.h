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
			   ����:������connect
			   ����:ms_timeout --����
			   ����ֵ���ɹ�0��ʧ��<0
			 */
			int AsyncConnect(const CSocketAddr &addr, unsigned int ms_timeout=0);
			/*
			   ����:���÷��͵ĳ�ʱʱ��
			   ����:sec --��usec--΢��
			   ����ֵ: ���óɹ�0
			   ʧ��-1
			 */
			int SetSendTimeOut(const unsigned int sec,const unsigned int usec );
			/*
			   ����:���ý��յĳ�ʱʱ��
			   ����:sec --��usec--΢��
			   ����ֵ: ���óɹ�0
			   ʧ��-1
			 */
			int SetRecvTimeOut(const unsigned int sec,const unsigned int usec );
			int Connect_tm(const CSocketAddr& addr,int timeout=-1);

			int handle_ready(int fd,struct timeval& timeout,int read_ready,int write_ready,int exception_ready);

			int Read(void *pMsg, int iLen);

			//ѭ����������iLen����ʱ���أ���ʱ�������ó�ʱ
			int Read_n(void *pMsg, int iLen,int timeout = -1) ;

			//ѭ����������iLen����ʱ���أ�selectʵ��ms����ʱ
			int Readn_ms(void *pMsg, int iLen,unsigned int timeout);

			//ѭ����������iLen����ʱ���أ���scoket option��SO_RCVTIMEOѡ�����ó�ʱ
			//����ֵ: ʵ�ʶ������ֽ���
			int Read_opt_n(void *pMsg,const int iLen);

			//
			int Write(const void *pMsg,int iLen);

			//ѭ��д����ʱ�������ó�ʱ
			int Write_n(const void *pMsg,int iLen,int timeout=-1);

			//ѭ��д����scoket option��SO_SENDTIMEOѡ�����ó�ʱ
			//����ֵ: ���͵��ֽڳ��ȣ����󷵻�<0
			int Write_opt_n(const void *pMsg,const int iLen);

			/*���� :�����Ƿ����
			  ����:block true Ϊ����
			  falseΪ������
			  ����ֵ: ���óɹ�����0,
			  ʧ�ܷ���-1
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

