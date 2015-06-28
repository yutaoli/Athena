/******************************************************

  ------------------
 *author:yutaoli
 *date:2014年11月24日 19:56:37

 ******************************************************/
#ifndef TEST_TCPCLIENT_BASE_H
#define TEST_TCPCLIENT_BASE_H

#include "test_parser.h"
#include "tcpcli.h"
#include "commlog.h" //log做debug用，后期稳定后做成一个基础库，把log去掉，错误信息通过GetErrMsg()返回调用方

using namespace Athena;

class CTcpClientBase
{
	public:

		CTcpClientBase(string ip, unsigned short port):m_ip(ip), m_port(port), m_in_buf(NULL), m_in_buf_len(0)
		{
			memset(m_errmsg, 0, sizeof(m_errmsg));

			SetKeepAlive(false);
			m_is_conneted = false;

			m_conn_timeout_ms = 100;//100ms
			m_send_timeout_ms = 200;//200ms
			m_recv_timeout_ms = 300;//300ms
		}

		virtual ~CTcpClientBase()
		{
			if(m_in_buf!=NULL)
			{
				delete []m_in_buf;
				m_in_buf = NULL;
			}
		}

		void SetKeepAlive(bool is_keep_alive){m_is_keep_alive = is_keep_alive;}

		int Open(const Athena::CSocketAddr &sock_addr)
		{
			if(m_is_keep_alive)//长连接
			{
				if(m_is_conneted)
				{
					DEBUG_LOG("m_tcpclient.AsyncConnect [done], ip[%s], port[%d], keep_alive[%d]",
							m_ip.c_str(), m_port, m_is_keep_alive);

					return 0;
				}
				else
				{
					int ret = m_tcpclient.AsyncConnect(sock_addr, m_conn_timeout_ms);
					if(ret!=0)
					{
						ERROR_LOG("m_tcpclient.AsyncConnect error, ret[%d], ip[%s], port[%d], conn_timeout[%dms], errmsg[%s]", ret, m_ip.c_str(), m_port, m_conn_timeout_ms, m_tcpclient.GetErrMsg());
						snprintf(m_errmsg, sizeof(m_errmsg), "m_tcpclient.AsyncConnect error, ret[%d], ip[%s], port[%d], conn_timeout[%dms], errmsg[%s]", ret, m_ip.c_str(), m_port, m_conn_timeout_ms, m_tcpclient.GetErrMsg());

						return ret;
					}
					m_is_conneted = true;

					DEBUG_LOG("m_tcpclient.AsyncConnect success, ip[%s], port[%d]", m_ip.c_str(), m_port);
				}
			}
			else
			{
				m_tcpclient.Close();
				int ret = m_tcpclient.AsyncConnect(sock_addr, m_conn_timeout_ms);
				if(ret!=0)
				{
					ERROR_LOG("m_tcpclient.AsyncConnect error, ret[%d], ip[%s], port[%d], conn_timeout[%dms], errmsg[%s]", ret, m_ip.c_str(), m_port, m_conn_timeout_ms, m_tcpclient.GetErrMsg());
					snprintf(m_errmsg, sizeof(m_errmsg), "m_tcpclient.AsyncConnect error, ret[%d], ip[%s], port[%d], conn_timeout[%dms], errmsg[%s]", ret, m_ip.c_str(), m_port, m_conn_timeout_ms, m_tcpclient.GetErrMsg());

					return ret;
				}

				DEBUG_LOG("m_tcpclient.AsyncConnect [done], ip[%s], port[%d], keep_alive[%d]", 
						m_ip.c_str(), (int)m_port, m_is_keep_alive);
			}

			return 0;
		}

		char * GetErrMsg(){return m_errmsg;}
		void SetCmd(int cmd){m_parser.GetHeaderRef().SetCmd(cmd);}
		//void SetHeader(int cmd, AUTH_INFO){}


