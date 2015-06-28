/*
 *  common.h
 *  ISDWebCharacter
 *
 *  Created by Xiao Xu on 09-7-13.
 *  Copyright 2009 Tencent.. All rights reserved.
 *
 */

#ifndef _ISDWEBCHARACTER_COMMON
#define _ISDWEBCHARACTER_COMMON

#include <cstdio>

namespace ISDWebCharacter {
	/**
	 * 错误码定义
	 */
	enum errorCode {
		ERR_NULL_PARAMS = -100,
		ERR_BUFFER_TOO_SMALL = -99,
		ERR_ENCODE = -98,
		OK = 0
	};

	/**
	 * 字符集检查/容错开关参数
	 */
	enum charsetCheck {
		NO_CHECK = 0,
		UTF8_CHECK = 1,
		GBK_CHECK = 2
	};
	
	static inline void showVersion(void){
		fprintf(stderr, "ISDWebCharacter: 1.0.1\n");
	};

    inline bool isUnicodeControl(unsigned char p[3])
    {
        if(p[0] == 0xe2 && p[1] == 0x80)
        {
            if(p[2] == 0xa8 || p[2] == 0xa9 || p[2] == 0xae)
            {
                return true;
            }
        }

        return false;
    }
};

#endif
