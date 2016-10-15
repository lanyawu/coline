#include <fstream>
#include <commonlib/debuglog.h>
#include <CommonLib/GraphicPlus.h>
#include <CommonLib/StringUtils.h>

#ifndef MAX
#define MAX max
#endif

#ifndef MIN
#define MIN min
#endif

#pragma warning(disable:4996)

 

CGraphicPlus::CGraphicPlus(void):
              m_pImage(NULL),
			  m_hBitmap(NULL)
{
}

CGraphicPlus::~CGraphicPlus(void)
{
	ClearImage();
}

BOOL  CGraphicPlus::LoadFromFile(const char *FileName, BOOL bGray)
{
	BOOL b = FALSE;
    if (FileName)
	{
		TCHAR szwFileName[MAX_PATH] = {0};
		CStringConversion::StringToWideChar(FileName, szwFileName, MAX_PATH - 1);
		ifstream ifs;
		ifs.open(szwFileName, std::ios::in | std::ios::binary);
		if (ifs.is_open())
		{
			ifs.seekg(0, std::ios::end);
			int nSize = ifs.tellg();
			ifs.seekg(0, std::ios::beg);
			char *pBuff = new char[nSize];
			ifs.read(pBuff, nSize);
			b = LoadFromBuff(pBuff, nSize, bGray);
			ifs.close();
			delete []pBuff;
		}
	}
	return b;
}

BOOL CGraphicPlus::LoadFromIcon(HICON hIcon)
{
	ClearImage();
	m_pImage = new CxImage(); 
	return m_pImage->CreateFromHICON(hIcon);
}

BOOL CGraphicPlus::SetGray()
{
	if (m_pImage)
	{
		return m_pImage->GrayScale(); 
	}
	return FALSE;
}

//hsl颜色空间到rgb空间的转换 
void HSL2RGB(float h, float s, float l, BYTE &r, BYTE &g, BYTE &b) 
{
	float fS = 0.0f, fL = 0.0f;
    r = 0;
	g = 0;
	b = 0;
	if ((h < 360) && (h >= 0) && (s <= 100) && (s >= 0) && (l <= 100)
		&& (l >= 0))
	{
		if (h <= 60)
		{
			r = 255;
			g = (BYTE)((float) 255 / 60 * h);
			b = 0;
		} else if (h <= 120)
		{
			r = (BYTE)(((255 - (float)255 / 60)) * (h - 60));
            g = 255;
			b = 0;
		} else if (h <= 180)
		{
			r = 0;
			g = 255;
			b = (BYTE)(((float) 255 / 60) * (h - 120));
		} else if (h <= 240)
		{
			r = 0;
			g = (BYTE)(255 - ((float)255 / 60) * (h - 180));
			b = 255;
		} else if (h <= 300)
		{
			r = (BYTE)(((float) 255 / 60) * (h - 240));
			g = 0;
			b = 255;
		} else 
		{
			r = 255;
			g = 0;
			b = (BYTE)(255 - ((float) 255 / 60) * (h - 300));
		}
		fS = abs(s - 100) / 100;
		r = (BYTE)(r - ((r - 128) * fS));
		g = (BYTE)(g - ((g - 128) * fS));
		b = (BYTE)(b - ((g - 128) * fS));
        fL = (fL - 50) / 50;
		if (fL > 0)
		{
			r = (BYTE)(r + ((255 - r) * fL));
			g = (BYTE)(g + ((255 - g) * fL));
			b = (BYTE)(b + ((255 - b) * fL));
		} else
		{
			r = (BYTE)(r + (r * fL));
			g = (BYTE)(g + (g * fL));
			b = (BYTE)(b + (b * fL));
		}
	}
}
 

/*------------------------------------------------------------------------------
               RGB空间到HSL空间的转换                                            */
