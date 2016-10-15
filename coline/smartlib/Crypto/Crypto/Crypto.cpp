#include <Crypto/Crypto.h>
#include <fstream>
#include <commonlib/debuglog.h>
#include <Commonlib/stringutils.h>

const char szBase64Table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

//zlib压缩
BOOL zlib_compress(BYTE **pDest, UINT32 *pDestLen, const BYTE *pSrc, const UINT32 SrcLen)
{
	//*pDestLen = SrcLen;
	//*pDest = new BYTE[*pDestLen];
	//memmove(*pDest, pSrc, SrcLen);
	//return TRUE;
	*pDestLen = compressBound(SrcLen);
	*pDest = new BYTE[*pDestLen];
	int nRes = compress((Bytef *)(*pDest), (unsigned long *)pDestLen, (Bytef *)pSrc, SrcLen);
	if (nRes == Z_OK)
	{
		return TRUE;
	} else
	{
		delete [](*pDest);
		(*pDest) = NULL;
		PRINTDEBUGLOG(dtInfo, "compress fail, src Size: %d, dest size: %d", SrcLen, *pDestLen);
		return FALSE;
	}
}

//获取zlib压缩后的最大长度，用来分配内存
uLong zlib_getcompressBound(uLong ulSrcSize)
{
	return compressBound(ulSrcSize);
}

//zlib压缩，已经分配内存
BOOL zlib_compress(BYTE *pDest, UINT32 *pDestLen, const BYTE *pSrc, const UINT32 SrcLen)
{
	int nRes = compress((Bytef *)(pDest), (unsigned long *)pDestLen, (Bytef *)pSrc, SrcLen);
	if (nRes == Z_OK)
	{
		return TRUE;
	} else
	{
		PRINTDEBUGLOG(dtInfo, "compress fail, src Size: %d, dest size: %d", SrcLen, *pDestLen);
		return FALSE;
	}
}

//zlib 解压缩
BOOL zlib_uncompress(BYTE *pDest, UINT32 *pDestLen, const BYTE *pSrc, const UINT32 SrcLen)
{
	//memmove(pDest, pSrc, *pDestLen);
	//return TRUE;
	int nRes =  uncompress((Bytef *)pDest, (unsigned long *)pDestLen, (Bytef *)pSrc, SrcLen);
	if (nRes == Z_OK)
	{
		return TRUE;
	} else
	{
		PRINTDEBUGLOG(dtInfo, "uncompress fail, src len :%d, result:%d", SrcLen, nRes);
		return FALSE;
	}
}

//md5编码
void md5_encode(const char *szInput, DWORD dwSize, char *szResult)
{
   md5_state_t state;
   md5_init(&state);
   md5_append(&state, (const md5_byte_t *)szInput, dwSize);
   md5_byte_t szTemp[16] = {0};
   md5_finish(&state, szTemp);
   md5_trans(szTemp, szResult);
}

//md5摘要
bool md5_abstractfile(const char *szFileName, char *szResult)
{
	const DWORD MAX_ABSTRACT_FILE_SIZE = 8196; //8K
	const DWORD ABSTRACT_PERCENT_SIZE  = 2048; //每片接要多少？ MAX_ABSTRACT_FILE_SIZE / 4
#ifdef _UNICODE
	TCHAR szTemp[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szFileName, szTemp, MAX_PATH);
	ifstream ifs(szTemp, std::ios::in | std::ios::binary);
#endif
	if (ifs.is_open())
	{
		md5_state_t state;
		md5_init(&state);
		char pBuff[MAX_ABSTRACT_FILE_SIZE + sizeof(DWORD)];
		int nSize;
		ifs.seekg(0, std::ios_base::end);
		DWORD dwFileSize = ifs.tellg();
		ifs.seekg(0, std::ios_base::beg);
		if (dwFileSize < MAX_ABSTRACT_FILE_SIZE) //读取全部数据
		{
			ifs.read(pBuff, MAX_ABSTRACT_FILE_SIZE);
			nSize = ifs.gcount();
		} else //读取摘要
		{
			//读取开始位置
			ifs.read(pBuff, ABSTRACT_PERCENT_SIZE);
			nSize = ifs.gcount();
			//读取1/4处
			ifs.seekg(dwFileSize / 4, std::ios::beg);
			ifs.read(pBuff + ABSTRACT_PERCENT_SIZE, ABSTRACT_PERCENT_SIZE);
			nSize += ifs.gcount();
			//读取1/2处
			ifs.seekg(dwFileSize / 2, std::ios::beg);
			ifs.read(pBuff + ABSTRACT_PERCENT_SIZE * 2, ABSTRACT_PERCENT_SIZE);
			nSize += ifs.gcount();
			//读取未尾
			ifs.seekg(dwFileSize - ABSTRACT_PERCENT_SIZE, std::ios::beg);
			ifs.read(pBuff + ABSTRACT_PERCENT_SIZE * 3, ABSTRACT_PERCENT_SIZE);
			nSize += ifs.gcount();
			//追加一下文件长度
			memmove(pBuff + ABSTRACT_PERCENT_SIZE * 4, &dwFileSize, sizeof(DWORD));
			nSize += sizeof(DWORD);
		}
		if (nSize > 0)
			md5_append(&state, (const md5_byte_t *)pBuff, nSize);
		md5_byte_t szDigit[16] = {0};
		md5_finish(&state, szDigit);
		md5_trans(szDigit, szResult);
		return true;
	}
	return false;
}

