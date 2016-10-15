#include "common.h"

#include <UILib/UITab.h>
/////////////////////////////////////////////////////////////////////////////////////
//
//

CTabFolderUI::CTabFolderUI(): 
              m_iCurSel(-1),
			  m_nTabAlign(TABALIGN_TOP),
			  m_pCurPage(NULL), 
			  m_aTabAreas(sizeof(RECT))
{
	m_chShortcut = VK_NEXT;
	m_szClientCorner.cx = 0;
	m_szClientCorner.cy = 0;
	m_szTab.cx = 0;
	m_szTab.cy = 0;
}

LPCTSTR CTabFolderUI::GetClass() const
{
	return _T("TabFolderUI");
}

void CTabFolderUI::Init()
{
	CContainerUI::Init();
	for (int iPage = 0; iPage < GetCount(); ++ iPage)
	{
		CTabPageUI* pPage = static_cast<CTabPageUI*>(GetItem(iPage));
		pPage->m_tabButton.SetManager(m_pManager, this);
		pPage->m_tabButton.SetText(pPage->GetText());
		pPage->m_tabButton.SetName(pPage->GetName());
	}
	if (m_iCurSel == -1)
	{
		SelectItem(0);
	}
}

bool CTabFolderUI::Add(CControlUI* pControl, const int nIdx)
{
	if (dynamic_cast<CTabPageUI*>(pControl) != 0)
	{
		pControl->SetVisible(false);
		return CContainerUI::Add(pControl);
	}
	return false;
}

int CTabFolderUI::GetCurSel() const
{
	return m_iCurSel;
}


BOOL CTabFolderUI::GetSelItemChildControl(const TCHAR *szClassName, TCHAR *szSelItemName, int *nSize)
{
	if ((m_iCurSel >= 0) && (m_iCurSel < m_ChildList.GetSize()))
	{
		CControlUI *pCtrl =  m_ChildList[m_iCurSel];
		IContainerUI *pContainer = static_cast<IContainerUI *>(pCtrl->GetInterface(L"Container"));
		if (pContainer)
		{
			CControlUI *pControl = pContainer->GetChildContrlByClass(szClassName);
			if (pControl)
			{
				if ((nSize) && (pControl->GetName().GetData() != NULL))
				{
					if (szSelItemName && (*nSize >= pControl->GetName().GetLength()))
					{
						::lstrcpy(szSelItemName, pControl->GetName().GetData());
						return TRUE;
					} else
					{
						*nSize =  pControl->GetName().GetLength(); 
					}
				} //end if ((nSize) && 
			} //end if (pControl
		} //end if (pContainer)
	} //end if ((m_iCurSel ..
	return FALSE;
}

BOOL CTabFolderUI::TabGetSelItemName(TCHAR *szSelItemName, int *nSize)
{ 
	if ((m_iCurSel >= 0) && (m_iCurSel < m_ChildList.GetSize()))
	{
		CControlUI *pCtrl = m_ChildList[m_iCurSel];
		if ((nSize) && (pCtrl->GetName().GetData() != NULL))
		{
			if (szSelItemName && (*nSize >= pCtrl->GetName().GetLength()))
			{
				::lstrcpy(szSelItemName, pCtrl->GetName().GetData());
				return TRUE;
			} else
			{
				*nSize =  m_pCurPage->GetName().GetLength(); 
			}
		} //end if ((nSize) && 
	} //end if (m_pCurPage...
	return FALSE;
}

bool CTabFolderUI::SelectItem(LPCTSTR pstrPageName)
{
	int idx = -1;
	for (int i = 0; i < m_ChildList.GetSize(); i ++)
	{
		CControlUI *pCtrl = m_ChildList[i];
		if (::lstrcmpi(pstrPageName, pCtrl->GetName().GetData()) == 0)
		{
			idx = i;
			break;
		}
	}
	if (idx >= 0)
		return SelectItem(idx);
	return false;
}

