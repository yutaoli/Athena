/*
 * =====================================================================================
 * 
 *        Filename:  test_server_config.cpp
 * 
 *     Description:  
 * 
 *         Version:  1.0
 *         Created:  2015年05月01日 20时11分49秒 CST
 *        Revision:  none
 *        Compiler:  gcc
 * 
 *          Author:  yutaoli , yutaoli@tencent.com
 *         Company:  Shenzhen Tencent Co.Ltd.
 * 
 * =====================================================================================
 */

#include "server_config.h"
#include <iostream>
#include "commlog.h"

using namespace Athena;

int main(int argc, char **argv)
{
	CServerConf conf;
	
	conf.ParseFile("./test_server_config.ini");

	int tssd_clear = atoi(conf["tssd.clear"].c_str());
	int need_one_file_process_only_once = atoi(conf["process.need_one_file_process_only_once"].c_str());
	string log_name = conf["log.name"];

	printf("tssd_clear[%d]\n", tssd_clear);
	printf("need_one_file_process_only_once[%d]\n", need_one_file_process_only_once);
	printf("logname[%s]\n", log_name.c_str());


	ServerLogInit(log_name.c_str(), 64*1024, 10, 8);
	DEBUG_LOG("%s says: hello world", "yutaoli");

	return 0;
}
