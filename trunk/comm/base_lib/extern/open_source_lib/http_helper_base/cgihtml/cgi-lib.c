/* cgi-lib.c - C routines that make writing CGI scripts in C a breeze
Eugene Kim, <eekim@eekim.com>
$Id: cgi-lib.c,v 1.17 1998/05/04 02:12:34 eekim Exp $
Motivation: Perl is a much more convenient language to use when
writing CGI scripts.  Unfortunately, it is also a larger drain on
the system.  Hopefully, these routines will make writing CGI
scripts just as easy in C.

Copyright (C) 1996,1997 Eugene Eric Kim
All Rights Reserved
*/
#ifndef _CGI_LIB_C_
#define _CGI_LIB_C_

#include "opt_stdio.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>

#ifdef WINDOWS
#include <io.h>
#endif

#include "cgi-lib.h"
#include "html-lib.h"
#include "string-lib.h"
#include "cgi-llist.h"  // for FREE define
/* symbol table for CGI encoding */
#define _NAME 0
#define _VALUE 1

static char *buffer_file = NULL;
static ExtNode_t *Exthead=NULL;

static int MYBF(const char *Text, int TextLen, const char* Pattern, const int len) //const 表示函数内部不会改变这个参数的值。
{
    if( NULL == Text || TextLen == 0|| NULL == Pattern|| len == 0 || TextLen < len)//
    {
        return -1;//空指针或空串，返回-1。
    }

    int i=0, j=0;
    for(i=0; i<TextLen; i++)
    {
        for(j=0; j<len; j++)
        {
            if(Text[i+j] == Pattern[j])
            {
                continue;
            }
            else
            {
                j=0;
                break;
            }
        }
        if (j != 0)
        {
            return i;
        }
    }
    return -1;
}

short accept_image()
{
	char *httpaccept = getenv("HTTP_ACCEPT");
	if (httpaccept == NULL)
		return 0;

	if (strstr(httpaccept,"image") == NULL)
		return 0;
	else
		return 1;
}

/* x2c() and unescape_url() stolen from NCSA code */
char x2c(char *what)
{
	register char digit;

	digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
	digit *= 16;
	digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));
	return(digit);
}

void unescape_url(char *url)
{
	register int x,y;

	for (x=0,y=0; url[y]; ++x,++y) {
		if(((url[x] = url[y]) == '%') && ((url[x+1] = url[y+1]) != '\0')) {
			url[x] = x2c(&url[y+1]);
			y+=2;
		}
	}
	url[x] = '\0';
}

char *get_DEBUG()
{
	int bufsize = 1024;
	char *buffer;
	int i = 0;
	char ch;

	if ((buffer = (char *)malloc(sizeof(char) * bufsize + 1)) == NULL)
		exit(1);

	fprintf(stderr,"\n--- cgihtml Interactive Mode ---\n");
	fprintf(stderr,"Enter CGI input string.  Remember to encode appropriate ");
	fprintf(stderr,"characters.\nPress ENTER when done:\n\n");
	while ( (i<=bufsize) && ((ch = getc(stdin)) != '\n') ) {
		buffer[i] = ch;
		i++;
		if (i>bufsize) {
			bufsize *= 2;
			if ((buffer = (char *)realloc(buffer,bufsize)) == NULL)
				exit(1);
		}
	}
	buffer[i] = '\0';
	fprintf(stderr,"\n Input string: %s\nString length: %d\n",buffer,i);
	fprintf(stderr,"--- end cgihtml Interactive Mode ---\n\n");
	return buffer;
}