bool CTabFolderUI::SelectItem(const int iIndex)
{
	int iPrevSel = m_iCurSel;
	if ((iIndex < 0) || (iIndex >= m_ChildList.GetSize()))
		return false;
	CTabPageUI* pSelPage = dynamic_cast<CTabPageUI*>(m_ChildList[iIndex]);
	if (!pSelPage->IsActive())
	   return false;
	if (iIndex == m_iCurSel) 
	   return true;
	// Assign page to internal pointers
	if (m_pCurPage != NULL) 
		m_pCurPage->SetVisible(false);
	m_iCurSel = iIndex;
	m_pCurPage = pSelPage;
	m_pCurPage->SetVisible(true);
	if (m_pManager != NULL) 
		m_pManager->SendNotify(this, _T("itemselect"), (WPARAM)m_iCurSel);
	// Set focus on page
	m_pCurPage->SetFocus();
	if (m_pManager != NULL) 
		m_pManager->SetNextTabControl();
	return true;
}

void CTabFolderUI::SetVisible(bool bVisible)
{
	// Hide children as well, scrollbar not included
	for (int it = 0; it < m_ChildList.GetSize(); it ++) 
	{
		m_ChildList[it]->SetVisible(false);
	}
	if (bVisible && m_pCurPage)
	{
		m_pCurPage->SetVisible(bVisible);
	}
	CControlUI::SetVisible(bVisible);
}

void CTabFolderUI::Event(TEventUI& event)
{
	if (IsEnabled())
	{
		if (event.Type == UIEVENT_BUTTONDOWN )
		{
			for (int i = 0; i < m_ChildList.GetSize() && i < m_aTabAreas.GetSize(); i++)
			{
				RECT* pRect = static_cast<LPRECT>(m_aTabAreas[i]);
				if (::PtInRect(pRect, event.ptMouse))
				{
					SelectItem(i);
					return;
				} //end if (::PtInRect(pRect...
			} //end for (int i = 0
		} else if (event.Type == UIEVENT_DRAG)
		{
			for (int i = 0; i < m_ChildList.GetSize() && i < m_aTabAreas.GetSize(); i++)
			{
				RECT* pRect = static_cast<LPRECT>(m_aTabAreas[i]);
				if (::PtInRect(pRect, event.ptMouse))
				{
					SelectItem(i);
					return;
				} //end if (::PtInRect(pRect...
			} //end for (int i = 0
		} else if (event.Type == UIEVENT_SYSKEY)
		{
			if (event.chKey == VK_PRIOR && (event.wKeyState & MK_ALT) != 0)
			{
				SelectItem(m_iCurSel - 1);
				return;
			}
			if (event.chKey == VK_NEXT && (event.wKeyState & MK_ALT) != 0)
			{
				SelectItem(m_iCurSel + 1);
				return;
			} //end if (event.chKey...
		} //end else if (event.Type ...
	} //end if (IsEnabled())
	CContainerUI::Event(event);
}

void CTabFolderUI::SetPos(RECT rc)
{
	CControlUI::SetPos(rc);
	// Determine size of embedded page and place it there
	switch (m_nTabAlign)
	{
		case TABALIGN_HIDDEN:
		case TABALIGN_BOTTOM:
		case TABALIGN_RIGHT:
		case TABALIGN_LEFT:
		case TABALIGN_TOP:
		default:
			 SetTabTopPos();
			 break;
	}
	//tab页面的可用区域
	if (m_pCurPage != NULL)
	{
		m_pCurPage->SetPos(m_rcClient);
	}
}

