#include "common.h"

#include <windowsx.h>
#include <UILib/UIEdit.h>
#include <olectl.h>
#include <CommonLib/StringUtils.h>
#include <list>
/////////////////////////////////////////////////////////////////////////////////////
//
//

class CSingleLineEditWnd : public CWindowWnd
{
public:
	CSingleLineEditWnd();
	~CSingleLineEditWnd();

	void Init(CSingleLineEditUI* pOwner);

	LPCTSTR GetWindowClassName() const;
	LPCTSTR GetSuperClassName() const;
	void OnFinalMessage(HWND hWnd);

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEditChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
protected:
	CSingleLineEditUI* m_pOwner;
	BOOL m_bEditChanged;
	HBRUSH m_hBrush;
};


CSingleLineEditWnd::CSingleLineEditWnd() 
	: m_pOwner(NULL),
	  m_bEditChanged( FALSE )
{
	m_hBrush = ::CreateSolidBrush( RGB(255, 255, 255) );
}

CSingleLineEditWnd::~CSingleLineEditWnd()
{
	::DeleteObject( m_hBrush );
}

void CSingleLineEditWnd::Init(CSingleLineEditUI* pOwner)
{
	CRect rcPos = pOwner->GetPos();
	rcPos.left += pOwner->GetLeftPadding();
	rcPos.top += pOwner->GetTopPadding();
	rcPos.right -= pOwner->GetRightPadding();
	rcPos.bottom -= pOwner->GetBottomPadding(); 
	Create(pOwner->GetManager()->GetPaintWindow(), NULL, WS_CHILD | pOwner->m_uEditStyle, 0, rcPos);
	SetWindowFont(m_hWnd, pOwner->GetManager()->GetThemeFont(UIFONT_NORMAL), TRUE);
	Edit_SetText(m_hWnd, pOwner->m_sText );
	Edit_SetModify(m_hWnd, FALSE);
	SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(2, 2));
	Edit_SetSel(m_hWnd, 0, -1);
	Edit_Enable(m_hWnd, pOwner->IsEnabled() == true);
	Edit_SetReadOnly(m_hWnd, pOwner->IsReadOnly() == true);
	if( pOwner->TextLimit() != -1 ){
		Edit_LimitText( m_hWnd, pOwner->TextLimit() );
	}

	::ShowWindow(m_hWnd, SW_SHOWNORMAL);
	::SetFocus(m_hWnd);	

	m_pOwner = pOwner;
}

LPCTSTR CSingleLineEditWnd::GetWindowClassName() const
{
   return _T("SingleLineEditWnd");
}

LPCTSTR CSingleLineEditWnd::GetSuperClassName() const
{
   return WC_EDIT;
}

void CSingleLineEditWnd::OnFinalMessage(HWND /*hWnd*/)
{
   // Clear reference and die
	if( m_pOwner )
		m_pOwner->m_pWindow = NULL;
	delete this;
}

LRESULT CSingleLineEditWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	BOOL bHandled = TRUE;
	switch( uMsg ){
	case WM_KILLFOCUS:
		lRes = OnKillFocus(uMsg, wParam, lParam, bHandled);
		break;
	case WM_KEYDOWN:
		lRes = OnKeyDown(uMsg, wParam, lParam, bHandled);
		break;
	case WM_CHAR:
		lRes = OnChar(uMsg, wParam, lParam, bHandled);
		break;
	case OCM_COMMAND:
		if( GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE ){
			lRes = OnEditChanged(uMsg, wParam, lParam, bHandled);
		}
		break;
	case WM_SYSKEYDOWN:
		lRes = OnKeyDown(uMsg, wParam, lParam, bHandled);
		break;
	case WM_SETCURSOR:
		lRes = OnSetCursor(uMsg, wParam, lParam, bHandled);
		break;
	case WM_CTLCOLORSTATIC + 0xbc00:
		::SetTextColor( (HDC)wParam, m_pOwner ? m_pOwner->m_clrTextReadOnly : 0 );
		lRes = (LRESULT)m_hBrush;
		bHandled = true;
		break;
	default:
		bHandled = false;
		break;
	}

	if( !bHandled ) 
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	return lRes;
}


LRESULT CSingleLineEditWnd::OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_pOwner && m_pOwner->IsLinkEnabled())
	{
		::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
		bHandled = true;
		return true;
	}
	bHandled = false;
	return 0;
}

