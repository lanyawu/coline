#include <CommonLib/GdiPlusImage.h>
#include <CommonLib/StringUtils.h>

#pragma warning(disable:4996)

ULONG_PTR CGdiPlusImage::m_pGdiplusToken = NULL;
LONG CGdiPlusImage::m_pTokenRef = 0;
std::map<UINT_PTR, CGdiPlusGif *> CGdiPlusGif::m_GifList;
//CSchedule CGdiPlusGif::m_Schedule;

// class CGdiPlusImage
CGdiPlusImage::CGdiPlusImage(void)
{
}

CGdiPlusImage::~CGdiPlusImage(void)
{
}

void CGdiPlusImage::InitGdiPlus()
{
	if (m_pGdiplusToken == NULL)
	{
		GdiplusStartupInput gdiplusStartupInput;
		GdiplusStartup(&m_pGdiplusToken, &gdiplusStartupInput, NULL);
	}
	::InterlockedIncrement(&m_pTokenRef);
}

void CGdiPlusImage::DestroyGdiPlus()
{
	::InterlockedDecrement(&m_pTokenRef);
	if (m_pTokenRef == 0)
	{
		GdiplusShutdown(m_pGdiplusToken);
		m_pGdiplusToken = NULL;
	}
}

int FitSize(Size& InSize, Size& ImageSize)
{
    float scaleX = (InSize.Width > 0) ? (float)InSize.Width/ImageSize.Width : 0;
    float scaleY = (InSize.Height > 0)? (float)InSize.Height/ImageSize.Height : 0;
    float scale = 1;
    if (scaleX && scaleY) 
        scale = min(scaleX, scaleY);
    else if ( scaleX||scaleY )
        scale = scaleX ? scaleX : scaleY;
    InSize.Width = (INT)(ImageSize.Width * scale);
    InSize.Height = (INT)(ImageSize.Height * scale);
    return (int)scale;
}

BOOL GetBitmapFromFile(Gdiplus::Bitmap* &pBitmap, const char *szFileName, int &nFrameCount, Size &sizeFrame, Size &sizeImage, UINT * &pFrameDelays, COLORREF &crBk)
{
#ifdef _UNICODE
	TCHAR szwFileName[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szFileName, szwFileName, MAX_PATH);
	Gdiplus::Bitmap *pTempBitmap = new Bitmap(szwFileName);
#else
	Gdiplus::Bitmap *pBitmap = new Bitmap(szFileName);
#endif
	if (!pTempBitmap)
		return NULL;
	GUID pageGuid = FrameDimensionTime;
	crBk = RGB(0, 0, 0);
	nFrameCount = max(1, pTempBitmap->GetFrameCount(&pageGuid));
	Size sizel(pTempBitmap->GetWidth(), pTempBitmap->GetHeight());
	sizeFrame = sizel;
	int fScale = FitSize(sizeFrame, sizel);
	pBitmap = new Bitmap(sizeFrame.Width * nFrameCount, sizeFrame.Height, PixelFormat32bppARGB);
	Graphics *g = new Graphics(pBitmap); 
	ImageAttributes attr;
 	if (fScale != 1)
	{
		g->SetInterpolationMode(InterpolationModeHighQualityBicubic);
		g->SetPixelOffsetMode(PixelOffsetModeHighQuality);
		if (fScale < 1)
			attr.SetGamma((REAL)1.2, ColorAdjustTypeBitmap); //some darker to made sharpen
	}
	g->Clear(Gdiplus::Color::White);
	
    if (nFrameCount > 1 )
    {
		pFrameDelays = new UINT[nFrameCount];
		int nSize = pTempBitmap->GetPropertyItemSize(PropertyTagFrameDelay);
		PropertyItem* pDelays = (PropertyItem*) new char[nSize];
		pTempBitmap->GetPropertyItem(PropertyTagFrameDelay, nSize, pDelays);
		for (int i = 0; i < nFrameCount; i ++)
		{
			pageGuid = FrameDimensionTime;
			pTempBitmap->SelectActiveFrame(&pageGuid, i);
			Rect rc( i * sizeFrame.Width, 0, sizeFrame.Width, sizeFrame.Height);
			if (fScale >= 1)
			{
				g->DrawImage(pTempBitmap, rc, 0, 0, pTempBitmap->GetWidth(), pTempBitmap->GetHeight(), UnitPixel, &attr);
			}else
			{
				Bitmap bm2(pTempBitmap->GetWidth(), pTempBitmap->GetHeight(), PixelFormat32bppARGB);
				Graphics g2(&bm2);
				g2.DrawImage(pTempBitmap, Rect(0, 0, bm2.GetWidth(), bm2.GetHeight()), 0, 0, pTempBitmap->GetWidth(),
					pTempBitmap->GetHeight(), UnitPixel);
				g->DrawImage(&bm2, rc, 0, 0, bm2.GetWidth(), bm2.GetHeight(), UnitPixel, &attr);
			}
			pFrameDelays[i] = 10 * max(((int *) pDelays->value)[i], 10);
		}
		delete [] pDelays;
    }
    else
    {
		Rect rc(0, 0, sizeFrame.Width, sizeFrame.Height);
		g->DrawImage(pTempBitmap, rc, 0, 0, pTempBitmap->GetWidth(), pTempBitmap->GetHeight(), UnitPixel, &attr);
		pFrameDelays = NULL;
    }
	sizeImage = Size(pTempBitmap->GetWidth(), pTempBitmap->GetHeight());
    delete g;
    delete pTempBitmap;
    return TRUE;
}

