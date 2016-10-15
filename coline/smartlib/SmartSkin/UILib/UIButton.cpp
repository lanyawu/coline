#include "common.h"

#include <UILib/UIButton.h>
#include <UILib/UIResource.h>
#include <CommonLib/StringUtils.h>
#include <commonlib/graphicplus.h>
/////////////////////////////////////////////////////////////////////////////////////
//
//

CButtonUI::CButtonUI(): 
		   m_bDown(FALSE), 
		   m_iBkgImageId(0),
		   m_uButtonState(0), 
		   m_uTextStyle(DT_SINGLELINE | DT_CENTER | DT_VCENTER)
{
	m_szPadding.cx = 4;
	m_szPadding.cy = 0;
}

LPCTSTR CButtonUI::GetClass() const
{
	return _T("ButtonUI");
}

UINT CButtonUI::GetControlFlags() const
{
	return UIFLAG_TABSTOP;
}

void CButtonUI::Event(TEventUI& e)
{
	switch(e.Type)
	{
		case UIEVENT_DBLCLICK:
			OnEventDblclk(e);
			break;
		case UIEVENT_BUTTONDOWN:
			OnEventButtonDown(e);
			break;
		case UIEVENT_BUTTONUP:
			OnEventButtonUp(e);
			break;
		case UIEVENT_MOUSEMOVE:
			OnEventMouseMove(e);
			break;
		case UIEVENT_MOUSEENTER:
			OnEventMouseEnter();
			break;
		case UIEVENT_MOUSELEAVE:
			OnEventMouseLeave();
			break;
		default:
			break;
	}		
	CControlUI::Event(e);	
}

void CButtonUI::OnEventButtonDown(TEventUI& e)
{
	if (::PtInRect(&m_rcItem, e.ptMouse) && IsEnabled())
	{
		m_uButtonState |= ( UISTATE_PUSHED | UISTATE_CAPTURED );
		Invalidate();
	}
}

void CButtonUI::OnEventDblclk(TEventUI& e)
{
	//
}

void CButtonUI::OnEventButtonUp(TEventUI& e)
{
	if (((m_uButtonState & UISTATE_CAPTURED) != 0) && (IsEnabled()))
	{
		//test if having capture
		m_uButtonState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
		Invalidate();
		if (::PtInRect( &m_rcItem, e.ptMouse))
		{ 
			Activate();//click
		}
	}
}

void CButtonUI::OnEventMouseMove(TEventUI& e)
{
}

void CButtonUI::OnEventMouseEnter()
{
	m_uButtonState |= UISTATE_HOT;
    Invalidate();
}

void CButtonUI::OnEventMouseLeave()
{
	m_uButtonState &= ~UISTATE_HOT;
	Invalidate();
}

void CButtonUI::SetText(LPCTSTR pstrText)
{
	CControlUI::SetText(pstrText);
	// Automatic assignment of keyboard shortcut
	if (_tcschr(pstrText, '&') != NULL)
	{
		m_chShortcut = *(_tcschr(pstrText, '&') + 1);
	}
}

bool CButtonUI::Activate()
{
	if (!CControlUI::Activate())
	{
		return false;
	}
	if (m_pManager != NULL)
	{
		m_pManager->SendNotify(this, _T("click"));
	}
	return true;
}
 

void CButtonUI::SetTextStyle( UINT uStyle )
{
	if (m_uTextStyle != uStyle)
	{
		m_uTextStyle = uStyle;
		Invalidate();
	}
}

UINT CButtonUI::GetTextStyle() const
{
	return m_uTextStyle;
}

BOOL CButtonUI::GetAttribute(LPCTSTR pstrName, TCHAR *szValue, int &nMaxValueSize)
{
	if (_tcsicmp(pstrName, _T("width")) == 0)
	{
		 
	} else if (_tcsicmp(pstrName, _T("height")) == 0)
	{
		 
	} else if (_tcsicmp(pstrName, _T("align")) == 0)
	{
		 
	} else if (_tcsicmp(pstrName, _T("background")) == 0)
	{
		 
	} else if (_tcsicmp(pstrName, _T("down")) == 0)
	{
	   if (nMaxValueSize >= 6)
	   {
		   if (m_bDown)
		   {
			   ::lstrcpy(szValue, L"true");
			   nMaxValueSize = 4;
		   } else
		   {
			   ::lstrcpy(szValue, L"false");
			   nMaxValueSize = 5;
		   }
		   return TRUE;
	   } 
	} else
		return CControlUI::GetAttribute(pstrName, szValue, nMaxValueSize);
	return FALSE;
}

void CButtonUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcsicmp(pstrName, _T("align")) == 0)
	{
		if (_tcsicmp(pstrValue, _T("center")) != NULL)
			m_uTextStyle |= DT_CENTER;
		if (_tcsicmp(pstrValue, _T("right")) != NULL) 
			m_uTextStyle |= DT_RIGHT;
	} else if (_tcsicmp(pstrName, _T("background")) == 0)
	{
		m_iBkgImageId = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("down")) == 0)
	{
		m_bDown = (_tcsicmp(pstrValue, L"true") == 0);
	} else if (_tcsicmp(pstrName, _T("padsize")) == 0)
	{
		LPTSTR pstr = NULL;
		int cx = _tcstol(pstrValue, &pstr, 10);
		int cy = 0;
		if (pstr)
			cy = _tcstol(pstr + 1, &pstr, 10); 
		SetPadding(cx, cy);
	} else
		CControlUI::SetAttribute(pstrName, pstrValue);
}

void CButtonUI::SetPadding(int cx, int cy)
{
	m_szPadding.cx = cx;
	m_szPadding.cy = cy; 
}

SIZE CButtonUI::EstimateSize(SIZE /*szAvailable*/)
{
	SIZE sz = {m_cxyFixed.cx, 12 + m_pManager->GetThemeFontInfo(UIFONT_NORMAL).tmHeight };
	if (m_cxyFixed.cx == 0 && m_pManager != NULL)
	{
		RECT rcText = { 0, 0, 9999, 20 };
		int nLinks = 0;
		CBlueRenderEngineUI::DoPaintPrettyText(m_pManager->GetPaintDC(), m_pManager, rcText, 
			                    m_sText, UICOLOR_STANDARD_BLACK, UICOLOR__INVALID, NULL, nLinks, DT_SINGLELINE | DT_CALCRECT);
		sz.cx = rcText.right - rcText.left;
	}
	sz.cx += m_szPadding.cx * 2;
	sz.cy += m_szPadding.cy * 2;
	return sz;
}

void CButtonUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
   // Draw button
   UINT uState = 0;
   if (IsFocused()) 
	   uState |= UISTATE_FOCUSED;
   if (!IsEnabled())
	   uState |= UISTATE_DISABLED;
   if (m_bDown)
	   uState |= UISTATE_PUSHED;
   RECT rcPadding = { m_szPadding.cx, m_szPadding.cy, m_szPadding.cx, m_szPadding.cy };
   CBlueRenderEngineUI::DoPaintButton(hDC, 
									  m_pManager, 
									  m_rcItem, 
									  m_sText, 
									  rcPadding, 
									  m_uButtonState | uState, 
									  m_uTextStyle, m_iBkgImageId);
}

UINT CButtonUI::SetButtonState(UINT uNewState)
{
	UINT uOldState = m_uButtonState;
	if (uNewState != m_uButtonState)
	{
		m_uButtonState = uNewState;
		Invalidate();
	}
	return uOldState;
}

UINT CButtonUI::GetButtonState() const
{
	return m_uButtonState;
}

/////////////////////////////////////////////////////////////////////////////////////
//
//CImageButtonUI
CImageButtonUI::CImageButtonUI():
                m_uImageId(IMGID_INVALID_),
				m_nStretchMode(SM_NORMALSTRETCH),
				m_bIsGray(FALSE),
				m_pFloatImg(NULL),
				m_nFloatImageId(0),
				m_bTransparent(FALSE),
				m_DrawStyle(DRAW_TEXT_NONE),
				m_nStretchFixed(0)
{
	memset(&m_rcFloatImageShrink, 0, sizeof(RECT));
	m_buttonGraph = NEWIMAGE;
}

CImageButtonUI::~CImageButtonUI()
{
	if (m_pFloatImg)
		delete m_pFloatImg;
	delete m_buttonGraph;
}