LRESULT CSingleLineEditWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LRESULT lRes = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
    PostMessage(WM_CLOSE);
	if (m_pOwner)
	{
		if (m_bEditChanged)
		{
			int cchLen = ::GetWindowTextLength(m_hWnd) + 1;
			LPTSTR pstr = (LPTSTR)_alloca( cchLen * sizeof(TCHAR));
			if (pstr)
			{
				::GetWindowText(m_hWnd, pstr, cchLen);
				m_pOwner->SetText(pstr);
			} //end if (pstr)
		} //end if (m_bEditChanged)
		m_pOwner->GetManager()->SendNotify(m_pOwner, _T("killfocus"));
	} //end if (m_pOwner)
    return lRes;
}

LRESULT CSingleLineEditWnd::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_pOwner)
		bHandled = m_pOwner->OnKeyDown(wParam, lParam);
	return 0;
}

LRESULT CSingleLineEditWnd::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	if( m_pOwner )
		bHandled = m_pOwner->OnChar(wParam, lParam);
	return 0;
}

LRESULT CSingleLineEditWnd::OnEditChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
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
//
#define EDIT_BORDER_WIDTH 5  //edit 边角象素宽

CSingleLineEditUI::CSingleLineEditUI():
					m_pWindow(NULL), 
					m_uEditStyle(ES_AUTOHSCROLL), 
					m_iTextLimit(-1),
					m_iBytesLimit(-1),
					m_bHandleChar(TRUE),
					m_bReadOnly(false),
					m_bPretty(FALSE),
					m_bEnableLink(false),
					m_clrText(0),
					m_bHot(FALSE),
					m_iLeftPadding(1),
					m_iTopPadding(3),
					m_iRightPadding(1),
					m_iBottomPadding(3),
					m_nBorderImageId(0),
					m_clrTextReadOnly(RGB(167,166,170))
{  
	m_BorderFixed.m_iBotHeight = EDIT_BORDER_WIDTH;
	m_BorderFixed.m_iBotLeftWidth = EDIT_BORDER_WIDTH;
	m_BorderFixed.m_iBotRightWidth = EDIT_BORDER_WIDTH;
	m_BorderFixed.m_iCenterLeftWidth = EDIT_BORDER_WIDTH;
	m_BorderFixed.m_iCenterHeight = EDIT_BORDER_WIDTH;
	m_BorderFixed.m_iCenterRightWidth = EDIT_BORDER_WIDTH;
	m_BorderFixed.m_iTopHeight = EDIT_BORDER_WIDTH;
	m_BorderFixed.m_iTopLeftWidth = EDIT_BORDER_WIDTH;
	m_BorderFixed.m_iTopRightWidth = EDIT_BORDER_WIDTH; 
}

LPCTSTR CSingleLineEditUI::GetClass() const
{
	return _T("SingleLineEditUI");
}

UINT CSingleLineEditUI::GetControlFlags() const
{
	return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
}

void CSingleLineEditUI::Event( TEventUI& e )
{
	static BOOL bCapture = false;
	switch(e.Type)
	{
	case UIEVENT_SETCURSOR:
		if (IsEnabled())
		{
			LPCTSTR lpszCursor = IsLinkEnabled() ? IDC_HAND : IDC_IBEAM;
			::SetCursor(::LoadCursor(NULL, lpszCursor));
			return;
		}
		break;
	case UIEVENT_WINDOWSIZE:
		if (m_pWindow != NULL)
		{
			m_pManager->SetFocus(NULL);
		}
		break;
	case UIEVENT_SETFOCUS:
		if (IsEnabled())
		{
			if (m_pWindow == NULL)
			{
				m_pWindow = new CSingleLineEditWnd();
				ASSERT(m_pWindow);
				m_pWindow->Init(this);
			} else
			{
				HWND hEditWnd = m_pWindow->GetHWND();
				int iTextLen = Edit_GetTextLength(hEditWnd);
				Edit_SetSel(hEditWnd, iTextLen, iTextLen);
				m_pWindow->ShowWindow();
			}
		}
		break;
	case UIEVENT_BUTTONDOWN:
		if (IsEnabled() && ::PtInRect(&m_rcItem, e.ptMouse))
		{
			bCapture = true;
		}
		if (IsFocused() && m_pWindow == NULL)
		{
			m_pWindow = new CSingleLineEditWnd();
			ASSERT(m_pWindow);
			m_pWindow->Init(this);
			return;
		}
		break;
	case UIEVENT_BUTTONUP:
		if (bCapture && ::PtInRect(&m_rcItem, e.ptMouse))
		{
			m_pManager->SendNotify(this, _T("link"));
		}
		bCapture = false;
		break;
	case UIEVENT_KEYDOWN:
		OnKeyDown(e.wParam, e.lParam);
		break;
	case UIEVENT_KILLFOCUS:
		if (m_pWindow)
		{
			::PostMessage(*m_pWindow, WM_CLOSE, 0, 0);
		}
		break;
	case UIEVENT_MOUSEENTER:
		OnEventMouseEnter();
		break;
	case UIEVENT_MOUSELEAVE:
		OnEventMouseLeave();
		break;
	}
	CControlUI::Event(e);
}

