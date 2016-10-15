#ifndef __BITMAPGRAPH_H__
#define __BITMAPGRAPH_H__

#include <CommonLib/Types.h>

#define EFFECT_VALUE      128
#define BITMAPMINSIZE     36

#define BITMAP_ERROR      -1
#define BITMAP_NO_SUPPORT -2
#define BITMAP_NO_ERROR   0

class COMMONLIB_API CBitmapGraph
{
public:
    CBitmapGraph(void);
    ~CBitmapGraph(void);
public:
	static void AlphaPaste(char *SrcBuff, int nSrcWidth, int nSrcHeight, char *AlphaBuff, int x, int y, 
		          int nAlphaWidth, int nAlphaHeight, double fAlpha);
	char *GetBitmapData() { return m_Data; };
	int  GetWidth();
	int  GetHeight();
    bool RefreshBitmap(const BITMAPINFOHEADER *Header, char *Data, unsigned int nDataSize);
	int  LoadFromFile(char *FileName);
	int  LoadFromBuff(char *lpBuff, DWORD dwSize); //��������������
    bool SaveToFile(char *FileName);
    bool Relievo(); //����Ч��
    bool Carve(); //���Ч��
    bool Mosaic(); //������Ч��
	//������ͼƬ���Ƶ�Ŀ��dc��������
    void DrawToDc(HDC dc,  int x, int y, int nWidth, int nHeight);
	//��ͼƬ��һ���ֻ��Ƶ�Ŀ��dc��������
	void DrawToDc( HDC dc, int xDst, int yDst, int cxDst, int cyDst,
						   int xSrc, int ySrc, int cxSrc, int cySrc );
	//��ͼƬ���Ƶ�ָ��Ŀ��dc�ڣ�������͸������
	//����û������ͼ,ֻ��ָ��͸��ɫ,���Խ�������
	void DrawTransBitmap( HDC hdcDest, int nXOriginDest, int nYOriginDest, int nWidthDest,
                     int nHeightDest, COLORREF crTransparent );
	void DrawIdxToDC(int idx, int nBmpHeight, HDC dc, int x, int y, int nWidth, int nHeight);
	void Alpha(char *Buff, int nSrcWidth, int nSrcHeight, int x, int y, double fAlpha);
    bool IsEmpty();
	HBITMAP GetBitmap();
private:
	static inline char MIN(int x1, int x2)
	{
		if (x1 > x2)
			return x2;
		else
			return x1;
	}
private:
   BITMAPINFOHEADER *m_Header;
   char *m_Data;
   HBITMAP m_hBitmap;
   unsigned int m_nDataSize;
};

#endif