void CTabFolderUI::SetTabTopPos()
{
	RECT rcItem = { 0, 0, m_szTab.cx, m_szTab.cy };
	RECT rcRet = { 0 };
	rcRet = CBlueRenderEngineUI::DoPaintTabFolder(m_pManager->GetPaintDC(), m_pManager, rcItem, 
		                            _T("标签"), 0, DT_SINGLELINE | DT_CALCRECT, TABALIGN_TOP);
	::CopyRect(&m_rcTabs, &m_rcItem);//tabs area
	m_rcTabs.bottom = m_rcTabs.top + rcRet.bottom - rcRet.top;
	::CopyRect(&m_rcClient, &m_rcItem);//client area
	m_rcClient.top += (rcRet.bottom - rcRet.top);
	//all the tab buttons
	m_aTabAreas.Empty();
	RECT rcTabArea = m_rcTabs;
	for (int i = 0; i < GetCount() ; i++ )
	{
		CTabPageUI* pPage = static_cast<CTabPageUI*>( GetItem(i));
		if (pPage != NULL)
		{
			rcTabArea.right = rcTabArea.left + m_szTab.cx;
			rcTabArea.bottom = rcTabArea.top + m_szTab.cy;
			RECT rcTabButton = CBlueRenderEngineUI::DoPaintTabFolder(m_pManager->GetPaintDC(), 
				m_pManager, rcTabArea, pPage->GetText(), 0, DT_SINGLELINE | DT_CALCRECT, m_nTabAlign);
			pPage->m_tabButton.SetPos(rcTabButton);
			m_aTabAreas.Add(&rcTabButton);
			rcTabArea.left = rcTabButton.right;
			rcTabArea.top = m_rcTabs.top;
		} //end if (pPage != NULL)
	} //end for (int i = 0...
}

void CTabFolderUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	RECT rcFill = { 0 };
	if (!::IntersectRect(&rcFill, &rcPaint, &m_rcItem))
		return;

	CRenderClip clip;
	CBlueRenderEngineUI::GenerateClip(hDC, rcPaint, clip);
	
	//client area border and background
	CBlueRenderEngineUI::DoPaintRectangle(hDC, m_pManager, m_rcClient, 
		UICOLOR_TAB_BORDER, UICOLOR_CONTROL_BACKGROUND_NORMAL);

	// Paint tab strip
	//只支持topalign,如果有其他风格，此处代码需修改
	for (int i = 0; i < GetCount() ; i++ )
	{
		CTabPageUI* pPage = static_cast<CTabPageUI*>(GetItem(i));
		pPage->m_tabButton.Select(m_iCurSel == i);
		pPage->m_tabButton.DoPaint(hDC, rcPaint);
	}

	//current selected page
	if (m_pCurPage != NULL)
	{
		m_pCurPage->DoPaint(hDC, rcPaint);
	}
}

void CTabFolderUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcsicmp(pstrName, _T("select")) == 0) 
		SelectItem(_ttoi(pstrValue));
	else if (_tcsicmp(pstrName, _T("tabalign")) == 0)
	{
		if (_tcsicmp(pstrValue, _T("bottom")) == 0)
			SetTabAlign(TABALIGN_BOTTOM);
		else if (_tcsicmp(pstrValue, _T("left")) == 0)
			SetTabAlign(TABALIGN_LEFT);
		else if (_tcsicmp(pstrValue, _T("right") ) == 0)
			SetTabAlign(TABALIGN_RIGHT );
		else if ( _tcsicmp(pstrValue, _T("hidden")) == 0)
			SetTabAlign(TABALIGN_HIDDEN);
		else
			SetTabAlign(TABALIGN_TOP);
	} else if (_tcsicmp(pstrName, _T("roundrect")) == 0)
	{
		m_szClientCorner.cx = m_szClientCorner.cy = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("tabwidth")) == 0)
	{
		SIZE sz = GetTabSize();
		sz.cx = _ttoi(pstrValue);
		SetTabSize(sz);
	} else if (_tcsicmp(pstrName, _T("tabheight")) == 0)
	{
		SIZE sz = GetTabSize();
		sz.cy = _ttoi(pstrValue);
		SetTabSize(sz);
	} else if (_tcsicmp(pstrName, _T("currentpage")) == 0)
	{
		SelectItem(pstrValue);
	} else
		CContainerUI::SetAttribute(pstrName, pstrValue);
}

