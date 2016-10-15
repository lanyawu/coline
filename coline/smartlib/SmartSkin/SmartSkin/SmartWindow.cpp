#include <commonlib/stringutils.h>
#include <Commonlib/DebugLog.h>
//aero 功能头文件
#include <Dwmapi.h>
#include "SmartUIResource.h"
#include "SmartWindow.h"
#include "SmartUIResource.h"
#include "SmartMessageBox.h"

#pragma warning(disable:4996)

//关闭窗口回调
BOOL CALLBACK EnumFunc(HWND hChild, LPARAM lParam)
{
	::SendMessage(hChild, WM_CLOSE, 0, lParam); 
	return TRUE;
}

//
CSmartFrame::CSmartFrame(HWND hWnd, UINT uDragWidth):
             m_hWnd( hWnd )
{
	Init(uDragWidth );
}

CSmartFrame::~CSmartFrame()
{
}

BOOL CSmartFrame::Init(UINT uDragWidth)
{
	if((m_hWnd == NULL) || (!::IsWindow(m_hWnd)))
		return FALSE;

	m_uDragWidth = uDragWidth;

	RECT rcWnd;
	::GetClientRect( m_hWnd, &rcWnd );
	int cxWnd = rcWnd.right - rcWnd.left;
	int cyWnd = rcWnd.bottom - rcWnd.top;

	//top-left corner border
	m_rcTopLeft.left = 0;
	m_rcTopLeft.top = 0;
	m_rcTopLeft.right = m_uDragWidth;
	m_rcTopLeft.bottom = m_uDragWidth;
	
	//top border
	m_rcTop.left = m_rcTopLeft.right;
	m_rcTop.top = m_rcTopLeft.top;
	m_rcTop.right = cxWnd - m_uDragWidth;
	m_rcTop.bottom = m_rcTopLeft.bottom;

	//top-right
	m_rcTopRight.left = m_rcTop.right;
	m_rcTopRight.top = m_rcTop.top;
	m_rcTopRight.right = cxWnd;
	m_rcTopRight.bottom = m_rcTop.bottom;

	//left
	m_rcLeft.left = m_rcTopLeft.left;
	m_rcLeft.top = m_rcTopLeft.bottom;
	m_rcLeft.right = m_rcTopLeft.right;
	m_rcLeft.bottom = cyWnd - m_uDragWidth;
	
	//right
	m_rcRight.left = m_rcTopRight.left;
	m_rcRight.top = m_rcTopRight.bottom;
	m_rcRight.right = m_rcTopRight.right;
	m_rcRight.bottom = m_rcLeft.bottom;

	//bot-left
	m_rcBotLeft.left = m_rcLeft.left;
	m_rcBotLeft.top = m_rcLeft.bottom;
	m_rcBotLeft.right = m_rcLeft.right;
	m_rcBotLeft.bottom = cyWnd;

	//bottom
	m_rcBottom.left = m_rcBotLeft.right;
	m_rcBottom.top = m_rcBotLeft.top;
	m_rcBottom.right = m_rcRight.left;
	m_rcBottom.bottom = m_rcBotLeft.bottom;

	//bot-right
	m_rcBotRight.left = m_rcRight.left;
	m_rcBotRight.top = m_rcRight.bottom;
	m_rcBotRight.right = m_rcRight.right;
	m_rcBotRight.bottom = m_rcBottom.bottom;

	//size-box
	m_rcSizeBox.left = m_rcBotRight.left - m_uDragWidth;
	m_rcSizeBox.top = m_rcBotRight.top - m_uDragWidth;
	m_rcSizeBox.right = m_rcBotRight.left;
	m_rcSizeBox.bottom = m_rcBotRight.top;
	return TRUE;
}

//CSmartWindow implementation

CSmartWindow::CSmartWindow(LPWINDOWDESTROY lpDestroy,const char *szWndName, CWindowWnd *pParent):
              m_pParent(pParent),
			  m_pFrame(NULL),
			  m_pOverlapped(NULL),
			  m_pEventCallBack(NULL),
			  m_pMsgCallBack(NULL),
			  m_pBtnMaxRestore(NULL),
			  m_pBtnMin(NULL),
			  m_pTitle(NULL),
			  m_bDockDesktop(FALSE),
			  m_bCanActived(TRUE),
			  m_minSize(0, 0),
			  m_maxSize(0, 0),
			  m_pWinDestroy(lpDestroy),
			  m_pCaption(NULL),
			  m_bResize(TRUE)

{
#if defined(UNICODE)
	TCHAR szwWndName[256] = { 0 };
	CStringConversion::StringToWideChar(szWndName, szwWndName, 255 );
	m_szWndClassName = szwWndName;
#else
	m_szWndClassName = szWndName;
#endif
	m_strWindowName = szWndName;
	if (!CPaintManagerUI::CheckMainFormIsExists())
		CPaintManagerUI::SetMainForm(this);

}

CSmartWindow::~CSmartWindow(void)
{
	m_pWinDestroy = NULL;
}


BOOL CSmartWindow::SetWindowMinSize_(int cx, int cy)
{
	if ((cx > 0) && (cy > 0))
	{
		m_minSize.cx = cx;
		m_minSize.cy = cy;
		return TRUE;
	} else
	{
		m_minSize.cx = 0;
		m_minSize.cy = 0;
	}
	return FALSE;
}

void CSmartWindow::SetCaptionSize(int cx, int cy)
{
	m_captionSize.cx = cx;
	m_captionSize.cy = cy;
}

BOOL CSmartWindow::SetWindowMaxSize_(int cx, int cy)
{
	if ((cx > 0) && (cy > 0))
	{
		m_maxSize.cx = cx;
		m_maxSize.cy = cy;
		return TRUE;
	} else
	{
		m_maxSize.cx = 0;
		m_maxSize.cy = 0;
	}
	return FALSE;
}

DWORD CSmartWindow::GetTypeByName(LPCTSTR pstrName)
{
	if (::lstrcmp(pstrName, _T("click")) == 0)
		return EVENT_TYPE_CLICK;
	else if (::lstrcmp(pstrName, _T("link")) == 0)
		return EVENT_TYPE_LINK;
	else if (::lstrcmp(pstrName, _T("change")) == 0)
		return EVENT_TYPE_CHANGED;
	else if (::lstrcmp(pstrName, _T("editchanged")) == 0)
		return EVENT_TYPE_CHANGED;
	else
		return EVENT_TYPE_UNKNOWN;
}

void CSmartWindow::Notify(TNotifyUI& msg)
{
	BOOL bDid = FALSE;
	if (m_pEventCallBack)
	{
		bDid = m_pEventCallBack(m_hWnd, msg.sType.GetData(), m_szWndClassName.GetData(), msg.pSender->GetName(), &(msg.ptMouse),
		                msg.wParam, msg.lParam, m_pOverlapped);
		msg.bHandled = bDid;
	}
	if ((!bDid) && (msg.sType == _T("click")))
	{
		if(msg.pSender->GetName() == STD_BTN_MIN)
		{
			OnNotifyClickMinBtn(msg);//最小化
		} else if(msg.pSender->GetName() == STD_BTN_MAX)
		{
			OnNotifyClickMaxBtn(msg);//最大化
		} else if(msg.pSender->GetName() == STD_BTN_RESTORE) 
		{
			OnNotifyClickRestoreBtn(msg); //恢复（缩小）
		} else if(msg.pSender->GetName() == STD_BTN_CLOSE)
		{ 
			OnNotifyClickCloseBtn(msg);//关闭
		} else
		{
			//回调
		}
	}
}

LPCTSTR CSmartWindow::GetWindowClassName() const
{
	return m_szWndClassName.GetData();
}

std::string CSmartWindow::GetWindowName()
{
	return m_strWindowName;
}

UINT CSmartWindow::GetClassStyle() const
{
	return CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
}

BOOL CSmartWindow::OrderWindowMessage_(UINT uMsg)
{
	m_OrderMsgList[uMsg] = 0;
	return TRUE;
}

void CSmartWindow::SetIsActive(BOOL bCanActived)
{
	m_bCanActived = bCanActived;
}

BOOL CSmartWindow::OnWMActive(WPARAM wParam, LPARAM lParam, LRESULT &lRes)
{
	if (LOWORD(wParam) != WA_INACTIVE)
		CSystemUtils::FlashWindow(GetHWND(), FALSE);
	if (!m_bCanActived)
	{
		WORD l = LOWORD(wParam);
		if (l == WA_ACTIVE)
		{
			HWND hWnd = ::GetNextWindow(m_hWnd, GW_HWNDNEXT);
			while (! ::IsWindowVisible(hWnd) || (::GetParent(hWnd) != 0))
			{
				hWnd = ::GetNextWindow(hWnd, GW_HWNDNEXT);
				if (!hWnd)
					break;
			}
			if (hWnd)
				SetActiveWindow(hWnd);
			lRes = 0;
			return TRUE;
		}
	}
	return FALSE;
}