void RGB2HSL(BYTE r, BYTE g, BYTE b, float &h, float &s, float l)
{
	float fDelta = 0.0f;
	float fMax = 0.0f, fMin = 0.0f;
	float fR = 0.0f, fG = 0.0f, fB = 0.0f, fH = 0.0f, fS = 0.0f, fL = 0.0f;
	fR = (float)r / 255;
	fG = (float)g / 255;
	fB = (float)b / 255;
	fMax = MAX(fR, MAX(fG, fB));
	fMin = MIN(fR, MIN(fG, fB));
	fL = (fMax + fMin) / 2;
	if ((fMax - fMin) < 0.001)
	{
		fS = 0;
		fH = 0;
	} else
	{
		if (fL < 0.5)
			fS = (fMax - fMin) / (fMax + fMin);
		else
			fS = (fMax - fMin) / (2 - fMax - fMin);
		fDelta = fMax - fMin;
		if (abs(fR - fMax) < 0.001)
			fH = (fG - fB) / fDelta;
		else if (abs(fG - fMax) < 0.001)
			fH = 2 + (fB - fR) / fDelta;
		else
			fH = 4 + (fR - fG) / fDelta;
		fH /= 6;
		if (fH < 0)
			fH += 1;
	}
    h = fH * 360;
	s = fS * 100;
	l = fL * 100;
}

//填充颜色
BOOL CGraphicPlus::FillColorToImage(BYTE r, BYTE g, BYTE b)
{
	if (m_pImage)
	{
		float fNewH = 0.0f, fNewS = 0.0f, fNewL = 0.0f;
		float fH = 0.0f, fS = 0.0f, fL = 0.0f;
		BYTE rNew = 0, gNew = 0, bNew = 0;
		RGBQUAD rgbTrans = {0};
		rgbTrans.rgbRed = r;
		rgbTrans.rgbGreen = g;
		rgbTrans.rgbBlue = b;
		RGBQUAD hslNew = CxImage::RGBtoHSL(rgbTrans);

		RGBQUAD rgb;
		RGBQUAD hsl;
		for (int y = 0; y < m_pImage->GetHeight(); y ++)
		{
			for (int x = 0; x < m_pImage->GetWidth(); x ++)
			{
				rgb = m_pImage->GetPixelColor(x, y);
				
				//m_pImage->SetPixelColor(x, y, rgb);
				hsl = CxImage::RGBtoHSL(rgb);
				hsl.rgbRed = hslNew.rgbRed;
				hsl.rgbGreen = hslNew.rgbGreen;
				/*if (r > 0)
					hsl.rgbRed = rgbTrans.rgbRed;
				if (g > 0)
					hsl.rgbGreen = rgbTrans.rgbGreen;
				if (b > 0)
					hsl.rgbBlue = rgbTrans.rgbBlue;*/
				rgb = CxImage::HSLtoRGB(hsl); 			
				m_pImage->SetPixelColor(x, y, rgb); 
			}
		}
		return TRUE;
	}
	return FALSE;
}

void CGraphicPlus::SetImageMask(int clr)
{
	//
}

//给图上色
BOOL CGraphicPlus::TransImageStyle(int r, int g, int b)
{
	if (m_pImage)
	{ 
		m_pImage->ShiftRGB(r, g, b);
		return TRUE;
	}
	return FALSE;
}

BOOL CGraphicPlus::DrawPlus(HDC hDc, const RECT& rc, const RECT& rcPaint, 
        const RECT& rcBmpPart, const RECT& rcCorners, LPALPHABLEND lpAlphaBlend, BYTE uFade, 
        bool hole, bool xtiled, bool ytiled)
{
	if (m_pImage)
		return CSystemUtils::DrawImage(hDc, GetBitmap(), rc, rcPaint, rcBmpPart, rcCorners, lpAlphaBlend,
		         m_pImage->IsTransparent(), uFade, hole, xtiled, ytiled);
	else
		return FALSE;
}

BOOL CGraphicPlus::MixImage(const CGraphicPlus *pSrc, BOOL bStretch)
{
	if (m_pImage)
	{
		m_pImage->Mix(*(pSrc->m_pImage), CxImage::OpAvg);
		return TRUE;
	}
	return FALSE;
}

//转换颜色 1, 2, 4, 8
BOOL CGraphicPlus::TranslateToBpp(const GraphicBppType nType)
{
	if (m_pImage)
	{
		PRINTDEBUGLOG(dtInfo, "before TranslateBpp Type:%d, DIB Size:%d", (DWORD)nType, m_pImage->GetSize());
		m_pImage->DecreaseBpp((DWORD)nType, false);
		PRINTDEBUGLOG(dtInfo, "after translateBpp Size:%d", m_pImage->GetSize());
	}
	return FALSE;
}

//从数据流中载入
BOOL  CGraphicPlus::LoadFromBuff(const char *lpBuff, DWORD dwSize, BOOL bGray)
{
	BOOL b = FALSE;
	if( lpBuff && (dwSize > 0))
	{
		ClearImage();
		m_pImage = new CxImage;
		try
		{
			b = m_pImage->Load((BYTE *)lpBuff, dwSize, 0);
			if (b && bGray)
				SetGray();
		} catch(...)
		{
			b = FALSE;
		}
		if (!b)
		{
			ClearImage();
		}
	}
	return b;
}

