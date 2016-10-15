#ifndef __IMAGE_AC_ENCODER_H___
#define __IMAGE_AC_ENCODER_H___

#include <commonlib/types.h>

// CACEncoder (high level implementation)
#define BITMASK       0x80
#define MSB           0x8000
#define NSB           0x4000
#define USB           0x3FFF

class CACEncoder
{
public:
	CACEncoder();
	CACEncoder(char *pBufferIn, DWORD dwSrcCount);
	~CACEncoder();
public:
	BOOL EncodeBuffer(char *pBufferIn, DWORD dwSrcCount, char **ppBufferOut, DWORD *pdwDestCount);
	BOOL DecodeBuffer(char *pBufferIn, DWORD dwSrcCount, char **ppBufferOut, DWORD *pdwDestCount);

	//已经分配好内存的编解码
	BOOL Encode(char *pBufferIn, DWORD dwSrcCount, BYTE *pBufferOut, DWORD &dwDestCount);
	BOOL Decode(char *pBufferIn, DWORD dwSrcCount, char *pBufferOut, DWORD &dwDestCount);

	void SetBuffer(char *pBufferIn, DWORD dwSrcCount);
	void GetBuffer(char **ppBufferOut, DWORD **ppdwDestCount);
	BOOL EncodeBuffer();
	BOOL DecodeBuffer();

protected:
	void  InitModel();
	void  ScaleCounts();
	DWORD RangeCounts();
	void  BuildMap();
	void  OutputBit(WORD wBit, BYTE  &byteOutSymbol, BYTE &byteBitMask,
		               DWORD &dwDestCount, BYTE * pBuffer);
	void  OutputUnderflowBits(WORD wBit, DWORD &dwUnderflowBits, BYTE &byteOutSymbol,
		                     BYTE &byteBitMask, DWORD &dwDestCount, BYTE *pBuffer);
	void  FlushBitMask(BYTE &byteBitMask, BYTE &byteOutSymbol, DWORD &dwDestCount, BYTE *pBuffer);

protected:
	char *m_pBufferIn;
	DWORD m_dwSrcCount;
	char *m_pBufferOut;
	DWORD m_dwDestCount;

private:
	DWORD m_ac[256];
	DWORD m_ac2[256];
	DWORD m_ar[257];
	DWORD m_aMap[16384];
};

#endif