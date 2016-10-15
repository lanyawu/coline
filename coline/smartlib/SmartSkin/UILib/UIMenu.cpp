#include "common.h"

#include <UILIb/UIMenu.h>
#include <UILib/UIBase.h>
#include <UILib/UIDlgBuilder.h>
#include <CommonLib/StringUtils.h>


const UINT MENU_IMAGE_WIDTH = 20;

///////////////////////////////////////////////////////////////////////////
//
//

CMenuButtonUI::CMenuButtonUI(void): 
               m_pMenu(NULL),
			   m_bDspMenuText(FALSE)
{
	//
}

CMenuButtonUI::~CMenuButtonUI(void)
{
	DestroyMenu();
}

LPCTSTR CMenuButtonUI::GetClass() const
{
	return _T("MenuButtonUI");
}

UINT CMenuButtonUI::GetControlFlags() const
{
	return UIFLAG_TABSTOP;
}

CMenuUI *CMenuButtonUI::GetPopMenu()
{
	return m_pMenu;
}

void CMenuButtonUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcscmp(pstrName, _T("menu")) == 0)
	{
		m_sMenuName = pstrValue;
	} else if (_tcscmp(pstrName, _T("displaymenutext")) == 0)
	{
		SetDisplayMenuText( _tcscmp(pstrValue, _T("true") ) == 0 );
	} else
		CImageButtonUI::SetAttribute(pstrName, pstrValue);
}

void CMenuButtonUI::Notify(TNotifyUI& msg)
{
	if (msg.sType == _T("exitmenuloop"))
	{
		SetButtonState( GetButtonState() & ~(UISTATE_PUSHED | UISTATE_CAPTURED) );
		Invalidate();
	} else if (msg.sType == _T("menucommand"))
	{
		CMenuItemUI* pItem = m_pMenu->FindItem(LOWORD(msg.wParam));
		if (pItem)
		{
			//SetText(pItem->GetText());
		}
	}
}

void CMenuButtonUI::OnEventButtonUp(TEventUI& e)
{
	if ((GetButtonState() & UISTATE_CAPTURED) != 0)
	{
		if (::PtInRect(&m_rcItem, e.ptMouse))
		{
			//don't create menu repeatly
			if (m_pManager)
			{
				TNotifyUI Msg;
				Msg.pSender = this;
				Msg.sType = _T("click");
				Msg.bHandled = FALSE;
				m_pManager->SendNotify(Msg);
				if (!Msg.bHandled)
				{
					m_pManager->SendNotify(this, _T("beforemenupop"));
					ShowMenu();//hang up
				} //end if (!Msg.bHandled)
			} //end if (m_pManager)			
		} //end if (::PtInRect(&m_rcItem...
		SetButtonState(GetButtonState() & ~(UISTATE_PUSHED | UISTATE_CAPTURED));
		Invalidate();
	} //end if ((GetButtonState()..
}

BOOL CMenuButtonUI::SetMenu(const CStdString& sMenuName)
{
	if (m_sMenuName != sMenuName)
	{
		m_sMenuName = sMenuName;
		LoadMenu();
		return true;
	}
	return false;
}

void CMenuButtonUI::AttachMenu(CMenuUI* pMenu)
{
	DestroyMenu();
	m_pMenu = pMenu;
	if (m_pMenu)
	{
		m_pMenu->SetManager(m_pManager, NULL);
		m_pMenu->SetOwner( this );	
	}
}

void CMenuButtonUI::SetDisplayMenuText(BOOL bDsp)
{
	if (m_bDspMenuText != bDsp)
	{
		m_bDspMenuText = bDsp;
		Invalidate();
	}
}

BOOL CMenuButtonUI::ShowMenu()
{
	POINT pt = { m_rcItem.left, m_rcItem.bottom };
	::ClientToScreen(m_pManager->GetPaintWindow(), &pt);
	if (m_pMenu)
	{
		//default menu parent window
		m_pMenu->TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y);//hang up here
		return true;
	}
	return false;
}

void CMenuButtonUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	RECT rcTemp = {0};
	if (!::IntersectRect(&rcTemp, &rcPaint, &m_rcItem)) 
		return;
	//button
	CImageButtonUI::DoPaint(hDC, rcPaint);
	//button text
	if (m_bDspMenuText)
	{
		int iLinks = 0;
		CRect rcText(m_rcItem);
		CBlueRenderEngineUI::DoPaintQuickText(hDC, m_pManager, rcText,
			GetText(), m_clrBorder, UIFONT_NORMAL, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	}
}

void CMenuButtonUI::Init()
{
	CImageButtonUI::Init();
	ASSERT(m_pManager != NULL);
	LoadMenu();
}

BOOL CMenuButtonUI::LoadMenu()
{
	DestroyMenu();
	ASSERT(m_pMenu == NULL);
	if (m_pManager)
	{
		m_pMenu = m_pManager->LoadMenuUI(m_sMenuName);
		if (m_pMenu)
		{
			m_pMenu->SetManager(m_pManager, NULL);
			m_pMenu->SetOwner(this);	
		}
		return (m_pMenu != NULL);
	}
	return false;
}

void CMenuButtonUI::DestroyMenu()
{
	if (m_pMenu != NULL)
	{
		if (m_pManager)
			m_pManager->ReleaseMenuUI(&m_pMenu); 
	}
}

//////
void CPlusMenuButtonUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcscmp(pstrName, _T("arrowimage")) == 0)
	{
		m_uArrowImage = _ttoi(pstrValue);
	} else if (_tcscmp(pstrName, _T("subimage")) == 0)
	{
		m_uSubImage = _ttoi(pstrValue);
	} else
		CMenuButtonUI::SetAttribute(pstrName, pstrValue);
}

SIZE CPlusMenuButtonUI::EstimateSize(SIZE szAvailable)
{
	if (GetHeight() > 0 && GetWidth() > 0)
		return CSize(GetHeight(), GetWidth());

 
	int ImageCx, ImageCy;
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
			ImageCx = cx;
			ImageCy = cy / piri->dwSubCount;
		} else	if (piri->dwListFmt == ILF_HORIZONTAL)
		{
			ImageCx = cx / piri->dwSubCount;
			ImageCy = cy;
		}
	}
	SIZE sz = {0, 0};
	if (!m_sText.IsEmpty())
		GetTextExtentPoint32(m_pManager->GetPaintDC(), m_sText, m_sText.GetLength(), &sz); 
	if (GetWidth() != 0)
	{
		return CSize(GetWidth(), ImageCy);
		 
	} else
	{ 
		return CSize(ImageCx + 20, ImageCy);
		  
	}
}

void CPlusMenuButtonUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	RECT rcTemp = {0};
	if (!::IntersectRect(&rcTemp, &rcPaint, &m_rcItem)) 
		return;
   // Draw button
   UINT uState = 0;
   if (IsFocused()) 
	   uState |= UISTATE_FOCUSED;
   if (!IsEnabled())
	   uState |= UISTATE_DISABLED;
   if (m_bDown)
	   uState |= UISTATE_PUSHED; 
   CBlueRenderEngineUI::DoPaintPlusMenuButton(hDC, 
									  m_pManager, 
									  m_rcItem, 
									  m_sText, 
									  m_szPadding,
									  m_uButtonState | uState, 
									  m_uTextStyle, m_iBkgImageId, m_uImageId, m_uSubImage, 
									  m_uArrowImage, m_bTransparent);
}

//////////////////////////////////////////////////////////////////////////////
//
//
CMenuItemUI::CMenuItemUI(BOOL bLine): 
             m_uID(-1),
		     m_uImageID(IMGID_INVALID_),
			 m_pSubMenu(NULL),
			 m_bLine(bLine),
			 m_bkColor(0),
			 m_bkImageClr(0),
			 m_clrSel(0),
			 m_bAutoChecked(FALSE),
			 m_uGroupId(0),
			 m_bChecked(FALSE),
			 m_clrText(0)
{
		
}