char *get_POST()
{
	/* this is modify by nofeeling 20030127 */
	/*
	* not use u_int to check if the len less than 0,
	* if the len less than 0 * the cgi may be not to malloc enough buffer
	*/
	int content_length;
	char *buffer = NULL;

	if (CONTENT_LENGTH != NULL) {
		content_length = atoi(CONTENT_LENGTH);
		if (content_length < 0) {
			fprintf(stderr,"caught by cgihtml: input length < 0\n");
			exit(1);
		}
		if (content_length > MAX_CONTENT_LENGTH) {
			fprintf(stderr,"caught by cgihtml: input length > max_length\n");
			exit(1);
		}
		if ((buffer = (char *)malloc(sizeof(char) * content_length + 1)) == NULL)
			exit(1);

		int iRemain = content_length;
		int iReadN = 0;
		int iRet = 0;
		while(iRemain > 0)
		{
			iRet = read(fileno(stdin), buffer + iReadN, iRemain);
			if(iRet > 0)
			{
				iRemain -= iRet;
				iReadN += iRet;
			}
			else
			{
				break;
			}				
		}
		if(iReadN != content_length)
		{
			/* consistency error. */
			fprintf(stderr,"caught by cgihtml: input length < CONTENT_LENGTH\n");
			exit(1);
		}
		
		buffer[content_length] = '\0';
	}
	return buffer;
#if 0
	unsigned int content_length;
	char *buffer;

	if (CONTENT_LENGTH != NULL) {
		content_length = atoi(CONTENT_LENGTH);
		if ((buffer = (char *)malloc(sizeof(char) * content_length + 1)) == NULL)
			exit(1);
		if (fread(buffer,sizeof(char),content_length,stdin) != content_length) {
			/* consistency error. */
			fprintf(stderr,"caught by cgihtml: input length < CONTENT_LENGTH\n");
			exit(1);
		}
		buffer[content_length] = '\0';
	}
	return buffer;
#endif
	/* modify end */
}

char *get_GET()
{
	char *buffer;

	if (QUERY_STRING == NULL)
		return NULL;
	buffer = newstr(QUERY_STRING);
	return buffer;
}

int parse_CGI_encoded(llist *entries, char *buffer)
{
	int i, j, num, token;
	int len = strlen(buffer);
	char *lexeme;
	entrytype entry;
	node *window;

	if ((lexeme = (char *)malloc(sizeof(char) * len + 1)) == NULL)
		exit(1);

	list_create(entries);
	window = entries->head;
	entry.name = NULL;
	entry.value = NULL;
	i = 0;
	num = 0;
	token = _NAME;
	while (i < len) {
		j = 0;
		while ( (buffer[i] != '=') && (buffer[i] != '&') && (i < len) ) {
			lexeme[j] = (buffer[i] == '+') ? ' ' : buffer[i];
			i++;
			j++;
		}
		lexeme[j] = '\0';
		if (token == _NAME) {
			entry.name = newstr(lexeme);
			unescape_url(entry.name);
			if ( (buffer[i] != '=') || (i == len - 1) ) {
				if ((entry.value = (char *)malloc(sizeof(char))) == NULL)
					exit(1);
				entry.value[0]=0;
				window = list_insafter(entries, window, entry);
				FREE(entry.name);
				FREE(entry.value);
				if (i == len - 1) /* null value at end of expression */
					num++;
				else { /* error in expression */
					FREE(lexeme);
					return -1;
				}
			}
			else
				token = _VALUE;
		}
		else {
			entry.value = newstr(lexeme);
			unescape_url(entry.value);
			window = list_insafter(entries, window, entry);
			FREE(entry.name);
			FREE(entry.value);
			token = _NAME;
			num++;
		}
		i++;
		j = 0;
	}
	FREE(lexeme);
	FREE(entry.name);
	FREE(entry.value);
	return num;
}

/* stolen from k&r and seriously modified to do what I want */
// modified by ianyang, 20090608, getline()是系统函数, 修改为getline_cgilib()

int getline_cgilib(char s[], int lim)
{
	int c=0, i=0, num;

	for (i=0; (i<lim) && ((c=getchar())!=EOF) && (c!='\n'); i++) {
		s[i] = c;
	}
	if (c == '\n') {
		s[i] = c;
	}
	if ((i==0) && (c!='\n'))
		num = 0;
	else if (i == lim)
		num = i;
	else
		num = i+1;
	return num;
}

