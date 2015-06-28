#include "tcpcli.h"
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>

namespace Athena{

	static void (*SigHandle)(int);

	static sigjmp_buf m_buf;

	static void AlarmHandle(int)
	{
		siglongjmp(m_buf,1);
	}

	CTcpConnector::CTcpConnector():m_iBlock(true),m_sockfd(-1)
	{
		memset(m_szErrMsg, 0, sizeof(m_szErrMsg));
	}

	CTcpConnector::~CTcpConnector(void)
	{
		Close();
	}

	/**********************************************************
	  功能 :设置是否堵塞
	  参数:block true 为阻塞
	  false为非阻塞
	  返回值: 设置成功返回0,
	  失败返回-1
	 ***********************************************************/
	int CTcpConnector::setBlock(bool block)
	{

		int iFlag = fcntl(m_sockfd, F_GETFL);
		if (iFlag < 0)
			return -1;

		if (block)
			iFlag &= ~O_NONBLOCK;
		else
			iFlag |= O_NONBLOCK;

		if (fcntl(m_sockfd, F_SETFL, iFlag) < 0)
			return -1;

		m_iBlock = block;
		return 0;

	}
	int CTcpConnector::AsyncConnect(const CSocketAddr &addr, unsigned int ms_timeout)
	{
		//socket前先把之前打开的fd close掉
		Close();

		m_sockfd = socket(AF_INET,SOCK_STREAM , 0);
		if(m_sockfd < 0 )
		{
			snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s", strerror(errno) );
			return -1;
		}

		// set nonblock
		int flags = fcntl(m_sockfd, F_GETFL, 0);
		if (fcntl(m_sockfd, F_SETFL, flags | O_NONBLOCK) < 0)
		{
			Close();
			return -2;
		}

		struct sockaddr_in serv_addr = addr.getAddress();
		socklen_t addr_len = sizeof(serv_addr);

		// connect
		int iRetCode = connect(m_sockfd, (struct sockaddr *)&serv_addr, addr_len);
		if (iRetCode < 0)
		{
			if (errno != EINPROGRESS)
			{
				snprintf(m_szErrMsg, sizeof(m_szErrMsg), "connect error: [%d] %s", errno, strerror(errno));
				Close();
				return -2;
			}
		}

		if (iRetCode == 0)
		{
			// connect completed immediately
			;
		}
		else
		{
			fd_set rset, wset;
			FD_ZERO(&rset);
			FD_SET(m_sockfd, &rset);
			wset = rset;

			struct timeval tval;

			if(ms_timeout == 0)
			{
				tval.tv_sec = 1;
				tval.tv_usec = 0;
			}
			else
			{
				tval.tv_sec = ms_timeout/1000;
				tval.tv_usec = (ms_timeout%1000)*1000;
			}

			iRetCode = select(m_sockfd + 1, &rset, &wset, NULL, &tval);
			if (iRetCode == 0)
			{
				// timeout
				snprintf(m_szErrMsg, sizeof(m_szErrMsg), "connect error: select timeout [%d] %s, fd=%d", errno, strerror(errno), m_sockfd);

				Close();
				return -3;
			}

			if (FD_ISSET(m_sockfd, &rset) || FD_ISSET(m_sockfd, &wset))
			{
				int error;
				socklen_t len = sizeof(error);
				if (getsockopt(m_sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
				{
					snprintf(m_szErrMsg, sizeof(m_szErrMsg), "connect error: getsockopt [%d] %s, fd=%d", errno, strerror(errno), m_sockfd);

					Close();
					return -4;
				}

				if (error)
				{
					snprintf(m_szErrMsg, sizeof(m_szErrMsg), "connect error: [%d] %s, fd=%d", errno, strerror(errno), m_sockfd);

					Close();
					return -5;
				}
			}
			else
			{
				snprintf(m_szErrMsg, sizeof(m_szErrMsg), "connect error: [%d] %s, fd=%d", errno, strerror(errno), m_sockfd);

				Close();
				return -6;
			}
		}

		// restore file status flags
		if (fcntl(m_sockfd, F_SETFL, flags) < 0)
		{
			snprintf(m_szErrMsg, sizeof(m_szErrMsg), "connect error: fcntl [%d] %s", errno, strerror(errno));

			Close();
			return -1;
		}

		return 0;
	}


	int CTcpConnector::Connect(const CSocketAddr &addr)
	{
		//socket前先把之前打开的fd close掉
		Close();

		m_sockfd = socket(AF_INET,SOCK_STREAM , 0 );
		if(m_sockfd < 0 )
		{
			snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s", strerror(errno) );
			return -1;
		}

		struct sockaddr_in address = addr.getAddress();

		int res = connect(m_sockfd,(struct sockaddr*)&address,sizeof(address));
		if(res < 0)
		{
			if ( (m_iBlock && errno == EINTR)
					|| (!m_iBlock && errno == EINPROGRESS) )
			{
				Close();
				return 1;
			}
			Close();
			snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s", strerror(errno) );
			return -1;
		}

		return 0;
	}


	int CTcpConnector::Connect_tm( const CSocketAddr& addr, int timeout )
	{
		//socket前先把之前打开的fd close掉
		Close();

		if((m_sockfd=socket(AF_INET,SOCK_STREAM,0))<0) {
			snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s", strerror(errno) );
			return -1;
		}

		struct sockaddr_in address = addr.getAddress();

		if(timeout== -1) {
			int res = connect(m_sockfd,(struct sockaddr*)&address,sizeof(address));
			if(res < 0)
			{
				if ( (m_iBlock && errno == EINTR)
						|| (!m_iBlock && errno == EINPROGRESS) )
				{
					Close();
					return 1;
				}
				Close();
				snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s", strerror(errno) );
				return -1;
			}
		}
		else
		{
			SigHandle = signal(SIGALRM,&AlarmHandle);
			if(sigsetjmp(m_buf,1))
			{
				alarm(0);
				signal(SIGALRM,SigHandle);
				Close();
				snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s", strerror(errno) );
				return -2; //time out
			}
			alarm(timeout);
			int res = connect(m_sockfd,(struct sockaddr*)&address,sizeof(address));
			if(res <0)
			{
				alarm(0);
				signal(SIGALRM,SigHandle);
				Close();
				snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s", strerror(errno) );
				return -1;
			}
			alarm(0);
			signal(SIGALRM,SigHandle);
		}

		return 0;
	}

	/*
	   功能:设置发送的超时时间
	   参数:sec --秒usec--微秒
	   返回值: 设置成功0
	   失败-1
	 */
	int CTcpConnector::SetSendTimeOut(const unsigned int sec,const unsigned int usec )
	{
		struct timeval tv;
		tv.tv_sec = sec;
		tv.tv_usec = usec;
		return setsockopt(m_sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
	}
	/*
	   功能:设置接收的超时时间
	   参数:sec --秒usec--微秒
	   返回值: 设置成功0
	   失败-1
	 */
	int CTcpConnector::SetRecvTimeOut(const unsigned int sec,const unsigned int usec )
	{
		struct timeval tv;
		tv.tv_sec = sec;
		tv.tv_usec = usec;
		return setsockopt(m_sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	}
	int CTcpConnector::Read(void* pBuf, int iLen)
	{
#if 0
		if(timeout !=-1) {
			SigHandle = signal(SIGALRM,&AlarmHandle);
			if(sigsetjmp(m_buf,1))
			{
				alarm(0);
				signal(SIGALRM,SigHandle);
				return -2; //time out
			}
			alarm(timeout);
		}
#endif
		return read(m_sockfd,pBuf,iLen);

	}

	int CTcpConnector::handle_ready(
			int fd,
			struct timeval& timeout,
			int read_ready,
			int write_ready,
			int exception_ready
			)
	{
		// Wait for data or for the timeout to elapse. 
		int select_width = fd +1;

		fd_set fdset;
		FD_ZERO(&fdset);
		FD_SET(fd, &fdset);
		int  retval = select(
				select_width,
				read_ready? &fdset:0 ,
				write_ready? &fdset:0,
				exception_ready? &fdset:0,
				&timeout);

		if(retval == 0) 
		{
			errno = ETIME;
			return 0;
		}

		return retval;

	}

	int CTcpConnector::Readn_ms(void *pMsg, int iLen,unsigned int timeout)
	{
		struct timeval tm;
		tm.tv_sec = timeout/1000;
		tm.tv_usec = (timeout%1000)*1000;
		int res; 
		if((res = handle_ready(m_sockfd,tm,1,0,0))>0)
		{ 
			char *pBuf =(char*)pMsg;
			int bytes_read = 0;

			int this_read;
			while( bytes_read < iLen )
			{
				do
				{
					this_read = read(m_sockfd, pBuf, iLen - bytes_read);
				}
				while ( ( this_read < 0 )  && ( errno == EINTR ) );

				if (this_read < 0)
				{
					return this_read;
				}
				else if(this_read == 0) {
					return bytes_read;
				}

				bytes_read += this_read;

				pBuf += this_read;
			}
			return bytes_read;
		}
		else  
		{
			return -1; 
		}
	}

	/*
	   用时钟来设置超时
	 */
	int CTcpConnector::Read_n(void *pBuf, int iLen,int timeout)
	{

		if(timeout !=-1) {
			SigHandle = signal(SIGALRM,&AlarmHandle);
			if(sigsetjmp(m_buf,1))
			{
				alarm(0);
				signal(SIGALRM,SigHandle);
				return -2; //time out
			}
			alarm(timeout);
		}

		char *pMsg =(char*)pBuf;
		int bytes_read = 0;

		int this_read;
		while( bytes_read < iLen )
		{
			do
			{
				this_read = read(m_sockfd, pMsg, iLen - bytes_read);
			}
			while ( ( this_read < 0 )  && ( errno == EINTR ) );

			if (this_read < 0)
			{
				return this_read;
			}
			else if(this_read == 0) {
				return bytes_read;
			}

			bytes_read += this_read;

			pMsg += this_read;
		}

		alarm(0);
		signal(SIGALRM,SigHandle);
		return iLen;
	}

	/*
	   循环读，超时依赖于scoket option 的recvie time out 选项
	   返回值: 实际读到的字节数
	 */
	int CTcpConnector::Read_opt_n(void *pMsg,const int iLen)
	{
		int nread = 0;
		int nleft = iLen;
		char *ptr = (char *)pMsg;

		if(iLen <= 0)
			return -1;
		while(nleft > 0)
		{
			if((nread = read(m_sockfd, ptr, nleft)) < 0)
			{
				if(errno == EINTR)
					//再次调用read
					nread = 0;
				else
					return -1;
			}
			else if(nread==0)
				break;
			nleft = nleft - nread;
			ptr = ptr + nread;
		}
		return(iLen - nleft);
	}



	int CTcpConnector::Write(const void *pBuf,int iLen)
	{
#if 0
		if(timeout !=-1) {
			SigHandle = signal(SIGALRM,&AlarmHandle);
			if(sigsetjmp(m_buf,1))
			{
				alarm(0);
				signal(SIGALRM,SigHandle);
				return -2; //time out
			}
			alarm(timeout);
		}
#endif

		return write(m_sockfd,pBuf,iLen);
	}

	/*
	   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	   用时钟来设置超时(注意这个函数已弃用,请先SetSendTimeOut再使用Write_opt_n ---againgan,allenlin 2008-12-18!!!!!!!)
	   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	 */
	int CTcpConnector::Write_n(const void *pBuf,int iLen,int timeout)
	{

		if(timeout !=-1) {
			SigHandle = signal(SIGALRM,&AlarmHandle);
			if(sigsetjmp(m_buf,1))
			{
				alarm(0);
				signal(SIGALRM,SigHandle);
				return -2; //time out
			}
			alarm(timeout);
		}

		int bytes_sent = 0;
		int this_write;
		const char* pMsg = (const char*)pBuf;
		while (bytes_sent < iLen)
		{
			do
			{
				this_write = write(m_sockfd, pMsg, iLen - bytes_sent);
			}
			while ( (this_write < 0) && ( errno == EINTR ));

			if( this_write <= 0 )
			{
				alarm(0);                   //add by allenlin -- 2008-12-18
				signal(SIGALRM,SigHandle);

				return this_write;
			}

			bytes_sent += this_write;

			pMsg += this_write;
		}

		alarm(0);
		signal(SIGALRM,SigHandle);

		return iLen;
	}

	/*
	   循环写，超时依赖于scoket option 的recvie time out 选项
	   返回值: 发送的字节长度
	 */
	int CTcpConnector:: Write_opt_n(const void *pMsg,const int iLen)
	{
		size_t nleft = 0;
		ssize_t nwritten = 0;
		const char *ptr = (char *)pMsg;

		nleft = iLen;
		while(nleft > 0)
		{
			if((nwritten = write(m_sockfd, ptr, nleft)) <= 0)
			{
				if(errno == EINTR)
				{
					//再次调用Write
					nwritten = 0;
				}
				else{
					return -1;
				}
			}
			nleft = nleft - nwritten;
			ptr = ptr + nwritten;
		}
		return iLen-nleft;
	}
}