CMenuItemUI::~CMenuItemUI()
{
	if (m_pSubMenu)
	{
		if (m_pManager)
			m_pManager->ReleaseMenuUI(&m_pSubMenu); 
	}
}

LPCTSTR CMenuItemUI::GetClass() const
{
	return _T("MenuItemUI");
}

void CMenuItemUI::Init()
{
	m_pSubMenu = m_pManager->LoadMenuUI(m_sMenuName);
	if (m_pSubMenu)
	{
		m_pSubMenu->SetManager(m_pManager, NULL);
		m_pSubMenu->SetOwner(this);
	}
}

SIZE CMenuItemUI::EstimateSize(SIZE szAvailable)
{
	//determine size by image width, height, text height and length
	//text length
	RECT rcText = { 0, 0, 9999, 9999 };
	CBlueRenderEngineUI::DoPaintQuickText(m_pManager->GetPaintDC(), m_pManager, rcText, 
		m_sText, RGB(0,0,0), UIFONT_NORMAL, DT_CALCRECT | DT_SINGLELINE);
	//image width determined by text height, say, an image with same width and 
	//height is preferred
	SIZE sz = { 0 };
	sz.cy = rcText.bottom - rcText.top + 6;
	sz.cx = rcText.right - rcText.left + sz.cy;
	if (HasPopupMenu())
	{
		//submenu triangle is in consideration
		sz.cx += 10;
	}
	return sz;
}

void CMenuItemUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcsicmp(pstrName, _T("image")) == 0)
	{
		SetImage(_ttoi(pstrValue));
	} else if (_tcsicmp( pstrName, _T("uid")) == 0)
	{
		SetID(_ttoi(pstrValue));
	} else if (_tcsicmp(pstrName, _T("submenu")) == 0)
	{
		m_sMenuName = pstrValue;
	} else if (_tcsicmp(pstrName, _T("line")) == 0)
	{
		m_bLine = (_tcscmp( pstrValue, _T("true")) == 0);
	} else if (_tcsicmp(pstrName, _T("backgroundcolor")) == 0)
	{
		m_bkColor = StringToColor(pstrValue);
	} else if (_tcsicmp(pstrName, _T("imagebackcolor")) == 0)
	{
		m_bkImageClr = StringToColor(pstrValue);
	} else if (_tcsicmp(pstrName, _T("selectcolor")) == 0)
	{
		m_clrSel = StringToColor(pstrValue);
	} else if (_tcsicmp(pstrName, _T("textcolor")) == 0)
	{
		m_clrText = StringToColor(pstrValue);
	} else if (_tcsicmp(pstrName, _T("autocheck")) == 0)
	{
		m_bAutoChecked = (_tcsicmp(pstrValue, _T("true")) == 0);
	} else if (_tcsicmp(pstrName, _T("menugroup")) == 0)
	{
		m_uGroupId = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("checked")) == 0)
	{
		m_bChecked = (_tcsicmp(pstrValue, _T("true")) == 0);
	} else
		CControlUI::SetAttribute(pstrName, pstrValue);
}

void CMenuItemUI::SetImage(UINT uID)
{
	if (m_uImageID != uID)
	{
		m_uImageID = uID;
		Invalidate();
	}
}

void CMenuItemUI::SetUnChecked(UINT uGrpId, UINT uExceptId)
{
	if (m_pSubMenu)
		m_pSubMenu->SetGroupUnCheckedExcept(NULL, uExceptId, uGrpId);
}

void CMenuItemUI::DrawSelectState(HDC hDC, const RECT& rcItem, UINT state)
{
	COLORREF uiBkclr;
	if (m_clrSel == 0)
		uiBkclr = m_pManager->GetThemeColor(UICOLOR_STANDARD_WHITE);
	else
		uiBkclr = m_clrSel;
	if ((state & ODS_SELECTED ) != 0)
	{
		uiBkclr = m_pManager->GetThemeColor(UICOLOR_TOOLBAR_BACKGROUND);
	}
	RECT rcHot = rcItem;
	rcHot.left += MENU_IMAGE_WIDTH;
	::InflateRect(&rcHot, -1, -1);
	CBlueRenderEngineUI::DoFillRect(hDC, m_pManager, rcHot, uiBkclr, m_bTransparent);
}