LPCTSTR CImageButtonUI::GetClass() const
{
	return _T("ImageButtonUI");
}

void CImageButtonUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent)
{
	CButtonUI::SetManager(pManager, pParent);
	if (m_pManager && (m_nFloatImageId > 0))
	{
		int id = m_nFloatImageId;
		SetFloatImage(id);
	}
}

SIZE CImageButtonUI::EstimateSize(SIZE szAvailable)
{
	m_ImageCx = 0;
	m_ImageCy = 0;
	LPUI_IMAGE_ITEM piri;
	if (m_pManager->GetImage(m_uImageId, &piri) 
		&& (piri->dwSubCount != 0))
	{
		//the size of the image depends on the format of
		//the image list-vertical or horizontal.
		int cx = piri->pGraphic->GetWidth();
		int cy = piri->pGraphic->GetHeight();
		if (piri->dwListFmt == ILF_VERTICAL)
		{
			m_ImageCx = cx;
			m_ImageCy = cy / piri->dwSubCount;
		} else	if (piri->dwListFmt == ILF_HORIZONTAL)
		{
			m_ImageCx = cx / piri->dwSubCount;
			m_ImageCy = cy;
		}
	}
	SIZE sz = {0, 0};
	if (!m_sText.IsEmpty())
		GetTextExtentPoint32(m_pManager->GetPaintDC(), m_sText, m_sText.GetLength(), &sz); 
	if (GetWidth() != 0)
	{
		switch(m_DrawStyle)
		{
		case DRAW_TEXT_NONE:
			 return CSize(GetWidth(), m_ImageCy);
		case DRAW_TEXT_VERTICAL:
			 return CSize(GetWidth(), m_ImageCy + sz.cy);
		case DRAW_TEXT_HORIZONTAL:
			 return CSize(GetWidth() + sz.cx, m_ImageCy);
		default:
			 return CSize(GetWidth(), m_ImageCy);
		}
	} else
	{
		switch(m_DrawStyle)
		{
		case DRAW_TEXT_NONE:
			 return CSize(m_ImageCx, m_ImageCy);
		case DRAW_TEXT_VERTICAL:
			 return CSize(m_ImageCx, m_ImageCy + sz.cy);
		case DRAW_TEXT_HORIZONTAL:
			 return CSize(m_ImageCx + sz.cx, m_ImageCy);
		default:
			 return CSize(m_ImageCx, m_ImageCy);
		} 
	}
}

void CImageButtonUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	//get button state
	UINT uState = 0;
	if (IsFocused()) 
		uState |= UISTATE_FOCUSED;
	if (!IsEnabled()) 
		uState |= UISTATE_DISABLED;
	if (m_bDown)
	   uState |= UISTATE_PUSHED;
	//首先选择m_buttonGraph，如果不存在，绘制m_nImgID指定图片
	CRect rcImage(m_rcItem);
	if (!m_buttonGraph->IsEmpty())
	{
		m_buttonGraph->DrawToDc(hDC, m_rcItem);//TBD 无法绘制状态
	} else if (m_uImageId > 0)
	{
		//拉伸模式
		if (!m_sText.IsEmpty())
		{
			switch(m_DrawStyle)
			{
			case DRAW_TEXT_VERTICAL:
				{
					RECT rc = {rcImage.left, rcImage.top + m_ImageCy, rcImage.right, rcImage.bottom};
					CBlueRenderEngineUI::DoPaintButtonText(hDC, 
									  m_pManager, 
									  rc, 
									  m_sText,  
									  GetButtonState() | uState, 
									  DT_SINGLELINE | DT_CENTER);
					rcImage.bottom = rcImage.top + m_ImageCy;
					break;
				} //
			case DRAW_TEXT_HORIZONTAL:
				{
					RECT rc = {rcImage.left + m_ImageCx, rcImage.top, rcImage.right, rcImage.bottom};
					rcImage.right = rcImage.left + m_ImageCx;
					
					CBlueRenderEngineUI::DoPaintButtonText(hDC, 
									  m_pManager, 
									  rc, 
									  m_sText,  
									  GetButtonState() | uState, 
									  DT_SINGLELINE | DT_CENTER);
					break;
				}
			}
		}
		StretchFixed sf;
		sf.SetFixed(m_nStretchFixed);
		CBlueRenderEngineUI::DoPaintImageButton(hDC, m_pManager, rcImage, 
			GetButtonState() | uState, m_uImageId, &sf, m_nStretchMode);
	} else
		CButtonUI::DoPaint(hDC, rcPaint);

	//绘制图片上的浮动图片
	if (m_pFloatImg)
	{
		CRect rcFloatImage(m_rcItem);
		rcFloatImage.left += m_rcFloatImageShrink.left;
		rcFloatImage.top += m_rcFloatImageShrink.top;
		rcFloatImage.right -= m_rcFloatImageShrink.right;
		rcFloatImage.bottom -= m_rcFloatImageShrink.bottom;	
		m_pFloatImg->DrawToDc(hDC, rcFloatImage);
		if (m_bIsGray)
			CBlueRenderEngineUI::GrayFrame(hDC, rcFloatImage.left, rcFloatImage.top, 
			                               rcFloatImage.right, rcFloatImage.bottom);
	} //end if (m_pFloatImg)
}