//从DIB中载入
BOOL CGraphicPlus::LoadFromDIB(char *pSrc, DWORD dwWidth, DWORD dwHeight, DWORD dwBitPerPixel, 
							   DWORD dwBytesPerLine, BOOL bFlipImage)
{
	if (pSrc)
	{
		ClearImage();
		m_pImage = new CxImage();
		if (m_pImage->CreateFromArray((BYTE *)pSrc, dwWidth, dwHeight, dwBitPerPixel, dwBytesPerLine, bFlipImage))
			return TRUE;
	}
	return FALSE;
}

int CGraphicPlus::GetFrameCount()
{
	if (m_pImage)
	{
		return m_pImage->GetFrame();
	}
	return 0;
}

int CGraphicPlus::GetFrameDelay()
{
	if (m_pImage)
	{
		return m_pImage->GetFrameDelay();
	}
	return 0xFFFFFFF;
}

//拷贝DIB数据
BOOL CGraphicPlus::CopyImageDibData(char *pDib, DWORD &dwDibSize)
{
	if (m_pImage)
	{
		if (m_pImage->GetSize() <= dwDibSize)
		{
			dwDibSize = m_pImage->GetSize();
			memmove(pDib, m_pImage->GetBits(), dwDibSize);
			return TRUE;
		} else
		{
			dwDibSize = m_pImage->GetSize();
			return FALSE;
		}
	}
	return FALSE;
}

BOOL CGraphicPlus::LoadFromBitmap(HBITMAP hBitmap, HPALETTE hPal)
{
    if (hBitmap)
	{
		ClearImage();
		m_pImage = new CxImage();
		m_pImage->CreateFromHBITMAP(hBitmap, hPal);
		return TRUE;
	}
	return FALSE;
}

int  CGraphicPlus::GetMask()
{
	return 0;
}

BOOL CGraphicPlus::GetAlphaChannel()
{
	return FALSE;
}

//从源图中载入
BOOL CGraphicPlus::LoadFromGraphic(IImageInterface *pSrc)
{
	if ((pSrc) && (!pSrc->IsEmpty()))
	{
		ClearImage();
		//m_pImage = new CxImage(*(pSrc->m_pImage));
		return TRUE;
	}
	return FALSE;
}

//保存到数据流
BOOL CGraphicPlus::SaveToStream(BYTE * &pBuff, long &nSize, DWORD image_type) 
{
	if (m_pImage)
	{
		if (image_type == GRAPHIC_TYPE_GIF)
		{
			m_pImage->SetFrame(1);
			m_pImage->DecreaseBpp(8, false);
		}
		BOOL bRet = m_pImage->Encode(pBuff, nSize, image_type);
		return bRet;
	}
	return FALSE;
}

//保存至数据流，已分配内存
BOOL CGraphicPlus::SaveToStream(BYTE *pBuff, DWORD &dwSize, DWORD image_type, BYTE byteQuality)
{
	if (m_pImage && pBuff)
	{
		if (image_type == GRAPHIC_TYPE_GIF)
		{
			m_pImage->SetFrame(1);
			m_pImage->DecreaseBpp(8, false);
		} else if (image_type == GRAPHIC_TYPE_JPG)
		{
			m_pImage->SetJpegQuality(byteQuality);
		}
		CxMemFile memfile;
		memfile.Open();
		BOOL bRet = m_pImage->Encode(&memfile, image_type);
		if (bRet)
		{
			if (dwSize >= memfile.Size())
			{
				dwSize = memfile.Size();
				memmove(pBuff, memfile.GetBuffer(false), dwSize);
			} else
			{
				PRINTDEBUGLOG(dtInfo, "buffer over, bufSize:%d mem size:%d", dwSize, memfile.Size());
				bRet = FALSE;
			}
		}
		memfile.Close();
		return bRet;
	}
	return FALSE;
}

