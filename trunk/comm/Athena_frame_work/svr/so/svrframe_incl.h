/********************************************
  @file:svrframe_incl.h
  @description:插件要实现的api都放在这里
  @author:yutaoli
  @date:2014年12月28日 19:37:34

 ********************************************/

#ifndef SVRFRAME_INCL_H
#define SVRFRAME_INCL_H

extern "C" int svrframe_check_complete(unsigned flow, char* buf, int len);
extern "C" int svrframe_handle_process(unsigned flow, void* argv1);

#endif //end of SVRFRAME_INCL_H