void CMenuItemUI::DrawText(HDC hDC, const RECT& rcItem, UINT state)
{
	//text
	COLORREF uiTextColor;
	if (m_clrText == 0)
		uiTextColor = m_pManager->GetThemeColor(UICOLOR_CONTROL_TEXT_NORMAL);
	else
		uiTextColor = m_clrText;
	if ((state & ODS_GRAYED ) != 0)
	{
		uiTextColor = m_pManager->GetThemeColor(UICOLOR_CONTROL_TEXT_DISABLED);
	}
	CRect rcText(rcItem);
	rcText.left += (MENU_IMAGE_WIDTH + 5);
	CBlueRenderEngineUI::DoPaintQuickText(hDC, m_pManager, rcText, GetText(), 
		uiTextColor, UIFONT_NORMAL, DT_LEFT | DT_SINGLELINE | DT_VCENTER);	
}

void CMenuItemUI::DrawItem(HDC hDC, const RECT& rcItem, UINT action, UINT state)
{
	//background
	COLORREF clr;
	if (m_bkColor == 0)
		clr = m_pManager->GetThemeColor(UICOLOR_STANDARD_WHITE);
	else
		clr = m_bkColor;
	CBlueRenderEngineUI::DoFillRect(hDC, m_pManager, rcItem, clr, m_bTransparent);
	RECT rcImage = rcItem;
	rcImage.right = rcImage.left + MENU_IMAGE_WIDTH;
	if (m_bkImageClr == 0)
		clr = m_pManager->GetThemeColor(UICOLOR_TOOLBAR_BACKGROUND);
	else
		clr = m_bkImageClr;
	CBlueRenderEngineUI::DoFillRect(hDC, m_pManager, rcImage, clr, m_bTransparent);
	if (m_uImageID > 0)
	{
		//image
		::InflateRect(&rcImage, -2, -2);
		CBlueRenderEngineUI::DoPaintGraphic(hDC, m_pManager, rcImage, m_uImageID);
	}
	//width of rcItem must be greater than the image width plus the text width
	if ((action & ODA_DRAWENTIRE ) != 0)
	{
		DrawText(hDC, rcItem, state);
	}
	if ((action & ODA_SELECT ) != 0)
	{
		//select state
		DrawSelectState(hDC, rcItem, state);
		//text
		DrawText(hDC, rcItem, state);	
	}

	//check sign TBD
	if ((state & ODS_GRAYED) == 0 && (state & ODS_CHECKED) != 0)
	{
		// not grayed
		CBlueRenderEngineUI::DoPaintGraphic(hDC, m_pManager, rcImage, 
			m_pManager->GetMenuCheckImage(), 0, NULL, SM_NOSTRETCH);
	}
}

BOOL CMenuItemUI::HasPopupMenu() const 
{ 
	return (m_pSubMenu != 0); 
}

HMENU CMenuItemUI::GetPopupMenuHandle() const
{
	if (HasPopupMenu())
		return m_pSubMenu->GetHMENU();
	return NULL;
}

CMenuUI *CMenuItemUI::GetPopupMenu() const
{
	if (HasPopupMenu())
	{
		return m_pSubMenu;
	}
	return NULL;
}

void CMenuItemUI::AttachMenu(CMenuUI* pMenu)
{
	if (m_pSubMenu != NULL)
	{
		delete m_pSubMenu;
		m_pSubMenu = NULL;
	}

	m_pSubMenu = pMenu;
	if (m_pSubMenu)
	{
		m_pSubMenu->SetManager(m_pManager, NULL);
		m_pSubMenu->SetOwner(this);	
	}
}