/* add by nofeeling 20030127 */
static int check_upload_file(char *_s_file_name)
{
	int i = 0, i_len; /* dir level */
	char *p, *p1, *s_file;
#ifdef WINDOWS
	char s_cut[2] = "\\";
#else
	char s_cut[2] = "/";
#endif

	if (_s_file_name == NULL)
		return -1;

	if (strlen(_s_file_name) == 0)
		return -1;

	s_file = newstr(_s_file_name);
	p1 = s_file;
	for(;;)
	{
		p = p1;
		if ((p1 = strstr(p, s_cut)) == NULL)
			break;
		p1[0] = 0x0;
		p1 ++;

		i_len = strlen(p);
		switch(i_len)
		{
		case 0:
			continue;
			break;
		case 1:
			if (p[0] == '.')
				continue;
			else
				i ++;
			break;
		case 2:
			if (p[0] == '.' && p[1] == '.')
				i--;
			else
				i++;
			if (i < 0) {
				FREE(s_file);
				return -1;
			}
			break;
		default:
			i++;
		}

	}

	FREE(s_file);

	return 0;
}
/* add end */

        
void cleanExtList()
{
    ExtNode_t *tmpnode = Exthead;
    while(NULL != tmpnode)
    {
        Exthead = Exthead->next;
        free(tmpnode);
        tmpnode = Exthead;
    }
    Exthead = NULL;

    if (NULL != buffer_file)
    {
        free(buffer_file);
        buffer_file = NULL;
    }
}

Extdata_t *getExtListData(char *name)
{
    if (NULL == name)return NULL;
    ExtNode_t *tmpnode = Exthead;
    while(NULL != tmpnode)
    {
        if (0 == strcmp(name, tmpnode->name))
        {
            return &(tmpnode->data);
        }
        tmpnode = tmpnode->next;
    }
    return NULL;
}

static void ParseNameValue(char *data, int data_len, llist* entries, node** window)
{
	entrytype entry;
    if (data_len < 2)
    {
        return;
    }
    data[data_len-2] = '\0'; //忽略内容最后的\r\n
    char *head_end = "\r\n\r\n";
    int head_end_len = strlen(head_end);
    char *endpos = strstr(data, head_end);
    if (NULL == endpos)
    {
        return;
    }

    endpos[0] = '\0'; //分开头和内容

    char *namestr = "name=\"";
    int namestrlen = strlen(namestr);
    char *namestrend = "\"";
    char *namestrpos = strstr(data, namestr);
    if (NULL == namestrpos) 
    {
        return;
    }
    char *namestrendpos = strstr(namestrpos+namestrlen, namestrend);
    if (NULL == namestrendpos) 
    {
        return;
    }

    char *filenamestr = "filename=\"";
    int filenamestrlen = strlen(filenamestr);
    char *filenamestrpos = strstr(data, filenamestr);
    if (NULL != filenamestrpos)
    {
        char *filenamestrendpos = strstr(filenamestrpos+filenamestrlen, namestrend);
        if (NULL == filenamestrendpos)
        {
            return;
        }
        namestrendpos[0] = '\0';
        entry.name  = namestrpos + namestrlen;
        
        filenamestrendpos[0] = '\0';
        entry.value = filenamestrpos + filenamestrlen;

        if (HTTP_USER_AGENT != NULL &&
            (strstr(lower_case(HTTP_USER_AGENT),"win") != 0) )
        { // windows下面去掉路径名
            char *tempstr = strrchr(entry.value, '\\');
            if (tempstr) {
                tempstr++;
                entry.value = tempstr;
            }
        }
		*window = list_insafter(entries,*window,entry);

        /* add by nofeeling 20030127 */
        /* check the filename to fix the cgi security */
        if (check_upload_file(entry.value) != 0) {
            /* jump over this file */
            return;
        }

        char *tmp = endpos+ head_end_len; 
        int totallen= data_len-(tmp-data)-2;

        if (NULL == sUploadFilePath)
        {
            ExtNode_t *tmpnode = (ExtNode_t*)malloc(sizeof(ExtNode_t));
            if (NULL == tmpnode)
            {
                return;
            }
            tmpnode->name = entry.name;
            tmpnode->data.data = tmp;
            tmpnode->data.len = totallen;
            tmpnode->next = Exthead;
            Exthead = tmpnode;
        }
        else
        {
	    int name_len=strlen(sUploadFilePath)+strlen(entry.name)+2;
            char filename[name_len];
            snprintf(filename,name_len,"%s_%s",sUploadFilePath,entry.name);
            FILE *uploadfile = NULL;
            if ( (uploadfile = fopen(filename,"w")) != NULL) 
            {
                int writelen = 0;
                int n = 0;

                while((n = fwrite(tmp+writelen, 1, totallen-writelen, uploadfile))>0)
                {
                    writelen += n;
                    if (writelen == totallen)
                        break;
                }
                fclose(uploadfile);
            }
        }
    }
    else
    {
        namestrendpos[0] = '\0';
        entry.name  = namestrpos + namestrlen;
        if (endpos+head_end_len-data == data_len)
        {
            entry.value = "";
        }
        else
        {
            entry.value =  endpos + head_end_len;
        }
		*window = list_insafter(entries,*window,entry);
    }

    return;
}

