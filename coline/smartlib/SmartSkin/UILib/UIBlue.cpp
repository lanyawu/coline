#include "common.h"

#include <UILib/UIBlue.h>
#include <UILib/UIResource.h>

/////////////////////////////////////////////////////////////////////////////////////
//
//
PGradientFill CBlueRenderEngineUI::m_lpGradientFill = NULL;

#ifndef BlendRGB
   #define BlendRGB(c1, c2, factor) \
      RGB( GetRValue(c1) + ((GetRValue(c2) - GetRValue(c1)) * factor / 100L), \
           GetGValue(c1) + ((GetGValue(c2) - GetGValue(c1)) * factor / 100L), \
           GetBValue(c1) + ((GetBValue(c2) - GetBValue(c1)) * factor / 100L) )
#endif


static COLORREF PixelAlpha(COLORREF clrSrc, double src_darken, COLORREF clrDest, double dest_darken)
{
    return RGB (GetRValue (clrSrc) * src_darken + GetRValue (clrDest) * dest_darken, 
        GetGValue (clrSrc) * src_darken + GetGValue (clrDest) * dest_darken, 
        GetBValue (clrSrc) * src_darken + GetBValue (clrDest) * dest_darken);

}

static BOOL WINAPI AlphaBitBlt(HDC hDC, int nDestX, int nDestY, int dwWidth, int dwHeight, HDC hSrcDC, \
                        int nSrcX, int nSrcY, int wSrc, int hSrc, BLENDFUNCTION ftn)
{
    HDC hTempDC = ::CreateCompatibleDC(hDC);
    if (NULL == hTempDC)
        return FALSE;

    //Creates Source DIB
    LPBITMAPINFO lpbiSrc = NULL;
    // Fill in the BITMAPINFOHEADER
    lpbiSrc = (LPBITMAPINFO) new BYTE[sizeof(BITMAPINFOHEADER)];
	if (lpbiSrc == NULL)
	{
		::DeleteDC(hTempDC);
		return FALSE;
	}
    lpbiSrc->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    lpbiSrc->bmiHeader.biWidth = dwWidth;
    lpbiSrc->bmiHeader.biHeight = dwHeight;
    lpbiSrc->bmiHeader.biPlanes = 1;
    lpbiSrc->bmiHeader.biBitCount = 32;
    lpbiSrc->bmiHeader.biCompression = BI_RGB;
    lpbiSrc->bmiHeader.biSizeImage = dwWidth * dwHeight;
    lpbiSrc->bmiHeader.biXPelsPerMeter = 0;
    lpbiSrc->bmiHeader.biYPelsPerMeter = 0;
    lpbiSrc->bmiHeader.biClrUsed = 0;
    lpbiSrc->bmiHeader.biClrImportant = 0;

    COLORREF* pSrcBits = NULL;
    HBITMAP hSrcDib = CreateDIBSection (
        hSrcDC, lpbiSrc, DIB_RGB_COLORS, (void **)&pSrcBits,
        NULL, NULL);

    if ((NULL == hSrcDib) || (NULL == pSrcBits)) 
    {
		delete [] lpbiSrc;
        ::DeleteDC(hTempDC);
        return FALSE;
    }

    HBITMAP hOldTempBmp = (HBITMAP)::SelectObject (hTempDC, hSrcDib);
    ::StretchBlt(hTempDC, 0, 0, dwWidth, dwHeight, hSrcDC, nSrcX, nSrcY, wSrc, hSrc, SRCCOPY);
    ::SelectObject (hTempDC, hOldTempBmp);

    //Creates Destination DIB
    LPBITMAPINFO lpbiDest = NULL;
    // Fill in the BITMAPINFOHEADER
    lpbiDest = (LPBITMAPINFO) new BYTE[sizeof(BITMAPINFOHEADER)];
	if (lpbiDest == NULL)
	{
        delete [] lpbiSrc;
        ::DeleteObject(hSrcDib);
        ::DeleteDC(hTempDC);
        return FALSE;
	}

    lpbiDest->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    lpbiDest->bmiHeader.biWidth = dwWidth;
    lpbiDest->bmiHeader.biHeight = dwHeight;
    lpbiDest->bmiHeader.biPlanes = 1;
    lpbiDest->bmiHeader.biBitCount = 32;
    lpbiDest->bmiHeader.biCompression = BI_RGB;
    lpbiDest->bmiHeader.biSizeImage = dwWidth * dwHeight;
    lpbiDest->bmiHeader.biXPelsPerMeter = 0;
    lpbiDest->bmiHeader.biYPelsPerMeter = 0;
    lpbiDest->bmiHeader.biClrUsed = 0;
    lpbiDest->bmiHeader.biClrImportant = 0;

    COLORREF* pDestBits = NULL;
    HBITMAP hDestDib = CreateDIBSection (
        hDC, lpbiDest, DIB_RGB_COLORS, (void **)&pDestBits,
        NULL, NULL);

    if ((NULL == hDestDib) || (NULL == pDestBits))
    {
        delete [] lpbiSrc;
        ::DeleteObject(hSrcDib);
        ::DeleteDC(hTempDC);
        return FALSE;
    }

    ::SelectObject (hTempDC, hDestDib);
    ::BitBlt (hTempDC, 0, 0, dwWidth, dwHeight, hDC, nDestX, nDestY, SRCCOPY);
    ::SelectObject (hTempDC, hOldTempBmp);

    double src_darken;
    BYTE nAlpha;

    for (int pixel = 0; pixel < dwWidth * dwHeight; pixel++, pSrcBits++, pDestBits++)
    {
        nAlpha = LOBYTE(*pSrcBits >> 24);
        src_darken = (double) (nAlpha * ftn.SourceConstantAlpha) / 255.0 / 255.0;
        if( src_darken < 0.0 ) src_darken = 0.0;
        *pDestBits = PixelAlpha(*pSrcBits, src_darken, *pDestBits, 1.0 - src_darken);
    } //for

    ::SelectObject (hTempDC, hDestDib);
    ::BitBlt (hDC, nDestX, nDestY, dwWidth, dwHeight, hTempDC, 0, 0, SRCCOPY);
    ::SelectObject (hTempDC, hOldTempBmp);

    delete [] lpbiDest;
    ::DeleteObject(hDestDib);

    delete [] lpbiSrc;
    ::DeleteObject(hSrcDib);

    ::DeleteDC(hTempDC);
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

CRenderClip::~CRenderClip()
{
	ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
	ASSERT(::GetObjectType(hRgn)==OBJ_REGION);
	ASSERT(::GetObjectType(hOldRgn)==OBJ_REGION);
	::SelectClipRgn(hDC, hOldRgn);
	::DeleteObject(hOldRgn);
	::DeleteObject(hRgn);
}

/////////////////////////////////////////////////////////////////////////////////////
//
//
StretchFixed::StretchFixed(UINT nFixed)
{
	SetFixed( nFixed );
}

void StretchFixed::SetFixed(UINT nFixed)
{
	m_iTopLeftWidth = nFixed;
	m_iTopHeight = nFixed;
	m_iTopRightWidth = nFixed;
	m_iBotLeftWidth = nFixed;
	m_iBotHeight = nFixed;
	m_iBotRightWidth = nFixed;
	m_iCenterLeftWidth = nFixed;
	m_iCenterRightWidth = nFixed;
}
////////////////////////////////////////////////////////////////////////////////
#define  HSLMAX   255	/* H,L, and S vary over 0-HSLMAX */
#define  RGBMAX   255   /* R,G, and B vary over 0-RGBMAX */
                        /* HSLMAX BEST IF DIVISIBLE BY 6 */
                        /* RGBMAX, HSLMAX must each fit in a BYTE. */
/* Hue is undefined if Saturation is 0 (grey-scale) */
/* This value determines where the Hue scrollbar is */
/* initially set for achromatic colors */
#define HSLUNDEFINED (HSLMAX*2/3)

RGBQUAD CBlueRenderEngineUI::RGBtoHSL(RGBQUAD lRGBColor)
{
	BYTE R,G,B;					/* input RGB values */
	BYTE H,L,S;					/* output HSL values */
	BYTE cMax,cMin;				/* max and min RGB values */
	WORD Rdelta,Gdelta,Bdelta;	/* intermediate value: % of spread from max*/

	R = lRGBColor.rgbRed;	/* get R, G, and B out of DWORD */
	G = lRGBColor.rgbGreen;
	B = lRGBColor.rgbBlue;

	cMax = max( max(R,G), B);	/* calculate lightness */
	cMin = min( min(R,G), B);
	L = (BYTE)((((cMax+cMin)*HSLMAX)+RGBMAX)/(2*RGBMAX));

	if (cMax==cMin){			/* r=g=b --> achromatic case */
		S = 0;					/* saturation */
		H = HSLUNDEFINED;		/* hue */
	} else {					/* chromatic case */
		if (L <= (HSLMAX/2))	/* saturation */
			S = (BYTE)((((cMax-cMin)*HSLMAX)+((cMax+cMin)/2))/(cMax+cMin));
		else
			S = (BYTE)((((cMax-cMin)*HSLMAX)+((2*RGBMAX-cMax-cMin)/2))/(2*RGBMAX-cMax-cMin));
		/* hue */
		Rdelta = (WORD)((((cMax-R)*(HSLMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin));
		Gdelta = (WORD)((((cMax-G)*(HSLMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin));
		Bdelta = (WORD)((((cMax-B)*(HSLMAX/6)) + ((cMax-cMin)/2) ) / (cMax-cMin));

		if (R == cMax)
			H = (BYTE)(Bdelta - Gdelta);
		else if (G == cMax)
			H = (BYTE)((HSLMAX/3) + Rdelta - Bdelta);
		else /* B == cMax */
			H = (BYTE)(((2*HSLMAX)/3) + Gdelta - Rdelta);

//		if (H < 0) H += HSLMAX;     //always false
		if (H > HSLMAX) H -= HSLMAX;
	}
	RGBQUAD hsl={L,S,H,0};
	return hsl;
}
////////////////////////////////////////////////////////////////////////////////
float Hue2RGB(float n1,float n2, float hue)
{
	//<F. Livraghi> fixed implementation for HSL2RGB routine
	float rValue;

	if (hue > 360)
		hue = hue - 360;
	else if (hue < 0)
		hue = hue + 360;

	if (hue < 60)
		rValue = n1 + (n2-n1)*hue/60.0f;
	else if (hue < 180)
		rValue = n2;
	else if (hue < 240)
		rValue = n1+(n2-n1)*(240-hue)/60;
	else
		rValue = n1;

	return rValue;
}
 
  
 
RGBQUAD CBlueRenderEngineUI::HSLtoRGB(RGBQUAD lHSLColor) 
{
//<F. Livraghi> fixed implementation for HSL2RGB routine
	float h,s,l;
	float m1,m2;
	BYTE r,g,b;

	h = (float)lHSLColor.rgbRed * 360.0f/255.0f;
	s = (float)lHSLColor.rgbGreen/255.0f;
	l = (float)lHSLColor.rgbBlue/255.0f;

	if (l <= 0.5)	m2 = l * (1+s);
	else			m2 = l + s - l*s;

	m1 = 2 * l - m2;

	if (s == 0) {
		r=g=b=(BYTE)(l*255.0f);
	} else {
		r = (BYTE)(Hue2RGB(m1,m2,h+120) * 255.0f);
		g = (BYTE)(Hue2RGB(m1,m2,h) * 255.0f);
		b = (BYTE)(Hue2RGB(m1,m2,h-120) * 255.0f);
	}

	RGBQUAD rgb = {b,g,r,0};
	return rgb;
}

RGBQUAD RGBtoRGBQUAD(COLORREF cr)
{
	RGBQUAD c;
	c.rgbRed = GetRValue(cr);	/* get R, G, and B out of DWORD */
	c.rgbGreen = GetGValue(cr);
	c.rgbBlue = GetBValue(cr);
	c.rgbReserved=0;
	return c;
}
 
COLORREF CBlueRenderEngineUI::AdjustColor(COLORREF clr, const short H, const short S, const short L)
{
    if (H == 180 && S == 100 && L == 100 ) 
		return clr; 
    float S1 = S / 100.0f;
    float L1 = L / 100.0f;
	float S2, L2;
    RGBQUAD hsl = RGBtoHSL(RGBtoRGBQUAD(clr));
	hsl.rgbRed += (H - 180);
    hsl.rgbRed = hsl.rgbRed > 0 ? hsl.rgbRed : hsl.rgbRed + 360;
	S2 = hsl.rgbGreen;
	L2 = hsl.rgbBlue;
	S2 *= S1;
	L2 *= L1;
	hsl.rgbGreen = S2;
	hsl.rgbBlue = L2;
    hsl = HSLtoRGB(hsl);
	return RGB(hsl.rgbRed, hsl.rgbGreen, hsl.rgbBlue);
}

void CBlueRenderEngineUI::DrawColor(HDC hDC, const RECT& rc, DWORD color)
{
    if( color <= 0x00FFFFFF ) return;
    if( color >= 0xFF000000 )
    {
        ::SetBkColor(hDC, RGB(GetBValue(color), GetGValue(color), GetRValue(color)));
        ::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
    }
    else
    {
		static LPALPHABLEND lpAlphaBlend = (LPALPHABLEND) ::GetProcAddress(::GetModuleHandle(_T("msimg32.dll")), "AlphaBlend");
		
		if ( lpAlphaBlend == NULL )
			lpAlphaBlend = AlphaBitBlt;
        // Create a new 32bpp bitmap with room for an alpha channel
        BITMAPINFO bmi = { 0 };
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = 1;
        bmi.bmiHeader.biHeight = 1;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        bmi.bmiHeader.biSizeImage = 1 * 1 * sizeof(DWORD);
        LPDWORD pDest = NULL;
        HBITMAP hBitmap = ::CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, (LPVOID*) &pDest, NULL, 0);
        if( !hBitmap ) return;

        *pDest = color;

        RECT rcBmpPart = {0, 0, 1, 1};
        RECT rcCorners = {0};
        CSystemUtils::DrawImage(hDC, hBitmap, rc, rc, rcBmpPart, rcCorners, lpAlphaBlend,
			TRUE, 255, false, false, false);
        ::DeleteObject(hBitmap);
    }
}
/*------------------------------------------------------------------------------
               RGB空间到HSL空间的转换                                            */
 

/////////////////////////////////////////////////////////////////////////////////////
//
//

void CBlueRenderEngineUI::GenerateClip(HDC hDC, RECT rcItem, CRenderClip& clip)
{
	RECT rcClip = { 0 };
	::GetClipBox(hDC, &rcClip);
	clip.hOldRgn = ::CreateRectRgnIndirect(&rcClip);
	clip.hRgn = ::CreateRectRgnIndirect(&rcItem);
	::ExtSelectClipRgn(hDC, clip.hRgn, RGN_AND);
	clip.hDC = hDC;
	clip.rcItem = rcItem;
}

void CBlueRenderEngineUI::DoFillRect(HDC hDC, CPaintManagerUI* pManager, RECT rcItem, UITYPE_COLOR Color, BOOL bTransparent)
{
	DoFillRect(hDC, pManager, rcItem, pManager->GetThemeColor(Color), bTransparent);
}

void CBlueRenderEngineUI::DoFillRect(HDC hDC, CPaintManagerUI* pManager, RECT rcItem, COLORREF clrFill, BOOL bTransparent)
{
	ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
	::SetBkColor(hDC, clrFill); 
	if (bTransparent)
		::SetBkMode(hDC, TRANSPARENT);
	::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rcItem, NULL, 0, NULL);
}

void CBlueRenderEngineUI::DoPaintLine(HDC hDC, CPaintManagerUI* pManager, RECT rcItem, UITYPE_COLOR Color)
{
	ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
	POINT ptTemp = { 0 };
	::SelectObject(hDC, pManager->GetThemePen(Color));
	::MoveToEx(hDC, rcItem.left, rcItem.top, &ptTemp);
	::LineTo(hDC, rcItem.right, rcItem.bottom);
}


void CBlueRenderEngineUI::DoPaintLine(HDC hDC,	const RECT& rc,	COLORREF clr)
{
	ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
	POINT ptTmp;
	HPEN hPen = ::CreatePen(PS_SOLID, 1, clr);
	HPEN hOldPen = (HPEN)::SelectObject(hDC, hPen);
	::MoveToEx(hDC, rc.left, rc.top, &ptTmp);
	::LineTo(hDC, rc.right, rc.bottom);
	::SelectObject(hDC, hOldPen);
	::DeleteObject(hPen);
}

//
void CBlueRenderEngineUI::DoPaintWidthLine(HDC hDC, const RECT &rc, COLORREF clr, int nWidth, BOOL bTransparent)
{
	ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
	POINT ptTmp;
	HPEN hPen = ::CreatePen(PS_SOLID, nWidth, clr);
	HPEN hOldPen = (HPEN)::SelectObject(hDC, hPen);
	::MoveToEx(hDC, rc.left, rc.top, &ptTmp);
	::LineTo(hDC, rc.right, rc.bottom);
	::SelectObject(hDC, hOldPen);
	::DeleteObject(hPen);
}

void CBlueRenderEngineUI::DoPaintRectangle(HDC hDC, CPaintManagerUI* pManager, 	RECT rcItem,
	                        UITYPE_COLOR Border, UITYPE_COLOR Fill)
{
	ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
	::SelectObject(hDC, pManager->GetThemePen(Border));
	::SelectObject(hDC, Fill == UICOLOR__INVALID ? ::GetStockObject(HOLLOW_BRUSH) : pManager->GetThemeBrush(Fill));
	::Rectangle(hDC, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
}

void CBlueRenderEngineUI::DoPaintRectangle(HDC hDC,  const RECT& rc,  COLORREF clrBorder,  COLORREF clrFill)
{
	ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
	HPEN hPen = ::CreatePen(PS_SOLID, 1, clrBorder);
	HPEN hOldPen = (HPEN)::SelectObject(hDC, hPen);
	HBRUSH hBrush = ::CreateSolidBrush(clrFill);
	HBRUSH hOldBrush = (HBRUSH)::SelectObject(hDC, hBrush);
	::Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);
	::DeleteObject(hPen);
	::DeleteObject(hBrush);
}

void CBlueRenderEngineUI::DoPaintRoundRect(HDC hDC, CPaintManagerUI* pManager, const RECT& rcItem,	
	                                       UITYPE_COLOR border,	UITYPE_COLOR fill,	const SIZE& corner)
{
	ASSERT( ::GetObjectType(hDC) == OBJ_DC || ::GetObjectType(hDC) == OBJ_MEMDC );
	::SelectObject(hDC, pManager->GetThemePen(border));
	::SelectObject(hDC, fill == UICOLOR__INVALID ? ::GetStockObject(HOLLOW_BRUSH) : pManager->GetThemeBrush(fill));
	::RoundRect(hDC, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom,
		corner.cx, corner.cy);
}

void CBlueRenderEngineUI::DoPaintRoundRect(HDC hDC,	const RECT& rc,	COLORREF clrBorder,	COLORREF clrFill,	const SIZE& corner)
{
	ASSERT(::GetObjectType(hDC) == OBJ_DC || ::GetObjectType(hDC) == OBJ_MEMDC);
	HPEN hPen = ::CreatePen(PS_SOLID, 1, clrBorder);
	HPEN hOldPen = (HPEN)::SelectObject(hDC, hPen);
	HBRUSH hBrush = ::CreateSolidBrush(clrFill);
	HBRUSH hOldBrush = (HBRUSH)::SelectObject(hDC, hBrush);
	::RoundRect(hDC, rc.left, rc.top, rc.right, rc.bottom, corner.cx, corner.cy);
	::SelectObject(hDC, hOldPen);
	::SelectObject(hDC, hOldBrush);
	::DeleteObject(hPen);
	::DeleteObject(hBrush); 
}

void CBlueRenderEngineUI::DoPaintPanel(HDC hDC, CPaintManagerUI* pManager, 	RECT rcItem)
{
	DoPaintFrame(hDC, pManager, rcItem, UICOLOR_TITLE_BORDER_LIGHT, UICOLOR_TITLE_BORDER_DARK, UICOLOR_TITLE_BACKGROUND);
}

void CBlueRenderEngineUI::DoPaintFrame(HDC hDC, CPaintManagerUI* pManager, RECT rcItem, 
	                                  UITYPE_COLOR Light, UITYPE_COLOR Dark, UITYPE_COLOR Background, UINT uStyle)
{
	if ((uStyle & UIFRAME_ROUND) != 0)
	{
		DoPaintRoundRect(hDC, pManager, rcItem, Light, Background, CSize(4,4));
	} else if ((uStyle & UIFRAME_FOCUS) != 0)
	{
	} else
	{
		DoPaintRectangle(hDC, pManager, rcItem, Light, Background);
	}
}

//绘制按钮文字，不带边框
void CBlueRenderEngineUI::DoPaintButtonText(HDC hDC, CPaintManagerUI *pManager, RECT rc, LPCTSTR pstrText,
		UINT uState, UINT uDrawStyle)
{ 
	COLORREF clrText;
	UITYPE_COLOR clrBack; 
	if ((uState & UISTATE_DISABLED) != 0) 
	{
		clrText = UICOLOR_BUTTON_TEXT_DISABLED;
		clrBack = UICOLOR_TOOL_BACKGROUND_DISABLED;
	} else if ((uState & UISTATE_PUSHED) != 0)
	{ 
		clrText = UICOLOR_BUTTON_TEXT_PUSHED;
		clrBack = UICOLOR_TOOL_BACKGROUND_PUSHED;
	} else 
	{
		clrText = UICOLOR_BUTTON_TEXT_NORMAL; 
		clrBack = UICOLOR_TOOL_BACKGROUND_NORMAL;
	}
	int nLinks = 0;
    // Gradient background
	COLORREF clrColor1, clrColor2;
	pManager->GetThemeColorPair(clrBack, clrColor1, clrColor2);
	DoPaintGradient(hDC, pManager, rc, clrColor1, clrColor2, true, 32);
	::DrawText(hDC, pstrText, ::lstrlen(pstrText), &rc, DT_SINGLELINE | uDrawStyle);  
}

	//绘制双图像菜单
void CBlueRenderEngineUI::DoPaintPlusMenuButton(HDC hDC, CPaintManagerUI *pManager, RECT rc, LPCTSTR pstrText,
		const SIZE &szPadding, UINT uState, UINT uDrawStyle, UINT uBkgImageId, UINT uImageId, UINT uSubIdx, UINT uArrowImage,
		BOOL bTransparent)
{
	ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
	UITYPE_COLOR clrText, clrBack;
 
	if (uBkgImageId > 0)
	{
		int iSubIndex = 0;
		if ((uState & UISTATE_DISABLED) != 0)
		{
			iSubIndex = 3;//button disabled
			clrText = UICOLOR_BUTTON_TEXT_DISABLED;
			clrBack = UICOLOR_BUTTON_BACKGROUND_DISABLED;
		} else if ((uState & UISTATE_PUSHED ) != 0)
		{
			iSubIndex = 2;//button pushed
			clrText = UICOLOR_BUTTON_TEXT_PUSHED;
			clrBack = UICOLOR_BUTTON_BACKGROUND_PUSHED;
		} else if ((uState & UISTATE_HOT) != 0)
		{
			iSubIndex = 1;//hot
			clrText = UICOLOR_BUTTON_TEXT_NORMAL;
			clrBack = UICOLOR_BUTTON_BACKGROUND_NORMAL;
		} else
		{
			iSubIndex = 0;//normal state
			clrText = UICOLOR_BUTTON_TEXT_NORMAL;
			clrBack = UICOLOR_BUTTON_BACKGROUND_NORMAL;
		}
		DoPaintGraphic(hDC, pManager, rc, uBkgImageId, iSubIndex);
	} else
	{
		// Draw focus rectangle
		if (((uState & UISTATE_FOCUSED) != 0) && pManager->GetSystemSettings().bShowKeyboardCues) 
		{
			CBlueRenderEngineUI::DoPaintFrame(hDC, pManager, rc, UICOLOR_BUTTON_BORDER_FOCUS, 
				          UICOLOR_BUTTON_BORDER_FOCUS, UICOLOR__INVALID, UIFRAME_ROUND);
			::InflateRect(&rc, -1, -1);
		}
		// Draw frame and body
		COLORREF clrColor1, clrColor2;
		UITYPE_COLOR clrBorder1, clrBorder2;
		if ((uState & UISTATE_DISABLED) != 0) 
		{
			clrBorder1 = UICOLOR_BUTTON_BORDER_DISABLED;
			clrBorder2 = UICOLOR_BUTTON_BORDER_DISABLED;
			clrText = UICOLOR_BUTTON_TEXT_DISABLED;
			clrBack = UICOLOR_BUTTON_BACKGROUND_DISABLED;
		} else if ((uState & UISTATE_PUSHED) != 0)
		{
			clrBorder1 = UICOLOR_BUTTON_BORDER_DARK;
			clrBorder2 = UICOLOR_BUTTON_BORDER_LIGHT;
			clrText = UICOLOR_BUTTON_TEXT_PUSHED;
			clrBack = UICOLOR_BUTTON_BACKGROUND_PUSHED;
		} else 
		{
			clrBorder1 = UICOLOR_BUTTON_BORDER_LIGHT;
			clrBorder2 = UICOLOR_BUTTON_BORDER_DARK;
			clrText = UICOLOR_BUTTON_TEXT_NORMAL;
			clrBack = UICOLOR_BUTTON_BACKGROUND_NORMAL;
		}
		// Draw button
		if (uState != 0)
			DoPaintFrame(hDC, pManager, rc, clrBorder1, clrBorder2, UICOLOR__INVALID, UIFRAME_ROUND);
		::InflateRect(&rc, -1, -1);
		// The pushed button has an inner light shade
		if ((uState & UISTATE_PUSHED) != 0) 
		{
			DoPaintFrame(hDC, pManager, rc, UICOLOR_STANDARD_LIGHTGREY, UICOLOR_STANDARD_LIGHTGREY, UICOLOR__INVALID);
			rc.top += 1;
			rc.left += 1;
		}
		// Gradient background
		if (!bTransparent)
		{
			pManager->GetThemeColorPair(clrBack, clrColor1, clrColor2);
			DoPaintGradient(hDC, pManager, rc, clrColor1, clrColor2, true, 32);
		}

		if (uState != 0)
		{
			// Draw hightlight inside button
			::SelectObject(hDC, pManager->GetThemePen(UICOLOR_DIALOG_BACKGROUND));
			POINT ptTemp;
			::MoveToEx(hDC, rc.left, rc.top, &ptTemp);
			::LineTo(hDC, rc.left, rc.bottom - 1);
			::LineTo(hDC, rc.right - 2, rc.bottom - 1);
			::LineTo(hDC, rc.right - 2, rc.top);  
		}
	}
	//绘制图像
 	RECT rcArrow = rc; 
	LPUI_IMAGE_ITEM pImage; 
	::InflateRect(&rcArrow, -1, -1);
	rcArrow.left += szPadding.cx;
	rcArrow.top += szPadding.cy;
	if (pManager->GetImage(uImageId, &pImage) && pImage->pGraphic)
	{ 
		rcArrow.right = rcArrow.left + pImage->pGraphic->GetWidth();
	}   
	rcArrow.bottom -= szPadding.cy; 
	if (rcArrow.right < rcArrow.left)
		rcArrow.right = rcArrow.right + 2;
	DoPaintGraphic(hDC, pManager, rcArrow, uImageId, uSubIdx);
	//
	RECT rcText = rc;
	rcText.left = rcArrow.right + szPadding.cx;
	rcText.right -= szPadding.cx;
	rcText.bottom -= szPadding.cy;
	rcText.bottom -= 1;
	int nLinks = 0;
	DoPaintPrettyText(hDC, pManager, rcText, pstrText, clrText, 
		UICOLOR__INVALID, NULL, nLinks, DT_SINGLELINE );
}

void CBlueRenderEngineUI::DoPaintButton(HDC hDC, CPaintManagerUI* pManager, RECT rc, LPCTSTR pstrText, 
	                                    const RECT& rcPadding, UINT uState, UINT uStyle, UINT uImageId)
{
	ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
	UITYPE_COLOR clrText, clrBack;
	if (uImageId == 0)
	{
		// Draw focus rectangle
		if (((uState & UISTATE_FOCUSED) != 0) && pManager->GetSystemSettings().bShowKeyboardCues) 
		{
			CBlueRenderEngineUI::DoPaintFrame(hDC, pManager, rc, UICOLOR_BUTTON_BORDER_FOCUS, 
				          UICOLOR_BUTTON_BORDER_FOCUS, UICOLOR__INVALID, UIFRAME_ROUND);
			::InflateRect(&rc, -1, -1);
		}
		// Draw frame and body
		COLORREF clrColor1, clrColor2;
		UITYPE_COLOR clrBorder1, clrBorder2;
		if ((uState & UISTATE_DISABLED) != 0) 
		{
			clrBorder1 = UICOLOR_BUTTON_BORDER_DISABLED;
			clrBorder2 = UICOLOR_BUTTON_BORDER_DISABLED;
			clrText = UICOLOR_BUTTON_TEXT_DISABLED;
			clrBack = UICOLOR_BUTTON_BACKGROUND_DISABLED;
		} else if ((uState & UISTATE_PUSHED) != 0)
		{
			clrBorder1 = UICOLOR_BUTTON_BORDER_DARK;
			clrBorder2 = UICOLOR_BUTTON_BORDER_LIGHT;
			clrText = UICOLOR_BUTTON_TEXT_PUSHED;
			clrBack = UICOLOR_BUTTON_BACKGROUND_PUSHED;
		} else 
		{
			clrBorder1 = UICOLOR_BUTTON_BORDER_LIGHT;
			clrBorder2 = UICOLOR_BUTTON_BORDER_DARK;
			clrText = UICOLOR_BUTTON_TEXT_NORMAL;
			clrBack = UICOLOR_BUTTON_BACKGROUND_NORMAL;
		}
		// Draw button
		DoPaintFrame(hDC, pManager, rc, clrBorder1, clrBorder2, UICOLOR__INVALID, UIFRAME_ROUND);
		::InflateRect(&rc, -1, -1);
		// The pushed button has an inner light shade
		if ((uState & UISTATE_PUSHED) != 0) 
		{
			DoPaintFrame(hDC, pManager, rc, UICOLOR_STANDARD_LIGHTGREY, UICOLOR_STANDARD_LIGHTGREY, UICOLOR__INVALID);
			rc.top += 1;
			rc.left += 1;
		}
		// Gradient background
		pManager->GetThemeColorPair(clrBack, clrColor1, clrColor2);
		DoPaintGradient(hDC, pManager, rc, clrColor1, clrColor2, true, 32);
	    POINT ptTemp;
		::MoveToEx(hDC, rc.left, rc.top, &ptTemp);
		::LineTo(hDC, rc.left, rc.bottom - 1);
		::LineTo(hDC, rc.right - 2, rc.bottom - 1);
		::LineTo(hDC, rc.right - 2, rc.top);		
		// Draw hightlight inside button
		::SelectObject(hDC, pManager->GetThemePen(UICOLOR_DIALOG_BACKGROUND));
    } else
	{
		int iSubIndex = 0;
		if ((uState & UISTATE_DISABLED) != 0)
		{
			iSubIndex = 3;//button disabled
			clrText = UICOLOR_BUTTON_TEXT_DISABLED;
			clrBack = UICOLOR_BUTTON_BACKGROUND_DISABLED;
		} else if ((uState & UISTATE_PUSHED ) != 0)
		{
			iSubIndex = 2;//button pushed
			clrText = UICOLOR_BUTTON_TEXT_PUSHED;
			clrBack = UICOLOR_BUTTON_BACKGROUND_PUSHED;
		} else if ((uState & UISTATE_HOT) != 0)
		{
			iSubIndex = 1;//hot
			clrText = UICOLOR_BUTTON_TEXT_NORMAL;
			clrBack = UICOLOR_BUTTON_BACKGROUND_NORMAL;
		} else
		{
			iSubIndex = 0;//normal state
			clrText = UICOLOR_BUTTON_TEXT_NORMAL;
			clrBack = UICOLOR_BUTTON_BACKGROUND_NORMAL;
		}
		DoPaintGraphic(hDC, pManager, rc, uImageId, iSubIndex);
	}

	// Draw text
	RECT rcText = rc;
	::InflateRect(&rcText, -1, -1);
	rcText.left += rcPadding.left;
	rcText.top += rcPadding.top;
	rcText.right -= rcPadding.right;
	rcText.bottom -= rcPadding.bottom;
	rcText.bottom -= 1;
	int nLinks = 0;
	DoPaintPrettyText(hDC, pManager, rcText, pstrText, clrText, 
		UICOLOR__INVALID, NULL, nLinks, DT_SINGLELINE | uStyle);
}

//只绘制可见部分
void CBlueRenderEngineUI::DoPaintGraphic(HDC hDC, CPaintManagerUI *pManager, const RECT &rc,
		      UINT nImageId, PAINT_ALIGN Align)
{
	LPUI_IMAGE_ITEM pImage;
	if (pManager->GetImage(nImageId, &pImage) && pImage->pGraphic)
	{
		RECT rcPaint = rc;
		switch(Align)
		{
		case PA_TOPLEFT: //左上
			{
				rcPaint.right = rcPaint.left + pImage->pGraphic->GetWidth();
				rcPaint.bottom = rcPaint.top + pImage->pGraphic->GetHeight();
				break;
			}
		case PA_TOPRIGHT: //右上
			{
				rcPaint.left = rcPaint.right - pImage->pGraphic->GetWidth();
				rcPaint.bottom = rcPaint.top + pImage->pGraphic->GetHeight();
				break;
			}
		case PA_BOTTOMLEFT: //左下
			{
				rcPaint.right = rcPaint.left + pImage->pGraphic->GetWidth();
				rcPaint.top = rcPaint.bottom - pImage->pGraphic->GetHeight();
				break;
			}
		case PA_BOTTOMRIGHT: //右下
			{
				rcPaint.left = rcPaint.right - pImage->pGraphic->GetWidth();
				rcPaint.top = rcPaint.bottom - pImage->pGraphic->GetHeight();
				break;
			}
		}
		DoPaintGraphic(hDC, pManager, rcPaint, *pImage); 
	}
}

void CBlueRenderEngineUI::DoPaintGraphic(HDC hDC, CPaintManagerUI* pManager, const RECT& rc, UINT nImageID,
                                         int iSubIndex,/*= -1, base from 0 */  const StretchFixed* pFixed,//= NULL
										 UINT nStretchMode /*SM_NORMALSTRETCH*/, BOOL bHole)
{
	LPUI_IMAGE_ITEM pImage;
	if (pManager->GetImage(nImageID, &pImage))
	{
		DoPaintGraphic(hDC, pManager, rc, *pImage, iSubIndex, pFixed, nStretchMode, bHole);
	}
}

//绘制动态图片,双图叠加
void CBlueRenderEngineUI::DoPaintGraphicPlus(HDC hDc, CPaintManagerUI *pManager, const RECT &rc, 
		                           UINT nImageId, UINT nPlusImageId, const StretchFixed * pFixed,
								   UINT nStretchMode)
{
	LPUI_IMAGE_ITEM img1;
	LPUI_IMAGE_ITEM img2;
	if (pManager->GetImage(nImageId, &img1) && pManager->GetImage(nPlusImageId, &img2))
	{
		DoPaintGraphic(hDc, pManager, rc, *img2, 0, pFixed, nStretchMode);
		RECT r = {rc.left + 3, rc.top + 3, rc.right - 3, rc.bottom - 3};
		DoPaintGraphic(hDc, pManager, r, *img1, 0, pFixed, nStretchMode);
	}
}


void CBlueRenderEngineUI::DoPaintGraphic(HDC hDC, CPaintManagerUI *pManager, const RECT& rc, const UI_IMAGE_ITEM &image, int iSubIndex,// = -1,
									const StretchFixed* pFixed /*=NULL */,	UINT nStretchMode /* SM_NORMALSTRETCH */, BOOL bHole)
{  
    static LPALPHABLEND lpAlphaBlend = (LPALPHABLEND) ::GetProcAddress(::GetModuleHandle(_T("msimg32.dll")), "AlphaBlend");

    if ( lpAlphaBlend == NULL ) 
		lpAlphaBlend = AlphaBitBlt;
	//现在不处理image的格式，即所有的image都按钮纵向排列处理
	if ((image.pGraphic != NULL) && (!image.pGraphic->IsEmpty()) && (image.dwSubCount > 0))
	{ 
		int cxSub = image.pGraphic->GetWidth();//sub image width
		int cySub = image.pGraphic->GetHeight() / image.dwSubCount;//sub image height
		RECT rcBmpPart = {0, 0, cxSub, image.pGraphic->GetHeight()};
		RECT rcCorner = {0};
		if (iSubIndex >= 0)
		{
		 
			//the image is treated as a image list no matter if it
			//contains more than 1 sub images
			//validate index
			if ((UINT)iSubIndex >= image.dwSubCount) 
				iSubIndex = image.dwSubCount - 1;
			 
			int y = iSubIndex * cySub;
			rcBmpPart.left = 0;
			rcBmpPart.top = y;
			rcBmpPart.right = cxSub;
			rcBmpPart.bottom = y + cySub; 
		}
		 
		//do paint
		switch( nStretchMode )
		{
			case SM_FIXED4CORNERS:
				rcCorner.left = pFixed->m_iTopLeftWidth;
				rcCorner.top = pFixed->m_iTopHeight;
				rcCorner.right = pFixed->m_iBotLeftWidth;
				rcCorner.bottom = pFixed->m_iBotHeight;
				image.pGraphic->DrawPlus(hDC, rc, rc, rcBmpPart, rcCorner, lpAlphaBlend, 0, bHole);
				break;
			case SM_HORIZONTAL:
				{
					ASSERT(pFixed != NULL);
					RECT rcNull = {0};
					//draw left
					CRect rcLeft(rcBmpPart.left, rcBmpPart.top, rcBmpPart.left + pFixed->m_iCenterLeftWidth,
						rcBmpPart.top + cySub); 
					CRect rcLeftDest(rc);
					rcLeftDest.right = rc.left + pFixed->m_iCenterLeftWidth; 
					image.pGraphic->DrawPlus(hDC, rcLeftDest, rcLeftDest, rcLeft, rcNull, lpAlphaBlend);   

					//draw mid
					CRect rcMid(rcBmpPart.left + pFixed->m_iCenterLeftWidth, rcBmpPart.top, 
						rcBmpPart.left + cxSub - pFixed->m_iCenterLeftWidth - pFixed->m_iCenterRightWidth, 
						rcBmpPart.top + cySub); 
					CRect rcMidDest(rc);
					rcMidDest.left += pFixed->m_iCenterLeftWidth;
					rcMidDest.right -= pFixed->m_iCenterRightWidth;
					image.pGraphic->DrawPlus(hDC, rcMidDest, rcMidDest, rcMid, rcNull, lpAlphaBlend); 

					//draw right
					CRect rcRight(rcBmpPart.left + cxSub - pFixed->m_iCenterRightWidth, rcBmpPart.top, 
						rcBmpPart.left + cxSub, rcBmpPart.top + cySub); 
					CRect rcRightDest(rc);
					rcRightDest.left = rcRightDest.right - pFixed->m_iCenterRightWidth;  
					image.pGraphic->DrawPlus(hDC, rcRightDest, rcRightDest, rcRight, rcNull, lpAlphaBlend);
				}
				break;
			case SM_VERTICAL:
				{
					ASSERT(pFixed != NULL); 
					RECT rcNull = {0};
					//draw top
					CRect rcTop(rcBmpPart.left , rcBmpPart.top, rcBmpPart.left + cxSub, rcBmpPart.top + pFixed->m_iTopHeight); 
					CRect rcTopDest(rc);
					rcTopDest.bottom = rc.top + pFixed->m_iTopHeight;
					image.pGraphic->DrawPlus(hDC, rcTopDest, rcTopDest, rcTop, rcNull, lpAlphaBlend);

					//draw center
					CRect rcCenter(rcBmpPart.left, rcBmpPart.top + pFixed->m_iTopHeight,
						rcBmpPart.left + cxSub, rcBmpPart.top + cySub - pFixed->m_iBotHeight); 
					CRect rcCenterDest(rc);
					rcCenterDest.top += pFixed->m_iTopHeight;
					rcCenterDest.bottom -= pFixed->m_iBotHeight;
					image.pGraphic->DrawPlus(hDC, rcCenterDest, rcCenterDest, rcCenter, rcNull, lpAlphaBlend);

					//draw bottom
					CRect rcBot(rcBmpPart.left, rcBmpPart.top + cySub - pFixed->m_iBotHeight, rcBmpPart.left + cxSub, 
						rcBmpPart.top + cySub); 
					CRect rcBotDest(rc);
					rcBotDest.top = rc.bottom - pFixed->m_iBotHeight;
					image.pGraphic->DrawPlus(hDC, rcBotDest, rcBotDest, rcBot, rcNull, lpAlphaBlend);
				}
				break;
			case SM_HORIZONTAL_CENTER:
				{ 
					//draw left
					RECT rcNull = {0};
					CRect rcLeft(rcBmpPart.left, rcBmpPart.top, rcBmpPart.left + pFixed->m_iCenterLeftWidth,
						rcBmpPart.top + cySub); 
					CRect rcLeftDest(rc);
					rcLeftDest.right = rc.left + pFixed->m_iCenterLeftWidth;
					image.pGraphic->DrawPlus(hDC, rcLeftDest, rcLeftDest, rcLeft, rcNull, lpAlphaBlend);

					//draw mid
					CRect rcMid(rcBmpPart.left + pFixed->m_iCenterLeftWidth, rcBmpPart.top, 
						rcBmpPart.left + cxSub - pFixed->m_iCenterLeftWidth - pFixed->m_iCenterRightWidth, 
						rcBmpPart.top + cySub); 
					CRect rcMidDest(rc);
					rcMidDest.left += pFixed->m_iCenterLeftWidth;
					rcMidDest.right -= pFixed->m_iCenterRightWidth;
					//拉伸
					int cxDraw = rcMidDest.right - rcMidDest.left;
					if (cxDraw > rcMid.right - rcMid.left)
					{
						RECT rcSrcClip = {rcBmpPart.left + pFixed->m_iCenterLeftWidth, rcBmpPart.top,
							rcBmpPart.left + cxSub - pFixed->m_iCenterLeftWidth + 2, rcBmpPart.top + cySub};
						RECT rcClip(rcMidDest); 
						rcClip.right = rcClip.left + (cxDraw - (rcMid.right - rcMid.left)) / 2;
						image.pGraphic->DrawPlus(hDC, rcClip, rcClip, rcSrcClip, rcNull, lpAlphaBlend); 
						//
						rcClip.right = rcMidDest.right;
						rcClip.left = rcMidDest.right - (cxDraw - (rcMid.right - rcMid.left)) / 2 - 1;
						image.pGraphic->DrawPlus(hDC, rcClip, rcClip, rcSrcClip, rcNull, lpAlphaBlend); 
						//
						rcMidDest.left += (cxDraw - (rcMid.right - rcMid.left)) / 2;
						rcMidDest.right = rcMidDest.top + (rcMid.right - rcMid.left);
					}
					image.pGraphic->DrawPlus(hDC, rcMidDest, rcMidDest, rcMid, rcNull, lpAlphaBlend); 

					//draw right
					CRect rcRight(rcBmpPart.left + cxSub - pFixed->m_iCenterRightWidth, rcBmpPart.top, 
						rcBmpPart.left + cxSub, rcBmpPart.top + cySub); 
					CRect rcRightDest(rc);
					rcRightDest.left = rcRightDest.right - pFixed->m_iCenterRightWidth; 
					image.pGraphic->DrawPlus(hDC, rcRightDest, rcRightDest, rcRight, rcNull, lpAlphaBlend);
					break;
				}
			case SM_VERTICAL_CENTER:
				{
					ASSERT(pFixed != NULL); 
					//draw top
					RECT rcNull = {0};
					CRect rcTop(rcBmpPart.left, rcBmpPart.top, rcBmpPart.left + cxSub, rcBmpPart.top + pFixed->m_iTopHeight); 
					CRect rcTopDest(rc);
					rcTopDest.bottom = rc.top + pFixed->m_iTopHeight;
					image.pGraphic->DrawPlus(hDC, rcTopDest, rcTopDest, rcTop, rcNull, lpAlphaBlend); 

					//draw center
					CRect rcCenter(rcBmpPart.left, rcBmpPart.top + pFixed->m_iTopHeight, 
						rcBmpPart.left + cxSub, rcBmpPart.top + cySub - pFixed->m_iBotHeight); 
					CRect rcCenterDest(rc);
					rcCenterDest.top += pFixed->m_iTopHeight;
					rcCenterDest.bottom -= pFixed->m_iBotHeight;
					//拉伸
					int cyDraw = rcCenterDest.bottom - rcCenterDest.top;
					if (cyDraw > rcCenter.bottom - rcCenter.top)
					{
						RECT rcSrcClip = {rcBmpPart.left, rcBmpPart.top + pFixed->m_iTopHeight, 
							rcBmpPart.left + cxSub, rcBmpPart.top + pFixed->m_iTopHeight + 2};
						RECT rcClip(rcCenterDest); 
						rcClip.bottom = rcClip.top + (cyDraw - (rcCenter.bottom - rcCenter.top)) / 2;
						image.pGraphic->DrawPlus(hDC, rcClip, rcClip, rcSrcClip, rcNull, lpAlphaBlend); 
						//
						rcClip.bottom = rcCenterDest.bottom;
						rcClip.top = rcCenterDest.bottom - (cyDraw - (rcCenter.bottom - rcCenter.top)) / 2 - 1;
						image.pGraphic->DrawPlus(hDC, rcClip, rcClip, rcSrcClip, rcNull, lpAlphaBlend); 
						//
						rcCenterDest.top += (cyDraw - (rcCenter.bottom - rcCenter.top)) / 2;
						rcCenterDest.bottom = rcCenterDest.top + (rcCenter.bottom - rcCenter.top);
					}
					image.pGraphic->DrawPlus(hDC, rcCenterDest, rcCenterDest, rcCenter, rcNull, lpAlphaBlend); 

					//draw bottom
					CRect rcBot(rcBmpPart.left, rcBmpPart.top + cySub - pFixed->m_iBotHeight, 
						rcBmpPart.left + cxSub, rcBmpPart.top + cySub); 
					CRect rcBotDest(rc);
					rcBotDest.top = rc.bottom - pFixed->m_iBotHeight;
					image.pGraphic->DrawPlus(hDC, rcBotDest, rcBotDest, rcBot, rcNull, lpAlphaBlend);
					break;
				}
			case SM_NOSTRETCH:
				{
					RECT rcNull = {0};
					int cxDest = rc.right - rc.left;
					int cyDest = rc.bottom - rc.top;
					RECT rcDest = rc;
					rcDest.left += (cxDest - cxSub) / 2;
					rcDest.right = rcDest.left + cxSub;
					rcDest.top += (cyDest - cySub) / 2;
					rcDest.bottom = rcDest.top + cySub;
					image.pGraphic->DrawPlus(hDC, rcDest, rcDest, rcBmpPart, rcNull, lpAlphaBlend); 
				}
				break;
			case SM_NORMALSTRETCH:
			default://no fixed stretch needed
				{
					RECT rcNull = {0};
					image.pGraphic->DrawPlus(hDC, rc, rc, rcBmpPart, rcNull, lpAlphaBlend);
				}
				break;
		}
	}
}


void CBlueRenderEngineUI::CopyAndDraw(HDC hDC, CPaintManagerUI* pManager, UI_IMAGE_ITEM &image, 
									  const RECT& rcDest, const RECT& rcCopy)
{
	//透色处理相关参数，已知bmp需要指定背景透明色，而ico，png不需要，其他的还不清楚
	BOOL bHasTrans = (image.dwTransColor != TRANSCOLOR__INVALID);
	COLORREF clrTrans = pManager->GetTransparentColor(image.dwTransColor);
	RGBQUAD rgbTrans = {GetBValue(clrTrans), GetGValue(clrTrans), GetRValue(clrTrans), 0};
	int index = 8;//TBD 

	/*CxImage* ximage = image.pGraphic->m_pImage;
	CxImage ximageSel;
	ximage->Crop(rcCopy, &ximageSel);
	if (bHasTrans)
	{
		ximageSel.SetTransColor(rgbTrans);
		ximageSel.SetTransIndex(index);
	}
	DWORD dwStart = ::GetTickCount();*/
	//ximageSel.Draw(hDC, rcDest);
	//DrawImage(hDC, image.pGraphic->GetBitmap(), 
}


void CBlueRenderEngineUI::DoPaintGraphic(HDC hDC,  CPaintManagerUI* pManager,  const RECT& rc,   UINT nImage,
                                         const StretchFixed& sf,  UINT nStretchMode/* = SM_NORMALSTRETCH*/)
{
	static LPALPHABLEND lpAlphaBlend = (LPALPHABLEND) ::GetProcAddress(::GetModuleHandle(_T("msimg32.dll")), "AlphaBlend");

    if ( lpAlphaBlend == NULL ) 
		lpAlphaBlend = AlphaBitBlt;
	LPUI_IMAGE_ITEM image;
	if (pManager->GetImage(nImage, &image) 
		&& (image->pGraphic != NULL)
		&& (!image->pGraphic->IsEmpty()) 
		&& (image->dwSubCount > 0))
	{	 
		int cx = image->pGraphic->GetWidth();
		int cy = image->pGraphic->GetHeight();
		RECT rcBmpPart = {0, 0, cx, cy}; 
			
		RECT rcNull = {0};
		if (nStretchMode == SM_NORMALSTRETCH)
		{
			RECT rcBmpPart = {0, 0, cx, cy};
			image->pGraphic->DrawPlus(hDC, rc, rc, rcBmpPart, rcNull, lpAlphaBlend);
			return;
		} else if (nStretchMode == SM_VERTICAL)
		{
			//top 
			RECT rcDest = {rc.left, rc.top, rc.right, rc.top + sf.m_iTopHeight};
			RECT rcBmpPart = {0, 0, cx, sf.m_iTopHeight};
			image->pGraphic->DrawPlus(hDC, rcDest, rcDest, rcBmpPart, rcNull, lpAlphaBlend); 

			//center
			rcDest.left = rc.left;
			rcDest.top = sf.m_iTopHeight;
			rcDest.right = rc.right;
			rcDest.bottom = rc.bottom - sf.m_iBotHeight;
			rcBmpPart.left = 0;
			rcBmpPart.top = sf.m_iTopHeight;
			rcBmpPart.right = cx;
			rcBmpPart.bottom = cy - sf.m_iBotHeight;
			image->pGraphic->DrawPlus(hDC, rcDest, rcDest, rcBmpPart, rcNull, lpAlphaBlend); 

			//bottom
			rcDest.left = rc.left;
			rcDest.top = rc.bottom - sf.m_iBotHeight;
			rcDest.right = rc.right;
			rcDest.bottom = rc.bottom;
			rcBmpPart.left = 0;
			rcBmpPart.top = cy - sf.m_iBotHeight;
			rcBmpPart.right = cx;
			rcBmpPart.bottom = cy;
			image->pGraphic->DrawPlus(hDC, rcDest, rcDest, rcBmpPart, rcNull, lpAlphaBlend); 
			return ;
		} else if (nStretchMode == SM_FIX4CVER)
		{ 
			//top-left
			RECT rcDest = {rc.left, rc.top, rc.left + sf.m_iTopLeftWidth, rc.top + sf.m_iTopHeight};
			RECT rcBmpPart = {0, 0, sf.m_iTopLeftWidth, sf.m_iTopHeight};
			image->pGraphic->DrawPlus(hDC, rcDest, rcDest, rcBmpPart, rcNull, lpAlphaBlend); 

			//top-center
			rcDest.left = rc.left + sf.m_iTopLeftWidth;
			rcDest.top = rc.top;
			rcDest.right = rc.right - sf.m_iTopRightWidth;
			rcDest.bottom = rc.top + sf.m_iTopHeight;
			rcBmpPart.left = sf.m_iTopLeftWidth;
			rcBmpPart.top = 0;
			rcBmpPart.right = cx - sf.m_iTopRightWidth;
			rcBmpPart.bottom = sf.m_iTopHeight;
			image->pGraphic->DrawPlus(hDC, rcDest, rcDest, rcBmpPart, rcNull, lpAlphaBlend); 
			
			//top-right
			rcDest.left = rc.right - sf.m_iTopRightWidth;
			rcDest.top = rc.top;
			rcDest.left = rc.right;
			rcDest.bottom = rc.top + sf.m_iTopHeight;
			rcBmpPart.left = cx - sf.m_iTopRightWidth;
			rcBmpPart.top = 0;
			rcBmpPart.right = cx;
			rcBmpPart.bottom = sf.m_iTopHeight;
			image->pGraphic->DrawPlus(hDC, rcDest, rcDest, rcBmpPart, rcNull, lpAlphaBlend);
 
			//vert
			rcDest.left = rc.left;
			rcDest.top = sf.m_iTopHeight;
			rcDest.right = rc.right;
			rcDest.bottom = sf.m_iTopHeight + sf.m_iCenterHeight;
			rcBmpPart.left = 0;
			rcBmpPart.top = sf.m_iTopHeight;
			rcBmpPart.right = cx;
			rcBmpPart.bottom = sf.m_iTopHeight + sf.m_iCenterHeight;
			image->pGraphic->DrawPlus(hDC, rcDest, rcDest, rcBmpPart, rcNull, lpAlphaBlend); 
			
			//stretch
			rcDest.left = rc.left;
			rcDest.top = sf.m_iTopHeight + sf.m_iCenterHeight;
			rcDest.right = rc.right;
			rcDest.bottom = rc.bottom - sf.m_iBotHeight;
			rcBmpPart.left = 0;
			rcBmpPart.top = sf.m_iTopHeight + sf.m_iCenterHeight;
			rcBmpPart.right = cx;
			rcBmpPart.bottom = cy - sf.m_iBotHeight;
			image->pGraphic->DrawPlus(hDC, rcDest, rcDest, rcBmpPart, rcNull, lpAlphaBlend); 
				//
			//bottom-left
			rcDest.left = rc.left;
			rcDest.top = rc.bottom - sf.m_iBotHeight;
			rcDest.right = rc.left + sf.m_iBotLeftWidth;
			rcDest.bottom = rc.bottom;
			rcBmpPart.left = 0;
			rcBmpPart.top = cy - sf.m_iBotHeight;
			rcBmpPart.right = sf.m_iBotLeftWidth;
			rcBmpPart.bottom = cy;
			image->pGraphic->DrawPlus(hDC, rcDest, rcDest, rcBmpPart, rcNull, lpAlphaBlend); 
			
			//bottom-center
			rcDest.left = rc.left + sf.m_iBotLeftWidth;
			rcDest.top = rc.bottom - sf.m_iBotHeight;
			rcDest.right = rc.right - sf.m_iBotRightWidth;
			rcDest.bottom = rc.bottom;
			rcBmpPart.left = sf.m_iBotLeftWidth;
			rcBmpPart.top = cy - sf.m_iBotHeight;
			rcBmpPart.right = cx - sf.m_iBotRightWidth;
			rcBmpPart.bottom = cy;
			image->pGraphic->DrawPlus(hDC, rcDest, rcDest, rcBmpPart, rcNull, lpAlphaBlend); 
			
			//bottom-right
			rcDest.left = rc.right - sf.m_iBotRightWidth;
			rcDest.top = rc.bottom - sf.m_iBotHeight;
			rcDest.right = rc.right;
			rcDest.bottom = rc.bottom;
			rcBmpPart.left = cx - sf.m_iBotRightWidth;
			rcBmpPart.top = cy - sf.m_iBotHeight;
			rcBmpPart.right = cx;
			rcBmpPart.bottom = cy;
			image->pGraphic->DrawPlus(hDC, rcDest, rcDest, rcBmpPart, rcNull, lpAlphaBlend); 
			if (sf.m_nTextureImage > 0)
			{
				//draw texture
				RECT rcTexture = {rc.left + sf.m_iTopLeftWidth, rc.top, rc.right - sf.m_iTopRightWidth, sf.m_iTopHeight};
				DoPaintTexture(hDC, rcTexture, pManager, sf.m_nTextureImage);
			} else if (sf.m_nTextureWidth > 0)
			{
				//draw texture
				RECT rcTexture = {rc.left + sf.m_iTopLeftWidth, rc.top, rc.right - sf.m_iTopRightWidth, sf.m_iTopHeight};
				DoPaintTexture(hDC, rcTexture, sf.m_crTexture1, sf.m_crTexture2, sf.m_nTextureWidth);
			}
    		  
		} else if (nStretchMode == SM_FIXED4CORNERS)
		{ 	
			//top-left
			RECT rcDest = {rc.left, rc.top, rc.left + sf.m_iTopLeftWidth, rc.top + sf.m_iTopHeight};
			RECT rcBmpPart = {0, 0, sf.m_iTopLeftWidth, sf.m_iTopHeight};
			image->pGraphic->DrawPlus(hDC, rcDest, rcDest, rcBmpPart, rcNull, lpAlphaBlend);
			//bmp only current now
	 
			//top-center
			rcDest.left = rc.left + sf.m_iTopLeftWidth;
			rcDest.top = rc.top;
			rcDest.right = rc.right - sf.m_iTopRightWidth;
			rcDest.bottom = rc.top + sf.m_iTopHeight;
			rcBmpPart.left = sf.m_iTopLeftWidth;
			rcBmpPart.top = 0;
			rcBmpPart.right = cx - sf.m_iTopRightWidth;
			rcBmpPart.bottom = sf.m_iTopHeight;
			image->pGraphic->DrawPlus(hDC, rcDest, rcDest, rcBmpPart, rcNull, lpAlphaBlend); 
			
			//top-right
			rcDest.left = rc.right - sf.m_iTopRightWidth;
			rcDest.top = rc.top;
			rcDest.right = rc.right;
			rcDest.bottom = rc.top + sf.m_iTopHeight;
			rcBmpPart.left = cx - sf.m_iTopRightWidth;
			rcBmpPart.top = 0;
			rcBmpPart.right = cx;
			rcBmpPart.bottom = sf.m_iTopHeight;
			image->pGraphic->DrawPlus(hDC, rcDest, rcDest, rcBmpPart, rcNull, lpAlphaBlend); 

			//bottom-left
			rcDest.left = rc.left;
			rcDest.top = rc.bottom - sf.m_iBotHeight;
			rcDest.right = rc.left + sf.m_iTopLeftWidth;
			rcDest.bottom = rc.bottom;
			rcBmpPart.left = 0;
			rcBmpPart.top = cy - sf.m_iBotHeight;
			rcBmpPart.right = sf.m_iBotLeftWidth;
			rcBmpPart.bottom = cy;
			image->pGraphic->DrawPlus(hDC, rcDest, rcDest, rcBmpPart, rcNull, lpAlphaBlend); 
			
			//bottom-center
			rcDest.left = rc.left + sf.m_iBotLeftWidth;
			rcDest.top = rc.bottom - sf.m_iBotHeight;
			rcDest.right = rc.right - sf.m_iBotRightWidth;
			rcDest.bottom = rc.bottom;
			rcBmpPart.left = sf.m_iBotLeftWidth;
			rcBmpPart.top = cy - sf.m_iBotHeight;
			rcBmpPart.right = cx - sf.m_iBotRightWidth;
			rcBmpPart.bottom = cy;
			image->pGraphic->DrawPlus(hDC, rcDest, rcDest, rcBmpPart, rcNull, lpAlphaBlend); 
				
			//bottom-right
			rcDest.left = rc.right - sf.m_iBotRightWidth;
			rcDest.top = rc.bottom - sf.m_iBotHeight;
			rcDest.right = rc.right;
			rcDest.bottom = rc.bottom;
			rcBmpPart.left = cx - sf.m_iBotRightWidth;
			rcBmpPart.top = cy - sf.m_iBotHeight;
			rcBmpPart.right = cx;
			rcBmpPart.bottom = cy;
			image->pGraphic->DrawPlus(hDC, rcDest, rcDest, rcBmpPart, rcNull, lpAlphaBlend); 

				
			//center-left
			rcDest.left = rc.left;
			rcDest.top = rc.top + sf.m_iTopHeight;
			rcDest.right = rc.left + sf.m_iCenterLeftWidth;
			rcDest.bottom = rc.bottom - sf.m_iBotHeight;
			rcBmpPart.left = 0;
			rcBmpPart.top = sf.m_iTopHeight;
			rcBmpPart.right = sf.m_iCenterLeftWidth;
			rcBmpPart.bottom = cy - sf.m_iBotHeight;
			image->pGraphic->DrawPlus(hDC, rcDest, rcDest, rcBmpPart, rcNull, lpAlphaBlend);
		 
			//center-center
			rcDest.left = rc.left + sf.m_iCenterLeftWidth;
			rcDest.top = rc.top + sf.m_iTopHeight;
			rcDest.right = rc.right - sf.m_iCenterRightWidth;
			rcDest.bottom = rc.bottom - sf.m_iBotHeight;
			rcBmpPart.left = sf.m_iCenterLeftWidth;
			rcBmpPart.top = sf.m_iTopHeight;
			rcBmpPart.right = cx - sf.m_iCenterRightWidth;
			rcBmpPart.bottom = cy - sf.m_iBotHeight;
			image->pGraphic->DrawPlus(hDC, rcDest, rcDest, rcBmpPart, rcNull, lpAlphaBlend); 
			
			//center-right
			rcDest.left = rc.right - sf.m_iCenterRightWidth;
			rcDest.top = rc.top + sf.m_iTopHeight;
			rcDest.right = rc.right;
			rcDest.bottom = rc.bottom - sf.m_iBotHeight;
			rcBmpPart.left = cx - sf.m_iCenterRightWidth;
			rcBmpPart.top = sf.m_iTopHeight;
			rcBmpPart.right = cx;
			rcBmpPart.bottom = cy - sf.m_iBotHeight;
			image->pGraphic->DrawPlus(hDC, rcDest, rcDest, rcBmpPart, rcNull, lpAlphaBlend);
	 	    if (sf.m_nTextureImage > 0)
			{
				//draw texture
				RECT rcTexture = {rc.left + sf.m_iTopLeftWidth, rc.top, rc.right - sf.m_iTopRightWidth, sf.m_iTopHeight};
				DoPaintTexture(hDC, rcTexture, pManager, sf.m_nTextureImage);
			} else if (sf.m_nTextureWidth > 0)
			{
				//draw texture
				RECT rcTexture = {rc.left + sf.m_iTopLeftWidth, rc.top, rc.right - sf.m_iTopRightWidth, sf.m_iTopHeight};
				DoPaintTexture(hDC, rcTexture, sf.m_crTexture1, sf.m_crTexture2, sf.m_nTextureWidth);
			}
			 
		}
	} //end if (pManager->GetImage(nImage, &image)...
}

//
void CBlueRenderEngineUI::DoPaintTexture(HDC hdc, const RECT &rc, CPaintManagerUI *pManager, int nImageId)
{
	LPUI_IMAGE_ITEM pImage;
	if (pManager->GetImage(nImageId, &pImage))
	{
		int nW = pImage->pGraphic->GetWidth();
		int nH = pImage->pGraphic->GetHeight();
		nW = nW *  (rc.bottom - rc.top) / nH;
		for (int i = rc.left; i < rc.right - nW; i += nW)
		{
			pImage->pGraphic->DrawToDc(hdc, i, rc.top + 1, nW, rc.bottom - 1);
		}
	}
}

//draw texture
void CBlueRenderEngineUI::DoPaintTexture(HDC hdc, const RECT &rc, COLORREF crTexture1, COLORREF crTexture2, int nTextureWidth)
{
	HPEN hP1 = ::CreatePen(PS_ENDCAP_MASK, nTextureWidth, crTexture1);
	HPEN hP2 = ::CreatePen(PS_ENDCAP_MASK, nTextureWidth, crTexture2);
	HPEN hOldPen = (HPEN)::SelectObject(hdc, hP1); 
	BOOL bFirst = TRUE;
	int j;
	int nHeight = rc.bottom - rc.top - 2;
	::SetBkMode(hdc, TRANSPARENT);
	for (int i = rc.left; i < rc.right; i += nTextureWidth)
	{
		j = i - nHeight;
		if (j < rc.left)
			continue;
		if (bFirst)
		{
			bFirst = FALSE;
			::SelectObject(hdc, hP1);
		} else
		{
			bFirst = TRUE;
			::SelectObject(hdc, hP2);
		}
		::MoveToEx(hdc, i, rc.top + 2, NULL);
		::LineTo(hdc, j, rc.bottom - 2);
	}
	::SelectObject(hdc, hOldPen);
	::DeleteObject(hP1);
	::DeleteObject(hP2);
}

void CBlueRenderEngineUI::DoPaintImageButton(HDC hDC, CPaintManagerUI* pManager, const RECT& rc, UINT uState,  
	                                         UINT nImageID,	const StretchFixed* pFixed,	UINT nStretchMode)
{
	ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
	int iSubIndex = 0;
	if ((uState & UISTATE_DISABLED) != 0)
	{
		iSubIndex = 3;//button disabled
	} else if ((uState & UISTATE_PUSHED ) != 0)
	{
		iSubIndex = 2;//button pushed
	} else if ((uState & UISTATE_HOT) != 0)
	{
		iSubIndex = 1;//hot
	} else
	{
		iSubIndex = 0;//normal state
	}
	DoPaintGraphic(hDC, pManager, rc, nImageID, iSubIndex, pFixed, nStretchMode);
}

void CBlueRenderEngineUI::DoPaintTabImageButton(HDC hDC, CPaintManagerUI* pManager,	const RECT& rc,	UINT uState,
	                                           UINT uBkgImageId, UINT nImageID, const StretchFixed* pFixed,	UINT nStretchMode)
{
	ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
	int index = 0;//default the first subimage with selected state
	if ((uState & UISTATE_DISABLED) != 0)
	{
		//not supported yet now!
		index = 3;
	} else if ((uState & UISTATE_SELECTED) != 0)
	{
		//select, first sub image
		index = 2;			
	} else if ((uState & UISTATE_PUSHED) != 0)
	{
		//pushed, forth sub image
		index = 2;
	} else if ((uState & UISTATE_HOT) != 0) 
	{
		//hot( hover or mouse in ), third subimage
		index = 1;
	} else
	{
		//normal state, second subimage
		index = 0;
	}
	if (uBkgImageId > 0)
	{
		DoPaintGraphic(hDC, pManager, rc, uBkgImageId, index);
	}
	DoPaintGraphic(hDC, pManager, rc, nImageID, index, pFixed, nStretchMode);
}

void CBlueRenderEngineUI::DoPaintEditBox(HDC hDC, CPaintManagerUI* pManager, const RECT& rcItem, UINT uState)
{
	ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
	if ((uState & UISTATE_DISABLED) != 0) 
	{
		DoPaintFrame(hDC, pManager, rcItem, UICOLOR_CONTROL_BORDER_DISABLED, 
			         UICOLOR_CONTROL_BORDER_DISABLED, UICOLOR_EDIT_BACKGROUND_DISABLED, uState);
	} else if ((uState & UISTATE_READONLY) != 0)
	{
		DoPaintFrame(hDC, pManager, rcItem, UICOLOR_CONTROL_BORDER_DISABLED, 
			         UICOLOR_CONTROL_BORDER_DISABLED, UICOLOR_EDIT_BACKGROUND_READONLY, uState);
	} else 
	{
		DoPaintFrame(hDC, pManager, rcItem, UICOLOR_CONTROL_BORDER_NORMAL, 
			         UICOLOR_CONTROL_BORDER_NORMAL, UICOLOR_EDIT_BACKGROUND_NORMAL, uState);
	}
}

void CBlueRenderEngineUI::DoPaintEditText(HDC hDC, CPaintManagerUI* pManager, const RECT& rcItem, LPCTSTR pstrText, 
	                                      COLORREF clrText, UINT uState, UINT uEditStyle)
{ 
	::SetBkMode(hDC, TRANSPARENT);
	::SetTextColor(hDC, clrText);
	::SelectObject(hDC, pManager->GetThemeFont(UIFONT_NORMAL));
	RECT rcEdit = rcItem;
	::InflateRect(&rcEdit, -3, -2);
	::DrawText(hDC, pstrText, -1, &rcEdit, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_EDITCONTROL | uEditStyle);
}

void CBlueRenderEngineUI::DoPaintOptionBox(HDC hDC, CPaintManagerUI* pManager, RECT rcItem, LPCTSTR pstrText, 
	                                       UINT uState, UINT uStyle, UINT nImageID)
{
	ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
	// Determine placement of elements
	RECT rcText = rcItem;
	RECT rcButton = rcItem;
	int cyItem = rcItem.bottom - rcItem.top;
	if ((uStyle & DT_RIGHT) != 0)
	{
		rcText.right -= cyItem;
		rcButton.left = rcButton.right - cyItem;
	} else 
	{
		rcText.left += cyItem;
		rcButton.right = rcButton.left + cyItem;
	}
	if (uState & UISTATE_CHECKED)
	{
		uState |= UISTATE_PUSHED;
	}
	//checked, unchecked ... images
	RECT rcPadding = { 0 };
	CBlueRenderEngineUI::DoPaintImageButton(hDC, pManager, rcButton, uState, nImageID, NULL, SM_NOSTRETCH);
	
	// Paint text
	UITYPE_COLOR iTextColor = ((uState & UISTATE_DISABLED) != 0) ? UICOLOR_EDIT_TEXT_DISABLED : UICOLOR_EDIT_TEXT_NORMAL;
	int nLinks = 0;
	CBlueRenderEngineUI::DoPaintPrettyText(hDC, pManager, rcText, pstrText, iTextColor, 
		                                   UICOLOR__INVALID, NULL, nLinks, DT_SINGLELINE | DT_VCENTER );
   
	// Paint focus rectangle
	if (((uState & UISTATE_FOCUSED) != 0) 
		&& pManager->GetSystemSettings().bShowKeyboardCues)
	{
		RECT rcFocus = { 0, 0, 9999, 9999 };
		int nLinks = 0;
		CBlueRenderEngineUI::DoPaintPrettyText(hDC, pManager, rcFocus, pstrText, iTextColor,
			         UICOLOR__INVALID, NULL, nLinks, DT_SINGLELINE | DT_CALCRECT);
		rcText.right = rcText.left + (rcFocus.right - rcFocus.left);
		rcText.bottom = rcText.top + (rcFocus.bottom - rcFocus.top);
		::InflateRect(&rcText, 2, 0);
		CBlueRenderEngineUI::DoPaintFrame(hDC, pManager, rcText, UICOLOR_STANDARD_BLACK, 
			                              UICOLOR_STANDARD_BLACK, UICOLOR__INVALID, UIFRAME_FOCUS);
	}
}

//tabbutton states: normal, hot, pushed, selected and disable
//but now, we will not handled the state disable. 
RECT CBlueRenderEngineUI::DoPaintTabFolder(HDC hDC, CPaintManagerUI* pManager, const RECT& rcItem, const LPCTSTR pstrText, 
	                                       UINT uState,	UINT uTextStyle, UINT uTabAlign)
{
	ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
	//text attribute
	::SetBkMode(hDC, TRANSPARENT);
	::SelectObject(hDC, pManager->GetThemeFont(UIFONT_NORMAL));
	CStdString sText = pstrText;
	sText.ProcessResourceTokens();
	int cchText = sText.GetLength();
	RECT rcTabButton = rcItem;
	SIZE szTextPadding = { 12, 8 };
	//calculate the size of the tab button, default determined by rcItem, say,
	//the user of this operation
	int cxItem = rcItem.right - rcItem.left;
	int cyItem = rcItem.bottom - rcItem.top;
	if ((cxItem == 0) || (cyItem == 0))
	{
		//auto-estimate the size of the tab button
		SIZE szText = { 0 };
		::GetTextExtentPoint32(hDC, sText, cchText, &szText);

		//finally the cxText
		if (cxItem == 0) 
			szText.cx += szTextPadding.cx;
		else 
			szText.cx = cxItem;
		//finally the cyText
		if (cyItem == 0) 
			szText.cy += szTextPadding.cy;
		else 
			szText.cy = cyItem;
		//the rcTabButton
		rcTabButton.right = rcTabButton.left + szText.cx;
		rcTabButton.bottom = rcTabButton.top + szText.cy;
	}

	if ((uTextStyle & DT_CALCRECT ) != 0)
		return rcTabButton;

	//begin painting
	if ((uState & UISTATE_DISABLED) != 0)
	{
		//not support yet!
	} else if ((uState & UISTATE_SELECTED) != 0)
	{
		//selected states
		switch(uTabAlign)
		{
		    case TABALIGN_BOTTOM:
		    case TABALIGN_LEFT:
		    case TABALIGN_RIGHT:
		         TRACE( _T("This alignment style is not supported yet now, but the TABALIGN_TOP will be applied as a default.") );
		    case TABALIGN_TOP:
			default:
				{
					//border and background
					POINT ptTemp = { 0 };
					::SelectObject(hDC, pManager->GetThemePen(UICOLOR_TAB_BORDER));
					::MoveToEx(hDC, rcTabButton.left, rcTabButton.bottom, &ptTemp);
					::LineTo(hDC, rcTabButton.left, rcTabButton.top + 2);
					::LineTo(hDC, rcTabButton.left + 1, rcTabButton.top + 1);
					::LineTo(hDC, rcTabButton.right - 1, rcTabButton.top + 1);
					::LineTo(hDC, rcTabButton.right, rcTabButton.top + 2);
					::LineTo(hDC, rcTabButton.right, rcTabButton.bottom);
					::SelectObject(hDC, pManager->GetThemePen(UICOLOR_TAB_BACKGROUND_NORMAL));
					::LineTo(hDC, rcTabButton.left, rcTabButton.bottom);
					//text
					DoPaintQuickText(hDC, pManager, rcTabButton, pstrText, UICOLOR_TAB_TEXT_NORMAL, 
						             UIFONT_NORMAL, uTextStyle);
				}
				break;
		}
	} else if ((uState & UISTATE_PUSHED) != 0) 
	{
		switch(uTabAlign)
		{
			case TABALIGN_BOTTOM:
			case TABALIGN_RIGHT:
			case TABALIGN_LEFT:
				TRACE( _T("This alignment style is not supported yet now, but the TABALIGN_TOP will be applied as a default.") );
			case TABALIGN_TOP:
			default:
				{
					//tab rect
					rcTabButton.top += 3;
					//background
					DoFillRect(hDC, pManager, rcTabButton, UICOLOR_TAB_BUTTON_BGPUSHED, FALSE);
					//border
					POINT ptTemp = { 0 };
					::SelectObject(hDC, pManager->GetThemePen(UICOLOR_TAB_BORDER));
					::MoveToEx(hDC, rcTabButton.left, rcTabButton.bottom, &ptTemp);
					::LineTo(hDC, rcTabButton.left, rcTabButton.top);
					::LineTo(hDC, rcTabButton.right, rcTabButton.top);
					::LineTo(hDC, rcTabButton.right, rcTabButton.bottom);
					::LineTo(hDC, rcTabButton.left, rcTabButton.bottom);
					//text
					RECT rcText = rcTabButton;
					rcText.left += 2;
					rcText.top += 2;
					DoPaintQuickText(hDC, pManager, rcText, pstrText, UICOLOR_TAB_TEXT_NORMAL,
						             UIFONT_NORMAL, uTextStyle);
				}
				break;
		}
	} else if ((UISTATE_HOT & uState) != 0)
	{
		//hot state
		switch(uTabAlign)
		{
			case TABALIGN_BOTTOM:
			case TABALIGN_LEFT:
			case TABALIGN_RIGHT:
				TRACE( _T("This alignment style is not supported yet now, but the TABALIGN_TOP will be applied as a default.") );
			case TABALIGN_TOP:
			default:
				{
					//tab rect
					rcTabButton.top += 3;
					//background
					DoFillRect(hDC, pManager, rcTabButton, UICOLOR_TAB_BUTTON_BGHOVER, FALSE);
					//border
					POINT ptTemp = { 0 };
					::SelectObject(hDC, pManager->GetThemePen(UICOLOR_TAB_BORDER));
					::MoveToEx(hDC, rcTabButton.left, rcTabButton.bottom, &ptTemp);
					::LineTo(hDC, rcTabButton.left, rcTabButton.top);
					::LineTo(hDC, rcTabButton.right, rcTabButton.top);
					::LineTo(hDC, rcTabButton.right, rcTabButton.bottom);
					::LineTo(hDC, rcTabButton.left, rcTabButton.bottom);
					//text
					DoPaintQuickText(hDC, pManager, rcTabButton, pstrText, UICOLOR_TAB_TEXT_NORMAL, 
						             UIFONT_NORMAL, uTextStyle);
				}
				break;
		}
	} else
	{
		//normal state
		switch (uTabAlign)
		{
			case TABALIGN_BOTTOM:
			case TABALIGN_LEFT:
			case TABALIGN_RIGHT:
				TRACE( _T("This alignment style is not supported yet now, but the TABALIGN_TOP will be applied as a default.") );
			case TABALIGN_TOP:
			default:
				{
					//tab rect
					rcTabButton.top += 3;
					//background
					DoFillRect(hDC, pManager, rcTabButton, UICOLOR_TAB_BUTTON_BGNORMAL, FALSE);
					//border
					POINT ptTemp = { 0 };
					::SelectObject(hDC, pManager->GetThemePen(UICOLOR_TAB_BORDER));
					::MoveToEx(hDC, rcTabButton.left, rcTabButton.bottom, &ptTemp);
					::LineTo(hDC, rcTabButton.left, rcTabButton.top);
					::LineTo(hDC, rcTabButton.right, rcTabButton.top);
					::LineTo(hDC, rcTabButton.right, rcTabButton.bottom);
					::LineTo(hDC, rcTabButton.left, rcTabButton.bottom);
					//text
					DoPaintQuickText(hDC, pManager, rcTabButton, pstrText, UICOLOR_TAB_TEXT_NORMAL, 
						             UIFONT_NORMAL, uTextStyle);
				}
				break;
		}
	}
	return rcTabButton;
}

void CBlueRenderEngineUI::DoPaintToolbarButton(HDC hDC, CPaintManagerUI* pManager, RECT rc, LPCTSTR pstrText, 
	                                           SIZE szPadding, UINT uState)
{
	ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
	if ((uState & UISTATE_PUSHED) != 0)
	{
		DoPaintFrame(hDC, pManager, rc, UICOLOR_TOOL_BORDER_PUSHED, UICOLOR_TOOL_BORDER_PUSHED, 
			         UICOLOR_TOOL_BACKGROUND_PUSHED, 0);
		rc.top += 2;
		rc.left++;
	} else if ((uState & UISTATE_HOT) != 0)
	{
		DoPaintFrame(hDC, pManager, rc, UICOLOR_TOOL_BORDER_HOVER, UICOLOR_TOOL_BORDER_HOVER, 
			         UICOLOR_TOOL_BACKGROUND_HOVER, 0);
	} else if ((uState & UISTATE_DISABLED) != 0)
	{
		// TODO
	}
	RECT rcText = rc;
	int nLinks = 0;
	::InflateRect(&rcText, -szPadding.cx, -szPadding.cy);
	DoPaintPrettyText(hDC, pManager, rcText, pstrText, UICOLOR_TITLE_TEXT, UICOLOR__INVALID,
		              NULL, nLinks, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
}

int CBlueRenderEngineUI::DoPaintQuickText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, const CStdString& sText, 
	                                      UITYPE_COLOR iTextColor, UITYPE_FONT iFont, UINT uStyle)
{
	return DoPaintQuickText( hDC, pManager, rc, sText, pManager->GetThemeColor(iTextColor), iFont, uStyle );
}

int CBlueRenderEngineUI::DoPaintQuickText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, const CStdString& sText,
                                          COLORREF clrText, UITYPE_FONT iFont, UINT uStyle)
{
	ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
	::SetBkMode(hDC, TRANSPARENT);
	::SetTextColor(hDC, clrText);
	::SelectObject(hDC, pManager->GetThemeFont(iFont));
	return ::DrawText(hDC, (LPCTSTR)sText, sText.GetLength(), &rc, uStyle);
}

void CBlueRenderEngineUI::DoPaintGroupNode(HDC hDC,	CPaintManagerUI* pManager,	const RECT& rc,	UINT nNodeType,
                                        	UINT nImageID, BOOL bExpanded)
{
	LPUI_IMAGE_ITEM pImage;
	if (pManager->GetImage(nImageID, &pImage))
	{
		DoPaintGroupNode(hDC, pManager, rc, nNodeType, pImage, bExpanded);
	}
	
}

//绘制树视图中节点图标（标示该节点展开或者缩起的图标）
void CBlueRenderEngineUI::DoPaintGroupNode(HDC hDC, CPaintManagerUI* pManager, const RECT& rc,
		UINT nNodeType,	const LPUI_IMAGE_ITEM Item,	BOOL bExpanded)
{
	ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
	if (Item->pGraphic)
	{
		RECT rcPaint = {0};
		rcPaint.left = rc.left + (rc.right - rc.left - Item->pGraphic->GetWidth()) / 2;
		rcPaint.right = rcPaint.left + Item->pGraphic->GetWidth();
		rcPaint.top = rc.top + (rc.bottom - rc.top - Item->pGraphic->GetHeight() / 2) / 2;
		rcPaint.bottom = rcPaint.top + Item->pGraphic->GetHeight() / 2;
		if (rcPaint.left < rc.left)
			rcPaint.left = rc.left;
		if (rcPaint.right > rc.right)
			rcPaint.right = rc.right;
		if (rcPaint.top < rc.top)
			rcPaint.top = rc.top;
		if (rcPaint.bottom > rc.bottom)
			rcPaint.bottom = rc.bottom;
		if (bExpanded)
		{
			DoPaintGraphic(hDC, pManager, rcPaint, *Item, 1);
		} else
		{
			DoPaintGraphic(hDC, pManager, rcPaint, *Item, 0);
		}
	}
}

void CBlueRenderEngineUI::DoPaintCheckStatus(HDC hDC, CPaintManagerUI* pManager,	const RECT &rc, UINT nImageID,	int nStatus)
{
	ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
	CBlueRenderEngineUI::DoPaintGraphic(hDC, pManager, rc, nImageID, nStatus);
}

void CBlueRenderEngineUI::DoPaintPrettyText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, LPCTSTR pstrText, UITYPE_COLOR iTextColor, 
	                                        UITYPE_COLOR iBackColor, RECT* prcLinks, int& nLinkRects, UINT uStyle)
{
	ASSERT(::GetObjectType(hDC)==OBJ_DC || ::GetObjectType(hDC)==OBJ_MEMDC);
	COLORREF clrText = pManager->GetThemeColor(iTextColor);
	COLORREF clrBk = pManager->GetThemeColor(iBackColor);
	DoPaintPrettyText(hDC, pManager, rc, pstrText, clrText, clrBk, prcLinks, nLinkRects, uStyle);
}
   
void CBlueRenderEngineUI::DoPaintPrettyText(HDC hDC, CPaintManagerUI* pManager, RECT& rc, LPCTSTR pstrText, COLORREF clrText, 
	                                        COLORREF clrBack, RECT* prcLinks, int& nLinkRects, UINT uStyle)
{
	//DWORD dwPaint = ::GetTickCount(); LPCTSTR lpstrtmp = pstrText;
	// The string formatter supports a kind of "mini-html" that consists of various short tags:
	//
	//   Link:             <a>text</a>
	//   Change font:      <f x>        where x = font id
	//   Bold:             <b>text</b>
	//   Indent:           <x i>        where i = indent in pixels
	//   Paragraph:        <p>
	//   Horizontal line:  <h>
	//   Icon:             <i x y>      where x = icon id and (optional) y = size (16/32/50)
	//                     <i x>        where x = icon resource name
	//   Color:            <c #xxxxxx>  where x = RGB in hex
	//                     <c x>        where x = color id
	//
	// In addition the standard string resource formats apply:
	//
	//   %{n}                           where n = resource-string-id
	//
	if (::IsRectEmpty(&rc)) 
		return;
	
	bool bDraw = (uStyle & DT_CALCRECT) == 0;
	
	RECT rcClip = { 0 };
	::GetClipBox(hDC, &rcClip);
	HRGN hOldRgn = ::CreateRectRgnIndirect(&rcClip);
	HRGN hRgn = ::CreateRectRgnIndirect(&rc);
	if (bDraw) 
		::ExtSelectClipRgn(hDC, hRgn, RGN_AND);

	//disable this for performance
	//   CStdString sText = pstrText;
	//   sText.ProcessResourceTokens();
	//   pstrText = sText;
	
	HFONT hOldFont = (HFONT) ::SelectObject(hDC, pManager->GetThemeFont(UIFONT_NORMAL));
	::SetBkMode(hDC, TRANSPARENT);
	::SetTextColor(hDC, clrText);

	// If the drawstyle includes an alignment, we'll need to first determine the text-size so
	// we can draw it at the correct position...
	if (((uStyle & DT_SINGLELINE) != 0) 
		&& ((uStyle & DT_VCENTER) != 0)
		&& ((uStyle & DT_CALCRECT) == 0))
	{
		RECT rcText = { 0, 0, 9999, 100 };
		int nLinks = 0;
		DoPaintPrettyText(hDC, pManager, rcText, pstrText, clrText, clrBack, NULL, nLinks, uStyle | DT_CALCRECT);
		rc.top = rc.top + ((rc.bottom - rc.top) / 2) - ((rcText.bottom - rcText.top) / 2);
		rc.bottom = rc.top + (rcText.bottom - rcText.top);
	}
	if (((uStyle & DT_SINGLELINE) != 0)
		&& ((uStyle & DT_CENTER) != 0)
		&& ((uStyle & DT_CALCRECT) == 0))
	{
		RECT rcText = { 0, 0, 9999, 100 };
		int nLinks = 0;
		DoPaintPrettyText(hDC, pManager, rcText, pstrText, clrText, clrBack, NULL, nLinks, uStyle | DT_CALCRECT);
		::OffsetRect(&rc, (rc.right - rc.left) / 2 - (rcText.right - rcText.left) / 2, 0);
	}
	if (((uStyle & DT_SINGLELINE) != 0)
		&& ((uStyle & DT_RIGHT) != 0) 
		&& ((uStyle & DT_CALCRECT) == 0))
	{
		RECT rcText = { 0, 0, 9999, 100 };
		int nLinks = 0;
		DoPaintPrettyText(hDC, pManager, rcText, pstrText, clrText, clrBack, NULL, nLinks, uStyle | DT_CALCRECT);
		rc.left = rc.right - (rcText.right - rcText.left);
	}
	
	// Paint backhground
	COLORREF clrInvalid = pManager->GetThemeColor(UICOLOR__INVALID);
	if (clrBack != clrInvalid)
		DoFillRect(hDC, pManager, rc, clrBack, FALSE);

	// Determine if we're hovering over a link, because we would like to
	// indicate it by coloring the link text.
	// BUG: This assumes that the prcLink has already been filled once with
	//      link coordinates! That is usually not the case at first repaint. We'll clear
	//      the remanining entries at exit.
	int i;
	bool bHoverLink = false;
	POINT ptMouse = pManager->GetMousePos();
	for (i = 0; !bHoverLink && i < nLinkRects; i++)
	{
		if (::PtInRect(prcLinks + i, ptMouse))
			bHoverLink = true;
	}
	
	TEXTMETRIC tm = pManager->GetThemeFontInfo(UIFONT_NORMAL);
	POINT pt = { rc.left, rc.top };
	int iLineIndent = 0;
	int iLinkIndex = 0;
	int cyLine = tm.tmHeight + tm.tmExternalLeading;
	int cyMinHeight = 0;
	POINT ptLinkStart = { 0 };
	bool bInLink = false;
	
	while (*pstrText != '\0')
	{
		if ((pt.x >= rc.right) || (*pstrText == '\n'))
		{
			// A new link was detected/requested. We'll adjust the line height
			// for the next line and expand the link hitbox (if any)
			if (bInLink && (iLinkIndex < nLinkRects)) 
				::SetRect(&prcLinks[iLinkIndex++], ptLinkStart.x, ptLinkStart.y, pt.x, pt.y + tm.tmHeight);
			if ((uStyle & DT_SINGLELINE) != 0) 
				break;
			if (*pstrText == '\n') 
				pstrText++;
			pt.x = rc.left + iLineIndent;
			pt.y += cyLine - tm.tmDescent;
			ptLinkStart = pt;
			cyLine = tm.tmHeight + tm.tmExternalLeading;
			if (pt.x >= rc.right)
				break;
			while (*pstrText == ' ')
				pstrText++;
		}  else if ((*pstrText == '<')
			        && ((pstrText[1] >= 'a') && (pstrText[1] <= 'z'))
					&& ((pstrText[2] == ' ') || (pstrText[2] == '>')))
		{
			pstrText++;
			switch (*pstrText++)
			{
			    case 'a':  // Link
					{
						::SetTextColor(hDC, pManager->GetThemeColor(bHoverLink ? UICOLOR_LINK_TEXT_HOVER : UICOLOR_LINK_TEXT_NORMAL));
						::SelectObject(hDC, pManager->GetThemeFont(UIFONT_LINK));
						tm = pManager->GetThemeFontInfo(UIFONT_LINK);
						cyLine = MAX(cyLine, tm.tmHeight + tm.tmExternalLeading);
						ptLinkStart = pt;
						bInLink = true;
					}
					break;
				case 'f':  // Font
					{
						UITYPE_FONT iFont = (UITYPE_FONT) _tcstol(pstrText, const_cast<LPTSTR*>(&pstrText), 10);
						::SelectObject(hDC, pManager->GetThemeFont(iFont));
						tm = pManager->GetThemeFontInfo(iFont);
						cyLine = MAX(cyLine, tm.tmHeight + tm.tmExternalLeading);
					}
					break;
				case 'b':  // Bold text
					{
						::SelectObject(hDC, pManager->GetThemeFont(UIFONT_BOLD));
						tm = pManager->GetThemeFontInfo(UIFONT_BOLD);
						cyLine = MAX(cyLine, tm.tmHeight + tm.tmExternalLeading);
					}
					break;
				case 'x':  // Indent
					{
						iLineIndent = (int) _tcstol(pstrText, const_cast<LPTSTR*>(&pstrText), 10);
						if (pt.x < rc.left + iLineIndent)
							pt.x = rc.left + iLineIndent;
					}
					break;
				case 'p':  // Paragraph
					{
						pt.x = rc.right;
						cyLine = MAX(cyLine, tm.tmHeight + tm.tmExternalLeading) + 5;
						iLineIndent = 0;
						::SelectObject(hDC, pManager->GetThemeFont(UIFONT_NORMAL));
						::SetTextColor(hDC, clrText);
						tm = pManager->GetThemeFontInfo(UIFONT_NORMAL);
					}
					break;
				case 'h':  // Horizontal line
					{
						::SelectObject(hDC, pManager->GetThemePen(UICOLOR_STANDARD_GREY));
						if (bDraw)
						{
							POINT ptTemp = { 0 };
							::MoveToEx(hDC, pt.x, pt.y + 5, &ptTemp);
							::LineTo(hDC, rc.right - iLineIndent, pt.y + 5);
						}
						cyLine = 12;
					}
					break;
				case 'i':  // Icon
					{
						int iSize = 16;
						if (*pstrText == ' ')
							pstrText++;
						if (isdigit(*pstrText))
						{
							UINT iIndex = (UINT) _tcstol(pstrText, const_cast<LPTSTR*>(&pstrText), 10);
							iSize = MAX(16, _ttoi(pstrText));
							if (bDraw)
							{
								//
								RECT rcImage = {pt.x, pt.y, pt.x + iSize, pt.y + iSize};
								DoPaintGraphic(hDC, pManager, rcImage, iIndex);
							}
						} else 
						{
							if (*pstrText == ' ')
								pstrText++;
							CStdString sRes;
							while (_istalnum(*pstrText) || (*pstrText == '.') || (*pstrText == '_')) 
								sRes += *pstrText++;
							HICON hIcon = (HICON) ::LoadImage(pManager->GetResourceInstance(), sRes, IMAGE_ICON,
								                              0, 0, LR_LOADTRANSPARENT | LR_SHARED);
							if (hIcon)
							{
								ICONINFO ii = { 0 };
								::GetIconInfo(hIcon, &ii);
								BITMAP bi = { 0 };
								::GetObject(ii.hbmColor, sizeof(BITMAP), &bi);
								iSize = bi.bmWidth;
								if (bDraw)
									::DrawIconEx(hDC, pt.x, pt.y, hIcon, iSize, iSize, 0, NULL, DI_NORMAL);
								::DestroyIcon(hIcon);
							} else
							{
								//
							}
						}
						// A special feature with an icon at the left edge is that it also sets
						// the paragraph indent.
						if (pt.x == rc.left) 
							iLineIndent = iSize + (iSize / 8); 
						else cyLine = MAX(iSize, cyLine);
						pt.x += iSize + (iSize / 8);
						cyMinHeight = pt.y + iSize;
					}
					break;
				case 'c':  // Color
					{
						if (*pstrText == ' ') 
							pstrText++;
						if (*pstrText == '#')
						{
							pstrText++;
							COLORREF clrColor = _tcstol(pstrText, const_cast<LPTSTR*>(&pstrText), 16);
							clrColor = RGB(GetBValue(clrColor), GetGValue(clrColor), GetRValue(clrColor));
							::SetTextColor(hDC, clrColor);
						}  else 
						{
							UITYPE_COLOR Color = (UITYPE_COLOR) _tcstol(pstrText, const_cast<LPTSTR*>(&pstrText), 10);
							::SetTextColor(hDC, pManager->GetThemeColor(Color));
						}
					}
					break;	
				case 'g': //<g url=http://www.smartdot.com.cn/k.jpg/>
					{
						if (*pstrText == ' ')
							pstrText ++;
						LPCTSTR pStart = pstrText;
						while ((*pstrText != '\0') && (*pstrText != '>'))
							pstrText ++;
						if (*pstrText != '\0')
						{
							int nChars = pstrText  - pStart - 1; //减去 />
							TCHAR *szTmp = new TCHAR[nChars + 1];
							memset(szTmp, 0, sizeof(TCHAR) * (nChars + 1));
							memmove(szTmp, pStart, sizeof(TCHAR) * nChars);
							int nId = pManager->GetGraphicLinkImageId(szTmp);
							if (nId > 0)
							{ 
								RECT rcImage = {pt.x, pt.y, pt.x + 20, pt.y + 20};
								LPUI_IMAGE_ITEM pImage;
								if (pManager->GetImage(nId, &pImage))
								{
									if (::PtInRect(&rcImage, ptMouse))
									{
										if (pImage->pGraphic)
										{
											int w = pImage->pGraphic->GetWidth();
											int h = pImage->pGraphic->GetHeight();
											rcImage.left = 10;
											rcImage.right = 10 + w;
											rcImage.top = pt.y - h;
											rcImage.bottom = pt.y;
											DoPaintGraphic(hDC, pManager, rcImage, nId);
										}
									}  else
									{
										DoPaintGraphic(hDC, pManager, rcImage, nId);
										pt.x += 22;
									}
								} //end if (pManager->GetImage(
							}
							delete []szTmp;
						}
					}
            } //end switch(..
			while ((*pstrText != '\0') && (*pstrText != '>')) 
				pstrText++;
			if (*pstrText != '\0')
				pstrText++;
	    } else if ((*pstrText == '<') && (pstrText[1] == '/'))
	    {
			pstrText += 2;
			switch (*pstrText++)
			{
			    case 'a':
					{
						if (iLinkIndex < nLinkRects) 
							::SetRect(&prcLinks[iLinkIndex++], ptLinkStart.x, ptLinkStart.y, pt.x, 
							          pt.y + tm.tmHeight + tm.tmExternalLeading);
						::SetTextColor(hDC, clrText);
						::SelectObject(hDC, pManager->GetThemeFont(UIFONT_NORMAL));
						tm = pManager->GetThemeFontInfo(UIFONT_NORMAL);
						bInLink = false;
					}
					break;
				case 'f':
				case 'b':
					{
						// TODO: Use a context stack instead
						::SelectObject(hDC, pManager->GetThemeFont(UIFONT_NORMAL));
						tm = pManager->GetThemeFontInfo(UIFONT_NORMAL);
					}
					break;
				case 'c':
					::SetTextColor(hDC, clrText);
					break;
			} //end switch(..
			while ((*pstrText != '\0') && (*pstrText != '>')) 
				pstrText++;
			pstrText++;
		}  else if (*pstrText == '&')
		{
			if ((uStyle & DT_NOPREFIX) == 0)
			{
				if (bDraw  && pManager->GetSystemSettings().bShowKeyboardCues) 
					::TextOut(hDC, pt.x, pt.y, _T("_"), 1);
			} else 
			{
				SIZE szChar = { 0 };
				::GetTextExtentPoint32(hDC, _T("&"), 1, &szChar);
				if (bDraw) 
					::TextOut(hDC, pt.x, pt.y, _T("&"), 1);
				pt.x += szChar.cx;
			}
			pstrText++;
		} else if (*pstrText == ' ')
		{
			SIZE szSpace = { 0 };
			::GetTextExtentPoint32(hDC, _T(" "), 1, &szSpace);
			// Still need to paint the space because the font might have
			// underline formatting.
			if (bDraw) 
				::TextOut(hDC, pt.x, pt.y, _T(" "), 1);
			pt.x += szSpace.cx;
			pstrText++;
		} else
		{
			POINT ptPos = pt;
			int cchChars = 0;
			int cchLastGoodWord = 0;
			LPCTSTR p = pstrText;
			SIZE szText = { 0 };
			if (*p == '<')
			{
				p++;
				cchChars++;
			}
			while ((*p != '\0') && (*p != '<') && (*p != '\n') && (*p != '&'))
			{
				// This part makes sure that we're word-wrapping if needed or providing support
				// for DT_END_ELLIPSIS. Unfortunately the GetTextExtentPoint32() call is pretty
				// slow when repeated so often.
				// TODO: Rewrite and use GetTextExtentExPoint() instead!
				cchChars++;
				szText.cx = (cchChars + 2) * tm.tmMaxCharWidth;
				if ((pt.x + szText.cx) >= rc.right)
				{
					::GetTextExtentPoint32(hDC, pstrText, cchChars, &szText);
				}
				if ((pt.x + szText.cx ) >= rc.right)
				{
					if (((uStyle & DT_WORDBREAK) != 0) && (cchLastGoodWord > 0))
					{
						cchChars = cchLastGoodWord;
						pt.x = rc.right;
					}
					if (((uStyle & DT_END_ELLIPSIS) != 0) && (cchChars > 2))
					{
						cchChars -= 2;
						pt.x = rc.right;
					}
					break;
				}
				if (*p == ' ') 
					cchLastGoodWord = cchChars;//bugs!造成文字显示不全! TBD
				p = ::CharNext(p);
			}
			if (cchChars > 0)
			{
				::GetTextExtentPoint32(hDC, pstrText, cchChars, &szText);
				if (bDraw)
				{
					::TextOut(hDC, ptPos.x, ptPos.y, pstrText, cchChars);
					if ((pt.x == rc.right) && ((uStyle & DT_END_ELLIPSIS) != 0)) 
						::TextOut(hDC, rc.right - 10, ptPos.y, _T("..."), 3);
				}
				pt.x += szText.cx;
				pstrText += cchChars;
			}
		}
		ASSERT(iLinkIndex <= nLinkRects);
    } //end while(...
	
	// Clear remaining link rects and return number of used rects
	for (i = iLinkIndex; i < nLinkRects; i++) 
		::ZeroMemory(prcLinks + i, sizeof(RECT));
	nLinkRects = iLinkIndex;
	
	// Return size of text when requested
	if ((uStyle & DT_CALCRECT) != 0)
	{
		rc.bottom = MAX(cyMinHeight, pt.y + cyLine);
		if (rc.right >= 9999)
		{
			if (_tcslen(pstrText) > 0)
				pt.x += 3;
			rc.right = pt.x;
		}
	}
	
	if (bDraw)
		::SelectClipRgn(hDC, hOldRgn);
	::DeleteObject(hOldRgn);
	::DeleteObject(hRgn);
	
	::SelectObject(hDC, hOldFont);
	//TRACE( _T("DoPaintPrettyText, %s, %d"), lpstrtmp, ::GetTickCount()-dwPaint );
}

void CBlueRenderEngineUI::DoPaintGradient(HDC hDC, CPaintManagerUI *pManager, POINT ptTriangle1, POINT ptTriangle2, 
	                                      POINT ptTriangle3, COLORREF crC1, COLORREF crC2, COLORREF crC3)
{
	if (!m_lpGradientFill)
		m_lpGradientFill = (PGradientFill) ::GetProcAddress(::GetModuleHandle(L"msimg32.dll"), "GradientFill");
	if (m_lpGradientFill)
	{
		//
		TRIVERTEX triv[3] = {
                               { ptTriangle1.x, ptTriangle1.y, GetRValue(crC1) << 8, GetGValue(crC1) << 8, GetBValue(crC1) << 8, 0xFF00 },
                               { ptTriangle2.x, ptTriangle2.y, GetRValue(crC2) << 8, GetGValue(crC2) << 8, GetBValue(crC2) << 8, 0xFF00 },
		                       { ptTriangle3.x, ptTriangle3.y, GetRValue(crC3) << 8, GetGValue(crC3) << 8, GetBValue(crC3) << 8, 0xFF00 }
                            };
		GRADIENT_TRIANGLE grc = {0, 1, 2 };
		m_lpGradientFill(hDC, triv, 3, &grc, 1, GRADIENT_FILL_TRIANGLE);
	}
}

void CBlueRenderEngineUI::DoPaintGradient(HDC hDC, CPaintManagerUI* pManager, RECT rc, COLORREF clrFirst, 
	                                      COLORREF clrSecond, bool bVertical, int nSteps)
{
	if (!m_lpGradientFill)
		m_lpGradientFill = (PGradientFill) ::GetProcAddress(::GetModuleHandle(L"msimg32.dll"), "GradientFill");
	if (m_lpGradientFill != NULL) 
	{
		// Use Windows gradient function from msimg32.dll
		// It may be slower than the code below but makes really pretty gradients on 16bit colors.
		TRIVERTEX triv[2] = {
			                   { rc.left, rc.top, GetRValue(clrFirst) << 8, GetGValue(clrFirst) << 8, GetBValue(clrFirst) << 8, 0xFF00 },
					           { rc.right, rc.bottom, GetRValue(clrSecond) << 8, GetGValue(clrSecond) << 8, GetBValue(clrSecond) << 8, 0xFF00 }
                             };
		GRADIENT_RECT grc = { 0, 1 };
		m_lpGradientFill(hDC, triv, 2, &grc, 1, bVertical ? GRADIENT_FILL_RECT_V : GRADIENT_FILL_RECT_H);
	}  else
	{
		// Determine how many shades
		int nShift = 1;
		if (nSteps >= 64)
			nShift = 6;
		else if (nSteps >= 32) 
			nShift = 5;
		else if (nSteps >= 16) 
			nShift = 4;
		else if (nSteps >= 8) 
			nShift = 3;
		else if (nSteps >= 4) 
			nShift = 2;
		int nLines = 1 << nShift;
		for (int i = 0; i < nLines; i++) 
		{
			// Do a little alpha blending
			BYTE bR = (BYTE) ((GetRValue(clrSecond) * (nLines - i) + GetRValue(clrFirst) * i) >> nShift);
			BYTE bG = (BYTE) ((GetGValue(clrSecond) * (nLines - i) + GetGValue(clrFirst) * i) >> nShift);
			BYTE bB = (BYTE) ((GetBValue(clrSecond) * (nLines - i) + GetBValue(clrFirst) * i) >> nShift);
			// ... then paint with the resulting color
			HBRUSH hBrush = ::CreateSolidBrush(RGB(bR, bG, bB));
			RECT r2 = rc;
			if (bVertical) 
			{
				r2.bottom = rc.bottom - ((i * (rc.bottom - rc.top)) >> nShift);
				r2.top = rc.bottom - (((i + 1) * (rc.bottom - rc.top)) >> nShift);
				if ((r2.bottom - r2.top) > 0) 
					::FillRect(hDC, &r2, hBrush);
			} else 
			{
				r2.left = rc.right - (((i + 1) * (rc.right - rc.left)) >> nShift);
				r2.right = rc.right - ((i * (rc.right - rc.left)) >> nShift);
				if ((r2.right - r2.left) > 0) 
					::FillRect(hDC, &r2, hBrush);
			}
			::DeleteObject(hBrush);
		}
	}
}

void CBlueRenderEngineUI::DoShiftBitmap(HDC hDC, CPaintManagerUI *pManager,	int r, int g, int b, RECT rc)
{
	for (int x = rc.left; x < rc.right; x ++)
	{
		for (int y = rc.top; y < rc.bottom; y ++)
		{
			COLORREF clr = ::GetPixel(hDC, x, y);
			int R = min(255, GetRValue(clr) + r);
			int G = min(255, GetGValue(clr) + g);
			int B = min(255, GetBValue(clr) + b);
			::SetPixel(hDC, x, y, RGB(R, G, B));
		}
	}
}

void CBlueRenderEngineUI::DoPaintAlphaBitmap(HDC hDC, CPaintManagerUI* pManager, HBITMAP hBitmap, RECT rc, BYTE iAlpha)
{
	// Alpha blitting is only supported of the msimg32.dll library is located on the machine.
	typedef BOOL (WINAPI *LPALPHABLEND)(HDC, int, int, int, int,HDC, int, int, int, int, BLENDFUNCTION);
	static LPALPHABLEND lpAlphaBlend = (LPALPHABLEND) ::GetProcAddress(::GetModuleHandle(L"msimg32.dll"), 
	                 "AlphaBlend");
	if (lpAlphaBlend == NULL)
		return;
	if (hBitmap == NULL)
		return;
	HDC hCloneDC = ::CreateCompatibleDC(pManager->GetPaintDC());
	HBITMAP hOldBitmap = (HBITMAP) ::SelectObject(hCloneDC, hBitmap);
	int cx = rc.right - rc.left;
	int cy = rc.bottom - rc.top;
	::SetStretchBltMode(hDC, COLORONCOLOR);
	BLENDFUNCTION bf = { 0 };
	bf.BlendOp = AC_SRC_OVER; 
	bf.BlendFlags = 0; 
	bf.AlphaFormat = AC_SRC_ALPHA;
	bf.SourceConstantAlpha = iAlpha;
	lpAlphaBlend(hDC, rc.left, rc.top, cx, cy, hCloneDC, 0, 0, cx, cy, bf);
	::SelectObject(hCloneDC, hOldBitmap);
	::DeleteDC(hCloneDC);
}

void CBlueRenderEngineUI::GrayFrame(HDC hdc, int nLeft, int nTop, int nRight, int nBottom)
{
	DWORD r,g, b;
	BYTE  c;
	COLORREF nColor;
	for (int i = nLeft; i < nRight; i ++)
	{
		for (int j = nTop; j < nBottom; j ++)
		{
			nColor = ::GetPixel(hdc, i, j);
			r = GetRValue(nColor);
			g = GetGValue(nColor);
			b = GetBValue(nColor);
			c = (BYTE)( ((306 * r + 601 * g + 117 * b) >> 10) & 0xFF );
			nColor = RGB(c, c, c);
			::SetPixel(hdc, i, j, nColor);
		}
	}
}

void CBlueRenderEngineUI::DoAnimateWindow(HWND hWnd, UINT uStyle, DWORD dwTime /*= 200*/)
{
	typedef BOOL (CALLBACK* PFNANIMATEWINDOW)(HWND, DWORD, DWORD);
#ifndef AW_HIDE
	const DWORD AW_HIDE = 0x00010000;
	const DWORD AW_BLEND = 0x00080000;
#endif
	// Mix flags
	DWORD dwFlags = 0;
	if ((uStyle & UIANIM_HIDE) != 0)
		dwFlags |= AW_HIDE;
	if ((uStyle & UIANIM_FADE) != 0)
		dwFlags |= AW_BLEND;
	PFNANIMATEWINDOW pfnAnimateWindow = (PFNANIMATEWINDOW) ::GetProcAddress(::GetModuleHandle(L"user32.dll"), "AnimateWindow");
	if (pfnAnimateWindow != NULL) 
		pfnAnimateWindow(hWnd, dwTime, dwFlags);
}

HBITMAP CBlueRenderEngineUI::GenerateAlphaBitmap(CPaintManagerUI* pManager, CControlUI* pControl, 
	                                             RECT rc, UITYPE_COLOR Background)
{
	typedef BOOL (WINAPI *LPALPHABLEND)(HDC, int, int, int, int,HDC, int, int, int, int, BLENDFUNCTION);
	static FARPROC lpAlphaBlend = ::GetProcAddress(::GetModuleHandle(_T("msimg32.dll")), "AlphaBlend");
	if (lpAlphaBlend == NULL) 
		return NULL;
	int cx = rc.right - rc.left;
	int cy = rc.bottom - rc.top;

	// Let the control paint itself onto an offscreen bitmap
	HDC hPaintDC = ::CreateCompatibleDC(pManager->GetPaintDC());
	HBITMAP hPaintBitmap = ::CreateCompatibleBitmap(pManager->GetPaintDC(), rc.right, rc.bottom);
	ASSERT(hPaintDC);
	ASSERT(hPaintBitmap);
	HBITMAP hOldPaintBitmap = (HBITMAP) ::SelectObject(hPaintDC, hPaintBitmap);
	DoFillRect(hPaintDC, pManager, rc, Background, FALSE);
	pControl->DoPaint(hPaintDC, rc);

	// Create a new 32bpp bitmap with room for an alpha channel
	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = cx;
	bmi.bmiHeader.biHeight = cy;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = cx * cy * sizeof(DWORD);
	LPDWORD pDest = NULL;
	HDC hCloneDC = ::CreateCompatibleDC(pManager->GetPaintDC());
	HBITMAP hBitmap = ::CreateDIBSection(pManager->GetPaintDC(), &bmi, DIB_RGB_COLORS, (LPVOID*) &pDest, NULL, 0);
	ASSERT(hCloneDC);
	ASSERT(hBitmap);
	if (hBitmap != NULL)
	{
		// Copy offscreen bitmap to our new 32bpp bitmap
		HBITMAP hOldBitmap = (HBITMAP) ::SelectObject(hCloneDC, hBitmap);
		::BitBlt(hCloneDC, 0, 0, cx, cy, hPaintDC, rc.left, rc.top, SRCCOPY);
		::SelectObject(hCloneDC, hOldBitmap);
		::DeleteDC(hCloneDC);  
		::GdiFlush();

		// Make the background color transparent
		COLORREF clrBack = pManager->GetThemeColor(Background);
		DWORD dwKey = RGB(GetBValue(clrBack), GetGValue(clrBack), GetRValue(clrBack));
		DWORD dwShowColor = 0xFF000000;
		for (int y = 0; y < abs(bmi.bmiHeader.biHeight); y++)
		{
			for (int x = 0; x < bmi.bmiHeader.biWidth; x++)
			{
				if (*pDest != dwKey ) 
					*pDest = *pDest | dwShowColor; 
				else *pDest = 0x00000000;
				pDest++;
			}
		}
	}

	// Cleanup
	::SelectObject(hPaintDC, hOldPaintBitmap);
	::DeleteObject(hPaintBitmap);
	::DeleteDC(hPaintDC);

	return hBitmap;
}

