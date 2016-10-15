// GifAnimate.cpp : CGifAnimate 的实现

#include "stdafx.h"
#include "GifAnimate.h"
#include <time.h>
#include <CommonLib/StringUtils.h>
#include <Commonlib/DebugLog.h>
#include <gdiplus.h>

#pragma warning(disable:4996)

#define WM_GIFANIMATE    WM_USER + 0x200

HRESULT CGifAnimate::OnDraw(ATL_DRAWINFO& di)
{
	if ((m_pImage) && (IsVisible(di)))
	{
		GetAmbientUserMode(m_bRunMode);
		if (m_bRunMode)
		{
			//运行时
			RECT rc = *(RECT *)(di.prcBounds);
			/*PRINTDEBUGLOG(dtInfo, "RECT(x:%d y:%d r:%d b:%d), Bound RECT (x:%d y:%d r:%d b:%d)",
				m_rcPos.left, m_rcPos.top, m_rcPos.right, m_rcPos.bottom,
				rc.left, rc.top, rc.right, rc.bottom);*/
			m_pImage->Paint(di.hdcDraw, *(RECT *)(di.prcBounds)); 
			m_rcPaint = *(RECT *)(di.prcBounds);
			m_tmLastTime = ::time(NULL);
		} else
		{
			//设计时
		}
	}
	return S_OK;
}
CGifAnimate::CGifAnimate()
{
	m_tmLastTime = ::time(NULL);
}

CGifAnimate::~CGifAnimate()
{
	m_spAdviseSink = NULL;
	if (m_pImage)
	{
		delete m_pImage;
		m_pImage = NULL;
	}		
	CGdiPlusImage::DestroyGdiPlus();
}

BOOL CGifAnimate::IsVisible(ATL_DRAWINFO &di)
{
	return ::RectVisible(di.hdcDraw, (RECT*)di.prcBounds);
}

void CGifAnimate::OnInvalidate(LPRECT lpRect)
{
	//if (m_hPaintWnd)
	//	::SendMessage(m_hPaintWnd, WM_GIFANIMATE, 0, (LPARAM)(&m_rcPos));
	DrawCurrFrame();
}

HRESULT CGifAnimate::IOleObject_Advise(IAdviseSink *pAdvSink, DWORD *pdwConnection)
{
	//m_spAdviseSink = pAdvSink;
	return CComControl<CGifAnimate>::IOleObject_Advise(pAdvSink, pdwConnection);
}

HRESULT CGifAnimate::OnPosRectChange(LPCRECT lpcRect)
{
	m_rcPos = *lpcRect;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CGifAnimate::Play()
{
    m_bPlay = TRUE;
	return S_OK;
}

void CGifAnimate::DrawCurrFrame()
{
    if ((m_hParentWnd && m_pImage) && m_bPlay)
	{ 
		/*RECT rc = {0};
		HWND hWnd = GetDisplayWndRect(&rc);
		if (hWnd != NULL)*/ 
		if (::time(NULL) - m_tmLastTime < 20)  //20秒无更新即不更新
			::InvalidateRect(m_hPaintWnd, &m_rcPaint, FALSE);
		/*if (m_bInPlaceActive)
		{
			if (m_bWndLess && m_spInPlaceSite != NULL)
				m_spInPlaceSite->InvalidateRect(NULL, FALSE);
		} else
			SendOnViewChange(DVASPECT_CONTENT);*/
		//FireViewChange();
	}
}

HRESULT STDMETHODCALLTYPE CGifAnimate::LoadFile(BSTR newVal, ULONG hWnd)
{
	m_bPlay = FALSE;
	TCHAR szTemp[MAX_PATH] = {0};
	::_tcscpy(szTemp, OLE2T(newVal));
	memset(m_szFileName, 0, MAX_PATH);
	CStringConversion::WideCharToString(szTemp, m_szFileName, MAX_PATH);
	if (!m_pImage)
	{
		RECT rc = m_rcPos;
		rc.right = rc.left;
		rc.bottom = rc.top;
		m_pImage = new CGdiPlusGif(this, m_szFileName);
		
		
		//
		/*SIZEL size5,size6;
		size5.cx = m_pImage->GetImageWidth();
		size5.cy = m_pImage->GetImageHeight();;
		AtlPixelToHiMetric(&size5, &size6);
		m_rcPos.right = m_pImage->GetImageWidth()+m_rcPos.left;
		m_rcPos.bottom = m_pImage->GetImageHeight()+m_rcPos.top;
		SetExtent(DVASPECT_CONTENT, &size6);
		if ((m_spInPlaceSite != NULL) && m_bInPlaceActive)
			m_spInPlaceSite->OnPosRectChange(&m_rcPos);
		else if (m_hWnd != NULL)
			SetWindowPos(NULL, m_rcPos.left, m_rcPos.top, m_pImage->GetImageWidth(), m_pImage->GetImageHeight(),
			                 SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE);*/

	}
	m_hPaintWnd = (HWND)hWnd;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CGifAnimate::LoadBitmap(ULONG hBitmap, ULONG hParent)
{
	m_bPlay = FALSE; 
	if (!m_pImage)
	{
		RECT rc = m_rcPos;
		rc.right = rc.left;
		rc.bottom = rc.top;
		m_pImage = new CGdiPlusGif(this, (HBITMAP)hBitmap);
	}
	m_hPaintWnd = (HWND)hParent;
	return S_OK;
}

LRESULT CGifAnimate::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	return S_OK;
}

LRESULT CGifAnimate::OnErase(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	return S_OK;
}

LRESULT CGifAnimate::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	return 1;
}