int parse_form_encoded(llist* entries)
{
	long content_length;
	node* window;
	char *boundary;
	int bytesread = 0;
	int numentries = 0;

	if (CONTENT_LENGTH != NULL)
		content_length = atol(CONTENT_LENGTH);
	else
		return 0;

	if (content_length < 0) 
    {
		return 0;
	}
	if (content_length > MAX_CONTENT_LENGTH) 
    {
		return 0;
	}

	if (CONTENT_TYPE == NULL)
    {
        return 0;
    }

	boundary = strstr(CONTENT_TYPE, "boundary=");
	if (boundary == NULL)
    {
        boundary = getenv("Qzone-Param-Boundary");
        if (NULL == boundary)
        {
            return 0;
        }
    }
	else
    {
		boundary += (sizeof(char) * 9);
    }
	/* create list */
	list_create(entries);
	window = entries->head;

    buffer_file = (char*)malloc(content_length*sizeof(char));
    if(NULL == buffer_file)
    {
        return 0;
    }

    int n = 0;
	//while ((n=fread(buffer_file+bytesread, sizeof(char),content_length-bytesread,stdin) )> 0)
	while ((n=read(fileno(stdin), buffer_file+bytesread, content_length-bytesread) )> 0)
    {
        bytesread += n;
    }

    if (bytesread != content_length)
    {
        return 0;
    }

    int boundlen = strlen(boundary)+2;

    char step[boundlen+1];
    step[0] = '-';
    step[1] = '-';
    memcpy(step+2, boundary, boundlen-2);
    step[boundlen] = '\0';

    int pos = MYBF(buffer_file, content_length, step, boundlen);
    if (-1 == pos)
    {
        return 0;
    }
    int oldpos = pos+boundlen+2;
    char *head_end = "\r\n\r\n";
    int head_end_len = strlen(head_end);
    int head_end_pos = 0;
    while(1)
    {   
        int isNewFormat = 0; //新的格式，在传文件的时候加上自定义参数length
        numentries++;
        head_end_pos = MYBF(buffer_file+oldpos,  content_length-oldpos, head_end, head_end_len);
        if (head_end_pos> 0)
        {
            char *tmp = buffer_file+oldpos;
            char temp = tmp[head_end_pos];
            tmp[head_end_pos] = '\0';
            if ((strstr(tmp, "filename=") != NULL) && (strstr(tmp, "name=") != NULL))
            {
               //int fd = open("/tmp/cgihtml.log", O_RDWR|O_APPEND, 0777);
               char *slen = strstr(tmp, "filelength=\"");
               int len = 0;
               if (NULL != slen)
               {
                   /*
                   if (-1 != fd)
                   {
                       char tt[1024];
                       snprintf(tt, 1024, "boundary file length=%s\n", slen);
                       write(fd, tt, strlen(tt));
                   }
                   */
                   len = atoi(slen+strlen("filelength=\""));
               }
               else 
               {
                   /*
                   if (-1 != fd)
                   {
                       char tt[1024];
                       snprintf(tt, 1024, "boundary file length=%s\n", "NULL");
                       write(fd, tt, strlen(tt));
                   }
                   */
                   slen = getenv("filelength");
                   if (NULL != slen)
                   {
                       /*
                       if (-1 != fd)
                       {
                           char tt[1024];
                           snprintf(tt, 1024, "env file length=%s\n", slen);
                           write(fd, tt, strlen(tt));
                       }
                       */
                       len = atoi(slen);
                   }
                   else
                   {
                       /*
                       if (-1 != fd)
                       {
                           char tt[1024];
                           snprintf(tt, 1024, "env file length=%s\n", "NULL");
                           write(fd, tt, strlen(tt));
                       }
                       */
                   }
               }
               if (len > 0 && len < content_length-oldpos-head_end_pos-head_end_len-boundlen-2)
               {
                   if (0 == memcmp(tmp+head_end_pos+head_end_len+len+2, step, boundlen))
                   {
                       isNewFormat = 1;
                       pos = head_end_pos+head_end_len+len+2;
                       /*
                       if (-1 != fd)
                       {
                           char tt[1024];
                           snprintf(tt, 1024, "new file format%s\n", "NULL");
                           write(fd, tt, strlen(tt));
                       }
                       */
                   }
               }
               //close(fd);
            }
            tmp[head_end_pos] = temp;
        }
        if (0 == isNewFormat)
        {
        pos = MYBF(buffer_file+oldpos,  content_length-oldpos, step, boundlen);
        if(-1 == pos)
        {
            int randlen = 2+boundlen+6; //const TCHAR c_szFormCompletionFormat[]  = _T("\r\n--%s--\r\n\r\n");
            if(oldpos+randlen< content_length)//兼容控件225最后一个为unicode分隔符
            {
                char *tmp = buffer_file + content_length - randlen;
                if (tmp[0] != '\r' || tmp[1] != 0 || tmp[2] != '\n' || tmp[3] != 0
                   || tmp[4] != '-' || tmp[5] != 0 || tmp[6] != '-' || tmp[7] != 0)
                {
                    break;
                }
                else
                {
                    pos = content_length-randlen-oldpos+2;
                    buffer_file[oldpos+pos-2]='\r';
                    buffer_file[oldpos+pos-1]='\n';
                    buffer_file[oldpos+pos+boundlen]='-';
                    buffer_file[oldpos+pos+boundlen+1]='-';
                }
            }
            else
            {
                break;
            }
        }
        }
        ParseNameValue(buffer_file+oldpos, pos, entries, &window);
        if (buffer_file[oldpos+pos+boundlen]=='-'&&buffer_file[oldpos+pos+boundlen+1]=='-')
        {
            break;
        }
        else if (buffer_file[oldpos+pos+boundlen]!='\r'||buffer_file[oldpos+pos+boundlen+1]!='\n')
        {
            break;
        }
        oldpos += pos+boundlen+2;
    }
	
	return numentries;
}