void CSingleLineEditUI::OnEventMouseEnter()
{
	m_bHot = TRUE;
}

void CSingleLineEditUI::OnEventMouseLeave()
{
	m_bHot = FALSE;
}


BOOL CSingleLineEditUI::OnChar(WPARAM wParam, LPARAM lParam)
{
	//是否要自动屏蔽char
	BOOL bHandled = !m_bHandleChar;

	//如果不自动屏蔽char，则检测长度限制
	if (!bHandled && m_iBytesLimit != -1)
	{
		if (wParam != 8 && wParam != 9 && wParam != 10 && wParam != 13)
		{
			int cchLen = ::GetWindowTextLength( *m_pWindow ) + 1;
			int iBytesLen = cchLen * sizeof(wchar_t);
			char* pstr = (char*)_alloca(iBytesLen);
			if (pstr)
			{
				::GetWindowTextA(*m_pWindow, pstr, iBytesLen);
				iBytesLen = ::strlen(pstr);
				if (iBytesLen >= m_iBytesLimit)
				{
					bHandled = true;
				} else if (wParam > 255 && iBytesLen + 2 > m_iBytesLimit)
				{
					bHandled = true;
				} //end else if (wParam > 255...
			} //end if (pstr)
		}	//end if (wParam != 8 ...	
	} //end if (!bHandled && ...
	
	return bHandled;
}


//键盘按下
BOOL CSingleLineEditUI::OnKeyDown(WPARAM wParam, LPARAM lParam)
{ 
	if (m_pManager != NULL)
	{
		TNotifyUI msg;
		msg.pSender = this;
		msg.sType = _T("keydown");
		msg.wParam = wParam;
		msg.lParam = lParam;
		m_pManager->SendNotify(msg);
		return msg.bHandled;
	}
	return FALSE;
}

CStdString CSingleLineEditUI::GetText() const
{
	if (m_pWindow)
	{
		int cchLen = ::GetWindowTextLength(*m_pWindow) + 1;
		//_alloca alloc on stack,not free
		LPTSTR pstr = (LPTSTR)_alloca(cchLen * sizeof(TCHAR));
		if (pstr)
		{
			::GetWindowText(*m_pWindow, pstr, cchLen);
			return pstr; 
		}
	}
	return m_sText;
}

void CSingleLineEditUI::SetText(LPCTSTR pstrText)
{
	if (m_sText != pstrText)
	{
		m_sText = pstrText;
		if (m_pWindow)
		{
			Edit_SetText(*m_pWindow, m_sText);
			int iLength = Edit_GetTextLength(*m_pWindow);
			Edit_SetSel(*m_pWindow, iLength, iLength);
		}
		Invalidate();
	}
}

void CSingleLineEditUI::SetEnabled(bool bEnabled)
{
	if (m_pWindow)
	{
		Edit_Enable(*m_pWindow, bEnabled);
	}
	CControlUI::SetEnabled(bEnabled);
}

void CSingleLineEditUI::SetReadOnly(bool bReadOnly)
{
	m_bReadOnly = bReadOnly;
	if (m_bReadOnly)
	{
		m_uEditStyle |= ES_READONLY;
	} else
	{
		m_uEditStyle &= ~ES_READONLY;
	}
	Invalidate();
}

bool CSingleLineEditUI::IsReadOnly() const
{
	return m_bReadOnly;
}

void CSingleLineEditUI::EnableLink(BOOL bEnable)
{
	m_bEnableLink = bEnable;
}


BOOL CSingleLineEditUI::IsLinkEnabled() const
{
	return m_bEnableLink;
}