void CTabFolderUI::SetTabSize( const SIZE& sz )
{
	if ((m_szTab.cx != sz.cx) || (m_szTab.cy != sz.cy))
	{
		m_szTab = sz;
		Invalidate();
	}
}

SIZE CTabFolderUI::GetTabSize() const
{
	return m_szTab;
}

bool CTabFolderUI::SetTabAlign( UINT nAlign )
{
	if ((nAlign >= TABALIGN_TOP) && (nAlign < TABALIGN__INVALID))
	{
		m_nTabAlign = nAlign;
		Invalidate();
		return true;
	}
	return false;
}


/////////////////////////////////////////////////////////////////////////////////////
//
void CTabButtonUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	RECT rcFill = { 0 };
	if (!::IntersectRect(&rcFill, &m_rcItem, &rcPaint))
		return;

	UINT uState = 0;
	if (IsFocused())
		uState |= UISTATE_FOCUSED;
	if (!IsEnabled())
		uState |= UISTATE_DISABLED;
	uState |= GetButtonState();

	CBlueRenderEngineUI::DoPaintTabFolder(hDC, m_pManager, m_rcItem, 
		GetText(), uState, DT_SINGLELINE | DT_CENTER | DT_VCENTER, TABALIGN_TOP);
}

void CTabButtonUI::Select(BOOL bSelect)
{
	if (bSelect)
		SetButtonState(GetButtonState() &  ~UISTATE_SELECTED);
	else
		SetButtonState(GetButtonState() | UISTATE_SELECTED);
}
/////////////////////////////////////////////////////////////////////////////////////
//
//

CTabPageUI::CTabPageUI(): 
            m_bActive( true )
{
	SetBorder( false );
}

LPCTSTR CTabPageUI::GetClass() const
{
	return _T("TabPageUI");
}

void CTabPageUI::Select( bool )
{
	//
}

void CTabPageUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcsicmp(pstrName, _T("active")) == 0)
	{
		Active(_tcscmp(pstrValue, _T("true")) == 0);
	} else
		CContainerUI::SetAttribute(pstrName, pstrValue);
}


void CTabPageUI::Active(bool bActive)
{
	m_bActive = bActive;
}

void CTabPageUI::SetEnabled(bool bEnable)
{
	if (m_bEnabled != bEnable)
	{
		m_tabButton.SetEnabled(bEnable);
		CContainerUI::SetEnabled(bEnable);
	}
}

bool CTabPageUI::IsActive() const
{
	return (IsEnabled() && m_bActive);
}

CControlUI* CTabPageUI::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags)
{
	if ((uFlags & UIFIND_ME_FIRST) != 0) 
	{
		CControlUI* pControl = CControlUI::FindControl(Proc, pData, uFlags);
		if (pControl != NULL) 
			return pControl;
	}
	if (m_tabButton.FindControl(Proc, pData, uFlags))
		return &m_tabButton;
	return CContainerUI::FindControl(Proc, pData, uFlags);
}
/////////////////////////////////////////////////////////////////////////////////////
//
//

CImageTabFolderUI::CImageTabFolderUI(): 
                   m_nImageID(IMGID_INVALID_),
				   m_nTabOffset(2),
				   m_nTabImageId(IMGID_INVALID_),
				   m_bTabBtnWidthByText(FALSE)
{
	//
}

CImageTabFolderUI::~CImageTabFolderUI()
{
}

LPCTSTR CImageTabFolderUI::GetClass() const
{
	return _T("ImageTabFolder");
}

void CImageTabFolderUI::Init()
{
	for (int iPage = 0; iPage < m_ChildList.GetSize(); ++ iPage)
	{
		CImageTabPageUI* pPage = dynamic_cast<CImageTabPageUI*>(GetItem(iPage));
		if (pPage)
			pPage->m_imageButton.SetManager(m_pManager, this);
	}
	CTabFolderUI::Init();
}

