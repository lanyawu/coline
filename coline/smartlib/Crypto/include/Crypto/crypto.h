#ifndef __CRYPTO_H__
#define __CRYPTO_H__

#include <Commonlib/types.h>
#include <Crypto/zlib.h>
#include <Crypto/md5.h>

//函数定义
//16位CRC测试
WORD CRC16(BYTE *ucbuf, int iLen, WORD wCRC);
//32位CRC测试
DWORD CRC32(BYTE *buf, int iLen, DWORD dwCRC32);

//zlib压缩
BOOL zlib_compress(BYTE **pDest, UINT32 *pDestLen, const BYTE *pSrc, const UINT32 SrcLen);

//zlib压缩，已经分配内存
BOOL zlib_compress(BYTE *pDest, UINT32 *pDestLen, const BYTE *pSrc, const UINT32 SrcLen);

//zlib 解压缩
BOOL zlib_uncompress(BYTE *pDest, UINT32 *pDestLen, const BYTE *pSrc, const UINT32 SrcLen);

//获取zlib压缩后的最大长度，用来分配内存
uLong zlib_getcompressBound(uLong ulSrcSize);

//md5编码
void md5_encode(const char *szInput, DWORD dwSize,  char *szResult);
//md5摘要
bool md5_abstractfile(const char *szFileName, char *szResult);
//md5编码文件
bool md5_encodefile(const char *szFileName, char *szResult);
//base64编码
char * base64_encode(const char *lpInbuff, int nInSize, char *lpOutBuff);
//base64解码
void * base64_decode(const char *lpInBuff, char *lpOutBuff, int &nOutSize);
#endif