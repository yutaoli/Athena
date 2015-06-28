#ifndef _H_TCPHELPER_H_
#define _H_TCPHELPER_H_

#include "Log.h"

class CTcphelper
{
public:
	CTcphelper(){}
	virtual ~CTcphelper(){};

	static int Read_n(int fd, void *buf, int count);
	static int Write_n(int fd, void *buf, int count);
	static int SetNonBlocking(int fd);
};

#endif //_H_TCPHELPER_H_
