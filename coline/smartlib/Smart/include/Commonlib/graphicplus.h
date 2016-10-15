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
	BOOL LoadFromFile(const char *FileName, BOOL bGray); //���ļ�������
	BOOL LoadFromBuff(const char *lpBuff, DWORD dwSize, BOOL bGray); //��������������
    BOOL SaveToFile(const char *FileName, DWORD image_type); //�����ļ�
	BOOL SaveToFile(const char* FileName, DWORD image_type, SIZE szNew);//�����ļ������޸ĳߴ�
	BOOL SaveToStream(BYTE * &pBuff, long &nSize, DWORD image_type); //���浽������
	BOOL SaveToStream(BYTE *pBuff, DWORD &dwSize, DWORD image_type, BYTE byteQuality = 127); //���������������ѷ����ڴ�
	BOOL LoadFromBitmap(HBITMAP hBitmap, HPALETTE hPal = NULL); //��bitmap������
	BOOL LoadFromDIB(char *pSrc, DWORD dwWidth, DWORD dwHeight, DWORD dwBitPerPixel, 
		              DWORD dwBytesPerLine, BOOL bFlipImage);  //��DIB������
	BOOL LoadFromGraphic(IImageInterface *pSrc); //��Դͼ������
	BOOL LoadFromIcon(HICON hIcon); //��Icon ������
	BOOL SetGray(); //�һ�ͼƬ
	BOOL TransImageStyle(int r, int g, int b); //��ͼ��ɫ
	BOOL FillColorToImage(BYTE r, BYTE g, BYTE b); //�����ɫ
	BOOL MixImage(const CGraphicPlus *pSrc, BOOL bStretch); //Mix
	//ת����ɫ 1, 2, 4, 8
	BOOL TranslateToBpp(const GraphicBppType nType);
	//����DIB����
	BOOL CopyImageDibData(char *pDib, DWORD &dwDibSize);
	//
	BOOL SetTransferColor(COLORREF clr);
	//��ͼƬ����ָ��dc
	void DrawToDc(HDC dc,  int x, int y, int nWidth, int nHeight);
	void DrawToDc(HDC dc, const RECT& rc );
	static void TransparentDraw(HDC hdc,  int nXDest, int nYDest,  int nDestWidth, int nDestHeight, HDC hSrc, int nXSrc, 
		    int nYSrc, int nSrcWidth, int nSrcHeight, COLORREF crTransparent);
	//��ɫֵת�� 16λת256ɫ

	//��ԴͼƬ��һ���ֻ���ָ��dc�ϣ������ƶ�Ҫ͸ȥ�ı���ɫ��
	//��������
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
	//֧�ֽ�ԭͼƬ��һ���ֻ���ָ��dc���ƶ����򣬿�������
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