#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string>
#include "Tcphelper.h"


/*
class CTcphelper
{
public:
	CTcphelper(){}
	virtual ~CTcphelper(){};

	static int Read_n(int count);
	static int Write_n(int count);
};
*/

/*
       循环读，超时依赖于scoket option 的recvie time out 选项
       返回值: 实际读到的字节数
     */
int CTcphelper::Read_n(int fd, void *buf, int count)
{
	char *ptr = (char*)buf;
	int nleft = count;	
	int nread = 0;

	if(count < 0) return -1;

	while(nleft>0)
	{
		nread = read(fd, ptr, nleft);	
		if(nread <0)//错误
		{
			//如果ET提示你有数据可读的时候，你应该连续的读一直读到返回 EAGAIN or EWOULDBLOCK 为止，即读取结束
			if(EAGAIN == errno || EWOULDBLOCK == errno)
			{
				break;
			}
			else if(EINTR == errno)
			{
				ERROR_LOG("CTcphelper::Read_n signal EINTR occur, fd[%d], read count[%d]", fd, count);
				continue;
			}
			
			return -1;
		}

		if(0==nread)//接受结束
		{
			break;
		}

		ptr += nread;
		nleft-=nread;
	}

	return count-nleft;
	
}

/*
       循环写，超时依赖于scoket option 的send time out 选项
       返回值: 实际写到的字节数
     */
int CTcphelper::Write_n(int fd, void *buf, int count)
{
	char *ptr = (char*)buf;
	int nwrite=0;
	int nleft = count;

	while(nleft>0)	
	{
		nwrite = write(fd, ptr, nleft);

		if(nwrite <0)
		{
			if(EAGAIN == errno || EWOULDBLOCK == errno)
			{
				break;
			}
			else if(EINTR == errno)
			{
				ERROR_LOG(" CTcphelper::write_n signal EINTR occur, fd[%d], write count[%d]", fd, count);
			}

			return -1;
		}
		else if(nwrite == 0)
		{
			break;
		}

		nleft -= nwrite;
		ptr += nwrite;
	}

	return count - nleft;
}


int CTcphelper::SetNonBlocking(int fd)
{
    int flags;

    /* Set the socket nonblocking.
     * Note that fcntl(2) for F_GETFL and F_SETFL can't be
     * interrupted by a signal. */
    if ((flags = fcntl(fd, F_GETFL)) == -1)
    {
        ERROR_LOG("fcntl(F_GETFL) error, errmsg[%s], fd[%d]", strerror(errno), fd);
        return -1;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        ERROR_LOG("fcntl(F_SETFL,O_NONBLOCK) error, errmsg[%s], fd[%d]", strerror(errno), fd);
        return -2;
    }

    return 0;
}
