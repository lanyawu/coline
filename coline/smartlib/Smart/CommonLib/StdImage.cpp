#include <Commonlib/stringutils.h>
#include <Commonlib/StdImage.h>

#define STD_IMAGE_BMP  1
#define STD_IMAGE_JPEG 2
#define STD_IMAGE_GIF  3
#define STD_IMAGE_PIC  4
#define STD_IMAGE_PNG  5
#define STD_IMAGE_PSD  6

extern "C"
{
    extern unsigned char *stbi_load_from_memory(unsigned char const *buffer, int len, int *x, int *y, \
        int *comp, int *img_format, int req_comp);
	extern void     stbi_image_free(void *retval_from_stbi_load);

};
 
static const float OneThird = 1.0f / 3;

void CStdImage::RGBtoHSL(COLORREF clr, float *H, float *S, float *L)
{
	const float
        R = (float)GetRValue(clr),
        G = (float)GetGValue(clr),
        B = (float)GetBValue(clr),
        nR = (R<0?0:(R>255?255:R))/255,
        nG = (G<0?0:(G>255?255:G))/255,
        nB = (B<0?0:(B>255?255:B))/255,
        m = min(min(nR,nG),nB),
        M = max(max(nR,nG),nB);
    *L = (m + M)/2;
    if (M==m) *H = *S = 0;
    else {
        const float
            f = (nR==m)?(nG-nB):((nG==m)?(nB-nR):(nR-nG)),
            i = (nR==m)?3.0f:((nG==m)?5.0f:1.0f);
        *H = (i-f/(M-m));
        if (*H>=6) *H-=6;
        *H*=60;
        *S = (2*(*L)<=1)?((M-m)/(M+m)):((M-m)/(2-M-m));
    }
}
 
COLORREF CStdImage::HSLtoRGB(float H, float S, float L) 
{
     const float
        q = 2*L<1?L*(1+S):(L+S-L*S),
        p = 2*L-q,
        h = H/360,
        tr = h + OneThird,
        tg = h,
        tb = h - OneThird,
        ntr = tr<0?tr+1:(tr>1?tr-1:tr),
        ntg = tg<0?tg+1:(tg>1?tg-1:tg),
        ntb = tb<0?tb+1:(tb>1?tb-1:tb),
        R = 255*(6*ntr<1?p+(q-p)*6*ntr:(2*ntr<1?q:(3*ntr<2?p+(q-p)*6*(2.0f*OneThird-ntr):p))),
        G = 255*(6*ntg<1?p+(q-p)*6*ntg:(2*ntg<1?q:(3*ntg<2?p+(q-p)*6*(2.0f*OneThird-ntg):p))),
        B = 255*(6*ntb<1?p+(q-p)*6*ntb:(2*ntb<1?q:(3*ntb<2?p+(q-p)*6*(2.0f*OneThird-ntb):p)));
    COLORREF clrR  = 0;
    clrR |= RGB( (BYTE)(R<0?0:(R>255?255:R)), (BYTE)(G<0?0:(G>255?255:G)), (BYTE)(B<0?0:(B>255?255:B)));
	return clrR;
}

#define RGB2GRAY(r,g,b) (((b)*117 + (g)*601 + (r)*306) >> 10)
CStdImage::CStdImage(void):
           m_hBitmap(NULL),
		   m_nWidth(0),
		   m_nHeight(0),
		   m_nMask(0),
		   m_bAlphaChannel(FALSE)
{
}


CStdImage::~CStdImage(void)
{
	if (m_hBitmap)
		::DeleteObject(m_hBitmap);
	m_hBitmap = NULL;
}