int read_cgi_input(llist* entries)
{
	char *input;
	int status;

	/* check for form upload.  this needs to be first, because the
	standard way of checking for POST data is inadequate.  If you
	are uploading a 100 MB file, you are unlikely to have a buffer
	in memory large enough to store the raw data for parsing.
	Instead, parse_form_encoded parses stdin directly.

	In the future, I may modify parse_CGI_encoded so that it also
	parses POST directly from stdin.  I'm undecided on this issue,
	because doing so will make parse_CGI_encoded less general. */
	if ((CONTENT_TYPE != NULL) &&
		(strstr(CONTENT_TYPE,"multipart/form-data") != NULL) )
		return parse_form_encoded(entries);

	/* get the input */
	if (REQUEST_METHOD == NULL)
		input = get_DEBUG();
	else if (!strcmp(REQUEST_METHOD,"POST"))
		input = get_POST();
	else if (!strcmp(REQUEST_METHOD,"GET"))
		input = get_GET();
	else { /* error: invalid request method */
		fprintf(stderr,"caught by cgihtml: REQUEST_METHOD invalid\n");
		exit(1);
	}
	/* parse the input */
	if (input == NULL)
		return 0;
	status = parse_CGI_encoded(entries,input);
	FREE(input);
	return status;
}

int read_file_upload(llist *entries, int maxfilesize)
{
	return parse_form_encoded(entries);
}

char *cgi_val(llist l, char *name)
{
	short FOUND = 0;
	node* window;

	window = l.head;
	while ( (window != 0) && (!FOUND) )
		if (!strcmp(window->entry.name,name))
			FOUND = 1;
		else
			window = window->next;
	if (FOUND)
		return window->entry.value;
	else
		return NULL;
}

/* cgi_val_multi - contributed by Mitch Garnaat <garnaat@wrc.xerox.com>;
modified by me */

