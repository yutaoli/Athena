/***************************************************
  @file:PluginLoader.cpp
  @description:获取所有插件函数，后续让EventHandler调用
  @author:yutaoli
  @date:2014年12月28日 18:18:20


 ***************************************************/

#include "PluginLoader.h"
#include "Log.h"
#include <stdio.h>
#include <string>
#include<dlfcn.h>

svrframe_dll_fun_t svrframe_dll={NULL};

/*
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

 */

int load_plugin(const char *filename, int mode)
{
	DEBUG_LOG("filename[%s], mode[%d]", filename, mode);

	if(svrframe_dll.handle!=NULL)
	{
		dlclose(svrframe_dll.handle);
	}

	memset(&svrframe_dll, 0, sizeof(svrframe_dll));

	void *handle = dlopen(filename, mode);
	if(!handle)
	{
		ERROR_LOG("[ERROR]dlopen filename[%s] fail, mode[%d], errmsg[%s]\n",
				filename, mode, dlerror());

		exit(-1);
	}

	//reset errors
	dlerror();
	//load the symbol
	svrframe_dll.svrframe_check_complete = (svrframe_check_complete_t)dlsym(handle, "svrframe_check_complete");
	const char *dlsym_error = dlerror();//没有错误时返回NULL
	if(dlsym_error)
	{
		ERROR_LOG("[ERROR]svrframe_check_complete not implemented.\n");

		exit(-1);
	}

	//reset errors
	dlerror();
	svrframe_dll.svrframe_handle_process = (svrframe_handle_process_t)dlsym(handle, "svrframe_handle_process");
	dlsym_error = dlerror();
	if(dlsym_error)
	{
		ERROR_LOG("[ERROR]svrframe_handle_process not implemented.\n");

		exit(-1);
	}

	svrframe_dll.handle = handle;

	DEBUG_LOG("load so %s [done]", filename);

	return 0;
}