CMenuItemUI *CMenuItemUI::FindItem(UINT nID)
{
	if (m_uID == nID)
		return this;
	if (m_pSubMenu) 
		return m_pSubMenu->FindItem(nID);
	return NULL;
}
//////////////////////////////////////////////////////////////////////////////
//
//


class CMenuWnd : public CWindowWnd
{
public:
	CMenuWnd():   m_pMenu(NULL) { }

	void AttatchMenu(CMenuUI* pMenu)
	{ 
		m_pMenu = pMenu; 
	}
protected:
	LPCTSTR GetWindowClassName() const
	{ 
		return _T("MenuOwnerWindow"); 
	}
    void OnFinalMessage(HWND hWnd)
	{ 
		delete this; 
		m_pMenu->m_pParentWnd = NULL;
	}

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		BOOL bHandled = FALSE;
		LRESULT lret = 0;
		switch( uMsg )
		{
		case WM_CREATE:
			m_pm.Init( m_hWnd );
			break;
		case WM_MEASUREITEM:
		case WM_DRAWITEM:
		case WM_COMMAND:
		case WM_INITMENUPOPUP:
		case WM_EXITMENULOOP:
			bHandled = m_pMenu->OnMessage( uMsg, wParam, lParam, &lret );
			break;
		default:
			break;
		}
		if( bHandled )
			return lret;
		else
			return CWindowWnd::HandleMessage( uMsg, wParam, lParam );
	}

public:
    CMenuUI* m_pMenu;
	CPaintManagerUI m_pm;
};
//////////////////////////////////////////////////////////////////////////////
//
//
//一个应用程序同时只能存在一个菜单，用m_pParentWnd作为当前菜单的父窗口，方便
//处理菜单消息（前提是TrackPopupMenu的hWnd参数为空，如果不为空，则由该窗口的
//窗口函数处理菜单消息）
CMenuWnd* CMenuUI::m_pParentWnd = 0;

CMenuUI::CMenuUI(): 
         m_hMenu(NULL),
	     m_pOwner(NULL)
{
}

CMenuUI::~CMenuUI()
{
	DestroyMenu();
}

LPCTSTR CMenuUI::GetClass() const
{
	return _T("MenuUI");
}

bool CMenuUI::Add(CControlUI* pControl, const int nIdx)
{
	//only CMenuItemUI can be added
	CMenuItemUI* pItem = dynamic_cast<CMenuItemUI*>(pControl);
	if (pItem && CContainerUI::Add( pControl ))
	{
		if (m_hMenu)
		{
			AppendMenuItem(*pItem);
		}
		return true;
	}
	return false;
}

bool CMenuUI::Remove(CControlUI* pControl)
{
	//remove from "container"
	//remove from menu object
	if (pControl)
	{
		CMenuItemUI *pItem = dynamic_cast<CMenuItemUI *>(pControl);
		if (pItem)
		{
			::RemoveMenu(m_hMenu, pItem->GetID(), MF_BYCOMMAND);
		}
	}
	return CContainerUI::Remove(pControl);
}

void CMenuUI::RemoveAll()
{
	//remove
	if (m_hMenu)
	{
		for (int it = 0; m_bAutoDestroy && it < m_ChildList.GetSize(); it++) 
		{
			CMenuItemUI* pItem = dynamic_cast<CMenuItemUI*>(m_ChildList[it]); 
			if (pItem)
			{
				::RemoveMenu(m_hMenu, pItem->GetID(), MF_BYCOMMAND);
			}
		}
	}
	CContainerUI::RemoveAll();
}

void CMenuUI::Notify(TNotifyUI& msg)
{
	if (msg.sType == _T("exitmenuloop") 
		|| (msg.sType == _T("menucommand")))
	{
		if (m_pOwner)
		{
			m_pOwner->Notify(msg);
		}
	}
}

