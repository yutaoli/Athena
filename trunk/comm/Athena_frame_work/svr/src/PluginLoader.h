/***************************************************
  @file:PluginLoader.cpp
  @description:获取所有插件函数，后续让EventHandler调用
  @author:yutaoli
  @date:2014年12月28日 18:18:20


 ***************************************************/


#ifndef PLUGIN_LOADER_H
#define PLUGIN_LOADER_H

typedef int (*svrframe_check_complete_t)(unsigned , char* , int len);
typedef int (*svrframe_handle_process_t)(unsigned , void*);

typedef struct
{
	void *handle;
	svrframe_check_complete_t svrframe_check_complete;
	svrframe_handle_process_t svrframe_handle_process; 
}svrframe_dll_fun_t;

extern svrframe_dll_fun_t svrframe_dll;

int load_plugin(const char *filename, int mode);

#endif //end of PLUGIN_LOADER_H
