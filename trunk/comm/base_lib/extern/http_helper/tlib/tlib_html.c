#ifndef _TLIB_HTML_C_
#define _TLIB_HTML_C_

#include "tlib_com.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "tlib_html.h"

#define TLIB_HTML_PMARK_COMMENT			"#"

#define TLIB_HTML_PMARK_BEGIN				"$"
#define TLIB_HTML_PMARK_END				"$"

#define TLIB_HTML_PMARK_BEGIN_FILLER	"<%"
#define TLIB_HTML_PMARK_END_FILLER		"%>"

#define TLIB_HTML_PART_BEGIN_FILLER	"<%!"
#define TLIB_HTML_PART_END_FILLER		"!%>"


char *TLib_Html_Load(char *sPath, char *sFileName)
{
	FILE *fp;
	char *pBuffer;
	long size, output_size;
	char html_file[200];

	strncpy(html_file, sPath,strlen(sPath)+1);
	strncat(html_file, sFileName,strlen(sFileName));

	if (html_file[0] != '/') {
		return NULL;
	}

	if ((fp = fopen(html_file, "r")) == NULL) {
		return NULL;
	}

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	pBuffer = malloc(size+1);
	fread(pBuffer, size, 1, fp);
	
	pBuffer[size] = 0;
	
	fclose(fp);
	return pBuffer;
}
//**********************************************************************************
//add  by hgt 2000-07-12
//**********************************************************************************
void TLib_Html_FillFile(FILE *fp, char *pBuffer, char *start_token, char *end_token)
{
	long size, output_size;
	char *temp, *p1, *p2;

	if (strlen(start_token) == 0) 
		p1 = pBuffer;
	else {
		p1 = strstr(pBuffer, start_token);
		if (p1 == NULL) {
			return;
		}
		p1 = p1 + strlen(start_token);
	}

	if (strlen(end_token) == 0) 
		p2 = pBuffer + strlen(pBuffer);
	else {
		p2 = strstr(pBuffer, end_token);
		if (p2 == NULL) {
			return;
		}
	}
	
	if (p1 >= p2) {
		return;
	}
	
   output_size = p2 - p1;
   temp = malloc(output_size+1);
   strncpy(temp, p1, output_size);
   temp[output_size] = '\0';
   fprintf(fp, "%s", temp);
   free(temp);
}

void TLib_Html_File(char *pBuffer, char *start_token, char *end_token)
{
	long size, output_size;
	char *temp, *p1, *p2;

	if (strlen(start_token) == 0) 
		p1 = pBuffer;
	else {
		p1 = strstr(pBuffer, start_token);
		if (p1 == NULL) {
			return;
		}
		p1 = p1 + strlen(start_token);
	}

	if (strlen(end_token) == 0) 
		p2 = pBuffer + strlen(pBuffer);
	else {
		p2 = strstr(pBuffer, end_token);
		if (p2 == NULL) {
			return;
		}
	}
	
	if (p1 >= p2) {
		return;
	}
	
   output_size = p2 - p1;
   temp = malloc(output_size+1);
   strncpy(temp, p1, output_size);
   temp[output_size] = '\0';
   printf("%s", temp);
   free(temp);
}

void TLib_Html_Free(char *pBuffer)
{
	free(pBuffer);
}

////////////////////////////////////////////////////////////////////////////////////

char *TLib_Html_GetTemplate(char *sBuffer, char *sMarkBegin, char *sMarkEnd)
{
	long lSize;
	char *sTemplate, *pcBegin, *pcEnd;

	if (strlen(sMarkBegin) == 0)
	{
		pcBegin = sBuffer;
	}
	else
	{
		pcBegin = strstr(sBuffer, sMarkBegin);
		if (pcBegin == NULL)
		{
			return NULL;
		}
		pcBegin = pcBegin + strlen(sMarkBegin);
	}

	if (strlen(sMarkEnd) == 0)
	{
		pcEnd = sBuffer + strlen(sBuffer);
	}
	else
	{
		//pcEnd = strstr(sBuffer, sMarkEnd);
		pcEnd = strstr(pcBegin, sMarkEnd);
		if (pcEnd == NULL)
		{
			return NULL;
		}
	}
	
	if (pcBegin >= pcEnd)
	{
		return NULL;
	}
	
   lSize = pcEnd - pcBegin;
   sTemplate = malloc(lSize+1);
   strncpy(sTemplate, pcBegin, lSize);
   sTemplate[lSize] = '\0';
   return sTemplate;
}