// class CGdiPlusGIF
CGdiPlusGif::CGdiPlusGif(IImageNotifyEvent *pNotify, const char *szFileName,  BOOL IsShowAnimate):
			 m_pNotify(pNotify),
			 m_TimerId(0),
			 m_nCurrPos(0),
			 m_bTerminated(FALSE),
			 m_pDelays(NULL),
			 m_hMem(NULL),
			 m_Bitmap(NULL),
			 m_bIsAlpha(FALSE),
			 m_crTransparent(0),
			 m_bTransparent(FALSE),
			 m_nFrameNum(1)
{
	Gdiplus::Bitmap *pBitmap = NULL;
	memset(m_szSrcFileName, 0, MAX_PATH);
	strncpy(m_szSrcFileName, szFileName, MAX_PATH - 1);
	GetBitmapFromFile(pBitmap, m_szSrcFileName, m_nFrameNum, m_sizeFrame, m_sizeImage, m_pDelays, m_crTransparent);

	if (pBitmap)
	{
		HDC hdc = ::GetDC(::GetDesktopWindow());
		m_Bitmap = ::CreateCompatibleBitmap(hdc, m_nFrameNum * m_sizeFrame.Width, m_sizeFrame.Height);
		m_hMem = ::CreateCompatibleDC(hdc);
		
		::SelectObject(m_hMem, m_Bitmap);
		Gdiplus::Graphics g(m_hMem); 
		g.DrawImage(pBitmap, 0, 0, m_nFrameNum * m_sizeFrame.Width, m_sizeFrame.Height);
	
        delete pBitmap;
		::ReleaseDC(::GetDesktopWindow(), hdc);
	}
	if ((m_nFrameNum > 1) && IsShowAnimate)
		OnTimer();
}

CGdiPlusGif::CGdiPlusGif(IImageNotifyEvent *pNotify, HBITMAP hBitmap):
             m_pNotify(pNotify),
			 m_TimerId(0),
			 m_nCurrPos(0),
			 m_bTerminated(FALSE),
			 m_pDelays(NULL),
			 m_hMem(NULL),
			 m_Bitmap(NULL),
			 m_bIsAlpha(FALSE),
			 m_crTransparent(0),
			 m_bTransparent(FALSE),
			 m_nFrameNum(1)
{
	Gdiplus::Bitmap *pBitmap = new Gdiplus::Bitmap(hBitmap, NULL);
	if (pBitmap)
	{
		HDC hdc = ::GetDC(::GetDesktopWindow());
		SIZE sz = {pBitmap->GetWidth(), pBitmap->GetHeight()};
		m_Bitmap = ::CreateCompatibleBitmap(hdc, sz.cx, sz.cy);
		m_hMem = ::CreateCompatibleDC(hdc);
		
		::SelectObject(m_hMem, m_Bitmap);
		Gdiplus::Graphics g(m_hMem);
		g.DrawImage(pBitmap, 0, 0, sz.cx, sz.cy);
        delete pBitmap;
		::ReleaseDC(::GetDesktopWindow(), hdc);
	}
}