		/*
		   功能:设置接收的超时时间
		   参数:stReq--请求包，stRsp--响应包，trylen--请求包的大小估算值，估算new出buffer的大小用
		   返回值: 成功0,失败!=0
		 */
		template<typename Req, typename Rsp>
			int SendAndRecv(const Req &stReq, Rsp& stRsp, unsigned int trylen)
			{
				//1.Encode Head+Body
				char *out_buf = NULL;
				unsigned int out_buf_len = 0;
				int ret = m_parser.Encode(stReq, trylen, out_buf, out_buf_len);
				if(ret!=0)
				{
					ERROR_LOG("m_parser.Encode error, ret[%d], errmsg[%s]", ret, m_parser.GetErrMsg());
					snprintf(m_errmsg, sizeof(m_errmsg), "m_parser.Encode error, ret[%d], errmsg[%s]", ret, m_parser.GetErrMsg());

					return ret;
				}

				DEBUG_LOG("1.encode Head+Body [done], encoded buf[%s], encoded buf address[%p], len[%d]", 
						string(out_buf, out_buf_len).c_str(), out_buf, out_buf_len);

				//2.Send
				Athena::CSocketAddr sock_addr(m_ip, m_port);
				ret = Open(sock_addr);
				if(ret !=0)
				{
					ERROR_LOG("open error ret[%d], ip[%s], port[%d], errmsg[%s]", ret, m_ip.c_str(), m_port, strerror(errno));
					snprintf(m_errmsg, sizeof(m_errmsg), "open error ret[%d], ip[%s], port[%d], errmsg[%s]", ret, m_ip.c_str(), m_port, strerror(errno));

					return ret;
				}

				m_tcpclient.SetSendTimeOut(0, m_send_timeout_ms * 1000);
				m_tcpclient.SetRecvTimeOut(0, m_recv_timeout_ms * 1000);

				ret = m_tcpclient.Write_opt_n(out_buf, out_buf_len);
				if(ret < 0)
				{
					ERROR_LOG("send to ip[%s], port[%d] error, ret[%d], errmsg[%s]", m_ip.c_str(), (int)m_port, ret, m_tcpclient.GetErrMsg());
					snprintf(m_errmsg, sizeof(m_errmsg), "send to ip[%s], port[%d] error, ret[%d], errmsg[%s]", m_ip.c_str(), (int)m_port, ret, m_tcpclient.GetErrMsg());

					return ret;
				}

				DEBUG_LOG("2.send request [done], send buf[%s], send buf address[%p], len[%d], ip[%s], port[%d]",
						string(out_buf, out_buf_len).c_str(), out_buf, ret, m_ip.c_str(), m_port);

				//3.Read Head
				int head_len= m_parser.GetHeaderRef().GetHeadLen();
				char *in_buf = new char[head_len];
				ret = m_tcpclient.Read_opt_n(in_buf, head_len);
				if(ret != m_parser.GetHeaderRef().GetHeadLen())
				{
					delete []in_buf;
					in_buf = NULL;

					ERROR_LOG("recv head error, header_len[%d], recv_len[%d], errmsg[%s]", m_parser.GetHeaderRef().GetHeadLen(), ret, m_tcpclient.GetErrMsg());
					snprintf(m_errmsg, sizeof(m_errmsg), "recv head error, header_len[%d], recv_len[%d], errmsg[%s]", m_parser.GetHeaderRef().GetHeadLen(), ret, m_tcpclient.GetErrMsg());

					return -1;
				}

				DEBUG_LOG("3.recv head [done]");

				//4.Decode Head, in order to Get Packlen
				ret = m_parser.DecodeHead(in_buf, head_len);
				if(ret !=0)
				{
					ERROR_LOG("m_parser.DecodeHead error, ret[%d], errmsg[%s]", ret, m_parser.GetErrMsg());
					snprintf(m_errmsg, sizeof(m_errmsg), "m_parser.DecodeHead error, ret[%d], errmsg[%s]", ret, m_parser.GetErrMsg());

					return ret;
				}

				unsigned int pack_len = m_parser.GetHeaderRef().GetPackLen();

				if(m_in_buf != NULL)
				{
					delete []m_in_buf;
					m_in_buf = NULL;
				}
				m_in_buf = new char[pack_len];
				m_in_buf_len = pack_len;

				memcpy(m_in_buf, in_buf, head_len);
				if(in_buf!=NULL)
				{
					delete []in_buf;
					in_buf = NULL;
				}

				DEBUG_LOG("4.decode head [done], head_len[%d], pack_len[%d]", head_len, pack_len);

				//5.Read Body
				ret = m_tcpclient.Read_opt_n(m_in_buf + head_len, pack_len - head_len);
				if(ret != int(pack_len - head_len))
				{
					ERROR_LOG("recv body error, body len[%d], recv len[%d], errmsg[%s]", pack_len-head_len, ret, m_tcpclient.GetErrMsg());
					snprintf(m_errmsg, sizeof(m_errmsg), "recv body error, body len[%d], recv len[%d], errmsg[%s]", pack_len-head_len, ret, m_tcpclient.GetErrMsg());

					return -2;
				}

				DEBUG_LOG("5.recv body [done], head_len[%d], body_len[%d], pack_len[%d]", head_len, pack_len - head_len, pack_len);

				//6.Decode Body
				ret = m_parser.Decode(m_in_buf, m_in_buf_len, stRsp);
				if(ret !=0)
				{
					ERROR_LOG("m_parser.Decode error, ret[%d], errmsg[%s]", ret, m_parser.GetErrMsg());
					snprintf(m_errmsg, sizeof(m_errmsg), "m_parser.Decode error, ret[%d], errmsg[%s]", ret, m_parser.GetErrMsg());

					return ret;
				}

				DEBUG_LOG("6.decode body [done], recv buf address[%p], recv buf len[%d]", m_in_buf, m_in_buf_len);

				return 0;
			}

	private:
		TestParser m_parser;
		Athena::CTcpConnector m_tcpclient;
		char m_errmsg[1024];

		bool m_is_keep_alive;
		bool m_is_conneted;
		string m_ip;
		unsigned short m_port;

		char *m_in_buf;
		unsigned int m_in_buf_len;

		unsigned int m_conn_timeout_ms;
		unsigned int m_send_timeout_ms;
		unsigned int m_recv_timeout_ms;
};

#endif