char *TLib_Html_TextEncode2(char *sDest, char *sSrc) //有处理回车符
{
	char *pcDest, *pcSrc;
	char *p;
	
	p = (char*)malloc(strlen(sSrc) + 1);
	strncpy(p,sSrc,strlen(sSrc)+1);
	
	pcDest = sDest;
	*pcDest = 0;
	pcSrc = p;
	while(1)
	{
		if (*pcSrc == 0)
		{
			break;
		}
		else if (*pcSrc == '\n')
		{
			strncpy(pcDest, "<br>",5);
		}
		else if (*pcSrc == '<')
		{
			strncpy(pcDest, "&lt;",5);
		}
		else if (*pcSrc == '>')
		{
			strncpy(pcDest, "&gt;",5);
		}
		else if (*pcSrc == '&')
		{
			strncpy(pcDest, "&amp;",6);
		}
		else if (*pcSrc == ' ')
    	{
	      if (*(pcSrc+1) == ' ')
			{
				strncpy(pcDest, "&nbsp;",7);
			}
			else
			{
				if (pcSrc == p || *(pcSrc-1) == '\n')
					strncpy(pcDest, "&nbsp;",7);
				else
					strncpy(pcDest, " ",2);
			}
		}
		else
		{
			*pcDest = *pcSrc;
			pcDest ++;
			*pcDest = 0;
		}
		pcDest = pcDest + strlen(pcDest);
		pcSrc ++;
	}
	
	free(p);
	return sDest;
}

char *TLib_Html_TextEncode3(char *sDest, char *sSrc) //回车替换为空格,过滤ｈｔｍｌ标记
{
	char *pcDest, *pcSrc;
	char *p;
	
	p = (char*)malloc(strlen(sSrc) + 1);
	strncpy(p,sSrc,strlen(sSrc)+1);
	
	pcDest = sDest;
	*pcDest = 0;
	pcSrc = p;
	while(1)
	{
		if (*pcSrc == 0)
		{
			break;
		}
		else if (*pcSrc == '\n')
		{
			strncpy(pcDest, " ",2);
		}
		else if (*pcSrc == '<')
		{
			strncpy(pcDest, "&lt;",5);
		}
		else if (*pcSrc == '>')
		{
			strncpy(pcDest, "&gt;",5);
		}
		else if (*pcSrc == '&')
		{
			strncpy(pcDest, "&amp;",6);
		}
		else
		{
			*pcDest = *pcSrc;
			pcDest ++;
			*pcDest = 0;
		}
		pcDest = pcDest + strlen(pcDest);
		pcSrc ++;
	}
	
	free(p);
	return sDest;
}


char *TLib_Html_TextEncode(char *sDest, char *sSrc) //不处理回车符
{
	char *pcDest, *pcSrc;
	
	char *p;
	
	p = (char*)malloc(strlen(sSrc) + 1);
	strncpy(p,sSrc,strlen(sSrc)+1);
	
	pcDest = sDest;
	*pcDest = 0;
	pcSrc = p;
	
	while(1)
	{
		if (*pcSrc == 0)
		{
			break;
		}
		else if (*pcSrc == '<')
		{
			strncpy(pcDest, "&lt;",5);
		}
		else if (*pcSrc == '>')
		{
			strncpy(pcDest, "&gt;",5);
		}
		else if (*pcSrc == '&')
		{
			strncpy(pcDest, "&amp;",6);
		}
		else if (*pcSrc == ' ')
   	{
	    if (*(pcSrc+1) == ' ' && *(pcSrc+9) != ' ')
			{
				strncpy(pcDest, "&nbsp;",7);
			}
			else
			{
				if (pcSrc == p || *(pcSrc-1) == '\n')
					strncpy(pcDest, "&nbsp;",7);
				else
					strncpy(pcDest, " ",2);
			}
		}
		else
		{
			*pcDest = *pcSrc;
			pcDest ++;
			*pcDest = 0;
		}
		pcDest = pcDest + strlen(pcDest);
		pcSrc ++;
	}
	
	free(p);
	return sDest;
}