BOOL CGraphicPlus::SaveToFile(const char *FileName, DWORD Image_Type)
{
	if (m_pImage && FileName != NULL)
	{
		FILE *fp = fopen(FileName, "w+b");
		if (fp)
		{
			if (Image_Type == GRAPHIC_TYPE_GIF)
			{
				m_pImage->SetFrame(1);
				m_pImage->DecreaseBpp(8, false);
			}
			CxMemFile memfile;
			memfile.Open();
			BOOL bRet = m_pImage->Encode(&memfile, Image_Type);
			if(bRet)
			{
				fwrite(memfile.GetBuffer(), 1, memfile.Size(), fp);
			}
			memfile.Close();
			fclose(fp);
			return bRet;
		}
	}
	return FALSE;
}


//保存文件并且修改尺寸
BOOL CGraphicPlus::SaveToFile(const char* FileName, DWORD image_type, SIZE szNew)
{
	if (szNew.cx == GetWidth() && szNew.cy == GetHeight())
	{
		return SaveToFile(FileName, image_type);
	} else
	{
		//destination dc and bitmap
		HDC hMemDesk = ::GetDC(::GetDesktopWindow());
		HDC hMemDst = ::CreateCompatibleDC(hMemDesk);
		HBITMAP hDstBmp = ::CreateCompatibleBitmap(hMemDesk, szNew.cx, szNew.cy);
		::SelectObject( hMemDst, hDstBmp );
		//copy the source bmp to destination bmp
		DrawToDc(hMemDst, 0, 0, szNew.cx, szNew.cy);
		//make image object depending on the new bmp
		CGraphicPlus graphic;
		graphic.LoadFromBitmap(hDstBmp);
		//save it
		BOOL bRet = graphic.SaveToFile(FileName, image_type);

		::DeleteObject( hDstBmp );

		::ReleaseDC(::GetDesktopWindow(), hMemDesk);
		::DeleteDC( hMemDst );

		return bRet;
	}
	return FALSE;
}

void CGraphicPlus::DrawToDc(HDC dc, const RECT& rc)
{
	if (m_pImage)
	{
		m_pImage->Draw(dc, rc);
	}
}

void CGraphicPlus::DrawToDc(HDC dc,  int x, int y, int nWidth, int nHeight)
{
	if (m_pImage)
	{
		RECT rc, cliprc;
		rc.left = x;
		rc.top = y;
		rc.right = x + nWidth;
		rc.bottom = y + nHeight;
        cliprc.left = x;
		cliprc.top = y;
		cliprc.right = rc.right;
		cliprc.bottom = rc.bottom;
		int iRet = m_pImage->Draw(dc, rc, &cliprc);
	}
}

void CGraphicPlus::StretchDrawToDc( HDC hDestDc, 
								    int nDestX, int nDestY,
									int nDestWidth, int nDestHeight,
									int nSrcX, int nSrcY, 
									int nSrcWidth, int nSrcHeight)
{
	if (IsEmpty())
		return; 
	/*CxImage clip;
	m_pImage->Crop(nSrcX, nSrcY, nSrcWidth + nSrcX, nSrcHeight + nSrcY, &clip); 
	clip.SetTransColor(m_pImage->GetTransColor());
	clip.AlphaCopy(*m_pImage);
	clip.AlphaFromTransparency();
	clip.Draw2(hDestDc, nDestX, nDestY, nDestWidth + nDestX, nDestHeight + nDestY);*/
	RECT rcSrc = {nSrcX, nSrcY, nSrcX + nSrcWidth, nSrcY + nSrcHeight};
	RGBQUAD rgb = m_pImage->GetTransColor(); 
	COLORREF clr = RGB(rgb.rgbRed, rgb.rgbGreen, rgb.rgbBlue);
	m_pImage->DrawClip(hDestDc, nDestX, nDestY, nDestWidth, nDestHeight, rcSrc, clr);
	/*BYTE* pImage = m_pImage->GetBits();
	BITMAPINFO* pBmpInfo = (BITMAPINFO *)m_pImage->GetDIB();
	::StretchDIBits( hDestDc, 
					 nDestX, nDestY, nDestWidth, nDestHeight, 
					 nSrcX, nSrcY, nSrcWidth, nSrcHeight, 
					 pImage,
					 pBmpInfo, 
					 DIB_RGB_COLORS, SRCCOPY );*/
}

