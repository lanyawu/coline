#include "common.h"

#include <UILib/UICombo.h>
#include <windowsx.h>
#include <olectl.h>
#include <UILib/uiscroll.h>
#include <CommonLib/StringUtils.h>
/////////////////////////////////////////////////////////////////////////////////////
//
//

CSingleLinePickUI::CSingleLinePickUI():  
				   m_nLinks(0), 
				   m_uButtonState(0)
{
	::ZeroMemory(&m_rcLinks, sizeof(m_rcLinks));
	::ZeroMemory(&m_rcButton, sizeof(m_rcButton));
}

LPCTSTR CSingleLinePickUI::GetClass() const
{
	return _T("SinglePrettyEditUI");
}

UINT CSingleLinePickUI::GetControlFlags() const
{
	return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
}

void CSingleLinePickUI::Event(TEventUI& event)
{
	if (event.Type == UIEVENT_SETCURSOR)
	{
		for (int i = 0; i < m_nLinks; i++)
		{
			if (::PtInRect(&m_rcLinks[i], event.ptMouse))
			{
				::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
				return;
			} //end if (::PtInRect(&...
		} //end for (int i = 0;..
	} //end if (event.Type == ...
	if (event.Type == UIEVENT_BUTTONDOWN && IsEnabled())
	{
		if (::PtInRect(&m_rcButton, event.ptMouse))
		{
			m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
			Invalidate();
		} else
		{
			// Check for link press
			for (int i = 0; i < m_nLinks; i++) 
			{
				if (::PtInRect(&m_rcLinks[i], event.ptMouse)) 
				{
					m_pManager->SendNotify(this, _T("link"));
					return;
				} //end if (::PtInRect(..
			} //end for (int..     
		} //end else if (::PtInRect...
		return;
	}
	if (event.Type == UIEVENT_MOUSEMOVE)
	{
		if ((m_uButtonState & UISTATE_CAPTURED) != 0)
		{
			if (::PtInRect(&m_rcButton, event.ptMouse)) 
				m_uButtonState |= UISTATE_PUSHED;
			else 
				m_uButtonState &= ~UISTATE_PUSHED;
			Invalidate();
		} //end if ((
	} //end if (event.Type...
	if (event.Type == UIEVENT_BUTTONUP)
	{
		if ((m_uButtonState & UISTATE_CAPTURED) != 0)
		{
			if (::PtInRect(&m_rcButton, event.ptMouse)) 
				m_pManager->SendNotify(this, _T("browse"));
			m_uButtonState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
			Invalidate();
		}
	}
	if (event.Type == UIEVENT_KEYDOWN) 
	{
      if (event.chKey == VK_SPACE && m_nLinks > 0) 
		  m_pManager->SendNotify(this, _T("link"));
      if (event.chKey == VK_F4 && IsEnabled())
		  m_pManager->SendNotify(this, _T("browse"));
	}
	CControlUI::Event(event);
}
 

SIZE CSingleLinePickUI::EstimateSize(SIZE /*szAvailable*/)
{
	SIZE sz = { 0, 12 + m_pManager->GetThemeFontInfo(UIFONT_NORMAL).tmHeight };
	if (m_cxyFixed.cx > 0)
	{
		sz.cx = m_cxyFixed.cx;
		RECT rcText = m_rcItem;
		::InflateRect(&rcText, -4, -2);
		m_nLinks = lengthof(m_rcLinks);
		CBlueRenderEngineUI::DoPaintPrettyText(m_pManager->GetPaintDC(), m_pManager, rcText, 
			                                   m_sText, UICOLOR_EDIT_TEXT_NORMAL, UICOLOR__INVALID, 
											   m_rcLinks, m_nLinks, DT_SINGLELINE | DT_CALCRECT);
		sz.cy = rcText.bottom - rcText.top;
	}
	return sz;
}

void CSingleLinePickUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	int cy = m_rcItem.bottom - m_rcItem.top;
	::SetRect(&m_rcButton, m_rcItem.right - cy, m_rcItem.top, m_rcItem.right, m_rcItem.bottom);
	RECT rcText = { m_rcItem.left, m_rcItem.top, m_rcButton.left - 4, m_rcItem.bottom };
	UITYPE_COLOR iTextColor = UICOLOR_EDIT_TEXT_NORMAL;
	UITYPE_COLOR iBorderColor = UICOLOR_CONTROL_BORDER_NORMAL;
	UITYPE_COLOR iBackColor = UICOLOR_CONTROL_BACKGROUND_NORMAL;
	if (IsFocused()) 
	{
		iTextColor = UICOLOR_EDIT_TEXT_NORMAL;
		iBorderColor = UICOLOR_CONTROL_BORDER_NORMAL;
		iBackColor = UICOLOR_CONTROL_BACKGROUND_HOVER;
	}
	if (!IsEnabled())
	{
		iTextColor = UICOLOR_EDIT_TEXT_DISABLED;
		iBorderColor = UICOLOR_CONTROL_BORDER_DISABLED;
		iBackColor = UICOLOR__INVALID;
	}
	CBlueRenderEngineUI::DoPaintFrame(hDC, m_pManager, rcText, iBorderColor, iBorderColor, iBackColor);
	::InflateRect(&rcText, -4, -2);
	m_nLinks = lengthof(m_rcLinks);
	CBlueRenderEngineUI::DoPaintPrettyText(hDC, m_pManager, rcText, m_sText, iTextColor,
		                                   UICOLOR__INVALID, m_rcLinks, m_nLinks, DT_SINGLELINE);
	RECT rcPadding = { 0 };
	CBlueRenderEngineUI::DoPaintButton(hDC, m_pManager, m_rcButton, NULL/*_T("<i 4>")*/, rcPadding, m_uButtonState, 0, 0);
}


/////////////////////////////////////////////////////////////////////////////////////
//
//
class CDropDownEditWnd : public CWindowWnd
{
public:
	CDropDownEditWnd(CDropDownUI* pOwner): 
	                 m_pOwner( pOwner ), 
		             m_bEditChanged(false) 
	{
		//
	}

	void Init(CDropDownUI* pOwner);

	LPCTSTR GetWindowClassName() const;
	LPCTSTR GetSuperClassName() const;
	void OnFinalMessage(HWND hWnd);

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEditChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);

public:
	CDropDownUI* m_pOwner;
	BOOL m_bEditChanged;
};

void CDropDownEditWnd::Init(CDropDownUI* pOwner)
{
	CRect rcPos = pOwner->GetPos();//edit area pos!
	rcPos.Deflate(1, 1);
	rcPos.top += 2;
	rcPos.right -= 20;
	/*style :) */
	HWND hPaintWnd = pOwner->GetManager()->GetPaintWindow();
	Create(hPaintWnd, NULL, WS_CHILD | ES_AUTOHSCROLL, 0, rcPos);
	SetWindowFont(m_hWnd, pOwner->GetManager()->GetThemeFont(UIFONT_NORMAL), TRUE);
	Edit_SetText(m_hWnd, pOwner->m_sText);
	Edit_SetModify(m_hWnd, FALSE);
	SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(2, 2));
	Edit_SetSel(m_hWnd, 0, -1);
	Edit_Enable(m_hWnd, pOwner->IsEnabled() == true);
	Edit_SetReadOnly(m_hWnd, false);
	::ShowWindow(m_hWnd, SW_SHOWNORMAL);
	::SetFocus(m_hWnd);
}

LPCTSTR CDropDownEditWnd::GetWindowClassName() const
{
	return _T("DropDownEditWnd");
}

LPCTSTR CDropDownEditWnd::GetSuperClassName() const
{
	return WC_EDIT;
}

void CDropDownEditWnd::OnFinalMessage(HWND /*hWnd*/)
{
	// Clear reference and die
	m_pOwner->m_pEditWindow = NULL;
	delete this;
}

