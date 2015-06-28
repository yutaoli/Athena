#ifndef _TLib_Html_H_
#define _TLib_Html_H_

#define TLIB_HTML_MAX_SHIFT_SET		200

////////////////////////////////////////////////////////////////
typedef struct _TLIB_HTML_TEMPLATE_
{
	char *pcBuffer;
	char *pcCurPos;
} TLIB_HTML_TEMPLATE;

typedef struct _TLIB_HTML_TEMPLATE_PART_
{
	char *pcHeader;
	char *pcBuffer;
} TLIB_HTML_TEMPLATE_PART;

typedef struct _TLIB_HTML_PARAM_
{
	char *sName;
	char *sValue;
	struct _TLIB_HTML_PARAM_ *pstNext;
} TLIB_HTML_PARAM;

////////////////////////////////////////////////////////////////

// exam: sPath='/usr/local/evs/htdocs', sFileName = /admin/login.htm
char *TLib_Html_Load(char *sPath, char *sFileName);
void TLib_Html_File(char *pBuffer, char *start_token, char *end_token);
void TLib_Html_FillFile(FILE *fp, char *pBuffer, char *start_token, char *end_token);//add  by hgt 2000-07-12
void TLib_Html_Free(char *pBuffer);

char *TLib_Html_GetTemplate(char *sBuffer, char *sMarkBegin, char *sMarkEnd);

int TLib_Html_SelectBox(char *sFieldName, char *sValue, char *sOption);
int TLib_Html_RadioBox(char *sFieldName, char *sValue, char *sOption);
int TLib_Html_Password(char *sFieldName, char *sValue);
int TLib_Html_TextBox(char *sFieldName, char *sValue);
int TLib_Html_TextArea(char *sFieldName, char *sValue, int iRow, int iCol);

void TLib_Html_UrlBack(void);
void TLib_Html_MessageBox(char *sMsg);
void TLib_Html_Redirect(char *sURL);
void TLib_Html_Location(char *sURL);

char *TLib_Html_UrlEncode(char *sDest, char *sSrc);
char *TLib_Html_TextEncode(char *sDest, char *sSrc); //不处理回车符
char *TLib_Html_TextEncode2(char *sDest, char *sSrc);
char *TLib_Html_TextEncode3(char *sDest, char *sSrc);
char *TLib_Html_TextEncode4(char *sDest, char *sSrc);

/////////////////////////////////////////////////////////////
int TLib_Html_ExLoad(char *sPath, char *sFileName, TLIB_HTML_TEMPLATE *pstTemplate, char *sErrMsg);
void TLib_Html_ExFree(TLIB_HTML_TEMPLATE *pstTemplate);
int TLib_Html_ExGetFirstTemplatePart(TLIB_HTML_TEMPLATE *pstTemplate, TLIB_HTML_TEMPLATE_PART *pstTemplatePart, char *sErrMsg);
int TLib_Html_ExGetNextTemplatePart(TLIB_HTML_TEMPLATE *pstTemplate, TLIB_HTML_TEMPLATE_PART *pstTemplatePart, char *sErrMsg);
void TLib_Html_ExFreePart(TLIB_HTML_TEMPLATE_PART *pstTemplatePart);

int TLib_Html_ParamInit(TLIB_HTML_PARAM *pstParam, char *sErrMsg);
int TLib_Html_ParamAppendNode(TLIB_HTML_PARAM *pstParam, char *sErrMsg, char *sName, char *sValue);
int TLib_Html_ParamAppend(TLIB_HTML_PARAM *pstParam, char *sErrMsg, ...);
void TLib_Html_ParamMark(TLIB_HTML_PARAM *pstSubParam, TLIB_HTML_PARAM *pstParam);
void TLib_Html_ParamTruncTo(TLIB_HTML_PARAM *pstParam, TLIB_HTML_PARAM *pstSubParam);
void TLib_Html_ParamFree(TLIB_HTML_PARAM *pstParam);

int TLib_Html_FillTemplateDef(char *sOutputBuffer, int iOutputBufferSize, FILE *pstStr, 
				char *sErrMsg, char *sTemplate, TLIB_HTML_PARAM *pstParam);
int TLib_Html_FillTemplateFile(FILE *pstStr, char *sErrMsg, char *sTemplate, ...);
int TLib_Html_FillTemplateStr(char *sOutputBuffer, int iOutputBufferSize, char *sErrMsg, char *sTemplate, ...);
void TLib_Html_FillTemplate(char *sTemplate, ...);
void TLib_Html_FormatStr(char *sOldContent, char *sNewContent, int iCol);
char* TLib_Html_GetShortStr(char *sOldContent, char *sNewContent, int iCol);

char *TLib_Html_NewStr(char *s);
char *TLib_Html_NewSubStr(char *pcBegin, char *pcEnd);
#endif