char *TLib_Html_TextEncode4(char *sDest, char *sSrc) //不处理回车符，处理双引号
{
	char *pcDest, *pcSrc;
	
	char *p;
	
	p = (char*)malloc(strlen(sSrc) + 1);
	strncpy(p,sSrc,strlen(sSrc)+1);
	
	pcDest = sDest;
	*pcDest = 0;
	pcSrc = p;
	
	while(1)
	{
		if (*pcSrc == 0)
		{
			break;
		}
		else if (*pcSrc == '<')
		{
			strncpy(pcDest, "&lt;", 5);
		}
		else if (*pcSrc == '>')
		{
			strncpy(pcDest, "&gt;",5);
		}
		else if (*pcSrc == '&')
		{
			strncpy(pcDest, "&amp;",6);
		}
		else if (*pcSrc == '\"')
		{
			strncpy(pcDest, "&quot;",7);
		}
		else if (*pcSrc == ' ')
   	{
	    if (*(pcSrc+1) == ' ' && *(pcSrc+9) != ' ')
			{
				strncpy(pcDest, "&nbsp;",7);
			}
			else
			{
				if (pcSrc == p || *(pcSrc-1) == '\n')
					strncpy(pcDest, "&nbsp;",7);
				else
					strncpy(pcDest, " ",2);
			}
		}
		else
		{
			*pcDest = *pcSrc;
			pcDest ++;
			*pcDest = 0;
		}
		pcDest = pcDest + strlen(pcDest);
		pcSrc ++;
	}
	
	free(p);
	return sDest;
}


char *TLib_Html_UrlEncode(char *sDest, char *sSrc)
{
	int i;

	sDest[0] = '\0';
	for (i = 0; i < strlen(sSrc); i++) {
		switch (sSrc[i]) {
			case '&': strncat(sDest, "%26",3); break;
			case '=': strncat(sDest, "%3D",3); break;
			case '%': strncat(sDest, "%25",3); break;
			case '+': strncat(sDest, "%2B",3); break;
			case '?': strncat(sDest, "%3F",3); break;
			case ' ': strncat(sDest, "+",1);   break;
			case ':': strncat(sDest,"%3A",3);	break;
			case '/': strncat(sDest,"%2F",3); 	break;
			default: snprintf(sDest,1024, "%s%c", sDest, sSrc[i]);
		}
	}
	return sDest;
}

void TLib_Html_Redirect(char *sURL)
{
/*	
	HTTP/1.1 302 Object moved
	Server: Microsoft-IIS/4.0
	Date: Sun, 17 Oct 1999 09:23:21 GMT
	Location: http://www.oicq.com
	Content-Type: text/html
	Set-Cookie: ASPSESSIONIDQGGQGQAZ=OCHBOKFBHPFEPNJDJDOJDEBM; path=/
	Cache-control: private
*/

	//改用直接输出ＪＡＶＡ脚本刷新，因为下面原来的方法用后会导致ＣＧＩ执行不正常。 
	//注意被刷新的页面应避免被CACHE，如在页头加入<meta http-equiv="pragma" content="no-cache"> (Agong)
	//在所导向的URL后面加上一个按时间生成的唯一参数， 形成不同的URL，避免浏览器读CACHE。 （AGONG 2000-1-17）
	
	time_t ltime;
	char url[2048];
	
	time(&ltime);
	
	strncpy(url, sURL, 500);
	
	if (strstr(url, "?") == NULL)
		strncat(url, "?",1);
	else
		strncat(url, "&",1);

	printf("<script language=javascript>\n");
	printf("window.location.href='%sPcacheTime=%d';\n", url, ltime);
	printf("</script>\n");
}

void TLib_Html_Location(char *sURL)
{
	time_t ltime;
	char url[2048];
	
	time(&ltime);
	strncpy(url, sURL,strlen(sURL)+1);
	
	if (strstr(url, "?") == NULL)
		strncat(url, "?",1);
	else
		strncat(url, "&",1);

	printf("Location: %sCacheTime=%ld", url, ltime);
	printf("\nCache-control: private");
	printf("\n\n\n");
}