void CImageButtonUI::SetGray(BOOL bIsGray)
{
	if (m_bIsGray != bIsGray)
	{
		m_bIsGray = bIsGray;
		Invalidate();
	}
}

void CImageButtonUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcsicmp(pstrName, _T("image")) == 0)
	{
		SetImage(_ttoi( pstrValue));
	} else if (_tcsicmp(pstrName, _T("fixedcorner")) == 0)
	{
	    SetFixedCorner(_ttoi( pstrValue));
	} else if (_tcsicmp(pstrName, _T("Gray")) == 0)
	{
		SetGray(_tcsicmp(pstrValue, _T("true")) == 0);
	} else if (_tcsicmp( pstrName, _T("stretchmode")) == 0)
	{
		if (_tcsicmp(pstrValue, _T("normal")) == 0) 
			SetStretchMode(SM_NORMALSTRETCH);
		else if ( _tcsicmp(pstrValue, _T("fix4corners")) == 0) 
			SetStretchMode(SM_FIXED4CORNERS);
		else if ( _tcsicmp(pstrValue, _T("horizontal")) == 0) 
			SetStretchMode(SM_HORIZONTAL);
		else if ( _tcsicmp(pstrValue, _T("vertical")) == 0) 
			SetStretchMode(SM_VERTICAL);
	} else if (_tcscmp(pstrName, _T("floatimagefilename")) == 0)
	{
#ifdef _UNICODE
		char szFileName[MAX_PATH] = {0};
		CStringConversion::WideCharToString(pstrValue, szFileName, MAX_PATH);
		SetFloatImage(szFileName);
#else
		SetFloatImage( pstrValue, GRAPHIC_TYPE_ICO );//TBD
#endif
	} else if (_tcsicmp(pstrName, _T("floatimageshrink")) == 0)
	{
		RECT rcShrink = { 0 };
		LPTSTR pstr = NULL;
		rcShrink.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
		rcShrink.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
		rcShrink.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
		rcShrink.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
		SetFloatImageShrink( rcShrink );		
	} else if (_tcsicmp(pstrName, _T("floatimage")) == 0)
	{
		SetFloatImage(_ttoi(pstrValue));
	} else if (_tcsicmp(pstrName, _T("floatshortcutname")) == 0)
	{
		char szTmp[MAX_PATH] = {0};
		CStringConversion::WideCharToString(pstrValue, szTmp, MAX_PATH - 1);
		char szExt[MAX_PATH] = {0};
		CSystemUtils::ExtractFileExtName(szTmp, szExt, MAX_PATH - 1);
		if ((::stricmp(szExt, "bmp") == 0) || (::stricmp(szExt, "png") == 0)
			|| (::stricmp(szExt, "jpg") == 0) || (::stricmp(szExt, "gif") == 0))
		{
			SetFloatImage(szTmp);
		} else
		{ 
			if (m_pFloatImg)
				delete m_pFloatImg;
			m_pFloatImg = NEWIMAGE;
			SHFILEINFO sfi = {0}; 
			if (::SHGetFileInfo(pstrValue, FILE_ATTRIBUTE_NORMAL,
						  &sfi, sizeof(sfi), (SHGFI_USEFILEATTRIBUTES | SHGFI_ICON | SHGFI_LARGEICON)))
			{ 
				//用cximage转
				CGraphicPlus *pTmp = new CGraphicPlus();
				pTmp->LoadFromIcon(sfi.hIcon);
				m_pFloatImg->LoadFromGraphic(pTmp);
				delete pTmp;
				//m_pFloatImg->LoadFromIcon(sfi.hIcon);
			} 
		}
	} else if (_tcsicmp(pstrName, _T("drawtextstyle")) == 0)
	{
		if (_tcsicmp(pstrValue, _T("vertical")) == 0)
			m_DrawStyle = DRAW_TEXT_VERTICAL;
		else if (_tcsicmp(pstrValue, _T("horizontal")) == 0)
			m_DrawStyle = DRAW_TEXT_HORIZONTAL;
		else 
			m_DrawStyle = DRAW_TEXT_NONE;
	} else if (_tcsicmp(pstrName, _T("transparent")) == 0)
	{
		m_bTransparent = (_tcsicmp(pstrValue, _T("true")) == 0);
	} else
	{
		CButtonUI::SetAttribute( pstrName, pstrValue );
	}
}