LRESULT CDropDownEditWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	BOOL bHandled = TRUE;
	switch(uMsg)
	{
		case WM_KILLFOCUS:
			lRes = OnKillFocus(uMsg, wParam, lParam, bHandled);
			break;
		case OCM_COMMAND:
			if( GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE )
				lRes = OnEditChanged(uMsg, wParam, lParam, bHandled);
			break;
		case WM_KEYDOWN:
			lRes = OnKeyDown(uMsg, wParam, lParam, bHandled);
			break;
		default:
			bHandled = FALSE;
	}
	
	if (!bHandled) 
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	return lRes;
}

LRESULT CDropDownEditWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LRESULT lRes = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	Close();
	if (m_pOwner && m_bEditChanged)
	{
		int cchLen = ::GetWindowTextLength(m_hWnd) + 1;
		LPTSTR pstr = (LPTSTR)_alloca(cchLen * sizeof(TCHAR));
		if (pstr)
		{
			::GetWindowText(m_hWnd, pstr, cchLen);
			m_pOwner->SetText(pstr);
		}
	}
	m_pOwner->GetManager()->SendNotify(m_pOwner, _T("inputcompleted"));
	return lRes;
}

LRESULT CDropDownEditWnd::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = m_pOwner->OnEditKeyDown(wParam, lParam);
	return 0;
}

LRESULT CDropDownEditWnd::OnEditChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_bEditChanged = TRUE;
	if (m_pOwner != NULL && m_pOwner->GetManager())
	{
		m_pOwner->GetManager()->SendNotify(m_pOwner, _T("editchanged"));
	}
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////
//
//下拉窗口
class CDropDownWnd : public CWindowWnd, public INotifyUI
{
public:
	CDropDownWnd(CDropDownUI* pOwner, CListUI* pList, CPaintManagerUI* pManager) 
		: m_pOwner(pOwner),
		  m_pList(pList),
		  m_ppm(pManager)
	{
		m_pList->SetStyle(LUIS_LIST);
		m_pList->SetBorder(true);
		if (m_ppm)
			m_pList->SetBorderColor( m_ppm->GetThemeColor(UICOLOR_CONTROL_BORDER_NORMAL) );
	}
	~CDropDownWnd()
	{
		if (m_ppm)
		{
			delete m_ppm;
		}
	}

	void Notify(TNotifyUI& msg);

	LPCTSTR GetWindowClassName() const;
	void OnFinalMessage(HWND hWnd);

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT OnCreate();
	LRESULT OnKeyDown(WPARAM wParam, LPARAM lParam, BOOL& bHandle);
public:
	CPaintManagerUI* m_ppm;
	CDropDownUI* m_pOwner;

	CListUI* m_pList;
	int m_iOldSel;
};

LPCTSTR CDropDownWnd::GetWindowClassName() const
{
    return _T("DropDownWnd");
}

void CDropDownWnd::OnFinalMessage(HWND hWnd)
{
	m_pOwner->m_pDropDownWnd = NULL;
	m_pList->SetManager( NULL, NULL );
	m_pList = NULL;
    delete this;
}

void CDropDownWnd::Notify(TNotifyUI& msg)
{
	if (msg.sType == _T("itemactivate"))
	{
		ShowWindow( false );
	} else if (msg.sType == _T("itemselect"))
	{
		//当前选择可能为-1,设置text为空,然后发送itemselect
		int iCurSel = m_pList->GetCurSel();
		CStdString sText;
		m_pList->GetItemText(iCurSel, 0, sText);
		m_pOwner->SetText(sText);
		m_pOwner->m_pManager->SendNotify(m_pOwner, _T("itemselect"));
	}
}


LRESULT CDropDownWnd::OnCreate()
{
	if (m_ppm == NULL)
	{
		return -1;
	}

	CWhiteCanvasUI* pWindow = new CWhiteCanvasUI();
	pWindow->SetAutoDestroy(false);
	pWindow->SetBorder(false);

	pWindow->Add(m_pList);
	
	m_ppm->Init(m_hWnd);
	m_ppm->AttachDialog(pWindow);
	m_ppm->AddNotifier(this);

	//twisted
	m_pList->EnableScrollBar(UISB_HORZ, false);
	return 0;
}