HRESULT CGifAnimate::IPersistStreamInit_Save(LPSTREAM pStm, BOOL  fClearDirty, const ATL_PROPMAP_ENTRY* pMap)
{
	_ASSERTE(pMap != NULL);
	if (pStm)
	{
		return S_OK;
	}
 	return E_NOTIMPL;
}

HRESULT CGifAnimate::IPersistStreamInit_Load(LPSTREAM pStm, const ATL_PROPMAP_ENTRY* pMap)
{
	_ASSERTE(pMap != NULL);
 	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CGifAnimate::SetClientSite(IOleClientSite *pClientSite)
{
      return IOleObjectImpl<CGifAnimate>::SetClientSite (pClientSite);
}

HRESULT CGifAnimate::InPlaceActivate(LONG iVerb, const RECT* prcPosRect)
{
	return CComControlBase::InPlaceActivate(iVerb, prcPosRect);
}

HRESULT STDMETHODCALLTYPE CGifAnimate::SetExtent(DWORD dwDrawAspect, SIZEL *psizel)
{
	return IOleObjectImpl<CGifAnimate>::SetExtent(dwDrawAspect, psizel);
}

HRESULT STDMETHODCALLTYPE CGifAnimate::GetExtent(DWORD dwDrawAspect, SIZEL *psizel)
{
	if (m_pImage)
	{
		SIZE sizel;
		sizel.cx = m_pImage->GetImageWidth();
		sizel.cy = m_pImage->GetImageHeight();
		AtlPixelToHiMetric(&sizel, psizel);
		m_sizeExtent = *psizel;
		m_sizeNatural = *psizel; //unscaled size in himetric 
	}
	return IOleObjectImpl<CGifAnimate>::GetExtent(dwDrawAspect, psizel);
}

HRESULT STDMETHODCALLTYPE CGifAnimate::CanInPlaceActivate()
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CGifAnimate::DoVerb(LONG iVerb, LPMSG  pMsg , IOleClientSite* pActiveSite, LONG  lindex ,
									 HWND hwndParent, LPCRECT lprcPosRect)
{
	HRESULT hr = IOleObjectImpl<CGifAnimate>::DoVerb(iVerb, pMsg, pActiveSite, lindex, hwndParent, lprcPosRect);
	
	if (lprcPosRect)
		m_rcPos = *lprcPosRect;
	if (iVerb == OLEIVERB_SHOW)
	{
		m_hParentWnd = hwndParent;
		DrawCurrFrame();
	}
	return hr;
}

HRESULT STDMETHODCALLTYPE CGifAnimate::SetObjectRects(LPCRECT lprcPosRect, LPCRECT lprcClipRect)
{
	return  IOleInPlaceObjectWindowlessImpl<CGifAnimate>::SetObjectRects(lprcPosRect, lprcClipRect);
}

HRESULT STDMETHODCALLTYPE CGifAnimate:: GetRect(/* [in] */ DWORD dwAspect, /* [out] */ __RPC__out LPRECTL pRect)
{
	return IViewObjectExImpl<CGifAnimate>::GetRect(dwAspect, pRect);
}

HWND CGifAnimate::GetContainerWindow() 
{
    HWND hwnd = NULL;
    HRESULT hr;

    //*****这段代码在VC++ 工作
    if (m_spInPlaceSite != NULL)
    {
        m_spInPlaceSite->GetWindow(&hwnd);
        return hwnd;
    }

    //****** 这段代码在Visual Basic工作

    LPOLECLIENTSITE pOleClientSite = NULL;
    GetClientSite(&pOleClientSite);
    if (pOleClientSite)
    {
        IOleWindow* pOleWindow;
        hr = pOleClientSite->QueryInterface( IID_IOleWindow, (LPVOID*)&pOleWindow );
        if ( pOleWindow )
        {
            pOleWindow->GetWindow( &hwnd );
            pOleWindow->Release();
            return hwnd;
        }
    }
    return NULL;
}

HWND CGifAnimate::GetDisplayWndRect(RECT * pRect)
{
    HWND hWnd = GetContainerWindow();
    if (pRect)
    {
		RECT rcPos, rcClip;
        if (m_hWndCD)
        {
            hWnd = m_hWndCD;
            ::GetClientRect(hWnd, &rcPos);
            *pRect = rcPos;
        } else
        {
            if (::IsWindow(hWnd) && m_spInPlaceSite) 
			{ 
				CComPtr<IOleInPlaceFrame> spInPlaceFrame;
                CComPtr<IOleInPlaceUIWindow> spInPlaceUIWindow;
                OLEINPLACEFRAMEINFO frameInfo;
                frameInfo.cb = sizeof(OLEINPLACEFRAMEINFO);
                m_spInPlaceSite->GetWindowContext(&spInPlaceFrame,
                    &spInPlaceUIWindow, &rcPos, &rcClip, &frameInfo);
                if (rcPos.right < rcClip.left
                    || rcPos.bottom < rcClip.top
                    || rcPos.left > rcClip.right
                    || rcPos.top > rcClip.bottom)
                {
                    // 判断是否超出显示范围
					hWnd = NULL;
                }  else
                {
                    *pRect = rcPos;
                } //end else if (rcPos.right
            } //end if (::IsWindow(hWnd)
        } //end else if (m_hWndCD)
    } //end if 
    return hWnd;
}
#pragma warning(default:4996)