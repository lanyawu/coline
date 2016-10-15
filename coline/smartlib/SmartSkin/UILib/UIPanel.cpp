#include "common.h"

#include <UILib/UIPanel.h>
#include <UILib/UIResource.h>

/////////////////////////////////////////////////////////////////////////////////////
//
//

CTaskPanelUI::CTaskPanelUI():
              m_hFadeBitmap(NULL)
{
	SetPadding(10);
	SetWidth(165);  // By default it gets a fixed 165 pixel width
}

CTaskPanelUI::~CTaskPanelUI()
{
	if (m_hFadeBitmap != NULL)
		::DeleteObject(m_hFadeBitmap);
	if (m_pManager != NULL)
		m_pManager->KillTimer(this, FADE_TIMERID);
}

LPCTSTR CTaskPanelUI::GetClass() const
{
	return _T("TaskPanelUI");
}

SIZE CTaskPanelUI::EstimateSize(SIZE szAvailable)
{
	// The TaskPanel dissapears if the windows size becomes too small
	// Currently it is set to vasnish when its width gets below 1/4 of window.
	SIZE sz = m_cxyFixed;
	if (m_pManager->GetClientSize().cx <= m_cxyFixed.cx * 4)
	{
		// Generate a bitmap so we can paint this panel as slowly fading out.
		// Only do this when the control's size suddenly go below the threshold.
		if ((m_rcItem.right - m_rcItem.left > 1) && (m_hFadeBitmap == NULL))
		{
			if (m_hFadeBitmap != NULL) 
				::DeleteObject(m_hFadeBitmap);
			m_hFadeBitmap = CBlueRenderEngineUI::GenerateAlphaBitmap(m_pManager, this, m_rcItem, UICOLOR_DIALOG_BACKGROUND);
			// If we successfully created the 32bpp bitmap we'll set off the
			// timer so we can get animating...
			if (m_hFadeBitmap != NULL)
				m_pManager->SetTimer(this, FADE_TIMERID, 50U);
			m_dwFadeTick = ::timeGetTime();
			m_rcFade = m_rcItem;
		}
		sz.cx = 1;
	}
	return sz;
}

void CTaskPanelUI::Event(TEventUI& event)
{
	if ((event.Type == UIEVENT_TIMER) 
		&& (event.wParam == FADE_TIMERID))
	{
		// The fading animation runs for 500ms. Then we kill
		// the bitmap which in turn disables the animation.
		if (event.dwTimestamp - m_dwFadeTick > FADE_DELAY)
		{
			m_pManager->KillTimer(this, FADE_TIMERID);
			::DeleteObject(m_hFadeBitmap);
			m_hFadeBitmap = NULL;
		}
		m_pManager->Invalidate(m_rcFade);
		return;
	}
	CVerticalLayoutUI::Event(event);
}

void CTaskPanelUI::SetPos(RECT rc)
{
	int cyFont = m_pManager->GetThemeFontInfo(UIFONT_NORMAL).tmHeight;
	RECT rcClient = { rc.left, rc.top + cyFont + 6, rc.right, rc.bottom };
	CVerticalLayoutUI::SetPos(rcClient);
	m_rcItem = rc;
}

void CTaskPanelUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	RECT rcTemp = {0};
	if (!::IntersectRect(&rcTemp, &rcPaint, &m_rcItem)) 
		return;
	// Handling gracefull fading of panel
	if (m_hFadeBitmap != NULL)
	{
		DWORD dwTimeDiff = ::timeGetTime() - m_dwFadeTick;
		TPostPaintUI job;
		job.rc = m_rcFade;
		job.hBitmap = m_hFadeBitmap;
		job.iAlpha = (BYTE) CLAMP(255 - (long)(dwTimeDiff / (FADE_DELAY / 255.0)), 0, 255);
		m_pManager->AddPostPaintBlit(job);
	}
	// A tiny panel (see explaination in EstimateSize()) is invisible
	if (m_rcItem.right - m_rcItem.left < 2) 
		return;
	// Paint caption
	int cyFont = m_pManager->GetThemeFontInfo(UIFONT_NORMAL).tmHeight;
	RECT rcArc = {m_rcItem.left, m_rcItem.top, m_rcItem.right, m_rcItem.top + cyFont + 14};
	//   CBlueRenderEngineUI::DoPaintArcCaption(hDC, m_pManager, rcArc, m_sText, UIARC_GRIPPER);
	// Paint background
	RECT rcClient = { m_rcItem.left, rcArc.bottom, m_rcItem.right, m_rcItem.bottom };
	COLORREF clrFirst, clrSecond;
	m_pManager->GetThemeColorPair(UICOLOR_TASK_BACKGROUND, clrFirst, clrSecond);
	CBlueRenderEngineUI::DoPaintGradient(hDC, m_pManager, rcClient, clrFirst, clrSecond, false, 128);
	CBlueRenderEngineUI::DoPaintFrame(hDC, m_pManager, rcClient, 
	   UICOLOR_TOOLBAR_BORDER, UICOLOR_TOOLBAR_BORDER, UICOLOR__INVALID, 0);
	// Paint elements
	CVerticalLayoutUI::DoPaint(hDC, rcPaint);
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CSearchTitlePanelUI::CSearchTitlePanelUI(): 
                     m_iIconIndex(-1)
{
}