char **cgi_val_multi(llist l, char *name)
{
	short FOUND = 0;
	node* window;
	char **ret_val = 0;
	int num_vals = 0, i;

	window = l.head;
	while (window != 0) {
		if (!strcmp(window->entry.name,name)) {
			FOUND = 1;
			num_vals++;
		}
		window = window->next;
	}
	if (FOUND) {
		/* copy the value pointers into the returned array */
		if ((ret_val = (char**) malloc(sizeof(char*) * (num_vals + 1))) == NULL)
			exit(1);
		window = l.head;
		i = 0;
		while (window != NULL) {
			if (!strcmp(window->entry.name,name)) {
				ret_val[i] = window->entry.value;
				i++;
			}
			window = window->next;
		}
		/* NULL terminate the array */
		ret_val[i] = 0;
		return ret_val;
	}
	else
		return NULL;
}

char *cgi_name(llist l, char *value)
{
	short FOUND = 0;
	node* window;

	window = l.head;
	while ( (window != 0) && (!FOUND) )
		if (!strcmp(window->entry.value,value))
			FOUND = 1;
		else
			window = window->next;
	if (FOUND)
		return window->entry.name;
	else
		return NULL;
}

char **cgi_name_multi(llist l, char *value)
{
	short FOUND = 0;
	node* window;
	char **ret_val = 0;
	int num_vals = 0, i;

	window = l.head;
	while (window != 0) {
		if (!strcmp(window->entry.value,value)) {
			FOUND = 1;
			num_vals++;
		}
		window = window->next;
	}
	if (FOUND) {
		/* copy the value pointers into the returned array */
		if ((ret_val = (char**) malloc(sizeof(char*) * (num_vals + 1))) == NULL)
			exit(1);
		window = l.head;
		i = 0;
		while (window != NULL) {
			if (!strcmp(window->entry.value,value)) {
				ret_val[i] = window->entry.name;
				i++;
			}
			window = window->next;
		}
		/* NULL terminate the array */
		ret_val[i] = 0;
		return ret_val;
	}
	else
		return NULL;
}

/* miscellaneous useful CGI routines */

int parse_cookies(llist *entries)
{
	char *cookies = getenv("HTTP_COOKIE");
	node* window;
	entrytype entry;
	int i,len;
	int j = 0;
	int numcookies = 0;
	short NM = 1;

	if (cookies == NULL)
		return 0;

	list_create(entries);
	window = entries->head;
	len = strlen(cookies);
	if ((entry.name = (char *)malloc(sizeof(char) * len + 1)) == NULL)
		exit(1);
	if ((entry.value = (char *)malloc(sizeof(char) * len + 1)) == NULL)
		exit(1);

	for (i = 0; i < len; i++)
	{
		if (cookies[i] == '=')
		{
			entry.name[j] = '\0';
			if (i == len - 1)
			{
				entry.value[0] = 0;
				window = list_insafter(entries,window,entry);
				numcookies++;
			}
			j = 0;
			NM = 0;
		}
#if 0  /*  delete by doudou 2006-04-25 */
		else if ( (cookies[i] == '&') || (i == len - 1) )
		{
			if (!NM)
			{
				if (i == len - 1)
				{
					entry.value[j] = cookies[i];
					j++;
				}
				entry.value[j] = '\0';
				window = list_insafter(entries,window,entry);
				numcookies++;
				j = 0;
				NM = 1;
			}
		}
#endif
		else if ( (cookies[i] == ';') || (i == len - 1) )
		{
			if (!NM) {
				if ( cookies[i] != ';' ) {
					entry.value[j] = cookies[i];
					j++;
				}
				entry.value[j] = '\0';
				window = list_insafter(entries,window,entry);
				numcookies++;
				i++;   /* erases trailing space */
				j = 0;
				NM = 1;
			}
		}
		else if (NM)
		{
			entry.name[j] = cookies[i];
			j++;
		}
		else if (!NM)
		{
			entry.value[j] = cookies[i];
			j++;
		}
	}
	FREE(entry.name);
	FREE(entry.value);
	return numcookies;
}

