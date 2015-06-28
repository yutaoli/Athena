/*
 *	@File: data_code.h
 *	@Author: tangwenning
 *	@Date: 2001-07-30
 *	@Copyright:
 */

#ifndef _DATA_CODE_H
#define _DATA_CODE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <netinet/in.h> /* ntohl htonl */
#include <sys/socket.h>
#include <sys/time.h>
typedef char byte;

typedef struct _taggbk_unicode_table_st {
	int gbk;
	int unicode;
} gbk_unicode_table;


inline int unicode2gbk(char *p, char *arr);
int string_unicode2gbk(char *p, char *q, int *len);

inline int gbk2unicode(const char *p, char *arr);
int string_gbk2unicode(const char *p, char *q, int srcLen);
#endif