LRESULT CSmartWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL bHandled = FALSE;
	LRESULT lRes = 0;
	POINT pt;
	switch(uMsg)
	{
		case WM_CREATE:
			bHandled = OnCreate(lRes);
			break;
		case WM_WINDOWPOSCHANGING:
			bHandled = OnWindowPosChanging((WINDOWPOS *)lParam, lRes);
			break;
		case WM_WINDOWPOSCHANGED:
			bHandled = OnWindowPosChanged((WINDOWPOS *)lParam, lRes);
			break;
		case WM_ACTIVATE:
			bHandled = OnWMActive(wParam, lParam, lRes);
			break;
		case WM_SIZE:
			bHandled = OnSize(wParam, lParam, lRes);
			break;
		case WM_GETMINMAXINFO://MINMAXINFO
			bHandled = OnGetMinMaxInfo(wParam, lParam, lRes);
			break;
		case WM_NCHITTEST:
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
			bHandled = OnNcHitTest(pt, lRes);
			break;
		case WM_NCPAINT:
			bHandled = OnNCPaint(wParam, lParam, lRes);
			break;
		case WM_ERASEBKGND:
			return 1;
			break;
		case WM_DESTROY:
			bHandled = OnDestroy(wParam, lParam, lRes);
			break;
		case WM_KILLFOCUS:
			bHandled = OnKillFocus(wParam, lParam, lRes);
			break;
		case WM_SETTEXT:
			if ((lParam != NULL) && m_pTitle)
				m_pTitle->SetText((TCHAR *)lParam);
			break;
		default:
			break;
	}
	BOOL bDoing = FALSE;
	if (m_pMsgCallBack)
	{
		if (uMsg == WM_DESTROY)
			bDoing =  m_pMsgCallBack(m_hWnd, uMsg, wParam, lParam, &lRes, m_pOverlapped);
		else
		{
			std::map<UINT, UINT>::iterator it = m_OrderMsgList.find(uMsg);
			if (it != m_OrderMsgList.end())
				bDoing = m_pMsgCallBack(m_hWnd, uMsg, wParam, lParam, &lRes, m_pOverlapped);
		}
	}
	//default message handling
	if (bHandled || bDoing) 
	{
		return lRes;
	} else if (m_paintMgr.MessageHandler(uMsg, wParam, lParam, lRes)) 
	{
		return lRes;
	} else 
	{
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}
}

BOOL CSmartWindow::OnDestroy(WPARAM wParam, LPARAM lParam, LRESULT &lRes)
{
	return FALSE;
}

BOOL CSmartWindow::OnKillFocus(WPARAM wParam, LPARAM lParam, LRESULT &lRes)
{
	return FALSE;
}


void CSmartWindow::OnFinalMessage(HWND hWnd)
{
	if (m_pWinDestroy)
		m_pWinDestroy(hWnd);
	delete this;
}

BOOL CSmartWindow::OnSize(WPARAM wParam, LPARAM lParam, LRESULT &lRes)
{
	CImageButtonUI* pButton = NULL;
	switch(wParam)
	{
		case SIZE_MAXIMIZED://最大化按钮替换为恢复按钮
			EnableResize(FALSE);
			if(m_pBtnMaxRestore)
			{
				m_pBtnMaxRestore->SetImage(m_paintMgr.GetRestoreBtnImageId());
				m_pBtnMaxRestore->SetName(STD_BTN_RESTORE);
			}
			break;
		case SIZE_RESTORED://恢复按钮替换为最大化按钮
			if(m_pBtnMaxRestore)
			{
				m_pBtnMaxRestore->SetImage(m_paintMgr.GetMaxBtnImageId());
				m_pBtnMaxRestore->SetName(STD_BTN_MAX);
			}
		case SIZE_MINIMIZED:
		default:
			EnableResize(TRUE);
			ResizeFrame();
			break;
	}
	return FALSE;
}

//设置透明
BOOL CSmartWindow::SetWindowTransparent(COLORREF crKey, BYTE Alpha, int FLAG)
{
	if ((m_hWnd != NULL) && ::IsWindow(m_hWnd))
	{
		LONG lExStyle = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);
		lExStyle |= WS_EX_LAYERED;
		::SetWindowLong(m_hWnd, GWL_EXSTYLE, lExStyle);
		::SetLayeredWindowAttributes(m_hWnd, crKey, Alpha, FLAG); 
	}
	return FALSE;
}

//实现窗口停靠
BOOL CSmartWindow::SetDockDesktop(BOOL bDock, COLORREF crKey, BYTE Alpha)
{
	if ((m_hWnd != NULL) && ::IsWindow(m_hWnd))
	{
		LONG lExStyle = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);
		LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);
		m_bDockDesktop = bDock;
		if (bDock)
		{ 
			/*lStyle &= ~WS_BORDER;
			lStyle &= ~WS_SIZEBOX;
			lExStyle |= WS_EX_LAYERED;  
			::SetWindowLong(m_hWnd, GWL_EXSTYLE, lExStyle);
			::SetWindowLong(m_hWnd, GWL_STYLE, lStyle);
			::SetLayeredWindowAttributes(m_hWnd, crKey, Alpha, LWA_COLORKEY | LWA_ALPHA);  */
			HWND hDesktop = CSystemUtils::FindDesktopWindow();
			if (hDesktop)
			{
				LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);
				lStyle |= WS_CHILD; 
				::SetWindowLong(m_hWnd, GWL_STYLE, lStyle); 
				//HWND hTmp = hDesktop;
				//hTmp = ::GetWindow(hDesktop, GW_CHILD);
			
				//::MoveWindow(hTmp, 0, 0, 1280, 780, TRUE);
			
				::SetParent(m_hWnd, hDesktop);
				//hTmp = ::GetWindow(hTmp, GW_CHILD);
				//::SetWindowLong(m_hWnd, GWL_STYLE
			}
		} else
		{
			lStyle &= ~WS_CHILD;
			lStyle |= WS_BORDER;
			lStyle |= WS_SIZEBOX;
			lStyle |= WS_POPUP;
			::SetWindowLong(m_hWnd, GWL_STYLE, lStyle);
			lExStyle &= ~WS_EX_LAYERED;
			::SetWindowLong(m_hWnd, GWL_EXSTYLE, lExStyle);
			::SetLayeredWindowAttributes(m_hWnd, 0, 255, LWA_ALPHA);
		} 
	}
	return FALSE;
}