BOOL CStdImage::LoadFromFile(const char *FileName, BOOL bGray)
{
	if (!FileName)
		return FALSE;
	TCHAR szwTmp[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(FileName, szwTmp, MAX_PATH - 1);
	HANDLE hFile = ::CreateFile(szwTmp, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;
    DWORD dwSize = ::GetFileSize(hFile, NULL);
    if (dwSize == 0) 
		return FALSE;
	
	DWORD dwRead = 0;
	BYTE *pData = new BYTE[dwSize];
    ::ReadFile(hFile, pData, dwSize, &dwRead, NULL);
	BOOL bSucc = FALSE;
	if (dwRead == dwSize)
		bSucc = LoadFromBuff((char *)pData, dwSize, bGray);
	delete []pData;
    ::CloseHandle(hFile);
	return bSucc;
}

//从数据流中载入
BOOL CStdImage::LoadFromBuff(const char *lpBuff, DWORD dwSize, BOOL bGray)
{
	if (m_hBitmap)
		::DeleteObject(m_hBitmap);
	m_hBitmap = NULL;
	m_nWidth = 0;
	m_nHeight = 0;
	m_bAlphaChannel = FALSE;
	LPBYTE pImage = NULL;
    int x,y,n;
	int img_format = 0;
	BOOL bHasAlphaChannel = FALSE;
    pImage = stbi_load_from_memory((unsigned char *)lpBuff, dwSize, &x, &y, &n, &img_format, 4); 
    if (!pImage) 
		return FALSE;
	if ((img_format == STD_IMAGE_PNG) || (img_format == STD_IMAGE_GIF))
		bHasAlphaChannel = TRUE;
    BITMAPINFO bmi;
    ::ZeroMemory(&bmi, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = x;
    bmi.bmiHeader.biHeight = -y;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = x * y * 4;

    bool bAlphaChannel = false;
    LPBYTE pDest = NULL;
    HBITMAP hBitmap = ::CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&pDest, NULL, 0);
    if (!hBitmap) 
		return FALSE;

	if (bGray)
	{
		BYTE r = 0;
		for( int i = 0; i < x * y; i++ ) 
	    {
	        pDest[i*4 + 3] = pImage[i*4 + 3];
	        if (bHasAlphaChannel && (pDest[i*4 + 3] < 255) )
	        {
				r = RGB2GRAY((BYTE)(DWORD(pImage[i*4 + 2])*pImage[i*4 + 3]/255),(BYTE)(DWORD(pImage[i*4 + 1])*pImage[i*4 + 3]/255),
					(BYTE)(DWORD(pImage[i*4])*pImage[i*4 + 3]/255));
	            pDest[i*4] = r;
	            pDest[i*4 + 1] = r;
	            pDest[i*4 + 2] = r; 
	            bAlphaChannel = true;
	        } else
	        {
				r = RGB2GRAY(pImage[i*4 + 2], pImage[i*4 + 1], pImage[i*4]);
	            pDest[i*4] =r;
	            pDest[i*4 + 1] = r;
	            pDest[i*4 + 2] = r; 
	        }

	        if (m_nMask && (*(DWORD*)(&pDest[i*4]) == m_nMask))
			{
	            pDest[i*4] = (BYTE)0;
	            pDest[i*4 + 1] = (BYTE)0;
	            pDest[i*4 + 2] = (BYTE)0; 
	            pDest[i*4 + 3] = (BYTE)0;
	            bAlphaChannel = true;
	        }
	    }
	} else
	{
		for( int i = 0; i < x * y; i++ ) 
	    {
	        pDest[i*4 + 3] = pImage[i*4 + 3];
	        if (bHasAlphaChannel && (pDest[i*4 + 3] < 255))
	        {
	            pDest[i*4] = (BYTE)(DWORD(pImage[i*4 + 2])*pImage[i*4 + 3]/255);
	            pDest[i*4 + 1] = (BYTE)(DWORD(pImage[i*4 + 1])*pImage[i*4 + 3]/255);
	            pDest[i*4 + 2] = (BYTE)(DWORD(pImage[i*4])*pImage[i*4 + 3]/255); 
	            bAlphaChannel = true;
	        } else
	        {
	            pDest[i*4] = pImage[i*4 + 2];
	            pDest[i*4 + 1] = pImage[i*4 + 1];
	            pDest[i*4 + 2] = pImage[i*4]; 
	        }

	        if (m_nMask && (*(DWORD*)(&pDest[i*4]) == m_nMask))
			{
	            pDest[i*4] = (BYTE)0;
	            pDest[i*4 + 1] = (BYTE)0;
	            pDest[i*4 + 2] = (BYTE)0; 
	            pDest[i*4 + 3] = (BYTE)0;
	            bAlphaChannel = true;
	        }
	    }
	}

    stbi_image_free(pImage);

    m_hBitmap = hBitmap;
    m_nWidth = x;
	m_nHeight = y;
	m_bAlphaChannel = bAlphaChannel;
	return TRUE;
}

BOOL CStdImage::GetAlphaChannel()
{
	return m_bAlphaChannel;
}


void CStdImage::SetImageMask(int nMask)
{
	m_nMask = nMask;
}

 //保存文件
BOOL CStdImage::SaveToFile(const char *FileName, DWORD image_type)
{
	return FALSE;
}

//保存文件并且修改尺寸
BOOL CStdImage::SaveToFile(const char* FileName, DWORD image_type, SIZE szNew)
{
	return FALSE;
}

//保存到数据流
BOOL CStdImage::SaveToStream(BYTE * &pBuff, long &nSize, DWORD image_type)
{
	return FALSE;
}

//保存至数据流，已分配内存
BOOL CStdImage::SaveToStream(BYTE *pBuff, DWORD &dwSize, DWORD image_type, BYTE byteQuality)
{
	return FALSE;
}

//从bitmap中载入
BOOL CStdImage::LoadFromBitmap(HBITMAP hBitmap, HPALETTE hPal)
{
	return FALSE;
}

//从DIB中载入
BOOL CStdImage::LoadFromDIB(char *pSrc, DWORD dwWidth, DWORD dwHeight, DWORD dwBitPerPixel, 
	              DWORD dwBytesPerLine, BOOL bFlipImage)
{
	return FALSE;
}

//从源图中载入
BOOL CStdImage::LoadFromGraphic(IImageInterface *pSrc)
{
	if (m_hBitmap)
		::DeleteObject(m_hBitmap);
	m_hBitmap = NULL;
	if (pSrc)
	{
		m_nWidth = pSrc->GetWidth();
		m_nHeight = pSrc->GetHeight();
		m_bAlphaChannel = pSrc->GetAlphaChannel();
		m_nMask = pSrc->GetMask();
		m_hBitmap = (HBITMAP)::CopyImage(pSrc->GetBitmap(), IMAGE_BITMAP, 0, 0, LR_COPYRETURNORG);
		if (m_hBitmap)
			return TRUE;
	}
	return FALSE;
}

int CStdImage::GetMask()
{
	return m_nMask;
}

//从Icon 中载入 暂时未实现
BOOL CStdImage::LoadFromIcon(HICON hIcon)
{
	return FALSE;
	/*if (m_hBitmap)
		::DeleteObject(m_hBitmap);
	m_hBitmap = NULL;
	if (hIcon)
		return FALSE;
 
	BOOL bResult = TRUE;
	ICONINFO iinfo;
	GetIconInfo(hIcon, &iinfo);

	BITMAP l_Bitmap;
	GetObject(iinfo.hbmColor, sizeof(BITMAP), &l_Bitmap);

	if(l_Bitmap.bmBitsPixel == 32)
	{
		BITMAPINFO l_BitmapInfo;
		l_BitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		l_BitmapInfo.bmiHeader.biWidth = l_Bitmap.bmWidth;
		l_BitmapInfo.bmiHeader.biHeight = l_Bitmap.bmHeight;
		l_BitmapInfo.bmiHeader.biPlanes = l_Bitmap.bmPlanes;
		l_BitmapInfo.bmiHeader.biBitCount = l_Bitmap.bmBitsPixel;
		l_BitmapInfo.bmiHeader.biCompression = BI_RGB;

		RGBQUAD *l_pRawBytes = new RGBQUAD[l_Bitmap.bmWidth * l_Bitmap.bmHeight];

		HDC dc = ::GetDC(NULL);

		if(dc)
		{
			if(GetDIBits(dc, iinfo.hbmColor, 0, l_Bitmap.bmHeight, l_pRawBytes, &l_BitmapInfo, DIB_RGB_COLORS))
				bResult = CreateFromArray((BYTE*)l_pRawBytes, l_Bitmap.bmWidth, l_Bitmap.bmHeight, l_Bitmap.bmBitsPixel, l_Bitmap.bmWidthBytes, false);
			else
				bResult = FALSE;

			::ReleaseDC(NULL, dc);
		}
		else
			bResult = FALSE;

		delete [] l_pRawBytes;
	}
	else
	{
		bResult = CreateFromHBITMAP(iinfo.hbmColor);
 
 
	}

	DeleteObject(iinfo.hbmColor); //<Sims>
	DeleteObject(iinfo.hbmMask);  //<Sims>
	
	return bResult;*/
}


//灰化图片
BOOL CStdImage::SetGray()
{
	if (m_hBitmap)
	{
		int nSize = m_nWidth * m_nHeight * 4;
		char *pBuf = new char[nSize];
		int l = ::GetBitmapBits(m_hBitmap, nSize, pBuf);
		BYTE b = 0;
		BYTE *p = (BYTE *) pBuf;
		for (int i = 0; i < m_nHeight; i ++)
		{
			for (int j = 0, x = 0; j < m_nWidth; j ++)
			{
				b = RGB2GRAY(*(p + x), *(p + x + 1), *(p + x + 2));
				*(p+x) = b;
				*(p + x + 1) = b;
				*(p + x + 2) = b;
				x += 4;
			}
			p += (m_nWidth * 4);
		}
		::SetBitmapBits(m_hBitmap, nSize, pBuf);
		delete []pBuf;

		return TRUE;
	}
	return FALSE;
}

//给图上色
BOOL CStdImage::TransImageStyle(int r, int g, int b)
{
	return FALSE;
}

//填充颜色
BOOL CStdImage::FillColorToImage(BYTE r, BYTE g, BYTE b)
{
	/*if (m_hBitmap)
	{
		float fNewH = 0.0f, fNewS = 0.0f, fNewL = 0.0f;
		float fH = 0.0f, fS = 0.0f, fL = 0.0f;
		RGBtoHSL(RGB(r, g, b), &fH, &fS, &fL);  
		float S1 = fS / 100.0f;
		float L1 = fL / 100.0f;
		HDC hDesktop = ::GetDC(::GetDesktopWindow());
		HDC hMem = ::CreateCompatibleDC(hDesktop);
		HBITMAP hOld = (HBITMAP)::SelectObject(hMem, m_hBitmap);
 
		COLORREF clr;
		for (int y = 0; y < m_nHeight; y ++)
		{
			for (int x = 0; x < m_nWidth; x ++)
			{
				clr = ::GetPixel(hMem, x, y);
				if ((clr != 0) || (m_bAlphaChannel))
				{ 
					RGBtoHSL(clr, &fNewH, &fNewS, &fNewL);
					//fNewH += (fH - 180);
					//fNewH = fNewH > 0 ? fNewH : fNewH + 360;
					//fNewS = S1;
					//fNewL = L1; 
					clr = HSLtoRGB(fNewH, fS, fL); 	
					::SetPixel(hMem, x, y, clr); 
				}
			}
		}
		::SelectObject(hMem, hOld);
		::DeleteObject(hMem);
		::ReleaseDC(::GetDesktopWindow(), hDesktop);
		return TRUE;
	}
	return FALSE;*/
	if (m_hBitmap)
	{
		float fNewH = 0.0f, fNewS = 0.0f, fNewL = 0.0f;
		float fH = 0.0f, fS = 0.0f, fL = 0.0f;
		int nSize = m_nWidth * m_nHeight * 4;
		char *pBuf = new char[nSize];
		int l = ::GetBitmapBits(m_hBitmap, nSize, pBuf);
		BYTE b = 0;
		BYTE *p = (BYTE *) pBuf;
		RGBtoHSL(RGB(r, g, b), &fH, &fS, &fL); 
		float S = fS / 100.0f;
		float L = fL / 100.0f;
		float NewS, NewL;
		COLORREF clR;
		for (int i = 0; i < m_nHeight; i ++)
		{
			for (int j = 0, x = 0; j < m_nWidth; j ++)
			{ 
				if ((!m_bAlphaChannel) || (*(p + x) != 0) || (*(p + x + 1) != 0) || (*(p + x + 2) != 0))
				{ 
					RGBtoHSL(RGB(*(p + x + 2), *(p + x + 1), *(p + x)), &fNewH, &fNewS, &fNewL);
					fNewH += (fH - 180);
					fNewH = fNewH > 0 ? fNewH : fNewH + 360;
					fNewS *= S;
					fNewL *= L; 
					clR = HSLtoRGB(fNewH, fNewS, fNewL); 
					*(p + x) = GetBValue(clR);
					*(p + x + 1) = GetGValue(clR);
					*(p + x + 2) = GetRValue(clR);
				} 
				x += 4;
			}
			p += (m_nWidth * 4);
		}
		::SetBitmapBits(m_hBitmap, nSize, pBuf);
		delete []pBuf;

		return TRUE;
	}
	 
	return FALSE;
}

//拷贝DIB数据
BOOL CStdImage::CopyImageDibData(char *pDib, DWORD &dwDibSize)
{
	return FALSE;
}

//
BOOL CStdImage::SetTransferColor(COLORREF clr)
{
	return FALSE;
}

//将图片画到指定dc
void CStdImage::DrawToDc(HDC dc,  int x, int y, int nWidth, int nHeight)
{
	RECT rc = {x, y, x + nWidth, y + nHeight};
	DrawToDc(dc, rc);
}

void CStdImage::DrawToDc(HDC dc, const RECT& rc )
{
	static LPALPHABLEND lpAlphaBlend = (LPALPHABLEND) ::GetProcAddress(::GetModuleHandle(_T("msimg32.dll")), "AlphaBlend");
	if (lpAlphaBlend)
	{
		RECT rcBmpPart = {0, 0, m_nWidth, m_nHeight};
		RECT rcNull = {0};
		DrawPlus(dc, rc, rc, rcBmpPart, rcNull, lpAlphaBlend);
	}
}

void CStdImage::TransparentDraw(HDC hdc,  int nXDest, int nYDest,  int nDestWidth, int nDestHeight, HDC hSrc, int nXSrc, 
	    int nYSrc, int nSrcWidth, int nSrcHeight, COLORREF crTransparent)
{
}

//颜色值转换 16位转256色

//将源图片的一部分画到指定dc上，必须制定要透去的背景色，
//可以拉伸
void CStdImage::TransparentDrawToDc(HDC hdc, 
						  int nXDest, 
						  int nYDest, 
						  int nDestWidth,
						  int nDestHeight,
						  int nXSrc,
						  int nYSrc,
						  int nSrcWidth,
						  int nSrcHeight,
						  COLORREF crTransparent)
{
}

//支持将原图片的一部分画到指定dc的制定区域，可以拉伸
void CStdImage::StretchDrawToDc( HDC hDestDc, 
					  int nDestX, 
					  int nDestY,
					  int nDestWidth, 
					  int nDestHeight,
					  int nSrcX, 
					  int nSrcY, 
					  int nSrcWidth, 
					  int nSrcHeight )
{
}

HBITMAP CStdImage::GetBitmap()
{
	return m_hBitmap;
}

BOOL CStdImage::IsEmpty() const
{
	return (m_hBitmap == NULL);
}

int  CStdImage::GetWidth() const
{
	return m_nWidth;
}

int  CStdImage::GetHeight() const
{
	return m_nHeight;
}

UINT CStdImage::GetGraphicType() const
{
	//
	return 0;
}

BOOL CStdImage::DrawPlus(HDC hDC, const RECT& rc, const RECT& rcPaint, 
        const RECT& rcBmpPart, const RECT& rcCorners, LPALPHABLEND lpAlphaBlend, BYTE uFade, 
        bool hole, bool xtiled, bool ytiled)
{
	return CSystemUtils::DrawImage(hDC, m_hBitmap, rc, rcPaint, rcBmpPart, rcCorners, 
		lpAlphaBlend, m_bAlphaChannel, uFade, hole, xtiled, ytiled);
}