void CSingleLineEditUI::SetTextLimit(int iLimit)
{
	m_iTextLimit = iLimit;
}

int CSingleLineEditUI::TextLimit() const
{
	return m_iTextLimit;
}

void CSingleLineEditUI::SetBytesLimit(int iLimit)
{
	m_iBytesLimit = iLimit;
}


int CSingleLineEditUI::GetBytesLimit() const
{
	return m_iBytesLimit;
}


void CSingleLineEditUI::SetEditStyle(UINT uStyle)
{
	m_uEditStyle = uStyle;
	Invalidate();
}

SIZE CSingleLineEditUI::EstimateSize(SIZE szAvailable)
{
	if (m_cxyFixed.cx != 0 || m_cxyFixed.cy != 0)
		return m_cxyFixed;
	else
	{
		if (m_bPretty)
		{
			RECT rcText = { 0, 0, szAvailable.cx, szAvailable.cy };
			UITYPE_FONT uiFont = UIFONT_NORMAL;
			CBlueRenderEngineUI::DoPaintQuickText( m_pManager->GetPaintDC(), m_pManager, 
			                rcText, m_sText, RGB(0,0,0), uiFont, DT_SINGLELINE | DT_CALCRECT );
			return CSize(rcText.right, rcText.bottom);
		} else
			return CSize(0, m_pManager->GetThemeFontInfo(UIFONT_NORMAL).tmHeight * 10 / 7);
	}
}

int CSingleLineEditUI::GetLeftPadding()
{
	return m_iLeftPadding;
}
int CSingleLineEditUI::GetRightPadding()
{
	return m_iRightPadding;
}

int CSingleLineEditUI::GetTopPadding()
{
	return m_iTopPadding;
}

int CSingleLineEditUI::GetBottomPadding()
{
	return m_iBottomPadding;
}

void CSingleLineEditUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcsicmp( pstrName, _T("password")) == 0 )
		SetPassword(_tcsicmp(pstrValue, _T("true")) == 0);
	else if (_tcsicmp(pstrName, _T("readonly")) == 0)
		SetReadOnly(_tcsicmp(pstrValue, _T("true")) == 0);
	else if (_tcsicmp(pstrName, _T("readonlytextclr")) == 0)
		SetReadOnlyTextColor(StringToColor( pstrValue));
	else if (_tcsicmp(pstrName, _T("textColor")) == 0)
		SetTextColor(StringToColor( pstrValue));
	else if (_tcsicmp(pstrName, _T("number")) == 0 )
		SetNumber(_tcsicmp(pstrValue, _T("true")) == 0);
	else if (_tcsicmp(pstrName, _T("borderimageid")) == 0)
	{
		m_nBorderImageId = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, _T("textlimit")) == 0)
	{
		SetTextLimit(_ttoi(pstrValue));
	} else if (_tcsicmp(pstrName, _T("byteslimit")) == 0)
	{
		SetBytesLimit(_ttoi(pstrValue));
	} else if (_tcsicmp(pstrName, _T("tip")) == 0)
	{
		m_strTip = pstrValue;
	} else if (_tcsicmp(pstrName, _T("pretty")) == 0)
	{
		m_bPretty = (_tcsicmp(pstrValue, _T("true")) == 0);
	} else if (_tcsicmp(pstrName, _T("handledchar")) == 0)
		m_bHandleChar = (_tcsicmp(pstrValue, _T("true")) == 0);
	else if (_tcsicmp(pstrName, _T("enablelink") ) == 0)
		EnableLink(_tcsicmp(pstrValue, _T("true")) == 0);
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
		CControlUI::SetAttribute(pstrName, pstrValue);
}

void CSingleLineEditUI::SetTextColor(COLORREF clr)
{
	m_clrText = clr;
	Invalidate();
}

void CSingleLineEditUI::SetReadOnlyTextColor(COLORREF clr)
{
	m_clrTextReadOnly = clr;
	Invalidate();
}


void CSingleLineEditUI::SetPassword(bool bTrue)
{
	if (bTrue)
		m_uEditStyle |= ES_PASSWORD;
	else
		m_uEditStyle &= ~ES_PASSWORD;
}

void CSingleLineEditUI::SetNumber(bool bNumberOnly)
{
	if (bNumberOnly)
	{
		m_uEditStyle |= ES_NUMBER;
	} else
	{
		m_uEditStyle &= ~ES_NUMBER;
	}
}

 

