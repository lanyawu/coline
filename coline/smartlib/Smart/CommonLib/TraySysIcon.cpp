#include <CommonLib/TraySysIcon.h>
#include <CommonLib/StringUtils.h>
#include <Commonlib/DebugLog.h>

#pragma warning(disable:4996)

CTraySysIcon::CTraySysIcon(HINSTANCE hInstance):
              m_OldWndProc(::DefWindowProc),
			  m_lpszTip(NULL),
			  m_bMouseEnter(FALSE),
			  m_hDefaultIcon(NULL),
			  m_hThread(NULL),
			  m_uptrMouseChk(100),
			  m_bAnimate(FALSE)
{
	m_hWnd = CreateWnd(hInstance);
	m_hWaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);  
}

CTraySysIcon::~CTraySysIcon(void)
{
	Hide();
	if (m_hDefaultIcon)
		::DeleteObject(m_hDefaultIcon);
	m_hDefaultIcon = NULL;
	if (m_lpszTip)
	{
		delete []m_lpszTip;
		m_lpszTip = NULL;
	}
	StopAnimate();
	::KillTimer(m_hWnd, m_uptrMouseChk);
	::PostMessage(m_hWnd, WM_QUIT, 0, 0);
	SetEvent(m_hWaitEvent);
	if (m_hThread)
	{
		::WaitForSingleObject(m_hThread, 5000);
        CloseHandle(m_hThread);
		m_hThread = NULL;
	}
	CloseHandle(m_hWaitEvent);
	m_hWaitEvent = NULL;
}

HWND CTraySysIcon::CreateWnd(HINSTANCE hInstance)
{
	WNDCLASS wc = { 0 };
	wc.style = 0;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hIcon = NULL;
	wc.lpfnWndProc = CTraySysIcon::__WndProc;
	wc.hInstance = NULL;
	wc.hCursor = NULL;
	wc.hbrBackground = NULL;
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = L"TRAYSYSICON";
	ATOM ret = ::RegisterClass(&wc);
	BOOL b = (ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
	if (b)
	{
		m_hWnd =  CreateWindowEx(WS_EX_TOOLWINDOW, L"TRAYSYSICON", NULL, WS_POPUP, 
			                     0, 0, 0, 0, NULL, NULL, hInstance, this);
	 
		return m_hWnd;
	} else
	   return NULL;
}

void CTraySysIcon::AddAnimateIcon(HICON hIcon)
{
	m_pIconList.push_back(hIcon);
}

HWND CTraySysIcon::GetHWND() const
{
	return m_hWnd;
}

void CTraySysIcon::SetTimerEnable(BOOL bEnable)
{
	if (m_hWnd)
	{
		if (bEnable)
			::SetTimer(m_hWnd, m_uptrMouseChk, 200, NULL);
		else
			::KillTimer(m_hWnd, m_uptrMouseChk);
	}
}

LRESULT CTraySysIcon::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   LRESULT lRes = NULL;
   if ((uMsg == TRAY_ICON_NOTIFYMESSAGE))
   { 
	   switch(lParam)
	   {
	   case WM_LBUTTONUP:
		   {
			   POINT  lpPos = {0};
			   int nShift = 0;
			   if (::GetKeyState(VK_SHIFT) < 0)
				   nShift |= SHIFT_STATE_SHIFT;
			   if (::GetKeyState(VK_CONTROL) < 0)
				   nShift |= SHIFT_STATE_CTRL;
			   if (::GetKeyState(VK_MENU) < 0)
				   nShift |= SHIFT_STATE_ALT;
			   ::GetCursorPos(&lpPos);
			   OnLButtonClick(nShift, lpPos.x, lpPos.y);
			   break;
		   }
	   case WM_RBUTTONUP:
		   {
			   POINT  lpPos = {0};
			   int nShift = 0;
			   if (::GetKeyState(VK_SHIFT) < 0)
				   nShift |= SHIFT_STATE_SHIFT;
			   if (::GetKeyState(VK_CONTROL) < 0)
				   nShift |= SHIFT_STATE_CTRL;
			   if (::GetKeyState(VK_MENU) < 0)
				   nShift |= SHIFT_STATE_ALT;
			   ::GetCursorPos(&lpPos);
			   OnRButtonClick(nShift, lpPos.x, lpPos.y);
			   break;
		   }
	   case WM_LBUTTONDBLCLK:
		   {
			   POINT  lpPos = {0};
			   int nShift = 0;
			   if (::GetKeyState(VK_SHIFT) < 0)
				   nShift |= SHIFT_STATE_SHIFT;
			   if (::GetKeyState(VK_CONTROL) < 0)
				   nShift |= SHIFT_STATE_CTRL;
			   if (::GetKeyState(VK_MENU) < 0)
				   nShift |= SHIFT_STATE_ALT;
			   ::GetCursorPos(&lpPos);
			   OnLButtonDblClick(nShift, lpPos.x, lpPos.y);
			   break;
		   }
	   case WM_MOUSEMOVE:
		   {
			   ::GetCursorPos(&m_ptEnter);
			   if (!m_bMouseEnter)
			   {
				   m_bMouseEnter = TRUE;
				   OnToolTipShow(TRUE);
				   SetTimerEnable(TRUE);
			   } 
			   break;
		   }
	   case WM_RBUTTONDBLCLK:
		   {
			   POINT  lpPos = {0};
			   int nShift = 0;
			   if (::GetKeyState(VK_SHIFT) < 0)
				   nShift |= SHIFT_STATE_SHIFT;
			   if (::GetKeyState(VK_CONTROL) < 0)
				   nShift |= SHIFT_STATE_CTRL;
			   if (::GetKeyState(VK_MENU) < 0)
				   nShift |= SHIFT_STATE_ALT;
			   ::GetCursorPos(&lpPos);
			   OnRButtonDblClick(nShift, lpPos.x, lpPos.y);
			   break;
		   }
	   case NIN_BALLOONSHOW:
		   {
			   OnBalloonShow();
			   break;
		   }
	   case NIN_BALLOONHIDE:
		   {
			   OnBalloonHide();
			   break;
		   }
	   case NIN_BALLOONTIMEOUT:
		   {
			   OnBalloonTimeout();
			   break;
		   }
	   case NIN_BALLOONUSERCLICK:
		   {
			   OnBalloonUserClick();
			   break;
		   }
	   }
   } else if (uMsg == WM_TIMER)
   {
	   if (wParam == m_uptrMouseChk)
	   {
		   if (m_bMouseEnter)
		   {
			   POINT pt = {0};
			   ::GetCursorPos(&pt);
			   if ((pt.x != m_ptEnter.x) || (pt.y != m_ptEnter.y))
			   {
				   m_bMouseEnter = FALSE;
				   OnToolTipShow(FALSE);
				   SetTimerEnable(FALSE);
			   }
		   } else
			   SetTimerEnable(FALSE);
	   }
   }
   return 0;
}

