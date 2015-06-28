/*
 */
#include "Log.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/file.h>

#include <pthread.h>

namespace SvrFrame
{

static char	szLogFile[64];		//log���ļ���
static bool m_NeedShift = false;	//LOG�ļ��Ƿ���Ҫ����������Ϊlog.1��log.2...����ʽ��һ������server��
static int g_uLogPriority = 0;//Ĭ������־
static char *g_sLogInfo[]=
{
	"ERROR", "WARN", "NOTICE", "INFO", "DEBUG", "TRACE"
};


int GetLocalLogLevel()
{
    return g_uLogPriority;
}


//080829 modify by againgan, ԭ�к�����Init���ڼ򵥣��׳�ͻ
int LogInit(const char *file, unsigned int  logsize , unsigned int lognum, unsigned int priority)
{
	if(file == NULL)
		return -1;
	if(logsize < 1 || lognum < 1) 
	{
		fprintf(stderr,"fail to init log.\n");
		return -1;
	}
	g_uLogPriority = priority;

	snprintf(szLogFile, sizeof(szLogFile), "%s.log", file);
	return 0;
}

int ServerLogInit(const char *file, unsigned int  logsize , unsigned int lognum, unsigned int priority)
{
	m_NeedShift = true;
	return LogInit(file, logsize, lognum, priority);
}

void WriteHex (const char* file_name, const char* buf, int length)
{
	FILE* fp;
	if(!(fp=fopen(szLogFile, "w+b")))
		return ;

	fwrite(buf, length, 1, fp);
	fclose(fp);	
}


int OpenLogFile()
{
	return open(szLogFile, O_WRONLY|O_APPEND|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH );
}

int ShiftFile(int *log_fd)
{
	struct stat file_stat;

	if( stat(szLogFile, &file_stat) == -1 || file_stat.st_size < MAX_LOG_SIZE)	
		//�����LOG�ļ��ȶ����LOG�ļ������ֵС���ѱ���������ɾ�����򻹿ɼ���д
		return 0;
	if( flock(*log_fd,LOCK_EX|LOCK_NB) == 0 )	//�����������
	{
		//	printf("[%d][%d] [size:%d] lock it \n",getpid(),pthread_self(),file_stat.st_size);
		if(m_NeedShift)
		{
//			if(stat(szLogFile, &file_stat) == -1 || file_stat.st_size < MAX_LOG_SIZE)
//				return 0;
			fsync(*log_fd);
			char szReLogName[70];
			for(int n=1; n<=MAX_LOG_NUM; n++) {
				snprintf(szReLogName, sizeof(szReLogName), "%s.%d", szLogFile, n);
				if( link(szLogFile, szReLogName) == 0 )
					break;
			}
		}
		if (0 != unlink(szLogFile))
        {
		    close(*log_fd);		//�ر��ļ��Զ�������
            *log_fd = -1;
            return -1; //ɾ��ʧ��ֱ�ӷ���
        }
		close(*log_fd);		//�ر��ļ��Զ�������
		//	printf("-[%d][%d] [size:%d] unlock it\n",getpid(),pthread_self(),file_stat.st_size);
		*log_fd = OpenLogFile();
	}
	else
	{
		//	printf("[%d][%d] [size:%d]locked..wait\n",getpid(),pthread_self(),file_stat.st_size);
		flock(*log_fd, LOCK_EX);	//����ʵ����һ������ʵ�ֵ�signal
		//	printf("[%d][%d] waited\n",getpid(),pthread_self());
		flock(*log_fd, LOCK_UN );
	}
	return *log_fd;
}

void WriteLog_mt(unsigned long long pid, const char* msgptr, const char* src_name, unsigned int line, unsigned char level, const char* fmt, ...)
{
	if(src_name == NULL) return;
	if(msgptr == NULL) return;

	if(level <= g_uLogPriority)
	{
		int log_fd = OpenLogFile();
		if(log_fd == -1)	//���ļ�ʧ��
		{
			//fprintf(stderr,"error opening log file.\n");
			return;
		}
		if( ShiftFile(&log_fd) == -1 )		//�������ƶ�LOG�ļ�ʧ�ܣ�Ҳֱ�ӷ���
		{
			if(log_fd != -1)
			{
				close(log_fd);
				//fprintf(stderr,"error shifting log file.\n");
			}
			return;
		}

		time_t now = time (NULL);
		struct tm tm;
		char log_buf [MAX_LOG_LINE_SIZE];
		localtime_r(&now, &tm);

		snprintf(log_buf, sizeof(log_buf), "%04d-%02d-%02d %02d:%02d:%02d|%s|%s,%-4d|%llu|%s|",
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, 
			tm.tm_sec, g_sLogInfo[level-1], src_name, line, pid, msgptr);

		unsigned int iOffset = strlen(log_buf);

		va_list ap;
		va_start(ap, fmt);
		vsnprintf(log_buf+iOffset, (sizeof(log_buf)-iOffset-1), fmt, ap);
		va_end(ap);

		iOffset = strlen(log_buf);
		if(iOffset < sizeof(log_buf)-1)
			log_buf[iOffset] = '\n';
		else
			log_buf[iOffset-1] = '\n';
		write(log_fd, log_buf, iOffset+1);

		close(log_fd);
	}
}

void WriteLog(unsigned long long pid, const char* src_name, unsigned int line, unsigned char level, const char* fmt, ...)
{
	if(src_name == NULL) return;

	if(level <= g_uLogPriority)
	{
		int log_fd = OpenLogFile();
		if(log_fd == -1)	//���ļ�ʧ��
		{
			//fprintf(stderr,"error opening log file.\n");
			return;
		}
		if( ShiftFile(&log_fd) == -1 )		//�������ƶ�LOG�ļ�ʧ�ܣ�Ҳֱ�ӷ���
		{
			if(log_fd != -1)
			{
				close(log_fd);
				//fprintf(stderr,"error shifting log file.\n");
			}
			return;
		}

		time_t now = time (NULL);
		struct tm tm;
		char log_buf [MAX_LOG_LINE_SIZE];
		localtime_r(&now, &tm);

		snprintf(log_buf, sizeof(log_buf), "%04d-%02d-%02d %02d:%02d:%02d|%s|%s,%-4d|%llu|",
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, 
			tm.tm_sec, g_sLogInfo[level-1], src_name, line, pid);

		unsigned int iOffset = strlen(log_buf);

		va_list ap;
		va_start(ap, fmt);
		vsnprintf(log_buf+iOffset, (sizeof(log_buf)-iOffset-1), fmt, ap);
		va_end(ap);

		iOffset = strlen(log_buf);
		if(iOffset < sizeof(log_buf)-1)
			log_buf[iOffset] = '\n';
		else
			log_buf[iOffset-1] = '\n';
		write(log_fd, log_buf, iOffset+1);

		close(log_fd);
	}
}

void WriteLog2(const char* src_name, unsigned int line, unsigned char level, const char* fmt, ...)
{
    if(src_name == NULL) return;

	if(level <= g_uLogPriority)
	{
        unsigned long long pid = getpid();
        
		int log_fd = OpenLogFile();
		if(log_fd == -1)	//���ļ�ʧ��
		{
			//fprintf(stderr,"error opening log file.\n");
			return;
		}
		if( ShiftFile(&log_fd) == -1 )		//�������ƶ�LOG�ļ�ʧ�ܣ�Ҳֱ�ӷ���
		{
			if(log_fd != -1)
			{
				close(log_fd);
				//fprintf(stderr,"error shifting log file.\n");
			}
			return;
		}

		time_t now = time (NULL);
		struct tm tm;
		char log_buf [MAX_LOG_LINE_SIZE];
		localtime_r(&now, &tm);

		snprintf(log_buf, sizeof(log_buf), "%04d-%02d-%02d %02d:%02d:%02d|%s|%s,%-4d|%llu|",
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, 
			tm.tm_sec, g_sLogInfo[level-1], src_name, line, pid);

		unsigned int iOffset = strlen(log_buf);

		va_list ap;
		va_start(ap, fmt);
		vsnprintf(log_buf+iOffset, (sizeof(log_buf)-iOffset-1), fmt, ap);
		va_end(ap);

		iOffset = strlen(log_buf);
		if(iOffset < sizeof(log_buf)-1)
			log_buf[iOffset] = '\n';
		else
			log_buf[iOffset-1] = '\n';
		write(log_fd, log_buf, iOffset+1);

		close(log_fd);
	}
}

void WriteLog3(const char* fmt, ...)
{
    int log_fd = OpenLogFile();
    if(log_fd == -1)	//���ļ�ʧ��
    {
        //fprintf(stderr,"error opening log file.\n");
        return;
    }
    if( ShiftFile(&log_fd) == -1 )		//�������ƶ�LOG�ļ�ʧ�ܣ�Ҳֱ�ӷ���
    {
        if(log_fd != -1)
        {
            close(log_fd);
            //fprintf(stderr,"error shifting log file.\n");
        }
        return;
    }

    char log_buf [MAX_LOG_LINE_SIZE];

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(log_buf, (sizeof(log_buf)-1), fmt, ap);
    va_end(ap);

    int iOffset = strlen(log_buf);
    if(iOffset < sizeof(log_buf)-1)
        log_buf[iOffset] = '\n';
    else
        log_buf[iOffset-1] = '\n';
    write(log_fd, log_buf, iOffset+1);

    close(log_fd);
}

};//end of namespace SvrFrame