UINT CImageTabFolderUI::GetImage() const
{
	return m_nImageID;
}

void CImageTabFolderUI::SetImage(UINT nImage)
{
	if (m_nImageID != nImage)
	{
		m_nImageID = nImage;
		Invalidate();
	}
}

void CImageTabFolderUI::SetAttribute( LPCTSTR pstrName, LPCTSTR pstrValue )
{
	if (_tcsicmp(pstrName, _T("taboffset")) == 0)
	{
		m_nTabOffset = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("backimage")) == 0)
	{
		SetImage( _ttoi( pstrValue ) );
	} else if (_tcsicmp(pstrName, _T("tabbkimage")) == 0)
	{
		m_nTabImageId = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("tabbtnwidthbypagetext" )) == 0)
	{
		m_bTabBtnWidthByText = (_tcsicmp(pstrValue, _T("true")) == 0);
	} else
	{
		CTabFolderUI::SetAttribute(pstrName, pstrValue);
	}
}

void CImageTabFolderUI::SetPos(RECT rc)
{
	CControlUI::SetPos(rc);
	// Determine size of embedded page and place it there
	switch (m_nTabAlign)
	{
		case TABALIGN_LEFT://left
			SetPosLeftTabs();
			break;
		case TABALIGN_HIDDEN://hidden
			SetPosHiddenTabs();
			break;
		case TABALIGN_RIGHT://others
		case TABALIGN_BOTTOM:
		case TABALIGN_TOP:
		default:
			SetPosTopTabs();
			break;
	}
   
	//tab页面的可用区域
	if (m_pCurPage != NULL)
	{
		m_pCurPage->SetPos(m_rcClient);
	}
}

void CImageTabFolderUI::SetPosHiddenTabs()
{
	//tabs area
	::SetRect( &m_rcTabs, 0, 0, 0, 0 );
	//client
	::CopyRect( &m_rcClient, &m_rcItem );
}

void CImageTabFolderUI::SetPosBottomTabs()
{
}

int CImageTabFolderUI::GetTabButtonWidth(const CStdString& sText)
{
	if (m_bTabBtnWidthByText)
	{
		CRect rc, rcRet;
		rcRet = CBlueRenderEngineUI::DoPaintTabFolder(m_pManager->GetPaintDC(), m_pManager, rc, 
			sText, 0, DT_SINGLELINE | DT_CALCRECT, TABALIGN_TOP );
		return (rcRet.right - rcRet.left);
	} else
	{
		return m_szTab.cx;
	}
}

void CImageTabFolderUI::SetPosTopTabs()
{
   //client
	::SetRect(&m_rcClient, m_rcItem.left, 
			   m_rcItem.top + m_szTab.cy, m_rcItem.right, m_rcItem.bottom);
	//tabs area
	::CopyRect(&m_rcTabs, &m_rcItem);
	m_rcTabs.left += m_szClientCorner.cx / 2;
	m_rcTabs.right -= m_szClientCorner.cx / 2;
	m_rcTabs.bottom = m_rcClient.top;
	//position each tab button
	CRect rcTabButton(m_rcTabs);
	rcTabButton.bottom += m_nTabOffset;//偏移量用于覆盖client区域的边框
	rcTabButton.right = rcTabButton.left;
	m_aTabAreas.Empty();
	for (int iPage = 0; iPage < m_ChildList.GetSize(); ++iPage)
	{
		CImageTabPageUI* pPage = static_cast<CImageTabPageUI*>(GetItem(iPage));

		//上一个tab按钮的右边框作为当前按钮的左边框
		rcTabButton.left = rcTabButton.right + m_iPadding;
		//最后得到当前按钮的位置
		rcTabButton.right = rcTabButton.left + GetTabButtonWidth(pPage->GetText());
		pPage->m_imageButton.SetPos( rcTabButton );
		if (m_bTabBtnWidthByText)
		{
			pPage->m_imageButton.SetFixedCorner(5);
			pPage->m_imageButton.SetStretchMode(SM_HORIZONTAL);
		} 
		m_aTabAreas.Add(&rcTabButton);
	}
}