//
void CTraySysIcon::OnToolTipShow(BOOL bShow)
{
	//
}

//
void CTraySysIcon::OnBalloonShow()
{
}

//
void CTraySysIcon::OnBalloonHide()
{
}
//
void CTraySysIcon::OnBalloonTimeout()
{
}

//
void CTraySysIcon::OnBalloonUserClick()
{
}

LRESULT CALLBACK CTraySysIcon::__WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CTraySysIcon* pThis = NULL;
	if (uMsg == WM_NCCREATE) 
	{
		LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pThis = static_cast<CTraySysIcon *>(lpcs->lpCreateParams);
		pThis->m_hWnd = hWnd;
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG>(pThis));
	} else 
	{
		pThis = reinterpret_cast<CTraySysIcon *>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if (uMsg == WM_NCDESTROY && pThis != NULL)
		{
			LRESULT lRes = ::CallWindowProc(pThis->m_OldWndProc, hWnd, uMsg, wParam, lParam);
			::SetWindowLongPtr(pThis->m_hWnd, GWLP_USERDATA, 0L);
			pThis->m_hWnd = NULL;
			pThis->OnFinalMessage(hWnd);
			return lRes;
		}
	}
	if (pThis != NULL) 
	{
		//消息处理
		pThis->HandleMessage(hWnd, uMsg, wParam, lParam);
	} 
	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

DWORD WINAPI CTraySysIcon::AnimateThread(LPVOID lpParam)
{
	CTraySysIcon *pThis = (CTraySysIcon *)lpParam;
	BOOL bFlag = TRUE;
	while(pThis->m_bAnimate)
	{
		if (pThis->m_pIconList.size() > 1)
		{
			HICON hIcon = pThis->m_pIconList.front();
			pThis->m_pIconList.pop_front();
			pThis->m_pIconList.push_back(hIcon);
			pThis->NotifyIcon(NIM_MODIFY, hIcon);
		} else
		{
			if (bFlag)
				pThis->NotifyIcon(NIM_MODIFY, pThis->m_hDefaultIcon);
			else
				pThis->NotifyIcon(NIM_MODIFY, NULL);
			bFlag = !bFlag;
		}
		WaitForSingleObject(pThis->m_hWaitEvent, TRAY_ICON_ANIMATE_INVERTAL);
	}
	return 0;
}

	
//鼠标事件  虚函数
	
//左键单击
void CTraySysIcon::OnLButtonClick(int nShiftState,int nX, int nY)
{
	//
}
	
//右键单击
void CTraySysIcon::OnRButtonClick(int nShiftState,int nX, int nY)
{
	//
}
	