LPCTSTR CSearchTitlePanelUI::GetClass() const
{
	return _T("SearchTitlePanelUI");
}

void CSearchTitlePanelUI::SetImage(int iIndex)
{
	m_iIconIndex = iIndex;
}

void CSearchTitlePanelUI::SetPos(RECT rc)
{
	RECT rcClient = { rc.left + 1, rc.top + 35, rc.right - 1, rc.bottom - 1 };
	CHorizontalLayoutUI::SetPos(rcClient);
	m_rcItem = rc;
}

void CSearchTitlePanelUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	RECT rcFrame = { m_rcItem.left, m_rcItem.top + 34, m_rcItem.right, m_rcItem.bottom };
	CBlueRenderEngineUI::DoPaintFrame(hDC, m_pManager, rcFrame, UICOLOR_HEADER_SEPARATOR,
		UICOLOR_HEADER_SEPARATOR, UICOLOR_HEADER_BACKGROUND);
	RECT rcArc = { m_rcItem.left, m_rcItem.top + 14, m_rcItem.right, m_rcItem.top + 34 };
	RECT rcTemp = { 0 };
	CHorizontalLayoutUI::DoPaint(hDC, rcPaint);
}

void CSearchTitlePanelUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcscmp(pstrName, _T("image")) == 0) 
		SetImage(_ttoi(pstrValue));
	else CHorizontalLayoutUI::SetAttribute(pstrName, pstrValue);
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CPaddingPanelUI::CPaddingPanelUI()
{
	m_cxyFixed.cx = m_cxyFixed.cy = 0;
}

CPaddingPanelUI::CPaddingPanelUI(int cx, int cy)
{
	m_cxyFixed.cx = cx;
	m_cxyFixed.cy = cy;
}
 

void CPaddingPanelUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    CControlUI::SetAttribute(pstrName, pstrValue);
}

LPCTSTR CPaddingPanelUI::GetClass() const
{
	return _T("PaddingPanel");
}

SIZE CPaddingPanelUI::EstimateSize(SIZE szAvailable)
{
	return m_cxyFixed;
}

void CPaddingPanelUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	//
}

///======  CNormalImagePanelUI ====
CNormalImagePanelUI::CNormalImagePanelUI():
                     m_pGraph(NULL),
					 m_bIsGray(FALSE)
{
	 
}

CNormalImagePanelUI::~CNormalImagePanelUI()
{
	if (m_pGraph)
		delete m_pGraph;
	m_pGraph = NULL;
}

LPCTSTR CNormalImagePanelUI::GetClass() const
{
	return L"NormalImagePanelUI";
}

 

BOOL CNormalImagePanelUI::SetImageByFileName(const TCHAR *szFileName)
{
	if (m_pGraph)
	   delete m_pGraph;
	SHFILEINFO sfi = {0}; 
	if (::SHGetFileInfo(szFileName,FILE_ATTRIBUTE_NORMAL,
                  &sfi, sizeof(sfi), (SHGFI_USEFILEATTRIBUTES | SHGFI_ICON | SHGFI_LARGEICON)))
	{

		m_pGraph = NEWIMAGE;
		m_pGraph->LoadFromIcon(sfi.hIcon); 
		return TRUE;
	}
	return FALSE;
}

SIZE CNormalImagePanelUI::EstimateSize(SIZE szAvailable)
{
	if (m_cxyFixed.cx != 0)
	{
		return m_cxyFixed;
	} else
	{	
		CSize szImage(0, 0);
		if (m_pGraph)
		{
			//the size of the image depends on the format of
			//the image list-vertical or horizontal.
			szImage.cx = m_pGraph->GetWidth();
			szImage.cy = m_pGraph->GetHeight();
		}
		return CSize(szImage.cx, szImage.cy);
	}
}

void CNormalImagePanelUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	RECT rcTemp = {0};
	if (!::IntersectRect(&rcTemp, &rcPaint, &m_rcItem)) 
		return;
	if(m_pGraph)
	{
		m_pGraph->DrawToDc(hDC, m_rcItem);
		if (m_bIsGray)
			CBlueRenderEngineUI::GrayFrame(hDC, m_rcItem.left, m_rcItem.top, m_rcItem.right, m_rcItem.bottom);
	} 
}

void CNormalImagePanelUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	 CControlUI::SetAttribute(pstrName, pstrValue);
}

void CNormalImagePanelUI::SetGray(BOOL bIsGray)
{
	if (m_bIsGray != bIsGray)
	{
		m_bIsGray = bIsGray;
		Invalidate();
	}
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

CImagePanelUI::CImagePanelUI(): 
	           m_iImageID(IMGID_INVALID_),
	           m_bIsGray(FALSE),
	           m_pGraph(NULL),
	           m_iSubImageIndex(-1)
{
	 
}

CImagePanelUI::~CImagePanelUI()
{
	if (m_pGraph)
		delete m_pGraph;
	m_pGraph = NULL;
}

BOOL CImagePanelUI::SetImage(const TCHAR *szwName, UINT nType)
{
	if (m_pGraph)
		delete m_pGraph;
	m_pGraph = NULL;
	if (szwName)
	{
		char szFileName[MAX_PATH] = {0};
		CStringConversion::WideCharToString(szwName, szFileName, MAX_PATH - 1);
		m_pGraph = NEWIMAGE;
		m_pGraph->LoadFromFile(szFileName, nType);
		Invalidate();
	}
	return TRUE;
}

BOOL CImagePanelUI::SetImage(char *szFileName, UINT uImageType)
{
	if (m_pGraph)
		delete m_pGraph;
	if (szFileName != NULL) 
	{
		m_pGraph = NEWIMAGE;
		m_pGraph->LoadFromFile(szFileName, FALSE);
	}
	Invalidate();
	return TRUE;
}

void CImagePanelUI::SetImage(int iIndex)
{
	if (m_iImageID != iIndex)
	{
		m_iImageID = iIndex;
		Invalidate();
	}
}

 
 


void CImagePanelUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcscmp(pstrName, _T("image")) == 0) 
		SetImage(_ttoi(pstrValue));
	else if (_tcscmp(pstrName, _T("subimageindex")) == 0)
		m_iSubImageIndex = _ttoi(pstrValue);
	else if (_tcsicmp(pstrName, _T("filename")) == 0)
	{ 
		SetImage(pstrValue, 0); 

	}
	else
		CControlUI::SetAttribute(pstrName, pstrValue);
}

LPCTSTR CImagePanelUI::GetClass() const
{
	return _T("ImagePanel");
}

SIZE CImagePanelUI::EstimateSize(SIZE szAvailable)
{
	CSize szImage(0, 0);
	LPUI_IMAGE_ITEM piri;
	if (m_pManager->GetImage(m_iImageID, &piri) 
		&& (piri->dwSubCount != 0))
	{
		//the size of the image depends on the format of
		//the image list-vertical or horizontal.
		szImage.cx = piri->pGraphic->GetWidth();
		szImage.cy = piri->pGraphic->GetHeight();
	}
	if (m_cxyFixed.cx != 0)
	{
		return m_cxyFixed;
	} else
	{
		return CSize(szImage.cx, szImage.cy);
	}
}

void CImagePanelUI::SetGray(BOOL bIsGray)
{
	if (m_bIsGray != bIsGray)
	{
		m_bIsGray = bIsGray;
		Invalidate();
	}
}

void CImagePanelUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	RECT rcTemp = {0};
	if (!::IntersectRect(&rcTemp, &rcPaint, &m_rcItem)) 
		return;
	if(m_pGraph)
	{
		m_pGraph->DrawToDc(hDC, m_rcItem);
		if (m_bIsGray)
			CBlueRenderEngineUI::GrayFrame(hDC, m_rcItem.left, m_rcItem.top, m_rcItem.right, m_rcItem.bottom);
	} else if (m_iImageID != IMGID_INVALID_)
		CBlueRenderEngineUI::DoPaintGraphic(hDC, m_pManager, m_rcItem, m_iImageID, m_iSubImageIndex);
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CTextPanelUI::CTextPanelUI(): 
              m_nLinks(0), 
			  m_uButtonState(0)
{
	m_uTextStyle = DT_WORDBREAK;
	::ZeroMemory(m_rcLinks, sizeof(m_rcLinks));
}

