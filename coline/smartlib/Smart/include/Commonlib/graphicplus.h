#ifndef __GRAPHICPLUS_H__
#define __GRAPHICPLUS_H__

#include <Commonlib/types.h>
#include <CxImage/xImage.h>
#include <commonlib/systemutils.h>

typedef enum GraphicType{
	GRAPHIC_TYPE_FIRST_ = 0,
	GRAPHIC_TYPE_BMP = 1,
	GRAPHIC_TYPE_GIF,
	GRAPHIC_TYPE_JPG,
	GRAPHIC_TYPE_PNG,
	GRAPHIC_TYPE_ICO,
	GRAPHIC_TYPE_TIF,
	GRAPHIC_TYPE_TGA,
	GRAPHIC_TYPE_PCX,
	GRAPHIC_TYPE_WBMP,
	GRAPHIC_TYPE_WMF,
	GRAPHIC_TYPE_JP2,
	GRAPHIC_TYPE_JPC,
	GRAPHIC_TYPE_PGX,
	GRAPHIC_TYPE_PNM,
	GRAPHIC_TYPE_RAS,
	GRAPHIC_TYPE_JBG,
	GRAPHIC_TYPE_MNG,
	GRAPHIC_TYPE_SKA, 
	GRAPHIC_TYPE_RAW,
	GRAPHIC_TYPE_LAST_,
	GRAHPIC_TYPE_UNKNOWN_
}GRAPHICTYPE;

class COMMONLIB_API CGraphicPlus :public IImageInterface
{
public:
	CGraphicPlus(void);
	~CGraphicPlus(void);
	enum GraphicBppType
	{
		GRAPHIC_BPP_TYPE_1 = 1,
		GRAPHIC_BPP_TYPE_2 = 2,
		GRAPHIC_BPP_TYPE_4 = 4,
		GRAPHIC_BPP_TYPE_8 = 8
	};
public:
	BOOL LoadFromFile(const char *FileName, BOOL bGray); //从文件中载入
	BOOL LoadFromBuff(const char *lpBuff, DWORD dwSize, BOOL bGray); //从数据流中载入
    BOOL SaveToFile(const char *FileName, DWORD image_type); //保存文件
	BOOL SaveToFile(const char* FileName, DWORD image_type, SIZE szNew);//保存文件并且修改尺寸
	BOOL SaveToStream(BYTE * &pBuff, long &nSize, DWORD image_type); //保存到数据流
	BOOL SaveToStream(BYTE *pBuff, DWORD &dwSize, DWORD image_type, BYTE byteQuality = 127); //保存至数据流，已分配内存
	BOOL LoadFromBitmap(HBITMAP hBitmap, HPALETTE hPal = NULL); //从bitmap中载入
	BOOL LoadFromDIB(char *pSrc, DWORD dwWidth, DWORD dwHeight, DWORD dwBitPerPixel, 
		              DWORD dwBytesPerLine, BOOL bFlipImage);  //从DIB中载入
	BOOL LoadFromGraphic(IImageInterface *pSrc); //从源图中载入
	BOOL LoadFromIcon(HICON hIcon); //从Icon 中载入
	BOOL SetGray(); //灰化图片
	BOOL TransImageStyle(int r, int g, int b); //给图上色
	BOOL FillColorToImage(BYTE r, BYTE g, BYTE b); //填充颜色
	BOOL MixImage(const CGraphicPlus *pSrc, BOOL bStretch); //Mix
	//转换颜色 1, 2, 4, 8
	BOOL TranslateToBpp(const GraphicBppType nType);
	//拷贝DIB数据
	BOOL CopyImageDibData(char *pDib, DWORD &dwDibSize);
	//
	BOOL SetTransferColor(COLORREF clr);
	//将图片画到指定dc
	void DrawToDc(HDC dc,  int x, int y, int nWidth, int nHeight);
	void DrawToDc(HDC dc, const RECT& rc );
	static void TransparentDraw(HDC hdc,  int nXDest, int nYDest,  int nDestWidth, int nDestHeight, HDC hSrc, int nXSrc, 
		    int nYSrc, int nSrcWidth, int nSrcHeight, COLORREF crTransparent);
	//颜色值转换 16位转256色

	//将源图片的一部分画到指定dc上，必须制定要透去的背景色，
	//可以拉伸
	void TransparentDrawToDc(HDC hdc, 
							  int nXDest, 
							  int nYDest, 
							  int nDestWidth,
							  int nDestHeight,
							  int nXSrc,
							  int nYSrc,
							  int nSrcWidth,
							  int nSrcHeight,
							  COLORREF crTransparent);
	//支持将原图片的一部分画到指定dc的制定区域，可以拉伸
	void StretchDrawToDc( HDC hDestDc, 
						  int nDestX, 
						  int nDestY,
						  int nDestWidth, 
						  int nDestHeight,
						  int nSrcX, 
						  int nSrcY, 
						  int nSrcWidth, 
						  int nSrcHeight );
	BOOL DrawPlus(HDC hDc, const RECT& rc, const RECT& rcPaint, 
        const RECT& rcBmpPart, const RECT& rcCorners, LPALPHABLEND lpAlphaBlend, BYTE uFade = 255, 
        bool hole = false, bool xtiled = false, bool ytiled = false);
	HBITMAP GetBitmap();
	void SetImageMask(int clr);
	BOOL IsEmpty() const;
	int  GetWidth() const;
	int  GetHeight() const;
	int  GetMask();
	BOOL GetAlphaChannel();
	UINT GetGraphicType() const;
	void ClearImage();
	int GetFrameCount();
	int GetFrameDelay();
private:

public:
	CxImage *m_pImage;
	HBITMAP m_hBitmap;
};

#endif