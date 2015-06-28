/*
 *  str.h
 *  ISDWebCharacter
 *
 *  Created by Xiao Xu on 09-7-13.
 *  Copyright 2009 Tencent.. All rights reserved.
 *
 */

#ifndef _ISDWEBCHARACTER_STR
#define _ISDWEBCHARACTER_STR

#include <string>
#include "common.h"

namespace ISDWebCharacter {

	struct escapeCStringDefineNode {
		size_t len;
		char ta[7];
	};
	
	/**
	 * 主要用于将用户数据封入字符串常量(主要是JSON封装使用)
	 * 由于javascript string 于 C/C++ string 有完全相同的转义关键字，因此称为CString
	 *     策略：
	 *         \        \\
	 *         "        \"
	 *         '        \'
	 *         (0x0A)   \n
	 *         (0x0D)   (丢弃)
	 *
	 */
	std::string escapeCString(const std::string& sourceStr, charsetCheck level = NO_CHECK);
	int escapeCString(std::string& resultStr, const std::string& sourceStr, charsetCheck level = NO_CHECK);
	int escapeCString(char * resultBuffer, const char * sourceStr, size_t resultBufferSize, charsetCheck level = NO_CHECK);
	/**
	 * 反向
	 *
	 */
	std::string unescapeCString(const std::string& sourceStr, charsetCheck level = NO_CHECK);
	int unescapeCString(std::string& resultStr, const std::string& sourceStr, charsetCheck level = NO_CHECK);
	int unescapeCString(char * resultBuffer, const char * sourceStr, size_t resultBufferSize, charsetCheck level = NO_CHECK);
	
	extern escapeCStringDefineNode CStringEscapeMap[128] ;
	
};

#endif
