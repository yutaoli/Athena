#ifndef _UTF8_CONVERT
#define _UTF8_CONVERT
#include "data_code.h"
#include "stdio.h"
#include <iostream>
#include <string>
#include <sys/time.h>
#include <unistd.h>
#include "iconv.h"

using namespace std;

unsigned char UCS2toUTF8Code(unsigned short ucs2_code, unsigned char* utf8_code);

int API_Gbk2Utf8(const char *szSource, string &strDest);
int API_Gbk2Utf8_ICONV(const char *szSource, string &strDest);
int API_Gbk2Utf8(const char *szSource, char *strDest, int iDestLen);
int API_Gbk2Utf8_ICONV(const char *szSource, char *strDest, int iDestLen);

int API_Utf8ToGbk(const char * rsIn, int rsInLength, char *rsOut, int &rsOutLength);
int API_Utf8ToGbk(const char* src, string & dest);

#endif