CGdiPlusGif::~CGdiPlusGif()
{
	//m_Schedule.DeleteSchedule(m_TimerId);
	
	if (m_TimerId > 0)
	{
		m_GifList.erase(m_TimerId);
		::KillTimer(NULL, m_TimerId);
	}
	m_bTerminated = TRUE;
	if (m_hMem)
	{
		::DeleteObject(m_hMem);
		m_hMem = NULL;
	}
	if (m_Bitmap)
	{
		::DeleteObject(m_Bitmap);
		m_Bitmap = NULL;
	}
    if (m_pDelays)
	{
		delete []m_pDelays;
		m_pDelays = NULL;
	}
}

void CGdiPlusGif::OnTimer()
{
	m_TimerId = 0;
	m_nCurrPos = (m_nCurrPos + 1) % m_nFrameNum;
	if (m_pNotify)
		m_pNotify->OnInvalidate(&m_rcClient);
	//下一帧
	if (m_nFrameNum > 0)
	{
		m_TimerId = ::SetTimer(NULL, (UINT_PTR)this, m_pDelays[m_nCurrPos], TimerProc);
		m_GifList[m_TimerId] = this;
		//m_TimerId = m_Schedule.AddSchedule(PaintNextFrame, this, m_pDelays[m_nCurrPos], FALSE, 1);
	}
}

VOID CALLBACK CGdiPlusGif::TimerProc(HWND hwnd,UINT uMsg,UINT_PTR idEvent, DWORD dwTime)
{
	CGdiPlusGif *pThis = NULL;
	std::map<UINT_PTR, CGdiPlusGif *>::iterator it = m_GifList.find(idEvent);
	if (it != m_GifList.end())
	{ 
		::KillTimer(hwnd, idEvent);
		pThis = (*it).second;
		m_GifList.erase(it);
	}
	if (pThis)
		pThis->OnTimer();
}

int CGdiPlusGif::GetImageHeight()
{
	return m_sizeFrame.Height;
}

int CGdiPlusGif::GetImageWidth()
{
	return m_sizeFrame.Width;
}

void CGdiPlusGif::SetTransparent(BOOL bTransparent, COLORREF crTransColor)
{
	m_bTransparent = bTransparent;
	m_crTransparent = crTransColor;
}

void CGdiPlusGif::Paint(HDC hdc, const RECT &rc)
{ 
	RECT rcDst = rc;
	RECT rcSrc = {m_nCurrPos * m_sizeFrame.Width, 0, m_nCurrPos * m_sizeFrame.Width + m_sizeFrame.Width, m_sizeFrame.Height};
	int xLeft = 0 - rcDst.left;
	int yTop = 0 - rcDst.right;
	if (xLeft > 0)
	{
		rcDst.left = 0;
		rcSrc.left += xLeft;
	}
	if (yTop > 0)
	{
		rcDst.top = 0;
		rcSrc.top += yTop;
	}
	if (m_bTransparent)
	{
		//::SetBkColor(hdc, m_bkColor);
		//::SetBkMode(hdc, OPAQUE);
		::TransparentBlt(hdc, rcDst.left, rcDst.top, rcDst.right - rcDst.left, rcDst.bottom - rcDst.top, 
			m_hMem, rcSrc.left, rcSrc.top, rcSrc.right - rcSrc.left, rcSrc.bottom - rcSrc.top, m_crTransparent);		
	} else
	{
		::StretchBlt(hdc, rcDst.left, rcDst.top, rcDst.right - rcDst.left, rcDst.bottom - rcDst.top,
			m_hMem, rcSrc.left, rcSrc.top, rcSrc.right - rcSrc.left, rcSrc.bottom - rcSrc.top, SRCCOPY); 
	}
}

//复制到剪切板
void CGdiPlusGif::CopyToClipboard()
{
	HDC hDeskTop = ::GetDC(::GetDesktopWindow());
	HBITMAP hSelected = ::CreateCompatibleBitmap(hDeskTop, m_sizeFrame.Width, m_sizeFrame.Height);
	HDC hdc = ::CreateCompatibleDC(hDeskTop);
	HBITMAP hOld = (HBITMAP)::SelectObject(hdc, hSelected);
	RECT rc = {0, 0, m_sizeFrame.Width, m_sizeFrame.Height};
	Paint(hdc, rc);
	::OpenClipboard(::GetDesktopWindow());
	::EmptyClipboard(); //先清空剪切板
	::SetClipboardData(CF_BITMAP, hSelected);
	::CloseClipboard();
	::SelectObject(hdc, hOld);
	::DeleteObject(hdc);
	::DeleteObject(hSelected);
	::ReleaseDC(::GetDesktopWindow(), hDeskTop);
}

#pragma warning(default:4996)
