/*************************************************************/
/* The User Number Generate Lib                              */
/*                                                           */
/* Company: qq                                          */
/* Author: Kenix                                             */
/* Date: 2000/11/2                                           */
/* Version: 1.0a                                             */
/* Description:                                              */
/*                                                           */
/*************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#include "md5c.h"



int MD5(input, len, output)
unsigned char *input;
unsigned int len;
unsigned char output[16];
{
	MD5_CTX context;
	
	if(len%4 == 0)
	{
		MD5cInit(&context);
		MD5cUpdate(&context, input, len);
		MD5cFinal(output, &context);
		
		return 0;
	}
	else
	{
		return -1;
	}
}

