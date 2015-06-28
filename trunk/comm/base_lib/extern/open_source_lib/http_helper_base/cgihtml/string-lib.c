/* string-lib.c - generic string processing routines
Eugene Kim, <eekim@eekim.com>
$Id: string-lib.c,v 1.8 1998/05/04 01:43:20 eekim Exp $

Copyright (C) 1996 Eugene Eric Kim
All Rights Reserved.
*/

#include "opt_stdio.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "string-lib.h"

char *newstr(char *str)
{
	char *tempstr;

	if ((tempstr = (char *)malloc(sizeof(char) * strlen(str) + 1)) == NULL)
		exit(1);

	if (tempstr != NULL)
		strncpy(tempstr,str,strlen(str)+1);
	return tempstr;
}

char *substr(char *str, int offset, int len)
{
	int slen, start, i;
	char *nstr;

	if (str == NULL)
		return NULL;
	else
		slen = strlen(str);
	if ((nstr = (char *)malloc(sizeof(char) * slen + 1)) == NULL)
		exit(1);
	if (offset >= 0)
		start = offset;
	else
		start = slen + offset - 1;
	if ( (start < 0) || (start > slen) ) /* invalid offset */
		return NULL;
	for (i = start; i < start+len; i++)
		nstr[i - start] = str[i];
	nstr[len] = '\0';
	return nstr;
}

char *replace_ltgt(char *str)
{
	unsigned int i,j = 0;
	char *newstring;


	if (str == NULL)
		return NULL;
	if ((newstring = (char *)malloc(sizeof(char) * (strlen(str) * 4 + 1))) == NULL)
		exit(1);
	for (i = 0; i < strlen(str); i++) {
		if (str[i] == '<') {
			newstring[j] = '&';
			newstring[j+1] = 'l';
			newstring[j+2] = 't';
			newstring[j+3] = ';';
			j += 3;
		}
		else if (str[i] == '>') {
			newstring[j] = '&';
			newstring[j+1] = 'g';
			newstring[j+2] = 't';
			newstring[j+3] = ';';
			j += 3;
		}
		else
			newstring[j] = str[i];
		j++;
	}
	newstring[j] = '\0';
	return newstring;
}

char *lower_case(char *buffer)
{
	char *tempstr = buffer;

	while (*buffer != '\0') {
		if (isupper(*buffer))
			*buffer = tolower(*buffer);
		buffer++;
	}
	return tempstr;
}