LRESULT CDropDownWnd::OnKeyDown(WPARAM wParam, LPARAM lParam, BOOL& bHandle)
{
	switch(wParam)
	{
		case VK_ESCAPE:
			 ShowWindow(false);
			 m_pOwner->SetFocus();
			 bHandle = true;
			 return 0;
		default:
			 bHandle = false;
	}
	return 0;
}


LRESULT CDropDownWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL bHandled = false;
	LRESULT lRes = 0;
	switch(uMsg)
	{
		case WM_CREATE:
			 return OnCreate();
		case WM_KEYDOWN:
			 lRes = OnKeyDown( wParam, lParam, bHandled );
			 break;
		case WM_KILLFOCUS:
			 ShowWindow( false );
			 break;
		default:
			 break;
	}

	if (bHandled)
		return lRes;
	else if (m_ppm && m_ppm->MessageHandler(uMsg, wParam, lParam, lRes))
	{
		return lRes;
	} else
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}


////////////////////////////////////////////////////////


CDropDownUI::CDropDownUI(): 
			m_uButtonState(0),
			m_pEditWindow(NULL),
			m_pDropDownWnd(NULL),
			m_iImgId(IMGID_INVALID_),
			m_bEditable(false),
			m_nBorderImageId(0),
			m_iLeftPadding(0),
			m_iTopPadding(0),
			m_iRightPadding(0),
			m_iBottomPadding(0),
			m_iMaxDropDownItems(5),
			m_pList(new CListUI())
{
	::ZeroMemory(&m_rcButton, sizeof(RECT));
}


CDropDownUI::~CDropDownUI()
{
	if (m_pDropDownWnd)
	{
		m_pDropDownWnd->Close();
	}

	if (m_pList){
		delete m_pList;
	}
}

LPCTSTR CDropDownUI::GetClass() const
{
	return _T("DropDownUI");
}

UINT CDropDownUI::GetControlFlags() const
{
	return UIFLAG_SETCURSOR | UIFLAG_TABSTOP | ( WantReturn() ? UIFLAG_WANTRETURN : 0 );
}

void CDropDownUI::Init()
{
	CContainerUI::Init();
	if (m_pDropDownWnd == NULL)
	{
		m_pDropDownWnd = new CDropDownWnd(this, m_pList, m_pManager->CreateInstance());
		m_pDropDownWnd->Create(m_pManager->GetPaintWindow(), NULL, WS_POPUP, WS_EX_TOOLWINDOW);
	}
}

int CDropDownUI::GetCurSel() const
{
	return m_pList->GetCurSel();
}

int CDropDownUI::GetItemCount() const
{
	return m_pList->GetCount();
}

bool CDropDownUI::SelectItem(const int iIndex)
{
	return m_pList->SelectItem( iIndex );
}

bool CDropDownUI::Add(CControlUI* pControl, const int nIdx)
{
	ASSERT( !_T("IContainerUI interfaces is forbidden!") );
	return false;
}

bool CDropDownUI::Remove(CControlUI* pControl)
{
	ASSERT( !_T("IContainerUI interfaces is forbidden!") );
	return false;
}


void CDropDownUI::RemoveAll()
{
	RemoveAllString();
}

int CDropDownUI::GetCount() const
{
	return GetItemCount();
}

int CDropDownUI::AddString(const CStdString& sText)
{
	ASSERT( m_pDropDownWnd != NULL );
	int iCount = m_pList->GetCount();
	int iIndex = m_pList->InsertItem(iCount, sText);
	return iIndex;
}

int CDropDownUI::InsertString(int idx, const TCHAR *szText)
{
	if (m_pDropDownWnd)
	{
		return m_pList->InsertItem(idx, szText);
	}
	return -1;
}

BOOL CDropDownUI::RemoveString(const CStdString& sText)
{
	ASSERT(m_pDropDownWnd != NULL);
	int iItem = FindString(sText);
	if (iItem != -1)
	{
		return DeleteItem(iItem);
	}
	return false;
}


