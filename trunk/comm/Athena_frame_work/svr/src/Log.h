#ifndef _LOG_H_
#define _LOG_H_

#ifdef TRACE_LOG
#undef TRACE_LOG
#endif
#define TRACE_LOG(args...) SvrFrame::WriteLog(getpid(), __FILE__, __LINE__, SvrFrame::LV_TRACE, args)

#ifdef DEBUG_LOG
#undef DEBUG_LOG
#endif
#define DEBUG_LOG(args...) SvrFrame::WriteLog(getpid(), __FILE__, __LINE__, SvrFrame::LV_DEBUG, args)

#ifdef ERROR_LOG
#undef ERROR_LOG
#endif
#define ERROR_LOG(args...) SvrFrame::WriteLog(getpid(), __FILE__, __LINE__, SvrFrame::LV_ERROR, args)

#define FILE_LOG(args...) SvrFrame::WriteLog3(args)

#define ERROR_LOG_REF(fmt,args...) ERROR_LOG("refer [%s], "fmt, m_Params["refer"].c_str(), args)

#define DEBUG_LOG2(args...) SvrFrame::WriteLog2( __FILE__, __LINE__, LV_DEBUG, args)

#define WRITE_BIN SvrFrame::WriteHex


namespace SvrFrame
{

/** @note: 多线程时，size会有大概(thread_num * (40-100))的误差 **/
#define MAX_LOG_SIZE  64*1024*1024		//最大一个文件64M
#define MAX_LOG_NUM   100

#define MAX_LOG_LINE_SIZE	4096	//一条LOG最多4096

//20080829为了兼容CGI中旧有宏的调用方式
#define _INIT_LOG(filename,LOG_PRIORITY)  \
{\
	LogInit(filename, MAX_LOG_SIZE, MAX_LOG_NUM, LOG_PRIORITY); \
}
#define INIT_SERVER_LOG ServerLogInit

enum
{
	LV_ERROR=1,
	LV_WARN=2,
	LV_NOTICE=3,
	LV_INFO=4,
	LV_DEBUG=5,
	LV_TRACE=6
};

int GetLocalLogLevel();

void WriteHex (const char* file_name, const char* buf, int length);
void WriteLog (unsigned long long pid, const char* src_name, 
    unsigned int line, unsigned char level, const char* fmt, ...)__attribute((format(printf, 5, 6)));

void WriteLog_mt (unsigned long long pid, const char * msgptr, const char* src_name, unsigned int line, unsigned char level, const char* fmt, ...)__attribute((format(printf, 6, 7)));

//avoid getpid() when not need to print out
void WriteLog2(const char* src_name, unsigned int line, 
    unsigned char level, const char* fmt, ...)__attribute((format(printf, 4, 5)));

//最小化的log，节省空间
void WriteLog3(const char* fmt, ...)__attribute((format(printf, 1, 2)));

/**
 * @target: 普通程序（bin程序、CGI等）LOG的初始化
 * @note:	只写一个LOG文件，最大为 MAX_LOG_SIZE，循环写
 **/
int LogInit(const char *file, unsigned int  logsize = MAX_LOG_SIZE, unsigned int lognum  = MAX_LOG_NUM, unsigned int priority = LV_TRACE);

/**
 * @target: Server LOG的初始化
 * @note:	当一个LOG文件达到MAX_LOG_SIZE时，将其移成*.log.1...，依次类推至MAX_LOG_NUM
 **/
int ServerLogInit(const char *file, unsigned int  logsize = MAX_LOG_SIZE, unsigned int lognum  = MAX_LOG_NUM, unsigned int priority = LV_TRACE);

};//end of namespace SvrFrame

#endif //end of _LOG_H_