CStdString CSingleLineEditUI::GetDisplayText()
{
	if ((m_uEditStyle & ES_PASSWORD) != 0)
	{
		//password
		CStdString sTmp = m_sText;
		for (int i = 0; i < m_sText.GetLength(); ++i)
		{
			sTmp.SetAt(i, '*');
		}
		return sTmp;
	} else 
	{
		//normal text
		return m_sText;
	}
}

COLORREF CSingleLineEditUI::GetDisplayTextColor()
{
	COLORREF clrText = m_clrText;
	if (m_sText.IsEmpty())
	{ 
		clrText = m_pManager->GetThemeColor(UICOLOR_TASK_TEXT);		
	}
	if (IsReadOnly())
	{
		clrText = m_clrTextReadOnly;
	}
	if (!IsEnabled())
	{
		clrText = m_clrTextReadOnly;
	}
	return clrText;
}

void CSingleLineEditUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	RECT rcTemp = {0};
	if (!::IntersectRect(&rcTemp, &rcPaint, &m_rcItem)) 
		return;
	UINT uState = 0;
	if (IsFocused()) 
		uState |= UISTATE_FOCUSED;
	if (IsReadOnly())
		uState |= UISTATE_READONLY;
	if (!IsEnabled())
		uState |= UISTATE_DISABLED;
	if (m_bHot)
		uState |= UISTATE_HOT;
	uState |= UIFRAME_ROUND;
	
	//edit border
	if (HasBorder())
	{
		if (m_nBorderImageId > 0)
			CBlueRenderEngineUI::DoPaintGraphic(hDC, m_pManager, m_rcItem, m_nBorderImageId, m_BorderFixed);
		else
			CBlueRenderEngineUI::DoPaintEditBox(hDC, m_pManager, m_rcItem, uState);
	}

	//text
	CStdString strText = GetDisplayText();
	if (strText.IsEmpty() && IsEnabled())
	{
		strText = m_strTip;
	    CBlueRenderEngineUI::DoPaintEditText(hDC, m_pManager, m_rcItem, 
		       strText, GetDisplayTextColor(), uState, m_uEditStyle);
	} else if (m_bPretty)
	{
		int nLinks = 0;
		CBlueRenderEngineUI::DoPaintPrettyText(hDC, m_pManager, m_rcItem, strText, UICOLOR_EDIT_TEXT_NORMAL,
			UICOLOR__INVALID, NULL, nLinks, m_uEditStyle);
	} else
		CBlueRenderEngineUI::DoPaintEditText(hDC, m_pManager, m_rcItem, 
		       strText, GetDisplayTextColor(), uState, m_uEditStyle);
}


/////////////////////////////////////////////////////////////////////////////////////
//
CTipEditUI::CTipEditUI()
{
}

LPCTSTR CTipEditUI::GetClass() const
{
	return _T("TipEditUI");
}

CStdString CTipEditUI::GetDisplayText()
{
	if (IsFocused() || !m_sText.IsEmpty())
	{
		return m_sText;
	} else
	{
		return m_sTipText;
	}
}

COLORREF CTipEditUI::GetDisplayTextColor()
{
	COLORREF clrText;
	if (IsFocused() || !m_sText.IsEmpty())
	{
		clrText = m_clrText;
	} else
	{
		clrText = m_pManager->GetThemeColor(UICOLOR_EDIT_TEXT_TIPTEXT);		
	}

	if (IsReadOnly())
	{
		clrText = m_clrTextReadOnly;
	}
	if (!IsEnabled())
	{
		clrText = m_clrTextReadOnly;
	}
	return clrText;
}

void CTipEditUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcsicmp(pstrName, _T("tiptext")) == 0)
	{
		SetTipText(pstrValue);
	}else
	{
		CSingleLineEditUI::SetAttribute(pstrName, pstrValue);
	}
}

void CTipEditUI::SetTipText(LPCTSTR sTip)
{
	if (m_sTipText != sTip)
	{
		m_sTipText = sTip;
		Invalidate();
	}
}
/////////////////////////////////////////////////////////////////////////////////////
//
//

class CMultiLineEditWnd : public CWindowWnd
{
public:
   CMultiLineEditWnd();
   ~CMultiLineEditWnd();
   void Init(CMultiLineEditUI* pOwner);

   LPCTSTR GetWindowClassName() const;
   LPCTSTR GetSuperClassName() const;
   void OnFinalMessage(HWND hWnd);

   LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
   LRESULT OnEditChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
   LRESULT OnChar(WPARAM wParam, LPARAM lParam, BOOL &bHandled);
   void AppendText(const char *szText); //追加数据

   LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
protected:
   CMultiLineEditUI* m_pOwner;
   HBRUSH  m_hBrush;
};


CMultiLineEditWnd::CMultiLineEditWnd():
                   m_pOwner(NULL)
{
	m_hBrush = ::CreateSolidBrush(RGB(255, 255, 255));
}

CMultiLineEditWnd::~CMultiLineEditWnd()
{
	::DeleteObject(m_hBrush);
}

LRESULT CMultiLineEditWnd::OnChar(WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	if (m_pOwner)
		bHandled = m_pOwner->OnChar(wParam, lParam);
	return 0;
}

void CMultiLineEditWnd::Init(CMultiLineEditUI* pOwner)
{
	RECT rcPos = pOwner->GetPos();
	::InflateRect(&rcPos, -1, -3);
	DWORD dwStyle = WS_CHILD | ES_MULTILINE | pOwner->m_dwEditStyle;
	Create(pOwner->m_pManager->GetPaintWindow(), NULL, dwStyle, 0, rcPos);
	SetWindowFont(m_hWnd, pOwner->m_pManager->GetThemeFont(UIFONT_NORMAL), TRUE);
	Edit_SetText(m_hWnd, pOwner->m_sText);
	Edit_SetModify(m_hWnd, FALSE);
	SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(2, 2));
	Edit_SetReadOnly(m_hWnd, pOwner->IsReadOnly() == true);
	Edit_Enable(m_hWnd, pOwner->IsEnabled() == true);
	if (pOwner->IsVisible()) 
		::ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
	m_pOwner = pOwner;
}

LPCTSTR CMultiLineEditWnd::GetWindowClassName() const
{
	return _T("MultiLineEditWnd");
}

LPCTSTR CMultiLineEditWnd::GetSuperClassName() const
{
	return WC_EDIT;
}

void CMultiLineEditWnd::OnFinalMessage(HWND /*hWnd*/)
{
	m_pOwner->m_pWindow = NULL;
	delete this;
}

LRESULT CMultiLineEditWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	BOOL bHandled = TRUE;
	if (uMsg == WM_SETFOCUS)
	{ 
		if (!m_pOwner->IsFocused())
		{	
			m_pOwner->SetFocus();
			lRes = 0;
		}
		else bHandled=false;
	} else if ((uMsg >= WM_CTLCOLORMSGBOX + 0xbc00) 
		       && (uMsg <= WM_CTLCOLORSTATIC + 0xbc00))
	{
		::SetBkColor((HDC)wParam, RGB(255, 255, 255));
		lRes = (LRESULT)m_hBrush;
	} else if (uMsg == OCM_COMMAND && GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE)
	{
		lRes = OnEditChanged(uMsg, wParam, lParam, bHandled);
	} else if (uMsg == WM_KEYDOWN)
	{
		lRes = OnKeyDown(uMsg, wParam, lParam, bHandled);
		if (bHandled)
			wParam = 0;
		else
		{
			if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				WORD vKey = (WORD) wParam;
				switch(vKey)
				{
				case 'A':
				case 'a':
					Edit_SetSel(m_hWnd, 0, -1);
					bHandled = TRUE;
					lRes = 1;
					break;
				}
			}
		}
	} else if (uMsg == WM_CHAR)
	{
		lRes = OnChar(wParam, lParam, bHandled);
	} else
	{
		bHandled = FALSE;
	}
	if (!bHandled) 
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	return lRes;
}

//追加数据
void CMultiLineEditWnd::AppendText(const char *szText)
{
	int nSize = ::strlen(szText);
	int nCount = (int)::SendMessage(GetHWND(), EM_GETLINECOUNT, 0, 0);
	nCount = (int)::SendMessage(GetHWND(), EM_LINEINDEX, nCount - 1, 0);
	nCount += (int)::SendMessage(GetHWND(), EM_LINELENGTH, nCount, 0);
	::SendMessage(GetHWND(), EM_SETSEL, nCount, nCount);

	TCHAR *szTemp = new TCHAR[nSize + 5];
	memset(szTemp, 0, sizeof(TCHAR) * (nSize + 5));
	CStringConversion::StringToWideChar(szText, szTemp, nSize);
	::lstrcat(szTemp, L"\r\n");
	::SendMessage(GetHWND(), EM_REPLACESEL, 0, LPARAM(szTemp)); 
	delete []szTemp;
}