BOOL CDropDownUI::RemoveAllString()
{
	ASSERT(m_pDropDownWnd != NULL);
	return m_pList->DeleteAllItems();
}


BOOL CDropDownUI::SetString(const int iIndex, const CStdString& sText)
{
	if ((iIndex >= 0) && (iIndex < GetItemCount()))
	{
		m_pList->SetItemText(iIndex, 0, sText);
		if (iIndex == GetCurSel())
		{
			SetText( sText );
		}
		return true;
	}
	return false;
}

BOOL CDropDownUI::GetString(int iIndex, CStdString &sText)
{
	ASSERT(m_pDropDownWnd != NULL);
	return m_pList->GetItemText(iIndex, 0, sText);
}


BOOL CDropDownUI::SetItemData(const int iIndex, void *pData)
{
	return m_pList->SetItemData(iIndex, pData);
}

int CDropDownUI::FindString(const CStdString& sText)
{
	CStdString sItemText;
	for (int iItem = 0; iItem < GetItemCount(); ++iItem)
	{
		if (GetString(iItem, sItemText) && sItemText == sText)
		{
			return iItem;
		}
	}
	return -1;
}

void *CDropDownUI::GetItemData(int iIndex) const
{
	return m_pList->GetItemData(iIndex);
}

void CDropDownUI::SetText(LPCTSTR pstrText)
{
	if (m_sText != pstrText)
	{
		m_sText = pstrText;
		if (m_pEditWindow)
		{
			Edit_SetText(*m_pEditWindow, m_sText);
			int iLength = Edit_GetTextLength(*m_pEditWindow);
			Edit_SetSel(*m_pEditWindow, iLength, iLength);
		} else
		{
			Invalidate();
		} //end if (m_pEditWindow)
	} //end if (m_sText != pstrText)
}

void CDropDownUI::OnEventSetCursor(TEventUI& e)
{
	//鼠标在编辑区域的时候改变光标
	if (!IsEnabled() || !::PtInRect(&m_rcItem, e.ptMouse))
	{
		return;
	}
	//now this UI is certainly enabled and the cursor point is in the item rect
	if (m_bEditable && !::PtInRect(&m_rcButton, e.ptMouse))
	{
		::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_IBEAM)));//edit
    } else
	{
		::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
	}
}

void CDropDownUI::OnEventButtonDown(TEventUI& e)
{
	if (!IsEnabled() || !::PtInRect(&m_rcItem, e.ptMouse))
	{
		return;
	}

	if (m_bEditable)
	{
		//no matter where the mouse click, the edit window will be activated
		if (::PtInRect( &m_rcButton, e.ptMouse))
		{
			Activate();//drop down window
			m_uButtonState |= (UISTATE_PUSHED | UISTATE_CAPTURED);
		}
		ActivateEditWnd();
	} else
	{
		//if this UI is not editable, the whole UI will behavior like the drop down button
		Activate();
		m_uButtonState |= (UISTATE_PUSHED | UISTATE_CAPTURED);
	} //else if (m_bEditable)
}

void CDropDownUI::OnEventMouseMove(TEventUI& e)
{
	if (!IsEnabled() || !::PtInRect(&m_rcItem, e.ptMouse))
	{
		return;
	}

	if (m_bEditable)
	{
		if (::PtInRect(&m_rcButton, e.ptMouse))
			m_uButtonState |= UISTATE_HOT;
		else
			m_uButtonState &= ~UISTATE_HOT;
		Invalidate();
	} //
}

void CDropDownUI::OnEventButtonUp(TEventUI& e)
{
	if ((m_uButtonState & UISTATE_CAPTURED ) != 0)
	{
		m_uButtonState &= ~( UISTATE_PUSHED | UISTATE_CAPTURED );
		Invalidate();
	}
}


