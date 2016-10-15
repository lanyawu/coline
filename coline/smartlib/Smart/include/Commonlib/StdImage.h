#ifndef __STD_IMAGE__H_____
#define __STD_IMAGE__H_____
#include <Commonlib/types.h>
#include <commonlib/systemutils.h>

class CStdImage :public IImageInterface
{
public:
	CStdImage(void);
	~CStdImage(void);
public:
	BOOL LoadFromFile(const char *FileName,  BOOL bGray); //从文件中载入
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
	void SetImageMask(int nMask);
	BOOL TransImageStyle(int r, int g, int b); //给图上色
	BOOL FillColorToImage(BYTE r, BYTE g, BYTE b); //填充颜色
 
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
						  int nSrcHeight);
	//绘制的目标区域 ==rc
	//是否处于绘制区 == rcPaint
	//源图区域 rcBmpPart
	//角钮区域 rcCorners
	BOOL DrawPlus(HDC hDc, const RECT& rc, const RECT& rcPaint, 
        const RECT& rcBmpPart, const RECT& rcCorners, LPALPHABLEND lpAlphaBlend, BYTE uFade = 255, 
        bool hole = false, bool xtiled = false, bool ytiled = false);

	HBITMAP GetBitmap();
	BOOL IsEmpty() const;
	int  GetWidth() const;
	int  GetHeight() const;
	BOOL GetAlphaChannel();
	int  GetMask();
	UINT GetGraphicType() const;

	//
	static void RGBtoHSL(COLORREF clr, float *H, float *S, float *L);
	static COLORREF HSLtoRGB(float H, float S, float L);
private:
	int m_nWidth;
	int m_nHeight;
	HBITMAP m_hBitmap;
	BOOL m_bAlphaChannel;
	int m_nMask;
};

#endif