//md5编码文件
bool md5_encodefile(const char *szFileName, char *szResult)
{
#ifdef _UNICODE
	TCHAR szTemp[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szFileName, szTemp, MAX_PATH);
	ifstream ifs(szTemp, std::ios::in | std::ios::binary);
#endif
	if (ifs.is_open())
	{
		md5_state_t state;
		md5_init(&state);
		char pBuff[2048];
		int nSize;
		while(true)
		{
			ifs.read(pBuff, 2048);
			nSize = ifs.gcount();
			if (nSize > 0)
				md5_append(&state, (const md5_byte_t *)pBuff, nSize);
			if (nSize < 2048)
				break;
		}
		md5_byte_t szDigit[16] = {0};
		md5_finish(&state, szDigit);
		md5_trans(szDigit, szResult);
		return true;
	}
	return false;
}

//base64编码
char * base64_encode(const char *lpInbuff, int nInSize, char *lpOutBuff)
{
	unsigned char char_array_3[3], char_array_4[4];
	int i = 0, j = 0;
	char *p = lpOutBuff;
	while(nInSize --)
	{
		char_array_3[i ++] = *lpInbuff ++;
		if (i == 3)
		{
			char_array_4[0] = (char_array_3[0] & 0xFC) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xF0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0F) << 2) + ((char_array_3[2] & 0xC0) >> 6);
			char_array_4[3] = char_array_3[2] &0x3F;
			for(int k = 0; k < 4; k ++)
				*p ++ = szBase64Table[char_array_4[k]];
			i = 0;
		}
	}
	if (i)
	{
		for (j = i; j < 3; j ++)
			char_array_3[j] = '\0';
		char_array_4[0] = (char_array_3[0] & 0xFC) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xF0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0F) << 2) + ((char_array_3[2] & 0xC0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3F;

		for (j = 0; j < (i + 1); j++)
		  *p ++ = szBase64Table[char_array_4[j]];

		while((i++ < 3))
		  *p ++ = '=';
	}
	return lpOutBuff;
}

static inline bool is_base64(unsigned char c) 
{
  return (isalnum(c) || (c == '+') || (c == '/'));
}

static inline unsigned char GetByteByChar(char c)
{
	const char *p = szBase64Table;
	while(true)
	{
		if ((*p ++ == c) ||(*p == '\0'))
			break;
	}
	return (unsigned char )(p - szBase64Table - 1);
}

//base64解码
void * base64_decode(const char *lpInBuff, char *lpOutBuff, int &nOutSize)
{
	if (!lpInBuff)
		return NULL;
	int nSrcSize = (int)::strlen(lpInBuff);
	if (nSrcSize % 4)
		return NULL;
	int i = 0, j = 0;
	char *p = (char *)lpOutBuff;
    unsigned char char_array_4[4], char_array_3[3];
	int nSrcPos = 0;
	nOutSize = 0;
	while ((nSrcSize --) && (lpInBuff[nSrcPos] != '=') && is_base64(lpInBuff[nSrcPos]))
	{
		char_array_4[i ++] = lpInBuff[nSrcPos ++];
		if (i == 4)
		{
			for (j = 0; j < 4; j ++)
				char_array_4[j] = GetByteByChar(char_array_4[j]);
			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xF) << 4) + ((char_array_4[2] & 0x3C) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
			for (j = 0; j < 3; j ++)
			{
				nOutSize ++;
				*p ++ = char_array_3[j];
			}
			i = 0;
		}
	}
	if (i)
	{
		for (j = i; j < 4; j ++)
			char_array_4[j] = 0;
		for (j = 0; j < 4; j ++)
			char_array_4[j] = GetByteByChar(char_array_4[j]);
		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xF) << 4) + ((char_array_4[2] & 0x3C) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
		for (j = 0; j < (i - 1); j++) 
		{
			nOutSize ++;
			*p ++ = char_array_3[j];
		}
	}
	return lpOutBuff;
}