void CDropDownUI::OnEventKeydown(TEventUI& e)
{
	switch(e.chKey) 
	{
		case VK_F4:
			Activate();
			return;
		case VK_UP:
			SelectItem(FindSelectable(GetCurSel() - 1, false));
			return;
		case VK_DOWN:
			SelectItem(FindSelectable(GetCurSel() + 1, true ));
			return;
		case VK_PRIOR:
			SelectItem(FindSelectable(GetCurSel() - 10, false));
			return;
		case VK_NEXT:
			SelectItem(FindSelectable(GetCurSel() + 10, true));
			return;
		case VK_HOME:
			SelectItem(0 );
			return;
		case VK_END:
			SelectItem(GetCount() - 1);
			return;
		case VK_ESCAPE:
		case VK_RETURN:
			ShowDropDownWnd(false);
			return;
	}
}

void CDropDownUI::OnEventScrollWheel(TEventUI& e)
{
	bool bDownward = (LOWORD(e.wParam) == SB_LINEDOWN);
	int iNewSel = GetCurSel() + (bDownward ? 1 : -1);
	SelectItem(FindSelectable(iNewSel, true));
}

void CDropDownUI::OnEventMouseEnter(TEventUI& e)
{
	if (!IsEnabled() || !::PtInRect( &m_rcItem, e.ptMouse))
	{
		return;
	}

	if (( m_bEditable && ::PtInRect(&m_rcButton, e.ptMouse)) 
		|| !m_bEditable)
	{
		m_uButtonState |= UISTATE_HOT;
		Invalidate();
	}
}

void CDropDownUI::OnEventMouseLeave(TEventUI &e)
{
	m_uButtonState &= ~UISTATE_HOT;
	Invalidate();
}

void CDropDownUI::OnEventWindowSize(TEventUI &e)
{
	if (m_pEditWindow != NULL)
	{
		m_pManager->SetFocus(NULL);
	}
	if (m_pDropDownWnd != NULL)
	{
		ShowDropDownWnd(false);
	}
}

void CDropDownUI::OnEventDblClick(TEventUI &e)
{
	if (!IsEnabled())
		return;

	Activate();
}


void CDropDownUI::OnEventSetFocus(TEventUI &e)
{
	if (!IsEnabled())
	{
		return;
	}

	if (m_bEditable)
	{
		ActivateEditWnd();
	}
}

void CDropDownUI::OnEventKillFocus(TEventUI& e)
{
	if (m_pEditWindow)
	{
		m_pEditWindow->Close();
	}
	if (m_pDropDownWnd && ::GetFocus() != *m_pDropDownWnd)
	{
		ShowDropDownWnd(false);
	}
}


void CDropDownUI::OnEventWindowPosChanging(TEventUI& e)
{
	ASSERT(m_pDropDownWnd != NULL);
	if (m_pDropDownWnd == NULL)
	{
		return;
	}

	WINDOWPOS* ppos = (WINDOWPOS *)(e.lParam);
	if ((ppos->flags & SWP_NOACTIVATE) != 0)
	{
		//parent window is deactivated
		if (::GetActiveWindow() != *m_pDropDownWnd)
			ShowDropDownWnd(false);
	}
	if ((ppos->flags & SWP_NOMOVE) == 0)
	{
		//the parent window rectangle is changing
		ShowDropDownWnd(false);
	} //end if ((ppos->flags && SWP_NOMOVE)..
}


void CDropDownUI::Event(TEventUI &e)
{
	switch(e.Type)
	{
		case UIEVENT_SETCURSOR:
			return OnEventSetCursor(e);
		case UIEVENT_BUTTONDOWN:
			OnEventButtonDown(e);
			break;
		case UIEVENT_MOUSEMOVE:
			OnEventMouseMove(e);
			break;
		case UIEVENT_BUTTONUP:
			OnEventButtonUp(e);
			break;
		case UIEVENT_DBLCLICK:
			OnEventDblClick(e);
			break;
		case UIEVENT_KEYDOWN:
			return OnEventKeydown(e);
		case UIEVENT_SCROLLWHEEL:
			return OnEventScrollWheel(e);
		case UIEVENT_MOUSEENTER:
			OnEventMouseEnter(e);
			break;
		case UIEVENT_MOUSELEAVE:
			OnEventMouseLeave(e);
			break;
		case UIEVENT_WINDOWSIZE:
			OnEventWindowSize(e);
			break;
		case UIEVENT_SETFOCUS:
			OnEventSetFocus(e);
			break;
		case UIEVENT_KILLFOCUS:
			OnEventKillFocus(e);
			break;
		case UIEVENT_WINDOWPOSCHANGING:
			OnEventWindowPosChanging(e);
			break;
		default:
			break;
	}
	CControlUI::Event(e);
}