//CControlUI oeverridable
void CMenuUI::Init()
{
	if (m_hMenu != NULL)
	{
		::DestroyMenu(m_hMenu);
		m_hMenu = NULL;
	}
	ASSERT(m_hMenu == NULL);
	m_hMenu = ::CreatePopupMenu();
	if (m_hMenu == NULL)
	{
		return;
	}
	//add all menuitems and popup menus if any
	for (int i = 0; i < m_ChildList.GetSize(); ++i)
	{
		CMenuItemUI* pItem = dynamic_cast<CMenuItemUI*>(m_ChildList[i]);
		ASSERT(pItem != NULL);
		AppendMenuItem(*pItem);
	} 
}


void CMenuUI::SetGroupUnCheckedExcept(HMENU h, int nExceptId, int nGroupId)
{
	if (h == NULL)
		h = m_hMenu;
	for (int i = 0; i < m_ChildList.GetSize(); ++i)
	{
		CMenuItemUI* pItem = dynamic_cast<CMenuItemUI*>(m_ChildList[i]); 
		if ((pItem->m_uGroupId == nGroupId) && (pItem->m_uID != nExceptId))
			CheckItem(h, pItem->m_uID, FALSE);
		if (pItem->HasPopupMenu())
		{

		} 
	}
}

BOOL CMenuUI::OnMessage(UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT* plret)
{
	switch( nMsg )
	{
		case WM_INITMENUPOPUP:
			{
				m_pManager->SendNotify(this, _T("initmenupopup"), wParam, lParam);
				*plret = 0;
				return TRUE;
			}
		case WM_MEASUREITEM:
			{
				*plret = MeasureItem((LPMEASUREITEMSTRUCT)lParam);
				return (*plret == TRUE);
			}
		case WM_DRAWITEM:
			{
				*plret = DrawItem((LPDRAWITEMSTRUCT)lParam);
				return ( *plret == TRUE );
			}
		case WM_COMMAND:
			{
				BOOL bChecked = DoCheckItem(m_hMenu, wParam);
				m_pManager->SendNotify(this, _T("menucommand"), wParam, LPARAM(bChecked));
				*plret = TRUE;
				return TRUE;
			}
		case WM_EXITMENULOOP:
			{
				m_pManager->SendNotify(this, _T("exitmenuloop"), wParam, lParam);
				*plret = 0;
				return TRUE;
			}
		default:
			break;
	}
	return FALSE;
}

BOOL CMenuUI::CheckGroupExcept(HMENU h, UINT uID, BOOL bChecked)
{
	if (bChecked)
		return DoCheckItem(h, uID);
	else
		return CheckItem(h, uID, FALSE);
}

BOOL CMenuUI::DoCheckItem(HMENU h, UINT uId)
{
	int nGroupId = 0;
	BOOL bChecked = FALSE;
	for (int i = 0; i < m_ChildList.GetSize(); ++i)
	{
		CMenuItemUI* pItem = dynamic_cast<CMenuItemUI*>(m_ChildList[i]);
		CMenuItemUI *p = NULL;
		if (pItem->m_uID == uId)
		{
			p = pItem; 
		} else
		{
			p = pItem->FindItem(uId); 
		}
		if (p)
		{ 
			if (p->m_uGroupId > 0)
			{
				nGroupId = p->m_uGroupId;
				CheckItem(m_hMenu, p->m_uID, TRUE);
				bChecked = TRUE;
			} else if (pItem->m_bAutoChecked)
			{
				MENUITEMINFO mii = { 0 };
				mii.cbSize = sizeof(MENUITEMINFO);
				mii.fMask = MIIM_STATE;
				::GetMenuItemInfo(m_hMenu, uId, FALSE, &mii);
				if ((mii.fState & MFS_CHECKED) > 0)
				{
					CheckItem(m_hMenu, uId, FALSE);
					bChecked = FALSE;
				} else
				{
					CheckItem(m_hMenu, uId, TRUE);
					bChecked = TRUE;
				}
			} //end else if (
			break;
		}  
	}
	if (nGroupId > 0)
		SetGroupUnCheckedExcept(m_hMenu, uId, nGroupId);
	return bChecked;
}

//operation
BOOL CMenuUI::CreatePopupMenu()
{
	DestroyMenu();
	m_hMenu = ::CreatePopupMenu();
	return ( m_hMenu != NULL );
}

