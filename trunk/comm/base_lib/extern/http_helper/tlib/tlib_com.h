#ifndef _TLIB_COM_H_
#define _TLIB_COM_H

//Uncomment this line under unix/linux
#define T_LIB_UNIX

#ifdef T_LIB_UNIX

#define O_BINARY		0
#define O_DENYNONE	0

#else

#include <io.h>

#endif

#define	TLIB_STR_ERROR_BASE		-1000
#define	TLIB_DBF_ERROR_BASE		-2000
#define	TLIB_INI_ERROR_BASE		-3000
#define	TLIB_MAIL_ERROR_BASE		-4000

#ifndef FASTCGI
#include <stdio.h>
#else
#include "fcgi_stdio.h" 
#endif

#endif