//左键双击
void CTraySysIcon::OnLButtonDblClick(int nShiftState,int nX, int nY)
{
	//
}
	
//右键双击
void CTraySysIcon::OnRButtonDblClick(int nShiftState, int nX, int nY)
{
	//
}

void CTraySysIcon::OnFinalMessage(HWND hWnd)
{
	
}

	
//外部调用函数 lpszTip提示信息 启动动画
BOOL CTraySysIcon::Animate(const char *lpszTip) 
{
	if (m_hThread)
		return FALSE;
	if (m_lpszTip)
	{
		delete []m_lpszTip;
		m_lpszTip = NULL;
	}
	if (lpszTip)
	{
		m_lpszTip = new char[strlen(lpszTip) + 1];
		memset(m_lpszTip, 0, strlen(lpszTip) + 1);
		strcpy(m_lpszTip, lpszTip);
	}
	m_bAnimate =  TRUE;
	m_hThread = ::CreateThread(NULL, 0, AnimateThread, this, 0, NULL);
	return m_hThread != NULL;
}
	
//设置提示信息
void CTraySysIcon::SetTip(const char *lpszTip, DWORD dwInfoFlags, const char *szTitle)
{
	if (lpszTip)
	{
		m_lpszTip = new char[strlen(lpszTip) + 1];
		memset(m_lpszTip, 0, strlen(lpszTip) + 1);
		strcpy(m_lpszTip, lpszTip);
		NotifyIcon(NIM_MODIFY, m_hDefaultIcon, dwInfoFlags, szTitle);
	}
}

//停止动画
BOOL CTraySysIcon::StopAnimate()
{
	if (m_hThread)
	{
		m_bAnimate = FALSE;
		SetEvent(m_hWaitEvent);
		WaitForSingleObject(m_hThread, 5000);
		CloseHandle(m_hThread);
		m_hThread = NULL;
		NotifyIcon(NIM_MODIFY, m_hDefaultIcon);
		//清除列表中图标
		std::list<HICON>::iterator it;
		for (it = m_pIconList.begin(); it != m_pIconList.end(); it ++)
		{
			if ((*it))
			{			
				::DeleteObject((*it));
			}
		}
		m_pIconList.clear();
		return TRUE;
	}
	return FALSE;
}

//显示图标
BOOL CTraySysIcon::NotifyIcon(DWORD dwMessage,  HICON hIcon, DWORD dwInfoFlags, const char *szTitle)
{
	memset(&m_NotifyIconData, 0, sizeof(NOTIFYICONDATAEX));
	m_NotifyIconData.cbSize = sizeof(NOTIFYICONDATAEX);
	m_NotifyIconData.hIcon = hIcon;
	m_NotifyIconData.hWnd = m_hWnd;
	m_NotifyIconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | 0x10 /*NIF_INFO*/;
	m_NotifyIconData.uCallbackMessage = TRAY_ICON_NOTIFYMESSAGE;
	m_NotifyIconData.uID = TRAY_ICON_ID;
	m_NotifyIconData.dwInfoFlags = dwInfoFlags | NIIF_USER;
	m_NotifyIconData.TimeoutOrVersion = 20 * 1000;  
	if (m_lpszTip)
	{
#ifdef _UNICODE
		WCHAR szTip[64] = {0};
		CStringConversion::StringToWideChar(m_lpszTip, m_NotifyIconData.szTip, 127);
#else
		memmove(m_NotifyIconData.szTip, m_lpszTip, 63);
#endif
	}
	if (szTitle)
		CStringConversion::StringToWideChar(szTitle, m_NotifyIconData.szInfoTitle, 63);
	return ::Shell_NotifyIcon(dwMessage, (PNOTIFYICONDATA)&m_NotifyIconData);
}

	
//显示图标
BOOL CTraySysIcon::Show()
{
	return NotifyIcon(NIM_ADD, m_hDefaultIcon);
}
	
//隐藏图标
BOOL CTraySysIcon::Hide()
{
	return NotifyIcon(NIM_DELETE, m_hDefaultIcon);
}

//设置默认图标
BOOL CTraySysIcon::SetDefaultData(HICON hIcon, const char *lpszTip)
{
	if (m_hDefaultIcon)
		::DeleteObject(m_hDefaultIcon);
	m_hDefaultIcon = hIcon;
	if (m_lpszTip)
	{
		delete []m_lpszTip;
		m_lpszTip = NULL;
	}
	if (lpszTip)
	{
		m_lpszTip = new char[strlen(lpszTip) + 1];
		memset(m_lpszTip, 0, strlen(lpszTip) + 1);
		strcpy(m_lpszTip, lpszTip);
	}
	NotifyIcon(NIM_MODIFY, m_hDefaultIcon);
	return FALSE;
}

#pragma warning(disable:4996)