LRESULT CMultiLineEditWnd::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	if (m_pOwner)
	{
		bHandled = m_pOwner->OnKeyDown(wParam, lParam);
	}
	return 0;
}

LRESULT CMultiLineEditWnd::OnEditChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if (m_pOwner == NULL)
	   return 0;
	// Copy text back
	int cchLen = ::GetWindowTextLength(m_hWnd) + 1;
	LPTSTR pstr = static_cast<LPTSTR>(_alloca(cchLen * sizeof(TCHAR)));
	ASSERT(pstr);
	::GetWindowText(m_hWnd, pstr, cchLen);
	m_pOwner->m_sText = pstr;
	if (m_pOwner != NULL && m_pOwner->GetManager())
	{
		m_pOwner->GetManager()->SendNotify(m_pOwner, _T("editchanged"));
	}
	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CMultiLineEditUI::CMultiLineEditUI(): 
                  m_pWindow(NULL),
				  m_dwEditStyle(0)
{
}

CMultiLineEditUI::~CMultiLineEditUI()
{
	if (m_pWindow != NULL && ::IsWindow(*m_pWindow))
	{
		m_pWindow->Close();
	}
}

void CMultiLineEditUI::Init()
{
	CControlUI::Init();
	m_pWindow = new CMultiLineEditWnd();
	ASSERT(m_pWindow);
	m_pWindow->Init(this);
}

LPCTSTR CMultiLineEditUI::GetClass() const
{
	return _T("MultiLineEditUI");
}

UINT CMultiLineEditUI::GetControlFlags() const
{
	if ((m_dwEditStyle & ES_WANTRETURN) != 0)
		return UIFLAG_TABSTOP | UIFLAG_WANTRETURN;
	else
		return UIFLAG_TABSTOP;
}

void CMultiLineEditUI::SetText(LPCTSTR pstrText)
{
	m_sText = pstrText;
	if (m_pWindow != NULL)
	{
		SetWindowText(*m_pWindow, pstrText);
		//
		int nCount = (int)::SendMessage(*m_pWindow, EM_GETLINECOUNT, 0, 0);
		nCount = (int)::SendMessage(*m_pWindow, EM_LINEINDEX, nCount - 1, 0);
		nCount += (int)::SendMessage(*m_pWindow, EM_LINELENGTH, nCount, 0);
		::SendMessage(*m_pWindow, EM_SETSEL, nCount, nCount);
	}
	if (m_pManager != NULL)
	{
		m_pManager->SendNotify(this, _T("changed"));
	}
	Invalidate();
}

CStdString CMultiLineEditUI::GetText() const
{
	if (m_pWindow != NULL)
	{
		int cchLen = ::GetWindowTextLength(*m_pWindow) + 1;
		LPTSTR pstr = static_cast<LPTSTR>(_alloca(cchLen * sizeof(TCHAR)));
		ASSERT(pstr);
		::GetWindowText(*m_pWindow, pstr, cchLen);
		return CStdString(pstr);
	}
	return m_sText;
}

void CMultiLineEditUI::SetVisible(bool bVisible)
{
	CControlUI::SetVisible(bVisible);
	if (m_pWindow != NULL)
	{
		::ShowWindow(*m_pWindow, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	}
}

void CMultiLineEditUI::SetEnabled(bool bEnabled)
{
	CControlUI::SetEnabled(bEnabled);
	if (m_pWindow != NULL)
	{
		::EnableWindow(*m_pWindow, bEnabled == true);
	}
}

void CMultiLineEditUI::SetReadOnly(bool bReadOnly)
{
	if (m_pWindow != NULL)
	{
		Edit_SetReadOnly(*m_pWindow, bReadOnly == true);
	}
	Invalidate();
}

bool CMultiLineEditUI::IsReadOnly() const
{
	return (GetWindowStyle(*m_pWindow) & ES_READONLY) != 0;
}

bool CMultiLineEditUI::SetTextLimit( int iLimit )
{
	if (m_pWindow != NULL)
	{
		Edit_LimitText(*m_pWindow, iLimit);
		Invalidate();
		return true;
	}
	return false;
}

SIZE CMultiLineEditUI::EstimateSize(SIZE /*szAvailable*/)
{
	return CSize( 0, 0 );
}

void CMultiLineEditUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcsicmp(pstrName, _T("textlimit")) == 0)
	{
		SetTextLimit( _ttoi( pstrValue ) );
	} else if (_tcsicmp(pstrName, _T("autovscroll")) == 0)
	{
		if (_tcsicmp(pstrValue, _T("true")) == 0)
		{
			m_dwEditStyle |= (WS_VSCROLL | ES_AUTOVSCROLL);
		} else
		{
			m_dwEditStyle &= ~(ES_AUTOVSCROLL);
		}
	} else if (_tcsicmp(pstrName, _T("wantreturn")) == 0)
	{
		if (_tcsicmp(pstrValue, _T("true")) == 0)
		{
			m_dwEditStyle |= ES_WANTRETURN;
		} else
		{
			m_dwEditStyle &= ~ES_WANTRETURN;
		}
	} else if (_tcsicmp(pstrName, _T("readonly")) == 0)
	{
		if (_tcsicmp(pstrValue, _T("true")) == 0)
		{
			m_dwEditStyle |= ES_READONLY;
		} else
		{
			m_dwEditStyle &= ~ES_READONLY;
		}
	} else
	{
		CControlUI::SetAttribute(pstrName, pstrValue);
	}
}

