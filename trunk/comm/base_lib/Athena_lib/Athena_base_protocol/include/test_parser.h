/********************************************************
  parser的buf内存管理策略：
1:自己new出来，释放由自己释放
2:外部由MapBuff传进来，释放由外部释放

---------------------
 *author:yutaoli
 *date:2014年11月24日 13:54:59

 ********************************************************/

#ifndef TEST_PARSER_H
#define TEST_PARSER_H

#include "test_protocol.h"
#include "Jce.h"
#include "commlog.h" //log做debug用，后期稳定后做成一个基础库，把log去掉，错误信息通过GetErrMsg()返回调用方

using namespace Athena;

class TestParser
{
	public:
		TestParser():
			m_buf(NULL), 
			m_buf_len(0),
			m_is_map_buf(false),
			m_header()
		{
			memset(m_errmsg, 0, sizeof(m_errmsg));
		}

		~TestParser()
		{
			if(!m_is_map_buf && m_buf!=NULL)
			{
				delete []m_buf;
				m_buf = NULL;
			}
		}

		inline TEST_HEAD &GetHeaderRef(){return m_header;}

		/*
		   >0 返回完整数据包大小
		   =0 继续收
		   <0 出错
		 */
		static int CheckAsyComplete(const char *buf, const int buf_len)
		{
			TEST_HEAD *header = (TEST_HEAD*)buf;
			if(buf_len < header->GetHeadLen())
			{
				return 0;
			}

			if(header->GetPackLen() < header->GetHeadLen())
			{
				return -1;
			}

			//已收完整
			if(header->GetPackLen() <= buf_len)
			{
				return header->GetPackLen();
			}	

			if(buf_len < header->GetPackLen())
			{
				return 0;
			}

			return -2;
		}

		void MapBuffer(char *buf, const int buf_len)
		{
			if(m_is_map_buf)
			{
				m_buf = buf;
				m_buf_len = buf_len;
			}	
			else
			{
				if(m_buf != NULL)
				{
					delete []m_buf;
					m_buf = NULL;
				}
				m_buf = buf;
				m_buf_len = buf_len;
				m_is_map_buf = true;
			}
		}

		template<typename T>
			int Encode(const T& objSrc, unsigned int trylen, char * &out_buf, unsigned int &out_buf_len)
			{
				for(int try_time = 1; try_time < 4; try_time++)
				{
					trylen *= try_time;
					unsigned int packlen = m_header.GetHeadLen() + trylen;

					DEBUG_LOG("try_time[%d], m_encode_buf[%p], m_encode_buf_len[%d], header_len[%d], m_is_map_buf[%d]",
							try_time, m_buf, m_buf_len, m_header.GetHeadLen(), m_is_map_buf);

					//获取打包用的buf
					if(NULL == m_buf || m_buf_len < packlen)
					{
						if(m_is_map_buf)//map buf not enough or m_buff == NULL
						{
							if(NULL==m_buf)
							{
								ERROR_LOG("NULL==m_buf ,but m_is_map_buf, [FATAL error: impossible situation]");
								snprintf(m_errmsg, sizeof(m_errmsg), "NULL==m_buf ,but m_is_map_buf, [FATAL error: impossible situation]");

								return -1;
							}
							else if(m_buf_len < packlen)
							{
								ERROR_LOG("map buf not enough, map buf address[%p], map buf len[%d], packlen[%d]", 
										m_buf, m_buf_len, packlen);
								snprintf(m_errmsg, sizeof(m_errmsg), "map buf not enough, map buf address[%p], map buf len[%d], packlen[%d]", 
										m_buf, m_buf_len, packlen);

								return -2;
							}
						}
						if(m_buf!=NULL)
						{
							delete []m_buf;
							m_buf = NULL;
						}

						m_buf = new(std::nothrow) char[packlen];
						if(NULL == m_buf)
						{
							ERROR_LOG("system new opeation error");		
							snprintf(m_errmsg, sizeof(m_errmsg), "system new opeation error");		

							return -3;
						}

						m_buf_len = packlen;

						if(NULL== m_buf)
						{
							DEBUG_LOG("first new buf [done], m_encode_buf address[%p], m_encode_buf_len[%d]",
									m_buf, m_buf_len);
						}
						else
						{
							DEBUG_LOG("renew buf [done], m_encode_buf address[%p], m_encode_buf_len[%d]", 
									m_buf, m_buf_len);
						}
					}

					//包头
					int icurr_pos = 0;
					TEST_HEAD *header = (TEST_HEAD*)(m_buf + icurr_pos);
					memcpy(header, (TEST_HEAD*)&m_header, sizeof(m_header));

					icurr_pos = m_header.GetHeadLen();

					DEBUG_LOG("encode header [done], header_len[%d]", icurr_pos);

					//包体
					try
					{
						taf::JceOutputStream<taf::BufferWriterBuff> os;
						os.setBuffer(reinterpret_cast<char *>(m_buf + icurr_pos), m_buf_len - icurr_pos);
						objSrc.writeTo(os);
						icurr_pos += static_cast<int>(os.getLength());
					}
					catch (taf::JceNotEnoughBuff & e)
					{
						DEBUG_LOG("taf::JceNotEnoughBuff");

						continue;
					}
					catch (taf::JceDecodeMismatch & e)
					{
						ERROR_LOG("encode body error[taf::JceDecodeMismatch]");
						snprintf(m_errmsg, sizeof(m_errmsg), "encode body error[taf::JceDecodeMismatch]");

						return -201;
					}
					catch (taf::JceDecodeInvalidValue & e)
					{
						ERROR_LOG("encode body error[taf::JceDecodeInvalidValue]");
						snprintf(m_errmsg, sizeof(m_errmsg), "encode body error[taf::JceDecodeInvalidValue]");

						return -204;
					}
					catch (taf::JceDecodeRequireNotExist & e)
					{
						ERROR_LOG("encode body error[taf::JceDecodeRequireNotExist]");
						snprintf(m_errmsg, sizeof(m_errmsg), "encode body error[taf::JceDecodeRequireNotExist]");

						return -203;
					}
					catch (...)
					{
						ERROR_LOG("encode body error[FATAL error: impossible situation]");
						snprintf(m_errmsg, sizeof(m_errmsg), "encode body error[FATAL error: impossible situation]");

						return -204;
					}

					//设置数据包长度
					header->SetPackLen(icurr_pos);

					out_buf = m_buf;
					out_buf_len = icurr_pos;

					DEBUG_LOG("encode Head+Body[done] encoded_output_buf address[%p], len[%d]", 
							out_buf, out_buf_len);

					return 0;
				}

				ERROR_LOG("encode body error[FATAL error: impossible situation]");
				snprintf(m_errmsg, sizeof(m_errmsg), "encode body error[FATAL error: impossible situation]");

				return -4;
			}