BOOL CMenuUI::DestroyMenu()
{
	RemoveAll();
	if (m_hMenu)
	{
		BOOL bRet = ::DestroyMenu(m_hMenu);
		m_hMenu = NULL;
		return bRet;
	}
	return true;
}


BOOL CMenuUI::Remove(int i)
{
	if ((i >= 0) && (i < GetCount()))
	{
		CControlUI* pCtrl = static_cast<CControlUI*>(GetItem(i));
		ASSERT(pCtrl != NULL);
		CContainerUI::Remove(pCtrl);

		if (m_hMenu)
		{
			ASSERT(GetCount() + 1 == ::GetMenuItemCount(m_hMenu));
			::DeleteMenu(m_hMenu, i, MF_BYPOSITION);
		}
		return true;
	}
	return false;
}

BOOL CMenuUI::TrackPopupMenu(UINT nFlags, int x, int y, BOOL bIconMenu, LPCRECT lpRect)
{
	ASSERT(m_pManager != NULL);
	if (m_hMenu && m_pManager)
	{
		if (m_pParentWnd == NULL)
		{
			//m_pParentWnd将在PaintWnd销毁时销毁
			m_pParentWnd = new CMenuWnd;
			if (GetPaintWnd())
			{
				//the window object will be deleted when its parent is closed.
				m_pParentWnd->Create(GetPaintWnd(), _T("DefMenuParent"), WS_CHILD, 0);
			} else
			{
				ASSERT(FALSE);
			}
		}
		m_pParentWnd->AttatchMenu(this);
		if (bIconMenu)
		{
			//如果是托盘按钮，这样做之后，菜单才会相应esc，也才会鼠标点击其他
			//窗口时消失
			::SetForegroundWindow( *m_pParentWnd );
		}
		BOOL bRet = ::TrackPopupMenu(m_hMenu, nFlags, x, y, 0, *m_pParentWnd, lpRect);
		if (bIconMenu)
		{
			PostMessage(*m_pParentWnd, WM_NULL, 0, 0);
		}
		return bRet;
	}
	return false;
}

HWND CMenuUI::GetPaintWnd()
{
	//如果程序同时有多个paint manager，m_pParentWnd只作为其中一个manager
	//控制的paint window的子窗口，这个并没有关系。paint window销毁时，菜单
	//的父窗口销毁，其他paint window中弹出菜单时，会重新创建m_pParentWnd。
	if (m_pManager)
		return m_pManager->GetPaintWindow();
	return NULL;
}


BOOL CMenuUI::GrayItem(HMENU h, UINT nID, BOOL bGrayed)
{
	MENUITEMINFO mii = { 0 };
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STATE;
	if (h == NULL)
		h = m_hMenu;
	::GetMenuItemInfo(h, nID, FALSE, &mii);
	
	if (bGrayed)
	{
		mii.fState |= MFS_GRAYED;
	} else
	{
		mii.fState &= ~MFS_GRAYED;
	}
	return ::SetMenuItemInfo(h, nID, FALSE, &mii );
}

BOOL CMenuUI::CheckItem(HMENU h, UINT nID, BOOL bCheck)
{
	MENUITEMINFO mii = { 0 };
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STATE;
	if (h == NULL)
		h = m_hMenu;
	::GetMenuItemInfo(h, nID, FALSE, &mii);

	if (bCheck)
	{
		mii.fState |= MFS_CHECKED;
	} else
	{
		mii.fState &= ~MFS_CHECKED;
	}
	return ::SetMenuItemInfo(h, nID, FALSE, &mii);
}