int TLib_Html_TextArea(char *sFieldName, char *sValue, int iRow, int iCol)
{
	printf("<textarea name='%s' rows='%d' cols='%d'>%s</textarea>\n", sFieldName, iRow, iCol, sValue);
}

int TLib_Html_TextBox(char *sFieldName, char *sValue)
{
	printf("<input type='text' size='30' name='%s' value='%s'>\n", sFieldName, sValue);
}

int TLib_Html_Password(char *sFieldName, char *sValue)
{
	printf("<input type='password' size='30' name='%s' value='%s'>\n", sFieldName, sValue);
}

static char *GetToken(char *sToken, char *sSrc, char cFilter)
{
	char *pb, *pe;
	int i;

	pb = sSrc;
	pe = strchr(pb, cFilter);
	if (pe == NULL)
	{
		strncpy(sToken, pb,strlen(pb)+1);
		return sSrc + strlen(sSrc);
	}
	else
	{
		i = 0;
		while (pb + i < pe)
		{
			sToken[i] = *(pb + i);
			i ++;
		}
		sToken[i] = 0;
		return sSrc + i + 1;
	}
}

int TLib_Html_RadioBox(char *sFieldName, char *sValue, char *sOption)
{
	char *pv, *pb, *pe;
	char sVal[100];
	char sName[100];
	int i;
	
	pb = sOption;
	while (*pb != 0)
	{
		pb = GetToken(sVal, pb, ',');
		pb = GetToken(sName, pb, ';');
		
		if (strcmp(sVal, sValue) == 0)
			printf("<input type='radio' name='%s' checked value='%s'>%s\n", sFieldName, sVal, sName);
		else
			printf("<input type='radio' name='%s' value='%s'>%s\n", sFieldName, sVal, sName);
	}
}

int TLib_Html_SelectBox(char *sFieldName, char *sValue, char *sOption)
{
	char *pv, *pb, *pe;
	char sVal[100];
	char sName[100];
	int i;
	
	printf("<select name='%s'>\n", sFieldName);
	pb = sOption;
	while (*pb != 0)
	{
		pb = GetToken(sVal, pb, ',');
		pb = GetToken(sName, pb, ';');
		
		if (strcmp(sVal, sValue) == 0)
			printf("<option selected value='%s'>%s</option>\n", sVal, sName);
		else
			printf("<option value='%s'>%s</option>\n", sVal, sName);
	}
	printf("</select>\n");
}

/////////////////////////////////////////////////////////////////////////

int TLib_Html_ExLoad(char *sPath, char *sFileName, TLIB_HTML_TEMPLATE *pstTemplate, char *sErrMsg)
{
	FILE *fp;
	long size, output_size;
	char html_file[200];

	strncpy(html_file, sPath,strlen(sPath)+1);
	strncat(html_file, sFileName,strlen(sFileName));

	if (html_file[0] != '/') 
	{
		snprintf(sErrMsg,1024, "Template File Name Must Start With '/'");
		return 1;
	}

	if ((fp = fopen(html_file, "r")) == NULL) 
	{
		snprintf(sErrMsg,1024, "Fail To Open Template File:%s", html_file);
		return 1;
	}

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	pstTemplate->pcBuffer = malloc(size+1);
	if (pstTemplate->pcBuffer == NULL)
	{
		snprintf(sErrMsg,1024, "TLib_Html_Load():Fail To Alloc Memory");
		return -1;
	}
	
	if (fread(pstTemplate->pcBuffer, size, 1, fp) < 0)
	{
		snprintf(sErrMsg,1024, "Fail To Read Template File:%s", html_file);
		return -1;
	}
		
	pstTemplate->pcBuffer[size] = 0;
	
	fclose(fp);
	
	pstTemplate->pcCurPos = pstTemplate->pcBuffer;
	return 0;
}

void TLib_Html_ExFree(TLIB_HTML_TEMPLATE *pstTemplate)
{
	free(pstTemplate->pcBuffer);
}

int TLib_Html_ExGetFirstTemplatePart(TLIB_HTML_TEMPLATE *pstTemplate, TLIB_HTML_TEMPLATE_PART *pstTemplatePart, char *sErrMsg)
{
	pstTemplate->pcCurPos = pstTemplate->pcBuffer;
	
	return TLib_Html_ExGetNextTemplatePart(pstTemplate, pstTemplatePart, sErrMsg);
}

