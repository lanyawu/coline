#ifndef __CRYPTO_H__
#define __CRYPTO_H__

#include <Commonlib/types.h>
#include <Crypto/zlib.h>
#include <Crypto/md5.h>

//��������
//16λCRC����
WORD CRC16(BYTE *ucbuf, int iLen, WORD wCRC);
//32λCRC����
DWORD CRC32(BYTE *buf, int iLen, DWORD dwCRC32);

//zlibѹ��
BOOL zlib_compress(BYTE **pDest, UINT32 *pDestLen, const BYTE *pSrc, const UINT32 SrcLen);

//zlibѹ�����Ѿ������ڴ�
BOOL zlib_compress(BYTE *pDest, UINT32 *pDestLen, const BYTE *pSrc, const UINT32 SrcLen);

//zlib ��ѹ��
BOOL zlib_uncompress(BYTE *pDest, UINT32 *pDestLen, const BYTE *pSrc, const UINT32 SrcLen);

//��ȡzlibѹ�������󳤶ȣ����������ڴ�
uLong zlib_getcompressBound(uLong ulSrcSize);

//md5����
void md5_encode(const char *szInput, DWORD dwSize,  char *szResult);
//md5ժҪ
bool md5_abstractfile(const char *szFileName, char *szResult);
//md5�����ļ�
bool md5_encodefile(const char *szFileName, char *szResult);
//base64����
char * base64_encode(const char *lpInbuff, int nInSize, char *lpOutBuff);
//base64����
void * base64_decode(const char *lpInBuff, char *lpOutBuff, int &nOutSize);
#endif