void CImageTabFolderUI::SetPosLeftTabs()
{
	//client area
	::CopyRect(&m_rcClient, &m_rcItem);
	m_rcClient.left += m_szTab.cx;
	//tabs area
	::CopyRect(&m_rcTabs, &m_rcItem);
	m_rcTabs.top += m_szClientCorner.cy / 2;
	m_rcTabs.right = m_rcClient.left;
	m_rcTabs.bottom -= m_szClientCorner.cy / 2;
	//each tab button,目前，如果所有图标的总高度超过了tabarea
	//的高度，有的图标将不会被绘出。扩展功能以后再添加
	CRect rcTabButton(m_rcTabs);
	rcTabButton.right += m_nTabOffset;
	rcTabButton.bottom = rcTabButton.top;
	m_aTabAreas.Empty(); 
	for (int iPage = 0; iPage < m_ChildList.GetSize(); ++iPage)
	{
		rcTabButton.top = rcTabButton.bottom + m_iPadding;
		rcTabButton.bottom = rcTabButton.top + m_szTab.cy;
		CImageTabPageUI* pPage = dynamic_cast<CImageTabPageUI*>(GetItem(iPage));
		pPage->m_imageButton.SetPos(rcTabButton); 
		m_aTabAreas.Add(&rcTabButton);
	}
}

void CImageTabFolderUI::SetPosRightTabs()
{
}

void CImageTabFolderUI::PaintTabButtons(HDC hDC, const RECT& rcPaint)
{
	for (int iPage = 0; iPage < GetCount(); ++iPage)
	{
		//draw tab buttons no matter if the their owner are visible
		CImageTabPageUI* pPage = static_cast<CImageTabPageUI*>(GetItem(iPage));
		pPage->m_imageButton.Select(m_iCurSel == iPage);
		pPage->m_imageButton.DoPaint(hDC, rcPaint);
		//paint text TBD
		UITYPE_COLOR tcText = (m_iCurSel == iPage) ? UICOLOR_TAB_TEXT_SELECTED : UICOLOR_CONTROL_TEXT_NORMAL;
		UITYPE_COLOR tcBack = UICOLOR__INVALID;
		UINT nTextStyle = DT_SINGLELINE | DT_CENTER | DT_VCENTER;
		RECT rcText = pPage->m_imageButton.GetPos();
		CStdString sText = pPage->GetText();
		if (!sText.IsEmpty())
			CBlueRenderEngineUI::DoPaintQuickText(hDC, m_pManager, rcText, sText,
			        tcText, UIFONT_NORMAL, nTextStyle);
	}
}

void CImageTabFolderUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	//checking
	CRect rcFill( 0, 0, 0, 0 );
	if (!::IntersectRect(&rcFill, &rcPaint, &m_rcItem)) 
		return;
	//region limit
	CRenderClip clip;
	CBlueRenderEngineUI::GenerateClip(hDC, m_rcItem, clip);

	//standard client area, border & backgound
	if (HasBorder())
	{
		CBlueRenderEngineUI::DoPaintRoundRect(hDC, m_rcClient,
			 GetBorderColor(), GetBkgndColor(), m_szClientCorner);
	} else if (m_nImageID != IMGID_INVALID_)
	{
		//background image
		StretchFixed sf;
		sf.SetFixed(8);
		CBlueRenderEngineUI::DoPaintGraphic(hDC, m_pManager, m_rcClient, m_nImageID, sf);
	} else if (m_nTabImageId != IMGID_INVALID_)
	{
		RECT rc(m_rcItem);
		if (m_nTabAlign == TABALIGN_LEFT)
		{
			rc.right = rc.left + m_szTab.cx;
		} else if (m_nTabAlign == TABALIGN_TOP)
		{
			rc.bottom = rc.left + m_szTab.cy;
		}
		CBlueRenderEngineUI::DoPaintGraphic(hDC, m_pManager, rc, m_nTabImageId);
	} else
	{
		//draw a line which separate the tabs and client areas
		CRect rcLine(m_rcClient);
		if (m_nTabAlign == TABALIGN_LEFT)
		{ 
			rcLine.right = rcLine.left;
			if (m_szTab.cx == 0)
				rcLine.bottom = rcLine.top;
		} else if (m_nTabAlign == TABALIGN_TOP)
		{ 
			rcLine.bottom = rcLine.top;
			if (m_szTab.cy == 0)
				rcLine.right = rcLine.left;
		} else
		{
			::SetRect(&rcLine, 0, 0, 0, 0);
		}
		if (!::IsRectEmpty(&rcLine))
			CBlueRenderEngineUI::DoPaintLine(hDC, rcLine, m_clrBorder);
	}

	//current select page
	if (m_pCurPage != NULL) 
		m_pCurPage->DoPaint(hDC, rcPaint);
	
	//tab buttons
	PaintTabButtons( hDC, rcPaint );
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

CImageTabPageUI::CImageTabPageUI()
{
}

CImageTabPageUI::~CImageTabPageUI()
{
}

LPCTSTR CImageTabPageUI::GetClass() const
{
	return _T("ImageTabPage");
}

void CImageTabPageUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcsicmp(pstrName, _T("image")) == 0)
	{
		//tabbutton image
		m_imageButton.SetAttribute(pstrName, pstrValue);
	} else if (_tcsicmp(pstrName, _T("background")) == 0)
	{
		m_imageButton.SetAttribute(pstrName, pstrValue);
	} else if (_tcsicmp(pstrName, _T("tabbutton_tooltip")) == 0)
	{
		SetTabButtonTooltip(pstrValue);
	} else
		CTabPageUI::SetAttribute(pstrName, pstrValue);
}

void CImageTabPageUI::SetEnabled(bool bEnabled)
{
	if (m_bEnabled != bEnabled)
	{
		m_imageButton.SetEnabled(bEnabled);
		CContainerUI::SetEnabled(bEnabled);
	}
}

void CImageTabPageUI::SetTabButtonTooltip(const CStdString& sTooltip)
{
	m_imageButton.SetToolTip(sTooltip);
}

CControlUI *CImageTabPageUI::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags)
{
	if ((uFlags & UIFIND_ME_FIRST) != 0) 
	{
		CControlUI* pControl = CControlUI::FindControl(Proc, pData, uFlags);
		if (pControl != NULL) 
			return pControl;
	}
	if (m_imageButton.FindControl(Proc, pData, uFlags))
		return &m_imageButton;
	return CTabPageUI::FindControl(Proc, pData, uFlags);
}
/////////////////////////////////////////////////////////////////////////////////////
//
//
CImageTabButtonUI::CImageTabButtonUI()
{
}

CImageTabButtonUI::~CImageTabButtonUI()
{
}

LPCTSTR CImageTabButtonUI::GetClass() const
{
	return _T("ImageTabButtonUI");
}

void CImageTabButtonUI::Select( BOOL bSel )
{
	if (bSel)
		SetButtonState(GetButtonState() | UISTATE_PUSHED);
	else
		SetButtonState(GetButtonState() & ~UISTATE_PUSHED);
}

void CImageTabButtonUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	RECT rcTemp = {0};
	if (!::IntersectRect(&rcTemp, &rcPaint, &m_rcItem)) 
		return;
	StretchFixed sf(GetFixedCorner());
	CBlueRenderEngineUI::DoPaintTabImageButton(hDC, m_pManager, m_rcItem, 
		GetButtonState(), m_iBkgImageId, GetImage(), &sf, GetStretchMode());
}
