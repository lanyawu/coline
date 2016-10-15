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
	BOOL LoadFromFile(const char *FileName,  BOOL bGray); //���ļ�������
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
	void SetImageMask(int nMask);
	BOOL TransImageStyle(int r, int g, int b); //��ͼ��ɫ
	BOOL FillColorToImage(BYTE r, BYTE g, BYTE b); //�����ɫ
 
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
						  int nSrcHeight);
	//���Ƶ�Ŀ������ ==rc
	//�Ƿ��ڻ����� == rcPaint
	//Դͼ���� rcBmpPart
	//��ť���� rcCorners
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