void CGraphicPlus::TransparentDraw(HDC hdc,  int nXDest, int nYDest,  int nDestWidth, int nDestHeight, HDC hSrc, int nXSrc, 
		    int nYSrc, int nSrcWidth, int nSrcHeight, COLORREF crTransparent)
{
	//创建掩码位图和dc
	HBITMAP hMaskBmp = ::CreateBitmap( nDestWidth, nDestHeight, 1, 1, NULL );
	HDC hMemMaskDC = ::CreateCompatibleDC( hdc );
	HBITMAP hOldMaskBmp = (HBITMAP)::SelectObject( hMemMaskDC, hMaskBmp );
	//创建兼容位图和dc
	HBITMAP hImgBmp = ::CreateCompatibleBitmap( hdc, nDestWidth, nDestHeight );
	HDC hMemImgDC = ::CreateCompatibleDC( hdc );
	HBITMAP hOldImgBmp = (HBITMAP)::SelectObject( hMemImgDC, hImgBmp );
	//将源位图copy到兼容dc中
	::StretchBlt(hMemImgDC, nXDest, nYDest, nDestWidth, nDestHeight, hSrc, nXSrc, nYSrc, nSrcWidth, nSrcHeight, SRCCOPY);
	//设置兼容dc的背景色
	::SetBkColor( hMemImgDC, crTransparent );
	//生成透明区域为白色，其它区域为黑色的掩码位图
	::BitBlt( hMemMaskDC, 0, 0, nDestWidth, nDestHeight, hMemImgDC, 0, 0, SRCCOPY );
	//生成透明区域为黑色，其它区域保持不变的位图
	::SetBkColor( hMemImgDC, RGB(0,0,0) );
	::SetTextColor( hMemImgDC, RGB(255,255,255) );
	::BitBlt( hMemImgDC, 0, 0, nDestWidth, nDestHeight, hMemMaskDC, 0, 0, SRCAND );
	//透明部分保持屏幕不变，其它部分变成黑色
	::SetBkColor( hdc, RGB(0xff,0xff,0xff) );
	::SetTextColor( hdc, RGB(0,0,0) );
	::BitBlt( hdc, nXDest, nYDest, nDestWidth, nDestHeight, 
				hMemMaskDC, 0, 0, SRCAND );
	//"或"运算,生成最终效果
	::BitBlt( hdc, nXDest, nYDest, nDestWidth, nDestHeight,
		hMemImgDC, 0, 0, SRCPAINT );

	::SelectObject(hMemImgDC, hOldImgBmp);
	::DeleteDC(hMemImgDC);
	::SelectObject(hMemMaskDC, hOldMaskBmp);
	::DeleteDC(hMemMaskDC);
	::DeleteObject(hImgBmp);
	::DeleteObject(hMaskBmp);
	
}

//
BOOL CGraphicPlus::SetTransferColor(COLORREF clr)
{
	if (!IsEmpty())
	{
		RGBQUAD rgb = {0};
		rgb.rgbRed = GetRValue(clr);
		rgb.rgbGreen = GetGValue(clr);
		rgb.rgbBlue = GetBValue(clr);
		m_pImage->SetTransColor(rgb); 
		return TRUE;
	}
	return FALSE;
}

void CGraphicPlus::TransparentDrawToDc(HDC hdc, 
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
	if( IsEmpty() )
		return;  
	// 将源DC中的位图拷贝到临时DC中,源DC已经载入位图
	 
	RECT rcSrc = {nXSrc, nYSrc, nXSrc + nSrcWidth, nYSrc + nSrcHeight};
	m_pImage->DrawClip2(hdc, nXDest, nYDest, nDestWidth, nDestHeight, rcSrc, crTransparent); 
}

BOOL CGraphicPlus::IsEmpty() const
{
	if (m_pImage)
		return FALSE;
	return TRUE;
}

HBITMAP CGraphicPlus::GetBitmap()
{
	if (m_hBitmap == NULL)
	{
		if( m_pImage )
		{
			m_hBitmap = m_pImage->MakeBitmap();
		}
	}
	return m_hBitmap;
}


void CGraphicPlus::ClearImage()
{
	if (m_hBitmap)
	{
		::DeleteObject(m_hBitmap);
		m_hBitmap = NULL;
	}
	if (m_pImage)
	{
		delete m_pImage;
		m_pImage = NULL;
	}
}

int  CGraphicPlus::GetWidth() const
{
	if( m_pImage )
		return m_pImage->GetWidth();
	return 0;
}

int  CGraphicPlus::GetHeight() const
{
	if( m_pImage )
		return m_pImage->GetHeight();
	return 0;
}

UINT CGraphicPlus::GetGraphicType() const
{
	if( m_pImage )
		return m_pImage->GetType();
	return GRAHPIC_TYPE_UNKNOWN_;
}

#pragma warning(default:4996)