char *TLib_Html_NewStr(char *s)
{
	int iSize;
	char *p;
	
	iSize = strlen(s);
	p = malloc(iSize+1);
	if (p == NULL)
	{
		return NULL;
	}
	
	strncpy(p, s,strlen(s));
	*(p+iSize) = 0;
	return p;
}

char *TLib_Html_NewSubStr(char *pcBegin, char *pcEnd)
{
	int iSize;
	char *p;
	char cTemp;
	
	cTemp = *pcEnd;
	*pcEnd = 0;
	p = TLib_Html_NewStr(pcBegin);
	*pcEnd = cTemp;
	
	return p;
}

int TLib_Html_ExGetNextTemplatePart(TLIB_HTML_TEMPLATE *pstTemplate, TLIB_HTML_TEMPLATE_PART *pstTemplatePart, char *sErrMsg)
{
	char *pc, *pc1, *pcPartBegin, *pcPartEnd;
	
	pstTemplatePart->pcHeader = NULL;
	pstTemplatePart->pcBuffer = NULL;

	if (pstTemplate->pcCurPos[0] == 0)
	{
		return 1;
	}

	pc = strstr(pstTemplate->pcCurPos, TLIB_HTML_PART_BEGIN_FILLER);
	if (pc == NULL)
	{
		pstTemplatePart->pcHeader = TLib_Html_NewStr("");
		pstTemplatePart->pcBuffer = TLib_Html_NewStr(pstTemplate->pcCurPos);
		
		if ((pstTemplatePart->pcHeader == NULL) || (pstTemplatePart->pcBuffer == NULL))
		{
			snprintf(sErrMsg,1024, "Fail To Alloc Template Part Mem");
			return -1;
		}
		pstTemplate->pcCurPos = pstTemplate->pcBuffer + strlen(pstTemplate->pcBuffer);
		return 0;
	}
	
	if (pc != pstTemplate->pcCurPos)
	{
		pstTemplatePart->pcHeader = TLib_Html_NewStr("");
		pstTemplatePart->pcBuffer = TLib_Html_NewSubStr(pstTemplate->pcCurPos, pc);
		
		if ((pstTemplatePart->pcHeader == NULL) || (pstTemplatePart->pcBuffer == NULL))
		{
			snprintf(sErrMsg,1024, "Fail To Alloc Template Part Mem");
			return -1;
		}
		pstTemplate->pcCurPos = pc;
		return 0;
	}

	pc = pc + strlen(TLIB_HTML_PART_BEGIN_FILLER);
	pc1 = strstr(pc, TLIB_HTML_PART_END_FILLER);
	if (pc1 == NULL)
	{
		snprintf(sErrMsg,1024, "Invalid Template Format: Part End Filler Needed");
		return -1;
	}
	
	pstTemplatePart->pcHeader = TLib_Html_NewSubStr(pc, pc1);
	
	pc = pc1 + strlen(TLIB_HTML_PART_END_FILLER);
	pc1 = strstr(pc, TLIB_HTML_PART_BEGIN_FILLER);
	
	if (pc1 == NULL)
	{
		pstTemplatePart->pcBuffer = TLib_Html_NewStr(pc);
	}
	else
	{
		pstTemplatePart->pcBuffer = TLib_Html_NewSubStr(pc, pc1);
	}

	if ((pstTemplatePart->pcHeader == NULL) || (pstTemplatePart->pcBuffer == NULL))
	{
		snprintf(sErrMsg,1024, "Fail To Alloc Template Part Mem");
		return -1;
	}
	
	if (pc1 == NULL)
	{
		pstTemplate->pcCurPos = pstTemplate->pcCurPos+strlen(pstTemplate->pcCurPos);
	}
	else
	{
		pstTemplate->pcCurPos = pc1;
	}
	return 0;
}

void TLib_Html_ExFreePart(TLIB_HTML_TEMPLATE_PART *pstTemplatePart)
{
	if (pstTemplatePart->pcHeader != NULL)
	{
		free(pstTemplatePart->pcHeader);
	}

	if (pstTemplatePart->pcBuffer != NULL)
	{
		free(pstTemplatePart->pcBuffer);
	}

	pstTemplatePart->pcHeader = NULL;
	pstTemplatePart->pcBuffer = NULL;
}