//键盘按下
BOOL CDropDownUI::OnEditKeyDown(WPARAM wParam, LPARAM lParam)
{		
	if (m_pDropDownWnd)
	{
		switch(wParam)
		{
			case VK_UP:
				SelectItem(FindSelectable(GetCurSel() - 1, false));
				return true;
			case VK_DOWN:
				SelectItem(FindSelectable(GetCurSel() + 1, true));
				return true;
			case VK_ESCAPE:
			case VK_RETURN:
				ShowDropDownWnd(false);
				return true;
		} //end switch(wParam)
	} //end if (m_pDropDownWnd)
	return false;
}


bool CDropDownUI::Activate()
{
	if (!CControlUI::Activate())
	{
		return false;
	}

	if (!::IsWindowVisible(*m_pDropDownWnd))
	{
		ShowDropDownWnd(true);
	} else
	{
		ShowDropDownWnd(false);
	}

	return true;
}


void CDropDownUI::ShowDropDownWnd(BOOL bShow)
{
	ASSERT( m_pDropDownWnd != NULL );
	if (bShow)
	{
		if (!::IsWindowVisible(*m_pDropDownWnd))
		{
			SIZE sz = GetDropDownSize();
			POINT pt = { m_rcItem.left, m_rcItem.bottom };
			pt.x += GetLeftPadding();
			pt.y += GetTopPadding(); 
			::ClientToScreen(m_pManager->GetPaintWindow(), &pt);
			::MoveWindow(*m_pDropDownWnd, pt.x, pt.y, m_rcItem.right - m_rcItem.left - GetLeftPadding() - GetRightPadding(), 
				         sz.cy, true);
			m_pDropDownWnd->ShowWindow(true, false);
			if (m_pManager != NULL)
			{
				m_pManager->SendNotify(this, _T("dropdown"));
			} //end if (m_pManager != NULL)
		} //end if (!::IsWindowVisible(..
	} else
	{
		if (::IsWindowVisible(*m_pDropDownWnd))
		{
			m_pDropDownWnd->ShowWindow(false);
		} //end if (::IsWindowVisible(..
	} //end else if (bShow)
}


CStdString CDropDownUI::GetText() const
{
	if (m_bEditable && m_pEditWindow != NULL)
	{
		int cchLen = ::GetWindowTextLength(*m_pEditWindow) + 1;
		LPTSTR pstr = (LPTSTR)_alloca( cchLen * sizeof(TCHAR));
		if (pstr)
		{
			::GetWindowText(*m_pEditWindow, pstr, cchLen);
			return pstr;
		}
	}
	return m_sText;
}

SIZE CDropDownUI::GetDropDownSize() const
{
	int iCount = min(GetItemCount(), m_iMaxDropDownItems);
	int cy = iCount * UILIST_ITEM_HEIGHT;
	if (iCount > 0)
		cy += 2;
	return CSize(0, cy);
}

void CDropDownUI::SetPos(RECT rc)
{
	// Position this control
	CControlUI::SetPos(rc);
	//position the drop down button
	int cy = m_rcItem.bottom - m_rcItem.top;
	::SetRect(&m_rcButton, m_rcItem.right - cy, m_rcItem.top, m_rcItem.right, m_rcItem.bottom);
	::InflateRect(&m_rcButton, -2, -2);
	//hide the drop down wnd
	ShowDropDownWnd(false);
}

int CDropDownUI::GetLeftPadding()
{
	return m_iLeftPadding;
}

int CDropDownUI::GetTopPadding()
{
	return m_iTopPadding;
}

int CDropDownUI::GetRightPadding()
{
	return m_iRightPadding;
}