void print_cgi_env()
{
	if (SERVER_SOFTWARE != NULL)
		printf("<p>SERVER_SOFTWARE = %s<br>\n",SERVER_SOFTWARE);
	if (SERVER_NAME != NULL)
		printf("SERVER_NAME = %s<br>\n",SERVER_NAME);
	if (GATEWAY_INTERFACE !=NULL)
		printf("GATEWAY_INTERFACE = %s<br>\n",GATEWAY_INTERFACE);

	if (SERVER_PROTOCOL != NULL)
		printf("SERVER_PROTOCOL = %s<br>\n",SERVER_PROTOCOL);
	if (SERVER_PORT != NULL)
		printf("SERVER_PORT = %s<br>\n",SERVER_PORT);
	if (REQUEST_METHOD != NULL)
		printf("REQUEST_METHOD = %s<br>\n",REQUEST_METHOD);
	if (PATH_INFO != NULL)
		printf("PATH_INFO = %s<br>\n",PATH_INFO);
	if (PATH_TRANSLATED != NULL)
		printf("PATH_TRANSLATED = %s<br>\n",PATH_TRANSLATED);
	if (SCRIPT_NAME != NULL)
		printf("SCRIPT_NAME = %s<br>\n",SCRIPT_NAME);
	if (QUERY_STRING != NULL)
		printf("QUERY_STRING = %s<br>\n",QUERY_STRING);
	if (REMOTE_HOST != NULL)
		printf("REMOTE_HOST = %s<br>\n",REMOTE_HOST);
	if (REMOTE_ADDR != NULL)
		printf("REMOTE_ADDR = %s<br>\n",REMOTE_ADDR);
	if (AUTH_TYPE != NULL)
		printf("AUTH_TYPE = %s<br>\n",AUTH_TYPE);
	if (REMOTE_USER != NULL)
		printf("REMOTE_USER = %s<br>\n",REMOTE_USER);
	if (REMOTE_IDENT != NULL)
		printf("REMOTE_IDENT = %s<br>\n",REMOTE_IDENT);
	if (CONTENT_TYPE != NULL)
		printf("CONTENT_TYPE = %s<br>\n",CONTENT_TYPE);
	if (CONTENT_LENGTH != NULL)
		printf("CONTENT_LENGTH = %s<br></p>\n",CONTENT_LENGTH);

	if (HTTP_USER_AGENT != NULL)
		printf("HTTP_USER_AGENT = %s<br></p>\n",HTTP_USER_AGENT);
}

void print_entries(llist l)
{
	node* window;

	window = l.head;
	printf("<dl>\n");
	while (window != NULL) {
		printf("  <dt> <b>%s</b>\n",window->entry.name);
		printf("  <dd> %s\n",replace_ltgt(window->entry.value));
		window = window->next;
	}
	printf("</dl>\n");
}

char *escape_input(char *str)
/* takes string and escapes all metacharacters.  should be used before
including string in system() or similar call. */
{
	unsigned int i,j = 0;
	char *newstring;

	if ((newstring = (char *)malloc(sizeof(char) * (strlen(str) * 2 + 1))) == NULL)
		exit(1);

	for (i = 0; i < strlen(str); i++) {
		if (!( ((str[i] >= 'A') && (str[i] <= 'Z')) ||
			((str[i] >= 'a') && (str[i] <= 'z')) ||
			((str[i] >= '0') && (str[i] <= '9')) )) {
				newstring[j] = '\\';
				j++;
			}
			newstring[j] = str[i];
			j++;
	}
	newstring[j] = '\0';
	return newstring;
}

/* boolean functions */

short is_form_empty(llist l)
{
	node* window;
	short EMPTY = 1;

	window = l.head;
	while ( (window != NULL) && (EMPTY == 1) ) {
		if (strcmp(window->entry.value,""))
			EMPTY = 0;
		window = window->next;
	}
	return EMPTY;
}

short is_field_exists(llist l, char *str)
{
	if (cgi_val(l,str) == NULL)
		return 0;
	else
		return 1;
}

/* is_field_empty returns true either if the field exists but is empty
or if the field does not exist. */
short is_field_empty(llist l, char *str)
{
	char *temp = cgi_val(l,str);

	if ( (temp == NULL) || (!strcmp(temp,"")) )
		return 1;
	else
		return 0;
}

#endif