LPCTSTR CTextPanelUI::GetClass() const
{
	return _T("TextPanelUI");
}

bool CTextPanelUI::Activate()
{
	if (!CLabelPanelUI::Activate())
		return false;	 
	if (m_nLinks > 0)
		m_pManager->SendNotify(this, _T("link"));
	return true;
}

UINT CTextPanelUI::GetControlFlags() const
{
	return (m_nLinks > 0) ? UIFLAG_SETCURSOR : 0;
}

void CTextPanelUI::Event(TEventUI& event)
{
	if (IsEnabled())
	{
		if (event.Type == UIEVENT_SETCURSOR)
		{
			for (int i = 0; i < m_nLinks; i++)
			{
				if (::PtInRect(&m_rcLinks[i], event.ptMouse))
				{
					::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
					return;
				}
			}      
		} else if (event.Type == UIEVENT_BUTTONDOWN)
		{
			for (int i = 0; i < m_nLinks; i++)
			{
				if (::PtInRect(&m_rcLinks[i], event.ptMouse))
				{
					m_uButtonState |= UISTATE_PUSHED;
					Invalidate();
					return;
				}
			}      
		} else if (event.Type == UIEVENT_BUTTONUP)
		{
			for (int i = 0; i < m_nLinks; i++)
			{
				if (::PtInRect(&m_rcLinks[i], event.ptMouse))
					Activate();
			}      
			m_uButtonState &= ~UISTATE_PUSHED;
			Invalidate();
			return;
		}
	}
	// When you move over a link
	CLabelPanelUI::Event(event);
}

void CTextPanelUI::SetPos(RECT rc)
{
	CLabelPanelUI::SetPos(rc);
	if (m_nLinks > 0) 
		::CopyRect(&m_rcLinks[0], &m_rcItem);
	else
		::ZeroMemory(m_rcLinks, sizeof(m_rcLinks));
}

void CTextPanelUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcscmp(pstrName, _T("singleline")) == 0)
	{
		if (_tcscmp(pstrValue, _T("true")) == 0)
		{
			m_uTextStyle |= DT_SINGLELINE;
		} else
		{
			m_uTextStyle &= ~DT_SINGLELINE;
		}
	} else if (_tcscmp(pstrName, _T("enablelinks")) == 0)
	{
		EnableLink(_tcscmp(pstrValue, _T("true")) == 0);
	} else if (_tcscmp(pstrName, _T("vertalign")) == 0)
	{
		m_uTextStyle &= ~(DT_TOP | DT_VCENTER | DT_BOTTOM);
		if (_tcscmp(pstrValue, _T("top")) == 0)
		{
			m_uTextStyle |= DT_TOP;
		} else if (_tcscmp(pstrValue, _T("bottom")) == 0)
		{ 
			m_uTextStyle |= DT_BOTTOM;
		} else 
		{
			//if( _tcscmp(pstrValue, _T("vcenter")) == 0 ) default vertical alignment
			m_uTextStyle |= DT_VCENTER;
		}
	} else
	{
		CLabelPanelUI::SetAttribute(pstrName, pstrValue);
	}
}

void CTextPanelUI::EnableLink(BOOL bEnable)
{
	BOOL bTmp = (m_nLinks > 0);
	if (bTmp != bEnable)
	{
		m_nLinks = bEnable ? 1 : 0;
	}
}

SIZE CTextPanelUI::EstimateSize(SIZE szAvailable)
{
	if (!m_bAutoEstimate)
	{
		return CSize( 0, 0 );
	} else if (m_cxyFixed.cx)
	{
		//determined by user
		return CSize(m_cxyFixed.cx, 0);
	} else
	{
		//determined by auto estimate
		RECT rcText = {0, 0, szAvailable.cx, szAvailable.cy};
		//font
		UITYPE_FONT uiFont = GetFont();
		//style
		UINT uStyle = 0;
		if (( m_uTextStyle & DT_SINGLELINE ) != 0)
		{
			uStyle = DT_SINGLELINE;
		} else
		{
			uStyle = DT_WORDBREAK;
		}
		CBlueRenderEngineUI::DoPaintQuickText( m_pManager->GetPaintDC(), m_pManager, 
			rcText, m_sText, RGB(0,0,0), uiFont, uStyle | DT_CALCRECT );
		return CSize( rcText.right, rcText.bottom );
	}
}