void CMultiLineEditUI::SetPos(RECT rc)
{
	if (m_pWindow != NULL) 
	{
	   CRect rcEdit = rc;
	   rcEdit.Deflate(3, 3);
	   ::SetWindowPos(*m_pWindow, HWND_TOP, rcEdit.left, rcEdit.top, 
		              rcEdit.GetWidth(), rcEdit.GetHeight(), SWP_NOACTIVATE);
	}
	CControlUI::SetPos(rc);
}

void CMultiLineEditUI::SetPos(int left, int top, int right, int bottom)
{
	SetPos(CRect(left, top, right, bottom));
}

//键盘按下
BOOL CMultiLineEditUI::OnKeyDown(WPARAM wParam, LPARAM lParam)
{ 
	if (m_pManager != NULL)
	{
		TNotifyUI msg;
		msg.pSender = this;
		msg.sType = _T("keydown");
		msg.wParam = wParam;
		msg.lParam = lParam;
		m_pManager->SendNotify(msg);
		return msg.bHandled;
	}
	return FALSE;
}

BOOL CMultiLineEditUI::OnChar(WPARAM wParam, LPARAM lParam)
{
	if (m_pManager != NULL)
	{
		TNotifyUI msg;
		msg.pSender = this;
		msg.sType = _T("onchar");
		msg.wParam = wParam;
		msg.lParam = lParam;
		m_pManager->SendNotify(msg);
		return msg.bHandled;
	}
	return FALSE;
}

void CMultiLineEditUI::Event(TEventUI& event)
{
	if (event.Type == UIEVENT_WINDOWSIZE)
	{
		if (m_pWindow != NULL)
		{
			m_pManager->SetFocus(NULL);
		}
	}
	if (event.Type == UIEVENT_SETFOCUS) 
	{
		m_bFocused = true;//twisted TBD
		if (m_pWindow != NULL)
		{
			::SetFocus(*m_pWindow);
		}
	}
	CControlUI::Event(event);
}

void CMultiLineEditUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
	RECT rcTemp = {0};
	if (!::IntersectRect(&rcTemp, &rcPaint, &m_rcItem)) 
		return;
	//border
	if (HasBorder())
	{
		UINT uState = 0;
		if (IsFocused())
		{
			uState |= UISTATE_FOCUSED;
		}
		if (IsReadOnly())
		{
			uState |= UISTATE_READONLY;
		}
		if (!IsEnabled())
		{
			uState |= UISTATE_DISABLED;
		}
		CBlueRenderEngineUI::DoPaintEditBox(hDC, m_pManager, m_rcItem, uState);
	} //end if (HasBorder())
}


void CMultiLineEditUI::Select(int iStart, int iEnd)
{
	if (m_pWindow)
	{
		::SendMessage(*m_pWindow, EM_SETSEL, (WPARAM)iStart, (LPARAM)iEnd);
	}
}

void CMultiLineEditUI::AppendText(const char *szText)
{
	if (m_pWindow)
		m_pWindow->AppendText(szText);
}

bool CMultiLineEditUI::Copy()
{
	if (m_pWindow)
	{
		::SendMessage(*m_pWindow, WM_COPY, 0, 0);
		return true;
	}
	return false;
}