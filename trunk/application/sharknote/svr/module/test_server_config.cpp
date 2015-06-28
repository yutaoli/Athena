/*
 * =====================================================================================
 * 
 *        Filename:  test_server_config.cpp
 * 
 *     Description:  
 * 
 *         Version:  1.0
 *         Created:  2015年05月03日 19时47分16秒 CST
 *        Revision:  none
 *        Compiler:  gcc
 * 
 *          Author:  yutaoli (), yutaoli@tencent.com
 *         Company:  Shenzhen Tencent Co.Ltd.
 * 
 * =====================================================================================
 */


#include "server_config.h"
#include "commlog.h"
#include <iostream>

using namespace std;
using namespace Athena;

/*
[tssd]
clear=1

[protocol]
uininfo_filed_count=4

[process]
need_proc_run=1
proc_num=200
record_count_per_save=130

need_pre_process=1
need_processing=0
need_post_process=0

[log]
name=../log/update_server
*/

int main()
{
	CServerConf conf("../conf/server.ini");

	int tssd_clear = atoi(conf["tssd.clear"].c_str());
    int need_one_file_process_only_once = atoi(conf["process.need_one_file_process_only_once"].c_str());
    string log_name = conf["log.name"];
    const int NEED_PRE_PROCESS = atoi(conf["process.need_pre_process"].c_str());
	const int NEED_PROCESSING = atoi(conf["process.need_processing"].c_str());
	const int NEED_POST_PROCESS = atoi(conf["process.need_post_process"].c_str());

    printf("tssd_clear[%d]\n", tssd_clear);
    printf("need_one_file_process_only_once[%d]\n", need_one_file_process_only_once);
    printf("logname[%s]\n", log_name.c_str());
    printf("need_preprocess[%d]\n", NEED_PRE_PROCESS);
    printf("need_processing[%d]\n", NEED_PROCESSING);
    printf("need_postprocess[%d]\n", NEED_POST_PROCESS);


    ServerLogInit(log_name.c_str(), 64*1024, 10, 8); 
    DEBUG_LOG("%s says: hello world", "yutaoli");



	//测试开关用法
	if(NEED_PRE_PROCESS)
	{
		printf("NEED_PRE_PROCESS open\n");
		//add preprocess logic here
	}

	if(NEED_PROCESSING)
	{
		printf("NEED_PROCESSING open\n");
		//add processing logic here
	}

	if(NEED_POST_PROCESS)
	{
		printf("NEED_POST_PROCESS open\n");
		//add post process logic here
	}

	return 0;
}
