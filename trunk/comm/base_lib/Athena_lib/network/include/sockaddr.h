#ifndef __SOCKET_ADDR
#define __SOCKET_ADDR

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>

namespace Athena{
	using namespace std;

	class CSocketAddr
	{
		public:
			CSocketAddr()
			{
				memset(&m_addr,0,sizeof(m_addr));
				m_addr.sin_family = AF_INET;
			}
			CSocketAddr( const string &host, int port )
			{
				memset(&m_addr,0,sizeof(m_addr));
				m_addr.sin_family = AF_INET;
				m_addr.sin_port = htons(port);
				m_addr.sin_addr = getNetIp(host);
			}
			CSocketAddr(const sockaddr_in &addr)
			{
				memset(&m_addr,0,sizeof(m_addr));
				m_addr = addr;
			}

			inline struct sockaddr_in getAddress(void) const
			{
				return m_addr;
			}
			struct in_addr getNetIp(const string& host)
			{
				struct in_addr stAddr;
				memset(&stAddr,0,sizeof(stAddr));
				if (inet_aton(host.c_str(), &stAddr)!=0)// valid
				{ 
					return stAddr;
				}

				// host is hostname.
				struct hostent  *pstHost;
				if((pstHost = gethostbyname(host.c_str()))==NULL)
				{
					return stAddr;
				}

				stAddr = *(struct in_addr*)pstHost->h_addr_list[0];
				return stAddr;
			}
		private:

			struct sockaddr_in m_addr;
	};
}




#endif  //__TCPSOCKETCONNECTOR__HH__

