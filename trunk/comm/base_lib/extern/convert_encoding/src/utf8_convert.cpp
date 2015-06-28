#include "utf8_convert.h"
//#include <iconv.h>
#include <errno.h>

unsigned char UCS2toUTF8Code(unsigned short ucs2_code, unsigned char* utf8_code)
{
	int length = 0;
	unsigned char* out = utf8_code;
	if(!utf8_code)
	{
		return length;
	}
	if(0x0080 > ucs2_code)
	{
		/* 1 byte UTF-8 Character.*/
		*out = (unsigned char)ucs2_code;
		length++; 
	}
	else if(0x0800 > ucs2_code)
	{
		/*2 bytes UTF-8 Character.*/
		*out = ((unsigned char)(ucs2_code >> 6)) | 0xc0;
		*(out+1) = ((unsigned char)(ucs2_code & 0x003F)) | 0x80;
		length += 2;
	}
	else
	{
		/* 3 bytes UTF-8 Character .*/
		*out = ((unsigned char)(ucs2_code >> 12)) | 0xE0;
		*(out+1) = ((unsigned char)((ucs2_code & 0x0FC0)>> 6)) | 0x80;
		*(out+2) = ((unsigned char)(ucs2_code & 0x003F)) | 0x80;
		length += 3; 
	}
	return length;
}

int API_Gbk2Utf8(const char *szSource, char *strDest, int iDestLen)
{
	//cout<<strlen(szSource)<<"size of szSource"<<endl;
	if (NULL == szSource || NULL == strDest || 0 == iDestLen)
	{
		if (NULL != strDest && iDestLen > 0)
			strDest[0] = 0;
		iDestLen = 0;
		return 0;
	}
    int iSrcLen=strlen(szSource);
	
	char *pUniString = (char*)malloc(iSrcLen*2+1);
    if(pUniString == NULL){
        return -1;
    }
    
	int iLen = string_gbk2unicode(szSource, pUniString, iSrcLen);

	unsigned char pTemp[4] = {0};
	unsigned short iTemp;
    int iRet = 0;
    int iIndex = 0;
	for (int i=0; i<iLen; i++)
	{
		
		iTemp = (pUniString[i]<<8) + (pUniString[i+1] & 0xff);
		i++;
		
		if ((iRet = UCS2toUTF8Code(iTemp, pTemp)) == 0)
		{
            free(pUniString);
			return -1;
		}
		if(iIndex+iRet >= iDestLen){
            break;
		}
        memcpy(strDest+iIndex, pTemp, iRet);
        iIndex += iRet;
	}
	strDest[iIndex] = 0;
    free(pUniString);

	return 0;
}
/*
	modify by tommmyliang
	to be more effictive!

*/
int API_Gbk2Utf8(const char *szSource, string &strDest)
{
	//cout<<strlen(szSource)<<"size of szSource"<<endl;
	int iDestLen=strlen(szSource)*2+1;

    char *pUTFString = (char *)malloc(iDestLen);
    if(pUTFString == NULL){
        return -1;
    }
    API_Gbk2Utf8(szSource, pUTFString, iDestLen);
    strDest = pUTFString;
    free(pUTFString);
    return 0;
}

int API_Gbk2Utf8_ICONV(const char *szSource, std::string &strDest){

	iconv_t cd = iconv_open("UTF-8//IGNORE","GB18030");
	if (cd == (iconv_t)-1)
	{
		iconv_close(cd);
		return -2;
	}
	int iInLen  = strlen(szSource);
	int iOutLen = 2*iInLen+1;
	char *sUTF= new char[iOutLen];
	if(!sUTF){
		iconv_close(cd);
		return -3;
	}	
	char * sPos = sUTF;
	int    iTemp = iOutLen;
	iconv(cd, (char**)&szSource,(size_t*)&iInLen, (char**)&sUTF, (size_t*)&iOutLen );
	
	sPos[iTemp-iOutLen] = '\0';	//success 
	strDest = sPos;
	delete[] sPos;	
	iconv_close(cd);
	return 0;	

}

int API_Gbk2Utf8_ICONV(const char *szSource, char *strDest, int iDestLen)
{
	iconv_t cd = iconv_open("UTF-8//IGNORE","GB18030");
	if (cd == (iconv_t)-1)
	{
		iconv_close(cd);
		return -2;
	}
	int iInLen  = strlen(szSource);
	int iOutLen = 2*iInLen+1;
	char * sUTF = strDest;
	int    iTemp = iOutLen;

	iconv(cd, (char**)&szSource,(size_t*)&iInLen, (char**)&sUTF, (size_t*)&iOutLen );
	iDestLen = iTemp-iOutLen;
	strDest[iDestLen] = '\0';	//success 
	iconv_close(cd);

	return 0;
}


int API_Utf8ToGbk(const char * rsIn, int rsInLength, char *rsOut, int &rsOutLength)
{
        int iLeftRoomLen,iLeftInLen;
        iconv_t stCvt;
        stCvt = iconv_open("GBK", "UTF-8");
        if(stCvt == 0){   return   -1;}
        
        iLeftInLen = rsInLength;
        iLeftRoomLen = rsOutLength;
		
        char * pszOutBuf = rsOut;

        int iRet;
        char *pInBuf = (char *)rsIn;
        while(iLeftInLen > 0 && iLeftRoomLen > 1)
        {
            iRet = iconv(stCvt, &pInBuf, (size_t*)&iLeftInLen, &pszOutBuf, (size_t *)&iLeftRoomLen);
            if (iRet == (int)((size_t)-1))
            {
                switch(errno)
                {
                    case E2BIG:
                    {
                        goto END;
                        break;
                    }
                    case EINVAL:
                    {
                        // just  skip
                        pInBuf++;
                        iLeftInLen--;
                        break;
                    }
                        
                    case EILSEQ:
                    {
                        pInBuf++;
                        iLeftInLen--;
                        break;
                    }
                    default:
                    {
                        *pszOutBuf = *pInBuf;
                        pInBuf++;
                        iLeftInLen--;
                        pszOutBuf++;
                        iLeftRoomLen--;
                        break;
                    }
                }
            }
        }
END:
        iconv_close(stCvt);
        *pszOutBuf = '\0';
        rsOutLength = pszOutBuf - rsOut;
        return 0;
}

int API_Utf8ToGbk(const char* src, string & dest)
{
	dest = "";
	int lensrc = strlen(src);
	int lendest = lensrc+1;
    
	char* rsOut = (char*)malloc(lendest);
    if(rsOut == NULL){
        return -1;
    }
	int ret = API_Utf8ToGbk(src, lensrc, rsOut, lendest);
	if(ret){
		if(rsOut != NULL)
		{
			free(rsOut);
			rsOut = NULL;
		}
		return ret;
	}
	dest = (string)rsOut;
	
	if(rsOut != NULL)
	{
		free(rsOut);
		rsOut = NULL;
	}
	return 0;
}

