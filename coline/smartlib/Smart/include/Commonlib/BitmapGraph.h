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
	int  LoadFromBuff(char *lpBuff, DWORD dwSize); //从数据流中载入
    bool SaveToFile(char *FileName);
    bool Relievo(); //浮雕效果
    bool Carve(); //雕刻效果
    bool Mosaic(); //马赛克效果
	//将整个图片绘制到目标dc的区域内
    void DrawToDc(HDC dc,  int x, int y, int nWidth, int nHeight);
	//将图片的一部分绘制到目标dc的区域内
	void DrawToDc( HDC dc, int xDst, int yDst, int cxDst, int cyDst,
						   int xSrc, int ySrc, int cxSrc, int cySrc );
	//将图片绘制到指定目标dc内，并可以透明背景
	//用于没有掩码图,只有指定透明色,可以进行伸缩
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