int CDropDownUI::GetBottomPadding()
{
	return m_iBottomPadding;
}

void CDropDownUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcsicmp(pstrName, _T("image")) == 0)
		m_iImgId = _ttoi( pstrValue );
	else if (_tcsicmp( pstrName, _T("editable")) == 0)
		m_bEditable = (_tcsicmp(_T("true"), pstrValue) == 0);
	else if (_tcsicmp(pstrName, _T("borderimageid")) == 0)
		m_nBorderImageId = _ttoi(pstrValue);
	else if (_tcsicmp(pstrName, _T("LeftPadding")) == 0)
	{
		m_iLeftPadding = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("RightPadding")) == 0)
	{
		m_iRightPadding = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("TopPadding")) == 0)
	{
		m_iTopPadding = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("BottomPadding")) == 0)
	{
		m_iBottomPadding = _ttoi(pstrValue);
	} else
		CContainerUI::SetAttribute(pstrName, pstrValue);
}

SIZE CDropDownUI::EstimateSize(SIZE /*szAvailable*/)
{
	return m_cxyFixed;
}

void CDropDownUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
    // Paint the nice frame
	//控件的状态 disable/focus/normal...
	UINT uState = 0;
	if (IsFocused())
	{
		uState |= UISTATE_FOCUSED;
	}
	if (!IsEnabled())
	{
		uState |= UISTATE_DISABLED;
	}
	uState |= UIFRAME_ROUND;
	if (m_nBorderImageId > 0)
		CBlueRenderEngineUI::DoPaintGraphic(hDC, m_pManager, m_rcItem, m_nBorderImageId);
	else
		CBlueRenderEngineUI::DoPaintEditBox(hDC, m_pManager, m_rcItem, uState);
 
	// Paint dropdown edit box
	RECT rcText = { m_rcItem.left, m_rcItem.top, m_rcButton.left + 1, m_rcItem.bottom };
	COLORREF clrText = m_pManager->GetThemeColor(UICOLOR_EDIT_TEXT_NORMAL);
	if (!IsEnabled())
	{
		clrText = m_pManager->GetThemeColor(UICOLOR_EDIT_TEXT_READONLY);
	}
	CBlueRenderEngineUI::DoPaintEditText(hDC, m_pManager, rcText, m_sText, 
		clrText, uState, m_bEditable ? DT_EDITCONTROL : DT_END_ELLIPSIS );
	// Paint dropdown button
	CBlueRenderEngineUI::DoPaintImageButton(hDC, m_pManager, m_rcButton,
		uState | m_uButtonState, m_iImgId);
}


void CDropDownUI::SetMaxDropDownItems(int iItem)
{
	 m_iMaxDropDownItems = iItem;
}

BOOL CDropDownUI::DeleteItem(const int iIndex)
{
	if (iIndex == -1)
	{
		return FALSE;
	}

	//editable, uneditable
	int iCurSel = GetCurSel();
	if (m_pList->DeleteItem(iIndex))
	{
		m_pList->SelectItem(-1);
		if (iCurSel == -1)
		{
#if defined (_DEBUG)
			if (m_bEditable)
			{
				//
			} else
			{
				ASSERT(m_sText == _T(""));
			}
#endif		
		} else if (iCurSel >= GetItemCount())
		{ 
			//select the last one
			SelectItem(GetItemCount() - 1);
		} else
		{
			//select the next one
			SelectItem(iCurSel);
		}
		return true;
	}
	return false;
}


BOOL CDropDownUI::WantReturn() const
{
	if (::IsWindowVisible(*m_pDropDownWnd))
		return true;
	return false;
}

void CDropDownUI::ActivateEditWnd()
{
	if (m_pEditWindow == NULL)
	{
		m_pEditWindow = new CDropDownEditWnd(this);
		m_pEditWindow->Init(this);
	}
}


int CDropDownUI::FindSelectable(int iIndex, bool bForward) const
{
	if (iIndex >= GetItemCount())
		iIndex = GetItemCount() - 1;
	if (iIndex < 0)
		iIndex = 0;
	return iIndex;
}