BOOL CImageButtonUI::SetStretchMode(UINT nMode)
{
	if ((nMode > SM_FIST_) && (nMode < SM_LAST_) && (nMode != m_nStretchMode))
	{
		m_nStretchMode = nMode;
		Invalidate();
		return TRUE;
	}
	return FALSE;
}

BOOL CImageButtonUI::SetFixedCorner(UINT nFixed)
{
	if (m_nStretchFixed != nFixed)
	{
		m_nStretchFixed = nFixed;
		Invalidate();
		return TRUE;
	}
	return FALSE;
}

BOOL CImageButtonUI::SetImage(UINT dwImgID)
{
	if (m_uImageId != dwImgID)
	{
		m_uImageId = dwImgID;
		Invalidate();
		return TRUE;
	}
	return FALSE;
}

BOOL CImageButtonUI::SetImage(const char* imageFile, UINT uImage_Type)
{
	if (m_buttonGraph->LoadFromFile(imageFile, FALSE))
	{
		Invalidate();
		return TRUE;
	}
	return FALSE;
}

BOOL CImageButtonUI::SetFloatImage(const char *szFileName, BOOL bIsTransParent)
{
	if (m_pFloatImg)
		delete m_pFloatImg;
	m_pFloatImg = NEWIMAGE;
	m_pFloatImg->LoadFromFile(szFileName, FALSE);
    Invalidate();
	return TRUE;
}

BOOL CImageButtonUI::SetFloatImage(int nImageId)
{
	if (m_pManager)
	{
		if (m_pFloatImg)
			delete m_pFloatImg;
		LPUI_IMAGE_ITEM Image;
		if (m_pManager->GetImage(nImageId, &Image))
		{
			m_pFloatImg = NEWIMAGE;
			m_pFloatImg->LoadFromFile(Image->m_strFileName.c_str(), FALSE);
			return TRUE;
		}
	} else
	{
		m_nFloatImageId = nImageId;
		return TRUE;
	}
	return FALSE;
}

BOOL CImageButtonUI::SetFloatImageShrink(const RECT& rc)
{
	if (!::EqualRect(&rc, &m_rcFloatImageShrink))
	{
		m_rcFloatImageShrink = rc;
		Invalidate();
		return TRUE;
	}
	return FALSE;
}


//class CPlugImageButtonUI
CPlugImageButtonUI::CPlugImageButtonUI():
                    m_nImageId(0),
					m_nNormalImgId(0),
					m_nHotImgId(0),
					m_nPushedImgId(0),
					m_nGrayImgId(0)
{
}

	//