		template<typename T>
			int Decode(const char *in_buf, unsigned int in_buf_len, T& objDest)
			{
				//包头
				TEST_HEAD *header = (TEST_HEAD*)in_buf;

				//包体
				try
				{
					taf::JceInputStream<taf::MapBufferReader> is;
					is.setBuffer(reinterpret_cast<const char*>(in_buf + header->GetHeadLen()), static_cast<size_t>(in_buf_len - header->GetHeadLen()));
					objDest.readFrom(is);
				}
				catch (taf::JceDecodeMismatch & e)
				{
					ERROR_LOG("decode body error[taf::JceDecodeMismatch]");
					snprintf(m_errmsg, sizeof(m_errmsg), "decode body error[taf::JceDecodeMismatch]");

					return -101;
				}
				catch (taf::JceDecodeInvalidValue & e)
				{
					ERROR_LOG("decode body error[taf::JceDecodeInvalidValue]");
					snprintf(m_errmsg, sizeof(m_errmsg), "decode body error[taf::JceDecodeInvalidValue]");

					return -104;
				}
				catch (taf::JceDecodeRequireNotExist & e)
				{
					ERROR_LOG("decode body error[taf::JceDecodeRequireNotExist]");
					snprintf(m_errmsg, sizeof(m_errmsg), "decode body error[taf::JceDecodeRequireNotExist]");

					return -103;
				}
				catch (...)
				{
					ERROR_LOG("decode body error[FATAL error: impossible situation]");
					snprintf(m_errmsg, sizeof(m_errmsg), "decode body error[FATAL error: impossible situation]");


					return -1;
				}

				return 0;
			}

		int DecodeHead(const char * buf, int buf_len)
		{
			if(buf_len < (int)sizeof(m_header))
			{
				ERROR_LOG("DecodeHead error, buf_len < header_len, buf_len[%d], header_len[%d]",
						buf_len, (int)sizeof(m_header));
				snprintf(m_errmsg, sizeof(m_errmsg), "DecodeHead error, buf_len < header_len, buf_len[%d], header_len[%d]",
						buf_len, (int)sizeof(m_header));

				return -1;
			}
			m_header = *(TEST_HEAD*)buf;

			DEBUG_LOG("DecodeHead [done], buf_len[%d], header_len[%d]", buf_len, (int)sizeof(m_header));

			return 0;
		}

		int GetRspCode()
		{
			return m_header._rsp_code;
		}

		char * GetErrMsg()
		{
			return m_errmsg;
		}

	private:
		char *m_buf;//空间可以是自己分配的，也可以是map外部传进来的buf，见函数MapBuffer()
		unsigned int m_buf_len;
		bool m_is_map_buf;
		TEST_HEAD m_header;

		char m_errmsg[1024];
};
#endif