BOOL CMenuUI::AppendMenuItem(const CMenuItemUI& item)
{
	ASSERT( m_hMenu != NULL );
	HMENU h = m_hMenu;
	if (item.IsLineOnly())
	{
		::AppendMenu(m_hMenu, MF_SEPARATOR | MF_OWNERDRAW, 0, 0);
	} else if (item.HasPopupMenu())
	{
		HMENU hSubMenu = item.GetPopupMenuHandle();
		::AppendMenu(m_hMenu, MF_OWNERDRAW | MF_POPUP, (UINT_PTR)hSubMenu, (LPCTSTR)(&item));
		h = m_hMenu;
	} else
	{
		::AppendMenu(m_hMenu, MF_OWNERDRAW | MF_BYCOMMAND, item.GetID(), (LPCTSTR)(&item)); 
	}
	if (item.m_bChecked)
		CheckItem(h, item.m_uID, TRUE);
	return TRUE;
}


void CMenuUI::DrawSeparatorLine(HDC hDC, const RECT& rcItem)
{
	//background
	CBlueRenderEngineUI::DoFillRect(hDC, m_pManager, rcItem, UICOLOR_STANDARD_WHITE, m_bTransparent);
	RECT rcImage = rcItem;
	rcImage.right = rcImage.left + MENU_IMAGE_WIDTH;
	CBlueRenderEngineUI::DoFillRect(hDC, m_pManager, rcImage, UICOLOR_TOOLBAR_BACKGROUND, m_bTransparent);
	//line
	RECT rcLine = rcItem;
	rcLine.top = rcLine.bottom = (rcItem.bottom + rcItem.top) / 2;
	rcLine.left += ( MENU_IMAGE_WIDTH + 2 );
	rcLine.right -= 2;
	CBlueRenderEngineUI::DoPaintLine(hDC, m_pManager, rcLine, UICOLOR_TOOLBAR_BACKGROUND);
}

BOOL CMenuUI::AppendSeparatorLine()
{
	CMenuItemUI* pMenuItem = new CMenuItemUI(true);
	return Add(pMenuItem, 99999);
}


CMenuItemUI * CMenuUI::FindItem(UINT nID)
{
	for (int i = 0; i < m_ChildList.GetSize(); ++i)
	{
		CMenuItemUI* pMenuItem = dynamic_cast<CMenuItemUI*>(m_ChildList[i]);
		ASSERT(pMenuItem != NULL);
		CMenuItemUI* pFindItem = pMenuItem->FindItem(nID);
		if (pFindItem)
			return pFindItem;
	}
	return NULL;
}

BOOL CMenuUI::SetMenuItemAttr(UINT nId, LPCTSTR pstrName, LPCTSTR pstrValue)
{
	for (int i = 0; i < m_ChildList.GetSize(); ++i)
	{
		CMenuItemUI* pMenuItem = dynamic_cast<CMenuItemUI*>(m_ChildList[i]);
		ASSERT(pMenuItem != NULL);
		CMenuItemUI* pFindItem = pMenuItem->FindItem(nId);
		if (pFindItem)
		{
			pFindItem->SetAttribute(pstrName, pstrValue);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CMenuUI::DrawItem(LPDRAWITEMSTRUCT lpdis)
{
	if (lpdis == NULL)
		return FALSE;

	CMenuItemUI * pItem = static_cast<CMenuItemUI*>((void*)lpdis->itemData);
	if (pItem)
	{
		pItem->DrawItem(lpdis->hDC, lpdis->rcItem, lpdis->itemAction, lpdis->itemState);
		return TRUE;
	} else
	{
		//default separator line?
		DrawSeparatorLine(lpdis->hDC, lpdis->rcItem);
		return TRUE;
	}
	return FALSE;
}

BOOL CMenuUI::MeasureItem(LPMEASUREITEMSTRUCT lpmis)
{
	if (lpmis == NULL)
		return FALSE;

	CMenuItemUI* pItem = static_cast<CMenuItemUI*>((void*)lpmis->itemData);
	if (pItem)
	{
		SIZE szAvailable = { 9999, 9999 };
		SIZE szItem = pItem->EstimateSize(szAvailable);
		lpmis->itemWidth = szItem.cx;
		lpmis->itemHeight = szItem.cy;
		return TRUE;
	} else
	{
		//default separator line?
		lpmis->itemWidth = 0;
		lpmis->itemHeight = 3;
		return TRUE;
	}
	return FALSE;
}