BOOL CSmartWindow::OnCreate(LRESULT &lRes)
{
	m_paintMgr.Init(m_hWnd);
	//m_paintMgr.Init(::GetDesktopWindow());
	//create layout
	SIZE szMin = {0}, szMax = {0}, szCaption = {0};
	UINT uBkgImageId = 0;
	LPCONTROLNODE lpRootNode = CSmartUIResource::Instance()->CreateWindowNode(GetWindowName().c_str(), 
		szMin, szMax, szCaption, uBkgImageId);
	if(lpRootNode == NULL)
	{
		lRes = -1;
		return TRUE;
	}
		//载入背景图片
	if (uBkgImageId > 0)
		LoadBkgndImage(uBkgImageId);
	CDialogBuilder dlgBuilder;
	CControlUI* pRoot = dlgBuilder.CreateFromNode(lpRootNode);
	if(pRoot == NULL)
	{
		CSmartUIResource::Instance()->ReleaseWindowNodes(GetWindowName().c_str());
		lRes = -1;
		return TRUE;
	}

	//window rgn
	RECT rc = {0};
	::GetWindowRect(m_hWnd, &rc);
	rc.right -= rc.left;
	rc.bottom -= rc.top;
	rc.left = rc.top = 0;
	SetWindowRgn(rc);

	m_paintMgr.AttachDialog(pRoot);
	m_paintMgr.AddNotifier(this);
	
	//设置ICON图标
	Init();
	::GetWindowRect(m_hWnd, &rc);
	BOOL bRefresh = FALSE;
	if ((szMin.cx > 0) && (rc.right - rc.left < szMin.cx))
	{
		rc.right = rc.left + szMin.cx;
		bRefresh = TRUE;
	}
	if ((szMin.cy > 0) && (rc.bottom - rc.top < szMin.cy))
	{
		rc.bottom = rc.top + szMin.cy;
		bRefresh = TRUE;
	}
	if ((szMax.cx > 0) && (rc.right - rc.left > szMax.cx))
	{
		rc.right = rc.left + szMax.cx;
		bRefresh = TRUE;
	}
	if ((szMax.cy > 0) && (rc.bottom - rc.top > szMax.cy))
	{
		rc.bottom = rc.top + szMax.cy; 
		bRefresh = TRUE;
	}
	if (bRefresh)
		::MoveWindow(m_hWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
	SetWindowMinSize_(szMin.cx, szMin.cy);
	SetWindowMaxSize_(szMax.cx, szMax.cy);
	SetCaptionSize(szCaption.cx, szCaption.cy);
	lRes = 0;
	return TRUE;
}

BOOL CSmartWindow::OnGetMinMaxInfo(WPARAM wParam, LPARAM lParam, LRESULT &lRes)
{
	MINMAXINFO* pInfo = reinterpret_cast<MINMAXINFO *>(lParam);
	pInfo->ptMinTrackSize = MinTrackSize();
	pInfo->ptMaxSize = MaxTrackSize();
	return TRUE;
}

BOOL CSmartWindow::Init()
{
	//minimize, maximize/resotore buttons
	m_pBtnMin = dynamic_cast<CImageButtonUI *>(FindControl(STD_BTN_MIN));
	m_pBtnMaxRestore = dynamic_cast<CImageButtonUI *>(FindControl(STD_BTN_MAX));

	//set title
	m_pTitle = dynamic_cast<CLabelPanelUI *>(FindControl(STD_TITLE));
	//find caption
	m_pCaption = dynamic_cast<CHorizontalLayoutUI *>(FindControl(STD_CAPTION));

	TCHAR szwTitle[512] = {0};
	if(GetWindowText(m_hWnd, szwTitle, 511))
	{
		SetText(szwTitle);
	}
    if (m_pEventCallBack)
		m_pEventCallBack(m_hWnd, L"init", m_szWndClassName.GetData(), NULL, NULL, 0, 0, m_pOverlapped);
	return TRUE;
}

BOOL CSmartWindow::NotifyEvent(const TCHAR *szCtrlName, const TCHAR *szEventName, WPARAM wParam, LPARAM lParam)
{
	CControlUI *pControl = FindControl(szCtrlName);
	if (pControl)
	{
		m_paintMgr.SendNotify(pControl, szEventName, wParam, lParam);
		return TRUE;
	}
	return FALSE;
}

BOOL CSmartWindow::SetCtrlVisible(const TCHAR *szControlName, BOOL bVisible)
{
	CControlUI *pControl = FindControl(szControlName);
	if (pControl)
	{
		pControl->SetVisible(bVisible == TRUE);
		return TRUE;
	}
	return FALSE;
}

BOOL CSmartWindow::SetControlFocus(const TCHAR *szCtrlName, BOOL bFocus)
{
	CControlUI *pControl = FindControl(szCtrlName);
	if (pControl)
	{
		if (bFocus)
			pControl->SetFocus();
		return TRUE;
	}
	return FALSE;
}

BOOL CSmartWindow::SetCtrlEnable(const TCHAR *szControlName, BOOL bEnable)
{
	CControlUI *pControl = FindControl(szControlName);
	if (pControl)
	{
		pControl->SetEnabled(bEnable == TRUE);
		return TRUE;
	}
	return FALSE;
}

BOOL CSmartWindow::OnWindowPosChanging(WINDOWPOS *pWndPos, LRESULT &lRes)
{
	//size of the window has changed, we need to rethink the region of the window
	if ((pWndPos->flags & SWP_NOSIZE) == 0)
	{
		if (m_minSize.cx > 0)
		{
			if (pWndPos->cx < m_minSize.cx)
				pWndPos->cx = m_minSize.cx;
		}
		if (m_minSize.cy > 0)
		{
			if (pWndPos->cy < m_minSize.cy)
				pWndPos->cy = m_minSize.cy;
		}
		if (m_maxSize.cx > 0)
		{
			if (pWndPos->cx > m_maxSize.cx)
				pWndPos->cx = m_maxSize.cx;
		}
		if (m_maxSize.cy > 0)
		{
			if (pWndPos->cy > m_maxSize.cy)
				pWndPos->cy = m_maxSize.cy;
		}
		if ((pWndPos->x + pWndPos->cx) < 0)
			pWndPos->x = 0;
		if ((pWndPos->y + pWndPos->cy) < 0)
			pWndPos->y = 0;
		RECT rc;
		rc.left = rc.top = 0;
		rc.right = pWndPos->cx;
		rc.bottom = pWndPos->cy;
		SetWindowRgn(rc);
		lRes = 0;
		return TRUE;
	}
	return FALSE;

}

BOOL CSmartWindow::OnWindowPosChanged(WINDOWPOS *pWndPos, LRESULT &lRes)
{
	/*BOOL bDid = FALSE;
	if (m_pEventCallBack)
	{
		WPARAM wParam = MAKELONG(pWndPos->x, pWndPos->y);
		LPARAM lParam = MAKELONG(pWndPos->cx, pWndPos->cy);
		bDid = m_pEventCallBack(m_hWnd, L"OnPosChanged", m_szWndClassName.GetData(), m_szWndClassName.GetData(), NULL,
		                wParam, lParam, m_pOverlapped);
	}*/
	return FALSE;
}

BOOL CSmartWindow::OnNcHitTest(const POINT &pt, LRESULT &lRes)
{
	//转换坐标
	POINT ptAbsoult = pt;
	::ScreenToClient(m_hWnd, &ptAbsoult);

	//模拟标题栏，拖动窗口,排除最大化
	CControlUI* pUI = m_paintMgr.FindControl(ptAbsoult);
	if(pUI != NULL)
	{ 
		if ((ptAbsoult.x > 3) && (ptAbsoult.x < m_captionSize.cx)
			&& (ptAbsoult.y > 3) && (ptAbsoult.y < m_captionSize.cy))
		{
			if ((_tcscmp(pUI->GetClass(), L"ImageButtonUI") != 0) 
				&& (_tcscmp(pUI->GetClass(), L"MenuButtonUI") != 0))
			{
				lRes = HTCAPTION;
				return TRUE;
			}
		}
		CStdString strName = pUI->GetName();
		if ((strName == STD_DRAG_AREA) || (strName == STD_TITLE) || (strName == STD_CAPTION)
			|| (strName == STD_LOGO))
		{
			lRes = HTCAPTION;
			return TRUE;
		}
	}

	//模拟边框
	if (m_bResize)
	{
		if(::PtInRect(&m_pFrame->m_rcTopLeft, ptAbsoult))
		{
			lRes = HTTOPLEFT;
			return TRUE;
		} else if(::PtInRect(&m_pFrame->m_rcTop, ptAbsoult))
		{
			lRes = HTTOP;
			return TRUE;
		} else if(::PtInRect(&m_pFrame->m_rcTopRight, ptAbsoult))
		{
			lRes = HTTOPRIGHT;
			return TRUE;
		} else if(::PtInRect(&m_pFrame->m_rcLeft, ptAbsoult))
		{
			lRes = HTLEFT;
			return TRUE;
		} else if(::PtInRect(&m_pFrame->m_rcRight, ptAbsoult))
		{
			lRes = HTRIGHT;
			return TRUE;
		} else if(::PtInRect(&m_pFrame->m_rcBotLeft, ptAbsoult))
		{
			lRes = HTBOTTOMLEFT;
			return TRUE;
		} else if(::PtInRect(&m_pFrame->m_rcBottom, ptAbsoult))
		{
			lRes = HTBOTTOM;
			return TRUE;
		} else if(::PtInRect(&m_pFrame->m_rcBotRight, ptAbsoult))
		{
			lRes = HTBOTTOMRIGHT;
			return TRUE;
		} else if(::PtInRect(&m_pFrame->m_rcSizeBox, ptAbsoult))
		{
			lRes = HTBOTTOMRIGHT;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CSmartWindow::OnNCPaint(WPARAM wParam, LPARAM lParam, LRESULT& lRes)
{
	return FALSE;
}

BOOL CSmartWindow::OnCloseQuery()
{ 
	BOOL bClosed = TRUE;
	if (m_pEventCallBack)
	{
		if (m_pEventCallBack(m_hWnd, L"onclosequery",
			m_szWndClassName.GetData(), m_szWndClassName.GetData(), NULL, 0, 0,  m_pOverlapped))
		{
			bClosed = FALSE;
		}
	}
	if (bClosed)
	{
		::EnumChildWindows(m_hWnd, EnumFunc, 0); 
		return TRUE;
	} else
		return FALSE;
}


BOOL CSmartWindow::OnNotifyClickMinBtn(TNotifyUI &msg)
{
	::ShowWindow(m_hWnd, SW_MINIMIZE);
	return TRUE;
}

BOOL CSmartWindow::OnNotifyClickMaxBtn(TNotifyUI &msg)
{
	::ShowWindow( m_hWnd, SW_MAXIMIZE );
	return TRUE;
}

BOOL CSmartWindow::OnNotifyClickRestoreBtn(TNotifyUI &msg)
{
	::ShowWindow(m_hWnd, SW_RESTORE);
	return TRUE;
}

BOOL CSmartWindow::OnNotifyClickCloseBtn(TNotifyUI &msg)
{
	Close();
	return TRUE;
}

void CSmartWindow::SetModalValue(int nValue)
{
	m_nModalResult = nValue;
}

UINT CSmartWindow::StdMsgBox(LPCTSTR szContent, LPCTSTR szCaption, UINT nStyle)
{
	UINT nRes = 0;
 
	CSmartMessageBox dlg;
	if (dlg.CreateMessageBox(m_hWnd, szContent, szCaption, nStyle))
		nRes = dlg.DoModal(); 
	return nRes;
}

CControlUI *CSmartWindow::FindControl(const char *szName)
{
	TCHAR szwName[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szName, szwName, MAX_PATH - 1);
	return FindControl(szwName);
}

CControlUI *CSmartWindow::FindControl(const TCHAR *szContrlName)
{
	return m_paintMgr.FindControl(szContrlName);
}

void CSmartWindow::HideCaption(BOOL bHide)
{
	if(m_pCaption)
	{
		m_pCaption->SetVisible(!bHide);
	}
}

void CSmartWindow::HideMinBtn(BOOL bHide)
{
	if(m_pBtnMin)
	{
		m_pBtnMin->SetVisible(!bHide);
	}
}

void CSmartWindow::HideMaxRestoreBtn(BOOL bHide)
{
	if(m_pBtnMaxRestore)
	{
		m_pBtnMaxRestore->SetVisible(!bHide);
	}
}

void CSmartWindow::EnableResize(BOOL bEnable)
{
	m_bResize = bEnable;
}

void CSmartWindow::SetBkgndImage(UINT uNewID, const StretchFixed &sf, BOOL bStretchChange)
{	
	CImageCanvasUI *pCanvas = dynamic_cast<CImageCanvasUI *>(FindControl(STD_WNDCANVAS));
	if (pCanvas)
	{
		pCanvas->SetImage(uNewID);
		if(bStretchChange)
		{
			pCanvas->SetStretch(sf);
		}
	}
	LoadBkgndImage(uNewID);
}

void CSmartWindow::SetParent(CWindowWnd *pParent)
{
	m_pParent = pParent;
}

void CSmartWindow::SetText(LPCTSTR szTitle)
{
	if(m_pTitle)
	{
		m_pTitle->SetText(szTitle);
	}
	::SetWindowText(m_hWnd, szTitle);
}

POINT CSmartWindow::MinTrackSize()
{
	POINT pt = {m_minSize.cx, m_minSize.cy};
	return pt;
}

POINT CSmartWindow::MaxTrackSize()
{
	RECT rcWorkArea = {0};
	::SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcWorkArea, NULL);
	int cxWorkArea = rcWorkArea.right - rcWorkArea.left;
	int cyWorkArea = rcWorkArea.bottom - rcWorkArea.top;
	WINDOWINFO wi = {0};
	::GetWindowInfo(m_hWnd, &wi);
	POINT pt = {cxWorkArea + 2 * wi.cxWindowBorders + 2, 
		cyWorkArea + 2 * wi.cyWindowBorders + 2 };
	return pt;
}

void CSmartWindow::ResizeFrame()
{
	if(m_pFrame == NULL)
	{
		m_pFrame = new CSmartFrame(m_hWnd);
	}
	WINDOWINFO wi = {0};
	::GetWindowInfo(m_hWnd, &wi);
	m_pFrame->Init(wi.cxWindowBorders);
}


void CSmartWindow::SetCtrlVisible(const CStdString &strCtrlName, BOOL bVisible)
{
	CControlUI *pCtrl = FindControl(strCtrlName);
	if( pCtrl )
		pCtrl->SetVisible(bVisible == TRUE);
}

BOOL CSmartWindow::GetControlVisible(const TCHAR *szCtrlName)
{
	CControlUI *pCtrl = FindControl(szCtrlName);
	if (pCtrl)
		return pCtrl->IsVisible();
	return FALSE;
}

BOOL CSmartWindow::GetControlEnable(const TCHAR *szCtrlName)
{
	CControlUI *pCtrl = FindControl(szCtrlName);
	if (pCtrl)
		return pCtrl->IsEnabled();
	return FALSE;
}

BOOL CSmartWindow::GetControlAttribute(const TCHAR *szControlName, const TCHAR *szAttrName, 
	                          TCHAR *szValue, int *nMaxValueSize)
{
	CControlUI *pCtrl = FindControl(szControlName);
	if (pCtrl)
		return pCtrl->GetAttribute(szAttrName, szValue, *nMaxValueSize);
	return FALSE;
}

void CSmartWindow::EnableCtrl(const CStdString &strCtrlName, BOOL bEnable)
{
	CControlUI *pCtrl = FindControl(strCtrlName);
	if (pCtrl)
		pCtrl->SetEnabled(bEnable == TRUE);
}


BOOL CSmartWindow::AddControlToUI(const TCHAR *szParentCtrlName, const char *szSkinXml,
	                     TCHAR *szFlag, int *nMaxFlagSize, const int nIdx)
{
	BOOL bSucc = FALSE;
	CControlUI *pCtrl = FindControl(szParentCtrlName);
	if (pCtrl)
	{
		IContainerUI *pContainer = static_cast<IContainerUI *>(pCtrl->GetInterface(L"Container"));
		if (pContainer)
		{
			CDialogBuilder dlgBuilder;
			LPCONTROLNODE lpRootNode = CSmartUIResource::Instance()->CreateNodeByXml(szSkinXml);
			if (lpRootNode)
			{
				CControlUI *pChild = dlgBuilder.CreateFromNode(lpRootNode);
				if (pChild)
				{
					char szTmp[MAX_PATH] = {0};
					if (szFlag && CSystemUtils::GetGuidString(szTmp, nMaxFlagSize))
					{
						CStringConversion::StringToWideChar(szTmp, szFlag, *nMaxFlagSize);
						pChild->SetAttribute(L"name", szFlag);						
					} //end if (CSystemUtils::... 
					pContainer->Add(pChild, nIdx);
					bSucc = TRUE;
				} //end if (pChild)	
				//delete lpRootNode
				CUIResource::ReleaseChildNodes(lpRootNode);
			} //end if (pChild)
		} //end if (pContainer)
	} //end if (pCtrl)
	return bSucc;
}

BOOL CSmartWindow::RemoveControlFromUI(const TCHAR *szParentCtrlName, const TCHAR *szCtrlFlag)
{
	CControlUI *pCtrl = FindControl(szParentCtrlName);
	CControlUI *pChild = FindControl(szCtrlFlag);
	if (pCtrl && pChild)
	{
		IContainerUI *pContainer = static_cast<IContainerUI *>(pCtrl->GetInterface(L"Container"));
		if (pContainer)
			return pContainer->Remove(pChild);
	}
	return FALSE;
}

#define BACKGROUND_IMAGE_CX  8
void CSmartWindow::SetWindowRgn(const RECT& rcWnd)
{
	if (!m_bDockDesktop)
	{
		RECT rc = rcWnd;
		WINDOWINFO wi = {0};
		::GetWindowInfo(m_hWnd, &wi);
		int cxFrame = wi.cxWindowBorders;
		int cyFrame = wi.cyWindowBorders;
		::InflateRect(&rc, -cxFrame, -cyFrame);
		HRGN hRgn = ::CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);
		if(!m_graphBkgnd.IsEmpty())
		{
			int cxImage = m_graphBkgnd.GetWidth();
			int cyImage = m_graphBkgnd.GetHeight();
			int cxDelta = rc.right - rc.left - cxImage;
			int cyDelta = rc.bottom - rc.top - cyImage;

			for (int i = 0; i < BACKGROUND_IMAGE_CX; ++i)
			{
				//top-left
				for (int j = 0; j < BACKGROUND_IMAGE_CX; ++j)
				{
					CombineRegion(hRgn, i, j, cxFrame, cyFrame);
				}
				//bottom-left
				for (int j = cyImage - 1; j > cyImage - BACKGROUND_IMAGE_CX; --j)
				{
					CombineRegion(hRgn, i, j, cxFrame, cyFrame + cyDelta);		
				}
			}

			for (int i = cxImage - 1; i > cxImage - BACKGROUND_IMAGE_CX; --i)
			{
				//top-right
				for (int j = 0; j < BACKGROUND_IMAGE_CX; ++j)
				{
					CombineRegion(hRgn, i, j, cxFrame + cxDelta, cyFrame);			
				}
				//bottom-right
				for (int j = cyImage - 1; j > cyImage - BACKGROUND_IMAGE_CX; --j)
				{
					CombineRegion(hRgn, i, j, cxFrame + cxDelta, cyFrame + cyDelta);			
				}
			}
		}
		::SetWindowRgn(m_hWnd, hRgn, TRUE); 
	}
}

void CSmartWindow::LoadBkgndImage(UINT uImageId)
{
	LPUI_IMAGE_ITEM pImage;
	if (CSmartUIResource::Instance()->GetImageById(uImageId, &pImage))
	{
		m_graphBkgnd.LoadFromFile(pImage->m_strFileName.c_str(), FALSE);
	}
}

void CSmartWindow::CombineRegion(HRGN hDst, int x, int y, int cxOffset, int cyOffset)
{
	RGBQUAD rgb = m_graphBkgnd.m_pImage->GetPixelColor(x, y);
	COLORREF clr = RGB(rgb.rgbRed, rgb.rgbGreen, rgb.rgbBlue);
	if(clr == STD_TRANSPARENT_CLR)
	{
		HRGN hRgnTmp = ::CreateRectRgn(x, y, x + 1, y + 1);
		::OffsetRgn(hRgnTmp, cxOffset, cyOffset);
		::CombineRgn(hDst, hDst, hRgnTmp, RGN_DIFF);
		::DeleteObject(hRgnTmp);
	}
}

BOOL CSmartWindow::GetControlText(const TCHAR *szControlName, TCHAR *szText, int *nSize)
{
	if (!nSize)
		return FALSE;
	CStdString strText;
	if (szControlName == NULL)
	{
		if (m_pTitle)
			strText = m_pTitle->GetText();
	} else
	{
		CControlUI *pControl = FindControl(szControlName);
		if (pControl)
			strText = pControl->GetText();
	}
	if (!strText.IsEmpty())
	{
		if (*nSize >= strText.GetLength())
		{
			*nSize = strText.GetLength();
			::lstrcpy(szText, strText);
			return TRUE;
		} else
			*nSize = strText.GetLength();
	}
	return FALSE;
}

BOOL CSmartWindow::GetControlRect_(const TCHAR *szCtrlName, RECT *rc)
{
	CControlUI *pControl = FindControl(szCtrlName);
	if (pControl)
	{
		*rc = pControl->GetPos();
		return TRUE;
	}
	return FALSE;
}

BOOL CSmartWindow::UpdateControl(const TCHAR *szCtrlName)
{
	CControlUI *pControl = FindControl(szCtrlName);
	if (pControl)
	{
		pControl->UpdateLayout();
		return TRUE;
	}
	return FALSE;
}

BOOL CSmartWindow::SetControlAttribute(const TCHAR *szControlName, const TCHAR *szAttrName, const TCHAR *szValue)
{
	CControlUI *pControl = FindControl(szControlName);
	if (pControl)
	{
		pControl->SetAttribute(szAttrName, szValue);
		return TRUE;
	}
	return FALSE;
}

BOOL CSmartWindow::SetControlText(const TCHAR *szControlName, const TCHAR *szText)
{
	if (szControlName == NULL)
	{
		SetText(szText);
		return TRUE;
	} else
	{
		CControlUI *pControl = FindControl(szControlName);
		if (pControl)
		{
			pControl->SetText(szText);
			return TRUE;
		}
	}
	return FALSE;
}

void CSmartWindow::SetBackGround(BYTE r, BYTE g, BYTE b)
{
	if ((r != 0) || (g != 0) || (b != 0))
	{
		TCHAR szBuf[32] = {0};
		wsprintf(szBuf, L"FF%02X%02X%02X", r, g, b);
		TCHAR *p = NULL;
		COLORREF clr = _tcstoul(szBuf, &p, 16);
		m_paintMgr.SetBkgndColor(clr);
	} else
		m_paintMgr.SetBkgndColor(0);
}

void CSmartWindow::Update()
{
	m_paintMgr.UpdateLayout();
}

void CSmartWindow::SetEventCallBack(LPSKIN_WINDOW_EVENT_CALLBACK pCallBack)
{
	m_pEventCallBack = pCallBack;
}

void CSmartWindow::SetOverlapped(void *pOverlapped)
{
	m_pOverlapped = pOverlapped;
}

void CSmartWindow::SetMsgCallBack(LPSKIN_WINDOW_MESSAGE_CALLBACK pCallBack)
{
	m_pMsgCallBack = pCallBack;
}

BOOL CSmartWindow::AddChatText(const TCHAR *szControlName, const char *szId, const  DWORD dwUserId, const char *szUserName, 
	                  const char *szTime, const char *szText, const CCharFontStyle *cfStyle, 
					  const int nNickColor, BOOL bIsUTF8, BOOL bAck)
{
	CRichEditUI *pEdit = dynamic_cast<CRichEditUI *>(FindControl(szControlName));
	if (pEdit)
	{
		return pEdit->AddChatText(szId, dwUserId, szUserName, szTime, szText, *cfStyle, nNickColor, bIsUTF8, bAck);
	}
	return FALSE;
}

BOOL CSmartWindow::GetRESelectImageFile(const TCHAR *szCtrlName, char *szFileName, int *nSize)
{
	CRichEditUI *pEdit = dynamic_cast<CRichEditUI *>(FindControl(szCtrlName));
	if (pEdit)
	{
		return pEdit->GetSelectImageFileName(szFileName, *nSize);
	}
	return FALSE;
}

int  CSmartWindow::GetRESelectStyle(const TCHAR *szCtrlName)
{
	CRichEditUI *pEdit = dynamic_cast<CRichEditUI *>(FindControl(szCtrlName));
	if (pEdit)
	{
		return pEdit->GetSelected(); 
	}
	return 0;
}

BOOL CSmartWindow::GetREChatId(const TCHAR *szCtrlName, char *szId)
{
	CRichEditUI *pEdit = dynamic_cast<CRichEditUI *>(FindControl(szCtrlName));
	if (pEdit)
	{
		return pEdit->GetCurrentChatId(szId); 
	}
	return 0;
}

BOOL CSmartWindow::REClearChatMsg(const TCHAR *szCtrlName, const char *szId)
{
	CRichEditUI *pEdit = dynamic_cast<CRichEditUI *>(FindControl(szCtrlName));
	if (pEdit)
	{
		return pEdit->ClearChatMsg(szId); 
	}
	return 0;
}

BOOL CSmartWindow::REInsertOlePicture(const TCHAR *szCtrlName, const char *szFileName)
{
	CRichEditUI *pEdit = dynamic_cast<CRichEditUI *>(FindControl(szCtrlName));
	if (pEdit)
	{
		return pEdit->InsertOlePicture(szFileName);
	}
	return FALSE;
}

BOOL CSmartWindow::GetRichEditOleFlag(const TCHAR *szCtrlName, char **pOleFlags)
{
	CRichEditUI *pEdit = dynamic_cast<CRichEditUI *>(FindControl(szCtrlName));
	if (pEdit)
	{
		CStringList_ strList;
		if (pEdit->GetOleFlags(strList))
		{
			int nSize = strList.size() * 64;
			*pOleFlags = (char *) malloc(nSize);
			memset(*pOleFlags, 0,  nSize);
			while (!strList.empty())
			{
				strcat(*pOleFlags, strList.back().c_str());
				strcat(*pOleFlags, ";");
				strList.pop_back();
			} //end while
			return TRUE;
		} //end if (pEdit->
	} // end if (pEdit)
	return FALSE;
}

BOOL CSmartWindow::CancelCustomLink(const TCHAR *szCtrlName, DWORD dwFlag)
{
	CRichEditUI *pEdit = dynamic_cast<CRichEditUI *>(FindControl(szCtrlName));
	if (pEdit)
	{
		pEdit->CancelCustomLink(dwFlag);
		return TRUE;
	}
	return FALSE;
}

//optionui相关
BOOL CSmartWindow::SetOptionData_(const TCHAR *szCtrlName, const int nData)
{
	COptionUI *pOption = dynamic_cast<COptionUI *>(FindControl(szCtrlName));
	if (pOption)
	{
		pOption->SetData(nData);
		return TRUE;
	}
	return FALSE;
}

//radio 相关
BOOL CSmartWindow::SetRadioCheck(const TCHAR *szCtrlName, BOOL bChecked)
{
	CRadioBoxUI *pRadio = dynamic_cast<CRadioBoxUI *>(FindControl(szCtrlName));
	if (pRadio)
	{
		pRadio->SetCheck(bChecked == TRUE);
		return TRUE;
	}
	return FALSE;
}

BOOL CSmartWindow::GetRadioCheck(const TCHAR *szCtrlName)
{
	CRadioBoxUI *pRadio = dynamic_cast<CRadioBoxUI *>(FindControl(szCtrlName));
	if (pRadio)
	{
		return pRadio->IsChecked();
	}
	return FALSE;
}

//checkbox 相关
BOOL CSmartWindow::SetCheckBoxStatus_(const TCHAR *szCtrlName, const int nStatus)
{
	CCheckBoxUI *pCheck = dynamic_cast<CCheckBoxUI *>(FindControl(szCtrlName));
	if (pCheck)
	{
		pCheck->SetCheck(nStatus > 0);
		return TRUE;
	}
	return FALSE;
}

int  CSmartWindow::GetCheckBoxStatus_(const TCHAR *szCtrlName)
{
	CCheckBoxUI *pCheck = dynamic_cast<CCheckBoxUI *>(FindControl(szCtrlName));
	if (pCheck)
	{
		if (pCheck->IsChecked())
			return 1;
	}
	return 0;
}

//Edit 相关
BOOL CSmartWindow::SetEditReadOnlyValue(const TCHAR *szCtrlName, BOOL bReadOnly)
{
	CSingleLineEditUI *pEdit = dynamic_cast<CSingleLineEditUI *>(FindControl(szCtrlName));
	if (pEdit)
	{
		pEdit->SetReadOnly(bReadOnly == TRUE);
		return TRUE;
	}
	return FALSE;
}

BOOL CSmartWindow::SetMultiEditReadOnlyValue(const TCHAR *szCtrlName, BOOL bReadOnly)
{
	CMultiLineEditUI *pEdit = dynamic_cast<CMultiLineEditUI *>(FindControl(szCtrlName));
	if (pEdit)
	{
		pEdit->SetReadOnly(bReadOnly == TRUE);
		return TRUE;
	}
	return FALSE;
}

BOOL CSmartWindow::SetRichEditReadOnlyValue(const TCHAR *szCtrlName, BOOL bReadOnly)
{
	CRichEditUI *pEdit = dynamic_cast<CRichEditUI *>(FindControl(szCtrlName));
	if (pEdit)
	{
		pEdit->SetReadOnly(bReadOnly);
		return TRUE;
	}
	return FALSE;
}

BOOL CSmartWindow::GetREText(const TCHAR *szCtrlName, DWORD dwStyle, char **pBuf, int &nSize)
{
	CRichEditUI *pEdit = dynamic_cast<CRichEditUI *>(FindControl(szCtrlName));
	if (pEdit)
		return pEdit->GetRichText(dwStyle, pBuf, nSize);
	else
		return FALSE;
}

char * CSmartWindow::GetOleText_(const TCHAR *szCtrlName, DWORD dwStyle)
{
	CRichEditUI *pEdit = dynamic_cast<CRichEditUI *>(FindControl(szCtrlName));
	if (pEdit)
		return pEdit->GetOleText(dwStyle);
	return NULL;
}

BOOL CSmartWindow::InsertImageToRichEdit_(const TCHAR *szCtrlName, const char *szFileName,
	                        const char *szTag, const int nPos)
{
	CRichEditUI *pEdit = dynamic_cast<CRichEditUI *>(FindControl(szCtrlName));
	if (pEdit)
	{
		pEdit->InsertGif(szFileName, NULL, szTag, nPos);
		return TRUE;
	}
	return FALSE;
}

BOOL CSmartWindow::ReplaceImageInRichEdit_(const TCHAR *szCtrlName, const char *szFileName, const char *szTag)
{
	CRichEditUI *pEdit = dynamic_cast<CRichEditUI *>(FindControl(szCtrlName));
	if (pEdit)
	{
		pEdit->ReplaceOleObj(szTag, szFileName);
		return TRUE;
	}
	return FALSE;
}

BOOL CSmartWindow::GetCurrentRichEditFont_(const TCHAR *szCtrlName, CCharFontStyle *cfStyle)
{
	CRichEditUI *pEdit = dynamic_cast<CRichEditUI *>(FindControl(szCtrlName));
	if (pEdit)
	{
		pEdit->GetFontStyle(*cfStyle);
		return TRUE;
	}
	return FALSE;
}

BOOL CSmartWindow::SetRichEditCallBack_(const TCHAR *szCtrlName, LPSKIN_RICHEDIT_EVENT_CALLBACK pCallBack, LPVOID lpOverlapped)
{
	CRichEditUI *pEdit = dynamic_cast<CRichEditUI *>(FindControl(szCtrlName));
	if (pEdit)
	{
		pEdit->Attach(pCallBack, lpOverlapped);
		return TRUE;
	}
	return FALSE;
}

BOOL CSmartWindow::RichEditAddFileLink(const TCHAR *szCtrlName, CCharFontStyle *cfStyle, DWORD dwOffset,
		                    const char *szTip, const char *szFileName)
{
	CRichEditUI *pEdit = dynamic_cast<CRichEditUI *>(FindControl(szCtrlName));
	if (pEdit)
	{
		return pEdit->InsertFileLink(cfStyle, dwOffset, szTip, szFileName);
	}
	return FALSE;
}

BOOL CSmartWindow::RichEditInsertTip_(const TCHAR *szCtrlName, CCharFontStyle *cfStyle,
		                   DWORD dwOffset, const TCHAR *szText)
{
	CRichEditUI *pEdit = dynamic_cast<CRichEditUI *>(FindControl(szCtrlName));
	if (pEdit)
	{
		return pEdit->InsertTip(cfStyle, dwOffset, szText);
	}
	return FALSE;
}

BOOL CSmartWindow::RichEditCommand_(const TCHAR *szCtrlName, const char *szCommand, LPVOID lpParams)
{
	CRichEditUI *pEdit = dynamic_cast<CRichEditUI *>(FindControl(szCtrlName));
	if (pEdit)
	{
		return pEdit->RichEditCommand_(szCommand, lpParams);
	}
	return FALSE;
}

BOOL CSmartWindow::SetREText(const TCHAR *szCtrlName, const char *szText, DWORD dwStyle)
{
	CRichEditUI *pEdit = dynamic_cast<CRichEditUI *>(FindControl(szCtrlName));
	if (pEdit)
	{
		pEdit->SetRichText(szText, dwStyle);
		return TRUE;
	}
	return FALSE;
}

BOOL CSmartWindow::SetEditAutoDetectLink(const TCHAR *szCtrlName, BOOL bAutoDetect)
{
	CRichEditUI *pEdit = dynamic_cast<CRichEditUI *>(FindControl(szCtrlName));
	if (pEdit)
	{
		pEdit->SetAutoDetectLink(bAutoDetect == TRUE);
		return TRUE;
	}
	return FALSE;
}

BOOL CSmartWindow::GetEditReadOnlyValue(const TCHAR *szCtrlName)
{
	CSingleLineEditUI *pEdit = dynamic_cast<CSingleLineEditUI *>(FindControl(szCtrlName));
	if (pEdit)
		return pEdit->IsReadOnly();
	return FALSE;
}

BOOL CSmartWindow::GetMultiEditReadOnlyValue(const TCHAR *szCtrlName)
{
	CMultiLineEditUI *pEdit = dynamic_cast<CMultiLineEditUI *>(FindControl(szCtrlName));
	if (pEdit)
		return pEdit->IsReadOnly();
	return FALSE;
}

BOOL CSmartWindow::GetRichEditReadOnlyValue(const TCHAR *szCtrlName)
{
	CRichEditUI *pEdit = dynamic_cast<CRichEditUI *>(FindControl(szCtrlName));
	if (pEdit)
		return pEdit->IsReadOnly();
	return FALSE;
}

//GIF Image 相关
BOOL CSmartWindow::SetGifImage(const TCHAR *szCtrlName, const TCHAR *szImageFileName, BOOL bTransParent, 
		     DWORD dwTransClr, BOOL bAnimate)
{
	CGifImagePanelUI *pImage = dynamic_cast<CGifImagePanelUI *>(FindControl(szCtrlName));
	if (pImage)
	{
		pImage->SetImage(szImageFileName, bTransParent, dwTransClr, bAnimate == TRUE);
		return TRUE;
	} 
	return FALSE;
}

//list box
int  CSmartWindow::InsertListItem(const TCHAR *szCtrlName, const TCHAR *szDspName, void *pData, int idx)
{
	CListUI *pList = dynamic_cast<CListUI *>(FindControl(szCtrlName));
	if (pList)
	{
		int n = pList->InsertItem(idx, szDspName, -1);
		if ((n >= 0) && pData)
			pList->SetItemData(n, pData);
		return n;
	}
	return -1;
}

BOOL CSmartWindow::DeleteListItem(const TCHAR *szCtrlName, const int nIdx)
{
	CListUI *pList = dynamic_cast<CListUI *>(FindControl(szCtrlName));
	if (pList)
	{
		return pList->DeleteItem(nIdx);
	}
	return FALSE;
}

int  CSmartWindow::AppendListItem(const TCHAR *szCtrlName, const char *szDspText, void *pData)
{
	CListUI *pList = dynamic_cast<CListUI *>(FindControl(szCtrlName));
	if (pList)
	{
		int n = pList->AppItem(szDspText);
		if ((n >= 0) && pData)
			pList->SetItemData(n, pData);
		return n;
	}
	return -1;
}

int CSmartWindow::AppendListSubItem(const TCHAR *szCtrlName, const int nIdx, const int nSubIdx, const char *szDspText)
{
	CListUI *pList = dynamic_cast<CListUI *>(FindControl(szCtrlName));
	if (pList)
	{
		int n = pList->AppSubItem(nIdx, nSubIdx, szDspText); 
		return n;
	}
	return -1;
}

int  CSmartWindow::GetListSelItem(const TCHAR *szCtrlName)
{
	CListUI *pList = dynamic_cast<CListUI *>(FindControl(szCtrlName));
	if (pList)
	{
		return pList->GetCurSel();
	}
	return -1;
}

BOOL CSmartWindow::GetListItemInfo(const TCHAR *szCtrlName, TCHAR *szDspName, void **pData, int idx)
{
	CListUI *pList = dynamic_cast<CListUI *>(FindControl(szCtrlName));
	if (pList)
	{
		CStdString strText;
		if (pList->GetItemText(idx, 0, strText))
		{
			::lstrcpy(szDspName, strText.GetData());
			*pData = pList->GetItemData(idx);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CSmartWindow::ListKeyDownEvent(const TCHAR *szCtrlName, WORD wKey)
{
	CListUI *pList = dynamic_cast<CListUI *>(FindControl(szCtrlName));
	if (pList)
	{
		pList->KeyDownEvent(wKey);
		return TRUE;
	}
	return FALSE;
}

int  CSmartWindow::GetListCount(const TCHAR *szCtrlName)
{
	CListUI *pList = dynamic_cast<CListUI *>(FindControl(szCtrlName));
	if (pList)
		return pList->GetCount();
	return 0;
}

BOOL CSmartWindow::SetListSelItem(const TCHAR *szCtrlName, int idx)
{
	CListUI *pList = dynamic_cast<CListUI *>(FindControl(szCtrlName));
	if (pList)
		return pList->SelectItem(idx);
	return FALSE;
}

BOOL CSmartWindow::RemoveListItem(const TCHAR *szCtrlName, int idx)
{
	CListUI *pList = dynamic_cast<CListUI *>(FindControl(szCtrlName));
	if (idx < 0)
	{
		pList->RemoveAll();
		return TRUE;
	}
	return FALSE;
}


//TreeView 相关
//设置树节点回调函数
BOOL CSmartWindow::SetFreeNodeDataCallBack_(const TCHAR *szCtrlName, LPSKIN_FREE_NODE_EXDATA pCallback)
{
	CUITreeView *pTree = dynamic_cast<CUITreeView *>(FindControl(szCtrlName));
	if (pTree)
	{
		pTree->SetFreeNodeFun(pCallback);
		return TRUE;
	}
	return FALSE;
}

//展开树节点
BOOL CSmartWindow::ExpandTreeView(const TCHAR *szCtrlName, void *pParentNode,
								  BOOL bExpanded, BOOL bRecursive)
{
	CUITreeView *pTree = dynamic_cast<CUITreeView *>(FindControl(szCtrlName));
	if (pTree)
	{
		CTreeNodeItem *pItem = NULL;
		if (pParentNode)
		{
			pItem = (CTreeNodeItem *)pParentNode;
		} else
		{
			pItem = pTree->GetNode();
		}
		if (pItem)
		{
			if (bExpanded)
				pItem->Expanded(bRecursive);
			else
				pItem->Reduce();
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CSmartWindow::TreeViewSelectAll(const TCHAR *szCtrlName, void *pNode, BOOL bRecursive)
{
	CUITreeView *pTree = dynamic_cast<CUITreeView *>(FindControl(szCtrlName));
	if (pTree)
	{
		pTree->SelectedAll(pNode, bRecursive);
		return TRUE;
	}
	return FALSE;
}

BOOL CSmartWindow::TreeViewUnSelected(const TCHAR *szCtrlName, void *pNode, BOOL bRecursive)
{
	CUITreeView *pTree = dynamic_cast<CUITreeView *>(FindControl(szCtrlName));
	if (pTree)
	{
		pTree->UnSelected(pNode, bRecursive);
		return TRUE;
	}
	return FALSE;
}

BOOL CSmartWindow::TreeScrollToNodeByKey(const TCHAR *szCtrlName, const char *szKey)
{
	CUITreeView *pTree = dynamic_cast<CUITreeView *>(FindControl(szCtrlName));
	if (pTree)
	{
		pTree->ScrollToNodeByKey(szKey);
		return TRUE;
	}
	return FALSE;
}

BOOL CSmartWindow::TreeViewDelSelected(const TCHAR *szCtrlName, BOOL bRecursive)
{
	CUITreeView *pTree = dynamic_cast<CUITreeView *>(FindControl(szCtrlName));
	if (pTree)
	{
		pTree->DeleteSelected(bRecursive);
		return TRUE;
	}
	return FALSE;
}

BOOL CSmartWindow::TreeViewGetSelectedUsers(const TCHAR *szCtrlName, char *szUsers, int *nSize, BOOL bRecursive)
{
	CUITreeView *pTree = dynamic_cast<CUITreeView *>(FindControl(szCtrlName));
	if (pTree && nSize)
	{
		std::string strUsers;
		pTree->GetSelectUsers(strUsers, bRecursive);
		if (*nSize >= strUsers.size())
		{
			memmove(szUsers, strUsers.c_str(), strUsers.size());
			*nSize = strUsers.size();
			return TRUE;
		} else
		{
			if (nSize)
				*nSize = strUsers.size();
		}
	}
	return FALSE;
}

LPVOID CSmartWindow::AdjustTreeNode(const TCHAR *szCtrlName, void *pParentNode, const TCHAR *szName, 
	                     CTreeNodeType tnType, void *pData, BOOL bAdd, BOOL bRecursive)
{
	CUITreeView *pTree = dynamic_cast<CUITreeView *>(FindControl(szCtrlName));
	if (pTree)
	{
		return pTree->AdjustChildNode(pParentNode, szName, tnType, pData, bAdd, bRecursive);
	}
	return NULL;
}

//
BOOL CSmartWindow::TreeViewClear(const TCHAR *szCtrlName)
{
	CUITreeView *pTree = dynamic_cast<CUITreeView *>(FindControl(szCtrlName));
	if (pTree)
	{
		pTree->Clear();
		return TRUE;
	}
	return FALSE;
}

//
BOOL CSmartWindow::SetTreeViewStatusOffline(const TCHAR *szCtrlName)
{
	CUITreeView *pTree = dynamic_cast<CUITreeView *>(FindControl(szCtrlName));
	if (pTree)
	{
		return pTree->SetTreeViewStatusOffline();
	}
	return FALSE;
}

//设置Icon的类型
BOOL CSmartWindow::SetTreeViewIconType(const TCHAR *szCtrlName, BYTE byteIconType)
{
	CUITreeView *pTree = dynamic_cast<CUITreeView *>(FindControl(szCtrlName));
	if (pTree)
	{
		pTree->SetNodeIconType((CTreeNodeIconType)byteIconType);
		return TRUE;
	}
	return FALSE;
}

//
BOOL CSmartWindow::TVGetOnlineCount(const TCHAR *szCtrlName, void *pParentNode, DWORD *dwTotalCount, DWORD *dwOnlineCount)
{
	CUITreeView *pTree = dynamic_cast<CUITreeView *>(FindControl(szCtrlName));
	if (pTree)
	{
		return pTree->GetUserCount(pParentNode, *dwTotalCount, *dwOnlineCount);
	}
	return FALSE;
}

//设置组节点是否可选
BOOL CSmartWindow::SetTreeViewGroupNodeIsSelect(const TCHAR *szCtrlName, BOOL bIsSelected)
{
	CUITreeView *pTree = dynamic_cast<CUITreeView *>(FindControl(szCtrlName));
	if (pTree)
	{
		pTree->EnableGroupNodeSelState(bIsSelected);
		return TRUE;
	}
	return FALSE;
}

//装载默认图标
BOOL CSmartWindow::LoadTreeViewDefaultImage(const TCHAR *szCtrlName, const char *szImageFileName)
{
	CUITreeView *pTree = dynamic_cast<CUITreeView *>(FindControl(szCtrlName));
	if (pTree)
	{
		pTree->SetDefaultImage(szImageFileName);
		return TRUE;
	}
	return FALSE;
}

//
BOOL CSmartWindow::GetTreeNodeById(const TCHAR *szCtrlName, const DWORD dwId, CTreeNodeType tnType, void **pNode, void **pData)
{
	CUITreeView *pTree = dynamic_cast<CUITreeView *>(FindControl(szCtrlName));
	if (pTree)
	{
		return pTree->GetTreeNodeById(dwId, tnType, pNode, pData);
	}
	return FALSE;
}

//
BOOL CSmartWindow::TVDelNodeByID(const TCHAR *szCtrlName, const int nId, CTreeNodeType tnType)
{
	CUITreeView *pTree = dynamic_cast<CUITreeView *>(FindControl(szCtrlName));
	if (pTree)
	{
		return pTree->DeleteNodeById(nId, tnType);
	}
	return FALSE;
}

//加入树节点
void * CSmartWindow::AddTreeChildNode_(const TCHAR *szCtrlName, const DWORD dwId, void *pParentNode, const TCHAR *szText,  CTreeNodeType tnType,
	void *pData, const TCHAR *szLabel, const TCHAR *szImageFileName, const TCHAR *szExtraData)
{
	CUITreeView *pTree = dynamic_cast<CUITreeView *>(FindControl(szCtrlName));
	if (pTree)
	{
		CTreeNodeItem *pItem = NULL;
		if (pParentNode)
		{
			pItem = (CTreeNodeItem *)pParentNode;
		} else
		{
			pItem = pTree->GetNode();
		}
		if (pItem)
		{
			CTreeNodeItem *pChild = pItem->AddChildNode(dwId, szText, tnType, szLabel, pData, szImageFileName, szExtraData);
			return  pChild;
		}
	}
	return NULL;
}

BOOL CSmartWindow::GetNodeIsExpanded_(void *pNode, BOOL *bExpanded)
{
	if (pNode)
	{
		*bExpanded = ((CTreeNodeItem *)pNode)->IsExpanded();
		return TRUE;
	} 
	return FALSE;
}

BOOL CSmartWindow::GetNodeByKey(const TCHAR *szCtrlName, void *pParentNode, const char *szKey, TCHAR *szName, int *nNameLen,
	                           void **pSelNode, CTreeNodeType *tnType, void **pData)
{
	CUITreeView *pTree = dynamic_cast<CUITreeView *>(FindControl(szCtrlName));
	if (pTree)
	{
		return pTree->GetNodeByKey(pParentNode, szKey, szName, nNameLen, pSelNode, tnType, pData);
	}
	return FALSE;
}

BOOL CSmartWindow::SortTreeNode(const TCHAR *szCtrlName, void *pNode, LPSKIN_COMPARENODE pCompare,
	                            BOOL bRecursive, BOOL bParent)
{
	CUITreeView *pTree = dynamic_cast<CUITreeView *>(FindControl(szCtrlName));
	if (pTree)
	{
		return pTree->SortTree(pNode, pCompare, bRecursive, bParent);
	}
	return FALSE;
}

BOOL CSmartWindow::GetNodeChildUserList_(void *pNode, char *szUserList, int *nSize, BOOL bRecursive)
{
	if (pNode)
	{
		std::string strUsers;
		((CTreeNodeItem *)pNode)->GetUserList(strUsers, bRecursive);
		if (*nSize >= (int) strUsers.size())
		{
			*nSize = (int) strUsers.size();
			strncpy(szUserList, strUsers.c_str(), *nSize);
			return TRUE;
		} else
			*nSize = (int) strUsers.size();
	}
	return FALSE;
}

BOOL CSmartWindow::SetGetKeyFun(const TCHAR *szCtrlName, LPSKIN_GET_TREE_NODE_KEY pCallBack)
{
	CUITreeView *pTree = dynamic_cast<CUITreeView *>(FindControl(szCtrlName));
	if (pTree)
	{
		pTree->SetGetNodeKeyFun(pCallBack);
		return TRUE;
	}
	return FALSE;
}

LPVOID CSmartWindow::UpdateUserStatusToNode_(const TCHAR *szCtrlName, const char *szUserName, 
                                 const char *szStatus, BOOL bMulti)
{
	CUITreeView *pTree = dynamic_cast<CUITreeView *>(FindControl(szCtrlName));
	if (pTree)
	{
		return pTree->GetNode()->UpdateUserStatusToNode_(szUserName, szStatus,  bMulti);
	}
	return FALSE;
}

//
BOOL CSmartWindow::ShowTreeExtraData(const TCHAR *szCtrlName, BOOL bShow)
{
	CUITreeView *pTree = dynamic_cast<CUITreeView *>(FindControl(szCtrlName));
	if (pTree)
	{
		pTree->SetShowExtraData(bShow);
		return TRUE;
	}
	return FALSE;
}

//
LPVOID CSmartWindow::UpdateTreeNodeExtraData(const TCHAR *szCtrlName, const char *szKey, const TCHAR *szExtraData, BOOL bMulti)
{
	CUITreeView *pTree = dynamic_cast<CUITreeView *>(FindControl(szCtrlName));
	if (pTree)
	{
		return pTree->SetExtraData(szKey, szExtraData, bMulti);
	}
	return FALSE;
}

//
LPVOID CSmartWindow::UpdateTreeNodeImageFile(const TCHAR *szCtrlName, const char *szKey, const char *szImageFile, BOOL bMulti)
{
	CUITreeView *pTree = dynamic_cast<CUITreeView *>(FindControl(szCtrlName));
	if (pTree)
	{
		return pTree->SetImageFile(szKey, szImageFile, bMulti);
	}
	return FALSE;
}

//
LPVOID CSmartWindow::UpdateTreeNodeExtraImageFile(const TCHAR *szCtrlName, const char *szKey, const int nImageId, BOOL bMulti)
{
	CUITreeView *pTree = dynamic_cast<CUITreeView *>(FindControl(szCtrlName));
	if (pTree)
	{
		return pTree->SetExtraImageFile(szKey, nImageId, bMulti);
	}
	return FALSE;
}

LPVOID CSmartWindow::UpdateUserLabelToNode_(const TCHAR *szCtrlName, const char *szUserName,
                                 const char *szUTF8Label,  BOOL bMulti)
{
	CUITreeView *pTree = dynamic_cast<CUITreeView *>(FindControl(szCtrlName));
	if (pTree)
	{ 
		return pTree->GetNode()->UpdateUserLabelToNode_(szUserName, szUTF8Label,  bMulti);
	}
	return FALSE;
}

//获取当前选择的树节点
BOOL CSmartWindow::GetSelectTreeNode_(const TCHAR *szCtrlName, TCHAR *szName, int *nNameLen, 
									  void **pSelNode, CTreeNodeType *tnType, void **pData)
{
	CUITreeView *pTree = dynamic_cast<CUITreeView *>(FindControl(szCtrlName));
	if (pTree)
	{
		CTreeNodeItem *pSel = pTree->GetSelected();
		if (pSel)
		{
			*pSelNode = pSel;
			if (nNameLen && (*nNameLen >= ::lstrlen(pSel->GetName())))
			{
				*nNameLen = ::lstrlen(pSel->GetName());
				::lstrcpy(szName, pSel->GetName());
			    if (tnType)
					*tnType = pSel->GetNodeType();
				*pData = pSel->GetData();
				return TRUE;
			}
		}
	}
	return FALSE;
}

//导航相关
BOOL CSmartWindow::NavigateURL2(const TCHAR *szCtrlName, const char *szUrl)
{
	CInternetExplorerUI *pNavigate = dynamic_cast<CInternetExplorerUI *>(FindControl(szCtrlName));
	if (pNavigate)
	{
		pNavigate->Navigate(szUrl);
		return TRUE;
	}
	return FALSE;
}

//Tab Control相关
BOOL CSmartWindow::TabNavigate2(const TCHAR *szCtrlName, const int nIdx)
{
	CTabFolderUI *pTab = dynamic_cast<CTabFolderUI *>(FindControl(szCtrlName));
	if (pTab)
	{
		pTab->SelectItem(nIdx);
		return TRUE;
	}
	return FALSE;
}

BOOL CSmartWindow::TabSelectItem(const TCHAR *szCtrlName, const TCHAR *szPageName)
{
	CTabFolderUI *pTab = dynamic_cast<CTabFolderUI *>(FindControl(szCtrlName));
	if (pTab)
	{
		pTab->SelectItem(szPageName);
		return TRUE;
	}
	return FALSE;
}

BOOL CSmartWindow::TabGetSelItemName(const TCHAR *szCtrlName, TCHAR *szSelName, int *nSize)
{
	CTabFolderUI *pTab = dynamic_cast<CTabFolderUI *>(FindControl(szCtrlName));
	if (pTab)
	{
		return pTab->TabGetSelItemName(szSelName, nSize);
	}
	return FALSE;
}

BOOL CSmartWindow::TabGetChildItemByClass(const TCHAR *szCtrlName, const TCHAR *szClassName, TCHAR *szName, int *nSize)
{
	CTabFolderUI *pTab = dynamic_cast<CTabFolderUI *>(FindControl(szCtrlName));
	if (pTab)
	{
		return pTab->GetSelItemChildControl(szClassName, szName, nSize);
	}
	return FALSE;
}


//Dropdown 相关
void  *CSmartWindow::GetDropdownItemData_(const TCHAR *szCtrlName, const int nIdx)
{
	CDropDownUI *pDropdown = dynamic_cast<CDropDownUI *>(FindControl(szCtrlName));
	if (pDropdown)
	{
		return pDropdown->GetItemData(nIdx);
	}
	return FALSE;
}

//设置dropdown 项 数据
BOOL CSmartWindow::SetDropdownItemData_(const TCHAR *szCtrlName, const int nIdx, void *pData)
{
	CDropDownUI *pDropdown = dynamic_cast<CDropDownUI *>(FindControl(szCtrlName));
	if (pDropdown)
	{
		return pDropdown->SetItemData(nIdx, pData);
	}
	return FALSE;
}

//获取dropdown 项string
BOOL CSmartWindow::GetDropdownItemString_(const TCHAR *szCtrlName, const int nIdx,
										  TCHAR *szText, int *nSize)
{
	CDropDownUI *pDropdown = dynamic_cast<CDropDownUI *>(FindControl(szCtrlName));
	if (pDropdown)
	{
		CStdString strText;
		if (pDropdown->GetString(nIdx, strText))
		{
			if (*nSize >= strText.GetLength())
			{
				*nSize = strText.GetLength();
				::lstrcpy(szText, strText.GetData());
				return TRUE;
			}
		}
	}
	return FALSE;
}

//设置dropdown 项 string
int CSmartWindow::SetDropdownItemString_(const TCHAR *szCtrlName, const int nIdx, const TCHAR *szText, void *pData)
{
	int n = -1;
	CDropDownUI *pDropdown = dynamic_cast<CDropDownUI *>(FindControl(szCtrlName));
	if (pDropdown)
	{
		if ((nIdx == -1) || (nIdx > pDropdown->GetCount()))
		{ 
			n = pDropdown->InsertString(nIdx, szText);
			if ((n != -1) && (pData != NULL))
				pDropdown->SetItemData(n, pData);
		} else
		{
			if (pDropdown->SetString(nIdx, szText))
			{
				n = nIdx;
				if (pData)
					pDropdown->SetItemData(n, pData);
			}
		}
	}
	return n;
}

//获取当前dropdown当前选择项
int  CSmartWindow::GetDropdownSelIndex(const TCHAR *szCtrlName)
{
	CDropDownUI *pDropdown = dynamic_cast<CDropDownUI *>(FindControl(szCtrlName));
	if (pDropdown)
	{
		return pDropdown->GetCurSel();
	}
	return -1;
}

//删除dropdown 某项
BOOL CSmartWindow::DeleteDropdownItem_(const TCHAR *szCtrlName, const int nIdx)
{
	CDropDownUI *pDropdown = dynamic_cast<CDropDownUI *>(FindControl(szCtrlName));
	if (pDropdown)
	{
		return pDropdown->DeleteItem(nIdx);
	}
	return FALSE;
}

//选择某项
BOOL CSmartWindow::SelectDropdownItem_(const TCHAR *szCtrlName, const int nIdx)
{
	CDropDownUI *pDropdown = dynamic_cast<CDropDownUI *>(FindControl(szCtrlName));
	if (pDropdown)
	{
		return pDropdown->SelectItem(nIdx);
	}
	return FALSE;
}

//获取dropdown count
int CSmartWindow::GetDropdownItemCount_(const TCHAR *szCtrlName)
{
	CDropDownUI *pDropdown = dynamic_cast<CDropDownUI *>(FindControl(szCtrlName));
	if (pDropdown)
	{
		return pDropdown->GetItemCount();
	}
	return FALSE;
}

BOOL CSmartWindow::AddEmotion(const TCHAR *szCtrlName, const char *szFileName, const char *szEmotionTag, 
		       const char *szEmotionShortCut, const char *szEmotionComment)
{
	CGifGridPanelUI *pUI = dynamic_cast<CGifGridPanelUI *>(FindControl(szCtrlName));
	if (pUI)
		return pUI->AddGifUI(szFileName, szEmotionTag, szEmotionShortCut, szEmotionComment);
	return FALSE;
}

int CSmartWindow::GetDisplayEmotionCount(const TCHAR *szCtrlName)
{
	CGifGridPanelUI *pUI = dynamic_cast<CGifGridPanelUI *>(FindControl(szCtrlName));
	if (pUI)
		return pUI->GetShowGifPanelCount();
	return 0;
}

BOOL CSmartWindow::ClearAllEmotion(const TCHAR *szCtrlName)
{
	CGifGridPanelUI *pUI = dynamic_cast<CGifGridPanelUI *>(FindControl(szCtrlName));
	if (pUI)
	{
		pUI->RemoveAll();
		return TRUE;
	}
	return FALSE;
}

BOOL CSmartWindow::GetSelEmotion(const TCHAR *szCtrlName, char *szFileName, int *nNameSize, char *szTag, int *nTagSize)
{
	CGifGridPanelUI *pUI = dynamic_cast<CGifGridPanelUI *>(FindControl(szCtrlName));
	if (pUI)
		return pUI->GetSelGifInfo(szFileName, nNameSize, szTag, nTagSize);
	return FALSE;
}

BOOL CSmartWindow::SetScintKeyWord_(const TCHAR *szCtrlName, const TCHAR *szKeyWord)
{
	CScintillaEditUI *pEdit = dynamic_cast<CScintillaEditUI *>(FindControl(szCtrlName));
	if (pEdit)
	{
		pEdit->SetKeyWord(szKeyWord);
		return TRUE;
	}
	return FALSE;
}

BOOL CSmartWindow::SetScintStyle_(const TCHAR *szCtrlName, int nStyle, COLORREF clrFore, 
       COLORREF clrBack, int nSize, TCHAR *szFace)
{
	CScintillaEditUI *pEdit = dynamic_cast<CScintillaEditUI *>(FindControl(szCtrlName));
	if (pEdit)
	{
		pEdit->SetWordStyle(nStyle, clrFore, clrBack, nSize, szFace);
		return TRUE;
	}
	return FALSE;
}

//menu 相关
BOOL CSmartWindow::CreateSkinMenu_(const TCHAR *szMenuName)
{
	CMenuUI *pMenu = m_paintMgr.LoadMenuUI(szMenuName);
	if (pMenu)
	{
		pMenu->SetManager(&m_paintMgr, NULL); 
		return TRUE;
	}
	return FALSE;
}

//
BOOL CSmartWindow::PopTrackSkinMenu_(const TCHAR *szMenuName, UINT uFlags, const int X, const int Y)
{ 
	CMenuUI *pMenu = m_paintMgr.LoadMenuUI(szMenuName);
	if (pMenu)
	{
		m_paintMgr.InitControls(pMenu);
		m_paintMgr.SendNotify(pMenu, _T("beforemenupop"));
		return pMenu->TrackPopupMenu(uFlags, X, Y);
	} 
	return FALSE;
}

//
BOOL CSmartWindow::DestroySkinMenu_(const TCHAR *szMenuName)
{
	CMenuUI *pMenu = m_paintMgr.LoadMenuUI(szMenuName);  
	if (pMenu)
	{
		pMenu->RemoveAll();
	}
	return FALSE;
}

//
BOOL CSmartWindow::GraySkinMenu_(const TCHAR *szMenuName, UINT uMenuID, BOOL bGray)
{
	CMenuUI *pMenu = m_paintMgr.LoadMenuUI(szMenuName);  
	if (pMenu)
	{
		return pMenu->GrayItem(NULL, uMenuID, bGray);
	}
 
	return FALSE;
}

BOOL CSmartWindow::MenuAppendItem(const TCHAR *szMenuName, UINT nParentId, const TCHAR *szMenuCaption, int nMenuId)
{
	CMenuUI *pMenu = m_paintMgr.LoadMenuUI(szMenuName);  
	if (pMenu)
	{ 
		CMenuItemUI *pItem = new CMenuItemUI();
		pItem->SetAttribute(L"text", szMenuCaption);
		pItem->SetID(nMenuId);
		if (nParentId > 0)
		{
			CMenuItemUI *pSub = pMenu->FindItem(nParentId);
			if (pSub)
			{
				CMenuUI *pPopSub = pSub->GetPopupMenu();
				if (!pPopSub)
				{
					pPopSub = new CMenuUI(); 
					pSub->AttachMenu(pPopSub);
				}
				if (pPopSub)
					return pPopSub->Add(pItem, 99999);
			} else
				return pMenu->Add(pItem, 99999);
		} else
			return pMenu->Add(pItem, 99999);
	}
	return FALSE;
}

//
BOOL CSmartWindow::MenuGetItemCaption(const TCHAR *szMenuName, int nMenuId, TCHAR *szwCaption, int *nSize)
{
	CMenuUI *pMenu = m_paintMgr.LoadMenuUI(szMenuName);
	if (pMenu)
	{
		CMenuItemUI *pItem = pMenu->FindItem(nMenuId);
		if (pItem)
		{
			return pItem->GetAttribute(L"text", szwCaption, *nSize); 
		}
	}
	return FALSE;
}

//
BOOL CSmartWindow::SetMenuItemAttr(const TCHAR *szMenuName, UINT uMenuId, const TCHAR *szAttrName, const TCHAR *szValue)
{
	CMenuUI *pMenu = m_paintMgr.LoadMenuUI(szMenuName);
	if (pMenu)
	{
		return pMenu->SetMenuItemAttr(uMenuId, szAttrName, szValue);
	}
	 
	return FALSE;
}

//
BOOL CSmartWindow::SetMenuChecked(const TCHAR *szMenuName, UINT uMenuID, BOOL bChecked)
{
	CMenuUI *pMenu = m_paintMgr.LoadMenuUI(szMenuName);
	if (pMenu)
	{
		return pMenu->CheckGroupExcept(NULL, uMenuID, bChecked);
	}
	return FALSE;
}

	//
BOOL CSmartWindow::AddAutoShortCut(const TCHAR *szCtrlName, const TCHAR *szCaption, 
	                 const TCHAR *szFileName, const int nImageId, const TCHAR *szTip, TCHAR *szFlag)
{
	CAutoShortCutVertList *pCtrl = static_cast<CAutoShortCutVertList *>(FindControl(szCtrlName));
	if (pCtrl)
		return pCtrl->AddShortCut(szCaption, szFileName, nImageId, szTip, szFlag);
	return FALSE;
}

#pragma warning(default:4996)