int TLib_Html_FillTemplateDef(char *sOutputBuffer, int iOutputBufferSize, FILE *pstStr, 
				char *sErrMsg, char *sTemplate, TLIB_HTML_PARAM *pstParam)
{
	char *pcOutputBegin, *pcOutputEnd;
	char *pcMarkBegin, *pcParamBegin, *pcParamEnd;
	char cTemp;
	int iTemp;
	int iInCommentRange;
	char *pcCommentParam;
	char *sNextOutput;
	int iOutputSizeLeft, iLen;
	TLIB_HTML_PARAM *pstCurParam;
	
	if (sOutputBuffer == NULL)
	{
		sNextOutput = NULL;
	}
	else
	{
		iLen = strlen(sOutputBuffer);
		sNextOutput = sOutputBuffer + iLen;
		iOutputSizeLeft = iOutputBufferSize - iLen - 1;
	}
	
	pcOutputBegin = sTemplate;
	iInCommentRange = 0;
	while(1)
	{
		if (*pcOutputBegin == 0)
		{
			return 0;
		}
		
		pcMarkBegin = strstr(pcOutputBegin, TLIB_HTML_PMARK_BEGIN);
		if (pcMarkBegin == NULL)
		{
			pcOutputEnd = pcOutputBegin + strlen(pcOutputBegin);
		}
		else
		{
			pcOutputEnd = pcMarkBegin;
			if (pcMarkBegin-pcOutputBegin >= strlen(TLIB_HTML_PMARK_BEGIN_FILLER))
			{
				if (strncmp(pcMarkBegin-strlen(TLIB_HTML_PMARK_BEGIN_FILLER), 
								TLIB_HTML_PMARK_BEGIN_FILLER,
								strlen(TLIB_HTML_PMARK_BEGIN_FILLER)) == 0)
				{
					pcOutputEnd = pcMarkBegin-strlen(TLIB_HTML_PMARK_BEGIN_FILLER);
				}
			}
		}
		
		if (iInCommentRange == 0)
		{
			cTemp = *pcOutputEnd;
			*pcOutputEnd = 0;
			if (sNextOutput != NULL)
			{
				iLen = strlen(pcOutputBegin);
				if (iOutputSizeLeft < iLen)
				{
					snprintf(sErrMsg,1024, "Output Buffer Out Of Size");
					return 1;
				}
				strncpy(sNextOutput, pcOutputBegin, strlen(pcOutputBegin)+1);
				sNextOutput += iLen;
				iOutputSizeLeft -= iLen;
			}
			else if (pstStr != NULL)
			{
				fprintf(pstStr, "%s", pcOutputBegin);
			}
			else
			{
				printf("%s", pcOutputBegin);
			}				
			*pcOutputEnd = cTemp;
		}
		
		if (*pcOutputEnd == 0)
		{
			return 0;
		}
		
		pcParamBegin = pcMarkBegin + strlen(TLIB_HTML_PMARK_BEGIN);
		pcParamEnd = strstr(pcParamBegin, TLIB_HTML_PMARK_END);
		if (pcParamEnd == NULL)
		{
			snprintf(sErrMsg,1024, "TLib_Html_ExFillTemplate():TLIB_HTML_PMARK_END not found");
			return 1;
		}
		
		if (iInCommentRange == 0)
		{
			pstCurParam = pstParam->pstNext;
			while (pstCurParam != NULL)
			{
//				if (strncmp(pstCurParam->sName, pcParamBegin, pcParamEnd-pcParamBegin) == 0)
				cTemp = *pcParamEnd;
				*pcParamEnd = 0;
				iTemp = TLib_Str_StrCmp(pcParamBegin, pstCurParam->sName);
				*pcParamEnd = cTemp;
				
				if (iTemp == 0)
				{
					if (sNextOutput != NULL)
					{
						iLen = strlen(pstCurParam->sValue);
						if (iOutputSizeLeft < iLen)
						{
							snprintf(sErrMsg,1024, "Output Buffer Out Of Size");
							return 1;
						}
						strncpy(sNextOutput, pstCurParam->sValue,strlen(pstCurParam->sValue)+1);
						sNextOutput += iLen;
						iOutputSizeLeft -= iLen;
					}
					else if (pstStr != NULL)
					{
						fprintf(pstStr, "%s", pstCurParam->sValue);
					}
					else
					{
						printf("%s", pstCurParam->sValue);
					}				
					break;
				}
				pstCurParam = pstCurParam->pstNext;
			}

			if (strncmp(TLIB_HTML_PMARK_COMMENT, pcParamBegin, strlen(TLIB_HTML_PMARK_COMMENT)) == 0)
			{
				iInCommentRange = 1;
				pcCommentParam = pcParamBegin;
			}
		}
		else if (strncmp(pcCommentParam, pcParamBegin, pcParamEnd-pcParamBegin) == 0)
		{
			iInCommentRange = 0;
		}
		
		pcOutputBegin = pcParamEnd + strlen(TLIB_HTML_PMARK_END);
		if (strncmp(pcOutputBegin, TLIB_HTML_PMARK_END_FILLER, strlen(TLIB_HTML_PMARK_END_FILLER))==0)
		{
			pcOutputBegin = pcOutputBegin + strlen(TLIB_HTML_PMARK_END_FILLER);
		}
	}
	return 0;
}