SIZE CPlugImageButtonUI::EstimateSize(SIZE szAvailable)
{ 
	m_ImageCx = 0;
	m_ImageCy = 0;
	LPUI_IMAGE_ITEM piri;
	if (m_pManager->GetImage(m_nImageId, &piri) 
		&& (piri->dwSubCount != 0))
	{
		//the size of the image depends on the format of
		//the image list-vertical or horizontal.
		int cx = piri->pGraphic->GetWidth();
		int cy = piri->pGraphic->GetHeight();
		if (piri->dwListFmt == ILF_VERTICAL)
		{
			m_ImageCx = cx;
			m_ImageCy = cy / piri->dwSubCount;
		} else	if (piri->dwListFmt == ILF_HORIZONTAL)
		{
			m_ImageCx = cx / piri->dwSubCount;
			m_ImageCy = cy;
		}
	}
	SIZE sz = {0, 0};
	if (!m_sText.IsEmpty())
		GetTextExtentPoint32(m_pManager->GetPaintDC(), m_sText, m_sText.GetLength(), &sz); 
	if (GetWidth() != 0)
	{ 
		return CSize(GetWidth(), m_ImageCy); 
	} else
	{ 
		return CSize(m_ImageCx, m_ImageCy); 
	}
}

void CPlugImageButtonUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	UINT uState = GetButtonState();
	int nPlusImgId = m_nNormalImgId; 
	if ((uState & UISTATE_DISABLED) != 0)
	{
		nPlusImgId = m_nGrayImgId;//button disabled
	} else if ((uState & UISTATE_PUSHED ) != 0)
	{
		nPlusImgId = m_nPushedImgId;//button pushed
	} else if ((uState & UISTATE_HOT) != 0)
	{
		nPlusImgId = m_nHotImgId;//hot
	} 
	CRect rc(m_rcItem);
	CBlueRenderEngineUI::DoPaintGraphicPlus(hDC, m_pManager, rc, m_nImageId, nPlusImgId);
}

void CPlugImageButtonUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcsicmp(pstrName, _T("normal")) == 0)
	{
		m_nNormalImgId = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("image")) == 0)
	{
		m_nImageId = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("hot")) == 0)
	{
		m_nHotImgId = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("pushed")) == 0)
	{
		m_nPushedImgId = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("gray")) == 0)
	{
		m_nGrayImgId = _ttoi(pstrValue);
	} else
		CButtonUI::SetAttribute(pstrName, pstrValue);
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

COptionUI::COptionUI(): 
           m_bSelected(false),
		   m_iImgID(IMGID_INVALID_),
		   m_iData(0)
{
	SetTextStyle( DT_LEFT | DT_SINGLELINE | DT_VCENTER );
}

bool COptionUI::IsChecked() const
{
	return m_bSelected;
}

void COptionUI::SetCheck(bool bSelected)
{
	if (m_bSelected == bSelected)
		return;
	m_bSelected = bSelected;
	if (m_pManager != NULL) 
	{
		m_pManager->SendNotify(this, _T("changed"));
	    Invalidate();
	}
}

void COptionUI::SetImage(UINT nImageID)
{
	if (m_iImgID != nImageID)
	{
		m_iImgID = nImageID;
		Invalidate();
	}
}

void COptionUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcsicmp(pstrName, _T("selected")) == 0)
	{ 
		if (_tcsicmp(pstrValue, _T("true")) == 0) 
			Activate();
	} else if (_tcsicmp(pstrName, _T("image")) == 0)
	{
		SetImage(_ttoi(pstrValue));
	} else if (_tcsicmp(pstrName, _T("checked")) == 0)
	{
		SetCheck(_tcsicmp(pstrValue, _T("true")) == 0);
	} else
		CButtonUI::SetAttribute(pstrName, pstrValue);
}

SIZE COptionUI::EstimateSize(SIZE szAvailable)
{
	SIZE szItem = { 0, 0 };
	if (GetWidth())
	{ 
		szItem.cx = GetWidth(); 
	} else
	{
		HDC hDC = m_pManager->GetPaintDC();
		HFONT hOldFont = (HFONT)::SelectObject(hDC, m_pManager->GetThemeFont(UIFONT_NORMAL));
		::GetTextExtentPoint32(hDC, m_sText.GetData(), m_sText.GetLength(), &szItem);
		::SelectObject(hDC, hOldFont);
		szItem.cx += szAvailable.cy;//按钮的预留位置 TBD
	}

	if (GetHeight())
	{
		szItem.cy = GetHeight();
	} else
	{
		szItem.cy = 0;
	}
	return szItem;
}

void COptionUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	UINT uState = 0;
	if (m_bSelected) 
		uState |= UISTATE_CHECKED;
	if (IsFocused()) 
		uState |= UISTATE_FOCUSED;
	if (!IsEnabled()) 
		uState |= UISTATE_DISABLED;
   if (m_bDown)
	   uState |= UISTATE_PUSHED;
	CBlueRenderEngineUI::DoPaintOptionBox(hDC, m_pManager, m_rcItem, 
		m_sText, GetButtonState() | uState, GetTextStyle(), m_iImgID);
}

void COptionUI::SetData(const int iData )
{
	m_iData = iData;
}

int COptionUI::GetData() const
{
	return m_iData;
}

/////////////////////////////////////////////////////////////////////////////////////
//
//
CCheckBoxUI::CCheckBoxUI()
{
}

CCheckBoxUI::~CCheckBoxUI()
{
}

LPCTSTR CCheckBoxUI::GetClass() const
{
	return _T("CheckBoxUI");
}

bool CCheckBoxUI::Activate()
{
	if (!CControlUI::Activate()) 
		return false;
	SetCheck(!IsChecked());
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////
//
//CRadioBoxUI
CRadioBoxUI::CRadioBoxUI():
	         m_iGroupID(-1),
	         m_iIndex(-1)
{
}

CRadioBoxUI::~CRadioBoxUI()
{
}

LPCTSTR CRadioBoxUI::GetClass() const
{
	return _T("RadioBoxUI");
}

bool CRadioBoxUI::Activate()
{
	if (!CControlUI::Activate())
		return false;
	//如果当前处于未选中状态，进行选中操作
    if (!IsChecked())
	{
		SetCheck(true);
	}
	return true;
}

void CRadioBoxUI::SetCheck(bool bSelected)
{
	COptionUI::SetCheck(bSelected);
	if (m_pManager)
	{
		if (IsChecked())
		{
			COptionUI* pCurSel = (COptionUI*)(m_pOwner->GetCurSelUI());
			if (pCurSel != NULL && pCurSel != this)
			{
				pCurSel->SetCheck(false);
			}
			m_pOwner->SetCurSelUI(this);
		}
		if (IsChecked())
		{
			SetButtonState(GetButtonState() | UISTATE_PUSHED);
		} else
		{
			SetButtonState(GetButtonState() & ~UISTATE_PUSHED);
		}
	}
}

void CRadioBoxUI::Init()
{
	COptionUI::Init();
	m_pOwner = m_pManager->RegisterGroup(m_iGroupID);
	ASSERT(m_pOwner != NULL);
	if (m_pOwner && m_pOwner->Add(this))
	{
		m_iIndex = m_pOwner->GetCount() - 1;
	}
	SetCheck(m_bSelected);
}

void CRadioBoxUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcsicmp(pstrName, _T("groupid")) == 0)
	{
		m_iGroupID = _ttoi(pstrValue);
	} else
	{
		COptionUI::SetAttribute(pstrName, pstrValue);
	}
}

int CRadioBoxUI::GetGroupID() const
{
	return m_iGroupID;
}

bool CRadioBoxUI::SetGroupID(int iGroup)
{
	//valid group id, and it should not equal to the current one if exists

	if (m_pManager && (iGroup != -1) && (m_iGroupID == -1))
	{
		m_pOwner = m_pManager->RegisterGroup( iGroup );
		ASSERT( m_pOwner != NULL );
		if (m_pOwner && m_pOwner->Add(this))
		{
			m_iIndex = m_pOwner->GetCount() - 1;
			m_iGroupID = iGroup;
			return true;
		} //end if (m_pOwner && ...
	}
	return false;
}

void CRadioBoxUI::OnEventButtonDown(TEventUI& e)
{
	if (::PtInRect(&m_rcItem, e.ptMouse) && IsEnabled())
	{
		SetButtonState(GetButtonState() | UISTATE_CAPTURED);
	}
}

void CRadioBoxUI::OnEventButtonUp(TEventUI& e)
{
	if ((GetButtonState() & UISTATE_CAPTURED) != 0) 
	{
		//test if having capture
		if (::PtInRect(&m_rcItem, e.ptMouse)) 
		{
			Activate();//click
		}
		SetButtonState(GetButtonState() & ~UISTATE_CAPTURED);
		Invalidate();
	} // if ((GetButtonState() && UISTATE_CAPTURED) ..
}