UITYPE_FONT CTextPanelUI::GetFont() const
{
	if (m_bBold)
		return UIFONT_BOLD;
	return UIFONT_NORMAL;
}


void CTextPanelUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	RECT rcTemp = {0};
	if (!::IntersectRect(&rcTemp, &rcPaint, &m_rcItem)) 
		return;
	//border
	if (HasBorder())
		CBlueRenderEngineUI::DoPaintRectangle(hDC, m_pManager, m_rcItem, 
			UICOLOR_TOOLBAR_BORDER, UICOLOR__INVALID);	

	//text
	//font
	UITYPE_FONT uiFont = GetFont();
	//text style
	UINT nTextStyle = m_uTextStyle;
	//if DT_SINGLELINE is not specified but we still want DT_VCENTER, we must
	//adjust the rectangle ourselves.
	RECT rcText = m_rcItem;
	if (((m_uTextStyle & DT_SINGLELINE) == 0) && ((m_uTextStyle & DT_VCENTER) != 0))
	{
		RECT rcTemp = m_rcItem;
		int cyText = CBlueRenderEngineUI::DoPaintQuickText(hDC, m_pManager, rcTemp, m_sText,
			                               m_clrText, uiFont, m_uTextStyle | DT_CALCRECT);
		int cyItem = m_rcItem.bottom - m_rcItem.top;
		if (cyText < cyItem)
		{
			//the calculated rect lies in m_rcItem, position it to the center
			rcText.top += (cyItem - cyText) / 2;
			rcText.bottom = rcText.top + cyText;
		}
	}
	CBlueRenderEngineUI::DoPaintQuickText(hDC, m_pManager, 
		                        rcText, m_sText, m_clrText, uiFont, nTextStyle );
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CWarningPanelUI::CWarningPanelUI(): 
                 m_BackColor(UICOLOR_STANDARD_YELLOW)
{
}

LPCTSTR CWarningPanelUI::GetClass() const
{
	return _T("WarningPanelUI");
}

void CWarningPanelUI::SetWarningType(UINT uType)
{
	switch(uType)
	{
	    case MB_ICONERROR:
			 m_BackColor = UICOLOR_STANDARD_RED;
			 break;
		case MB_ICONWARNING:
			 m_BackColor = UICOLOR_STANDARD_YELLOW;
			 break;
		default:
			 m_BackColor = UICOLOR_WINDOW_BACKGROUND;
			 break;
	}
}

void CWarningPanelUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcscmp(pstrName, _T("type")) == 0)
	{
		if (_tcscmp(pstrValue, _T("error")) == 0)
			SetWarningType(MB_ICONERROR);
		if (_tcscmp(pstrValue, _T("warning")) == 0)
			SetWarningType(MB_ICONWARNING);
	} else 
		CTextPanelUI::SetAttribute(pstrName, pstrValue);
}

SIZE CWarningPanelUI::EstimateSize(SIZE szAvailable)
{
	RECT rcText = {0, 0, szAvailable.cx, szAvailable.cy};
	::InflateRect(&rcText, -6, -4);
	int nLinks = 0;
	CBlueRenderEngineUI::DoPaintPrettyText(m_pManager->GetPaintDC(), m_pManager, rcText, m_sText,
		                             UICOLOR_EDIT_TEXT_NORMAL, UICOLOR__INVALID, NULL, nLinks, DT_WORDBREAK | DT_CALCRECT);
	return CSize(0, (rcText.bottom - rcText.top) + 16);
}

void CWarningPanelUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	RECT rcTemp = {0};
	if (!::IntersectRect(&rcTemp, &rcPaint, &m_rcItem)) 
		return;
	RECT rcSign = m_rcItem;
	rcSign.bottom -= 8;
	CBlueRenderEngineUI::DoPaintFrame(hDC, m_pManager, rcSign, UICOLOR_STANDARD_GREY, UICOLOR_STANDARD_GREY, m_BackColor);
	RECT rcText = rcSign;
	::InflateRect(&rcText, -6, -4);
	m_nLinks = lengthof(m_rcLinks);
	CBlueRenderEngineUI::DoPaintPrettyText(hDC, m_pManager, rcText, m_sText, 
		UICOLOR_BUTTON_TEXT_NORMAL, UICOLOR__INVALID, m_rcLinks, m_nLinks, DT_WORDBREAK);
}