int TLib_Html_ParamInit(TLIB_HTML_PARAM *pstParam, char *sErrMsg)
{
	pstParam->pstNext = NULL;
	return 0;
}

int TLib_Html_ParamAppendNode(TLIB_HTML_PARAM *pstParam, char *sErrMsg, char *sName, char *sValue)
{
	TLIB_HTML_PARAM *pstNewParam;
	
	pstNewParam = malloc(sizeof(TLIB_HTML_PARAM));
	if (pstNewParam == NULL)
	{
		snprintf(sErrMsg,1024, "Fail To Alloc Memory For Param Node");
		return -1;
	}
	
	pstNewParam->sName = TLib_Html_NewStr(sName);
	if (pstNewParam->sName == NULL)
	{
		snprintf(sErrMsg,1024, "Fail To Alloc Memory For Param Node");
		free(pstNewParam);
		return -1;
	}		

	pstNewParam->sValue = TLib_Html_NewStr(sValue);
	if (pstNewParam->sValue == NULL)
	{
		snprintf(sErrMsg,1024, "Fail To Alloc Memory For Param Node");
		free(pstNewParam->sName);
		free(pstNewParam);
		return -1;
	}
	
	pstNewParam->pstNext = pstParam->pstNext;
	pstParam->pstNext = pstNewParam;
	
	return 0;
}

int TLib_Html_VParamAppend(TLIB_HTML_PARAM *pstParam, char *sErrMsg, va_list ap)
{
	char *sParam, *sVal;
	int iRetCode;

	sParam = va_arg(ap, char *);
	while (sParam != NULL)
	{
		sVal = va_arg(ap, char *);
		
		iRetCode = TLib_Html_ParamAppendNode(pstParam, sErrMsg, sParam, sVal);
		if (iRetCode != 0)
		{
			return iRetCode;
		}
		
		sParam = va_arg(ap, char *);
	}
	return 0;
}

int TLib_Html_ParamAppend(TLIB_HTML_PARAM *pstParam, char *sErrMsg, ...)
{
	va_list ap;
	int iRetCode;
	
	va_start(ap, sErrMsg);
	iRetCode = TLib_Html_VParamAppend(pstParam, sErrMsg, ap);
	va_end(ap);
	
	return iRetCode;
}

void TLib_Html_ParamFreeNode(TLIB_HTML_PARAM *pstParam)
{
	free(pstParam->sName);
	free(pstParam->sValue);
	free(pstParam);
}

void TLib_Html_ParamMark(TLIB_HTML_PARAM *pstSubParam, TLIB_HTML_PARAM *pstParam)
{
	pstSubParam->pstNext = pstParam->pstNext;
}

void TLib_Html_ParamTruncTo(TLIB_HTML_PARAM *pstParam, TLIB_HTML_PARAM *pstSubParam)
{
	TLIB_HTML_PARAM *pstParamCur, *pstParamNext, *pstParamEnd;
	
	if (pstSubParam == NULL)
	{
		pstParamEnd = NULL;
	}
	else
	{
		pstParamEnd = pstSubParam->pstNext;
	}
	
	pstParamCur = pstParam->pstNext;
	
	while ((pstParamCur != NULL) && (pstParamCur != pstParamEnd))
	{
		pstParamNext = pstParamCur->pstNext;
		TLib_Html_ParamFreeNode(pstParamCur);
		pstParamCur = pstParamNext;
	}
	
	pstParam->pstNext = pstParamCur;
}

void TLib_Html_ParamFree(TLIB_HTML_PARAM *pstParam)
{
	TLib_Html_ParamTruncTo(pstParam, NULL);
}

int TLib_Html_VFillTemplateDef(char *sOutputBuffer, int iOutputBufferSize, FILE *pstStr, 
				char *sErrMsg, char *sTemplate, va_list ap)
{
	TLIB_HTML_PARAM stParam;
	int iRetCode;
	
	iRetCode = TLib_Html_ParamInit(&stParam, sErrMsg);
	if (iRetCode != 0)
	{
		return iRetCode;
	}
	
	iRetCode = TLib_Html_VParamAppend(&stParam, sErrMsg, ap);
	if (iRetCode != 0)
	{
		return iRetCode;
	}
	
	iRetCode = TLib_Html_FillTemplateDef(sOutputBuffer, iOutputBufferSize, pstStr, sErrMsg, sTemplate, &stParam);
	
	TLib_Html_ParamFree(&stParam);
	
	return iRetCode;
	
}


int TLib_Html_FillTemplateFile(FILE *pstStr, char *sErrMsg, char *sTemplate, ...)
{
	int iRetCode;
	va_list ap;
	
	va_start(ap, sTemplate);
	iRetCode = TLib_Html_VFillTemplateDef(NULL, 0, pstStr, sErrMsg, sTemplate, ap);
	va_end(ap);

	return iRetCode;
}

int TLib_Html_FillTemplateStr(char *sOutputBuffer, int iOutputBufferSize, char *sErrMsg, char *sTemplate, ...)
{
	int iRetCode;
	va_list ap;
	
	va_start(ap, sTemplate);
	iRetCode = TLib_Html_VFillTemplateDef(sOutputBuffer, iOutputBufferSize, NULL, sErrMsg, sTemplate, ap);
	va_end(ap);

	return iRetCode;
}

void TLib_Html_FillTemplate(char *sTemplate, ...)
{
	int iRetCode;
	va_list ap;
	char sErrMsg[300];
	
	va_start(ap, sTemplate);
	iRetCode = TLib_Html_VFillTemplateDef(NULL, 0, NULL, sErrMsg, sTemplate, ap);
	va_end(ap);

	if (iRetCode != 0)
	{
		printf("File Template Error: %s\n", sErrMsg);
	}
}


/*****************************************************
折行!如果一行超过iCol个字符, 换行
*****************************************************/
void TLib_Html_FormatStr(char *sOldContent, char *sNewContent, int iCol)
{
	int j;
	int iNew;
	int iOld;	
	
	iNew = 0; iOld = 0; j = 0;
	if (iCol<=0)  iCol=100;
	
	while (iOld<strlen(sOldContent))
	{
		if (j>=iCol) 
		{
			sNewContent[iNew] = ' ';
			iNew ++;
			j = 0;
		}
		
		if (sOldContent[iOld]=='\n') j = 0;
			
		if (sOldContent[iOld]<0)
		{
			sNewContent[iNew] = sOldContent[iOld];
			iOld++; iNew++;j++;
		}
		
		sNewContent[iNew] = sOldContent[iOld];
		iOld++; iNew++;j++;
	}
	sNewContent[iNew] = 0;	
}

/*****************************************************
如果一行超过iCol个字符, CUT掉
*****************************************************/
char* TLib_Html_GetShortStr(char *sOldContent, char *sNewContent, int iCol)
{
	unsigned int iNew,iOld;
	
	iNew = 0; iOld = 0;
	if (iCol<=0)  iCol=100;
	
	while ((iOld<strlen(sOldContent)) && (iOld<iCol))
	{
		if (sOldContent[iOld]<0)
		{
			sNewContent[iNew] = sOldContent[iOld];
			iOld++; iNew++;
		}
		
		sNewContent[iNew] = sOldContent[iOld];
		iOld++; iNew++;
	}
	sNewContent[iNew] = 0;

	return sNewContent;

}

#endif

