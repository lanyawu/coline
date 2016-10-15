#include "StdAfx.h"
#include <Commonlib/systemutils.h>
#include "ScreenDialog.h"
 

COLORREF  SCREEN_INFO_EDGECOLOR	    = RGB(255, 142, 0);
COLORREF  SELECT_RECTCOLOR		    = RGB(87, 133, 52);
#define DISPLAY_INFO_RECT_OFFSET 5   //显示信息框的偏移
#define DISPLAY_INFO_RECT_WIDTH  235 //显示信息框的宽度
#define DISPLAY_INFO_RECT_HEIGHT 147 //显示信息框的高度
#define DISPLAY_INFO_TEXT_OFFSET 24  //文字偏移
#define DISPLAY_INFO_TEXT_H_OFF  48  //垂直偏移
#define DISPLAY_INFO_TEXT_HEIGHT 22  //文字高度

#pragma warning(disable:4996)

CScreenDialog::CScreenDialog(void):
               m_hMem(NULL),
			   m_hScreenBitmap(NULL),
			   m_hBkBitmap(NULL),
			   m_OldWndProc(::DefWindowProc),
			   m_hWnd(NULL),
			   m_Status(csNormal),
			   m_hSelected(NULL),
			   m_hOldBitmap(NULL)
{
	m_ptStart.x = 0;
	m_ptStart.y = 0;
	m_ptCurr.x = 0;
	m_ptCurr.y = 0;
}

CScreenDialog::~CScreenDialog(void)
{
	if (m_hMem)
	{
		if (m_hScreenBitmap && m_hOldBitmap)
		{
			::SelectObject(m_hMem, m_hOldBitmap);
			::DeleteObject(m_hScreenBitmap);
			m_hScreenBitmap = NULL;
			m_hOldBitmap = NULL;
		}
		::DeleteObject(m_hMem);
	}
	if (m_hBkBitmap)
		::DeleteObject(m_hBkBitmap);
	m_hBkBitmap = NULL;
	if (m_hSelected)
		::DeleteObject(m_hSelected);
	m_hSelected = NULL;
}

BOOL CScreenDialog::RegisterWindowClass(HINSTANCE hRes)
{
	WNDCLASS wc = { 0 };
	wc.style = GetClassStyle();
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hIcon = NULL;
	wc.lpfnWndProc = CScreenDialog::__WndProc;
	wc.hInstance = hRes;
	wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = GetWindowClassName();
	ATOM ret = ::RegisterClass(&wc); 
	return ((ret != NULL) || (::GetLastError() == ERROR_CLASS_ALREADY_EXISTS));
}

LRESULT CALLBACK CScreenDialog::__WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CScreenDialog *pThis = NULL;
	if (uMsg == WM_NCCREATE)
	{
		LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pThis = static_cast<CScreenDialog *>(lpcs->lpCreateParams);
		pThis->m_hWnd = hWnd;
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(pThis));
	} else
	{
		pThis = reinterpret_cast<CScreenDialog *>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if ((uMsg == WM_NCDESTROY) && (pThis != NULL)) 
		{
			if (!::IsWindowEnabled(pThis->m_hWnd))
			{ 
				::PostMessage(pThis->m_hWnd, WM_NCDESTROY, wParam, lParam);
				return 0;
			} else
			{
				LRESULT lRes = ::CallWindowProc(pThis->m_OldWndProc, hWnd, uMsg, wParam, lParam);
				::SetWindowLongPtr(pThis->m_hWnd, GWLP_USERDATA, 0L); 
				pThis->m_hWnd = NULL;
				pThis->OnFinalMessage(hWnd);
				return lRes;
			}
		}  else if ((uMsg == WM_CLOSE) && (pThis != NULL))
		{
			 
		}
	}
	if (pThis != NULL)
	{
		return pThis->HandleMessage(uMsg, wParam, lParam);
	} else 
	{
		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

int CScreenDialog::CaptureScreen(HINSTANCE hRes, HWND hParent, BOOL bHideParent)
{
	if (!RegisterWindowClass(hRes))
		return FALSE;
	//初始化参数
 	if (bHideParent)
	{
		::ShowWindow(hParent, SW_HIDE);
		::Sleep(500);
	}
	m_Status = csNormal;
	m_sizeScreen.cx = ::GetSystemMetrics(SM_CXSCREEN);
	m_sizeScreen.cy = ::GetSystemMetrics(SM_CYSCREEN);
	SetCursor(LoadCursor(NULL,IDC_WAIT));
	HDC hdc = ::GetDC(::GetDesktopWindow());
	
	m_hMem = ::CreateCompatibleDC(hdc);
	m_hScreenBitmap = ::CreateCompatibleBitmap(hdc, m_sizeScreen.cx, m_sizeScreen.cy);
	m_hOldBitmap = (HBITMAP)::SelectObject(m_hMem, m_hScreenBitmap);
	if (bHideParent)
	{	
		::BitBlt(m_hMem, 0, 0, m_sizeScreen.cx, m_sizeScreen.cy, hdc, 0, 0, SRCCOPY);
	} else
		::BitBlt(m_hMem, 0, 0, m_sizeScreen.cx, m_sizeScreen.cy, hdc, 0, 0, SRCCOPY | CAPTUREBLT);
	::ReleaseDC(::GetDesktopWindow(), hdc);
	SetCursor(LoadCursor(NULL,IDC_ARROW));
	RECT rc = {0, 0, m_sizeScreen.cx, m_sizeScreen.cy};
	m_hWnd = ::CreateWindowEx(WS_EX_TOOLWINDOW, GetWindowClassName(),  L"SCREENCAPTURE", 
		WS_POPUP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 
		hParent, NULL, hRes, this); 
	//get background image m_hBkBitmap 
	m_hBkBitmap = ::LoadBitmap(hRes, L"BKG");
	if (bHideParent)
	{
		::ShowWindow(hParent, SW_SHOW);
	}
	return ShowModal();
}

LPCTSTR CScreenDialog::GetWindowClassName() const
{
	return L"screendialog";
}

UINT CScreenDialog::GetClassStyle() const
{
	return CS_DBLCLKS;
}

void CScreenDialog::Close()
{
	if (!::IsWindow(m_hWnd)) 
		return;
	::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
}

int CScreenDialog::ShowModal()
{
	::ShowWindow(m_hWnd, SW_SHOWNORMAL);
	CSystemUtils::BringToFront(m_hWnd);
	HWND hWndParent = GetWindowOwner(m_hWnd);
	::EnableWindow(hWndParent, FALSE);

	MSG msg = { 0 };
	int nResult = IDCANCEL;
	BOOL bRet = 0;
	while ((bRet = ::GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (bRet == -1)
		{
			nResult = 0;
		} else
		{
			if ((msg.message == WM_CLOSE) && (msg.hwnd == m_hWnd))
			{
				nResult = m_nModalResult;
				//this is must be called before the dialog is destroyed,
				//otherwise the parent window will lose focus
				::EnableWindow(hWndParent, TRUE); 
			} 
			//dispatch messages
			::TranslateMessage(&msg);
			::DispatchMessage(&msg); 
		}

		if (!::IsWindow(m_hWnd))
			break;
	}
	//when system command is received( ALT+F4 ), the msg.message == WM_CLOSE
	//will never occur.
	if (msg.message ==  WM_QUIT)
	{
		::PostQuitMessage((int)msg.wParam);
	}
	return nResult;
}

LRESULT CScreenDialog::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_LBUTTONDBLCLK:
		{
			if (m_Status == csSelected)
			{
				::OpenClipboard(m_hWnd);
				::EmptyClipboard(); //先清空剪切板
				::SetClipboardData(CF_BITMAP, m_hSelected);
				::CloseClipboard();
				m_nModalResult = IDOK;
				Close();
			}
			break;
		}
	case WM_LBUTTONDOWN:
		{
			if (m_Status == csNormal)
			{
				m_ptStart.y = lParam >> 16 & 0x0000FFFF;
				m_ptStart.x = lParam & 0x0000FFFF;
				m_ptCurr = m_ptStart;
				m_Status = csSelecting;
				DisplayInfo();
			}
			break;
		}
	case WM_LBUTTONUP:
		{
			if (m_Status == csSelecting)
			{
				m_ptCurr.y = lParam >> 16 & 0x0000FFFF;
				m_ptCurr.x = lParam & 0x0000FFFF;
				if (m_hSelected)
				{
					::DeleteObject(m_hSelected);
					m_hSelected = NULL;
				}
				int nDisX = ::abs(m_ptCurr.x - m_ptStart.x);
				int nDisY = ::abs(m_ptCurr.y - m_ptStart.y);
				if ((nDisX > MIN_SELECT_RECT) && (nDisY > MIN_SELECT_RECT))
				{
					POINT ptTop;
					if (m_ptCurr.x < m_ptStart.x)
					{
						ptTop.x = m_ptCurr.x;
					} else
					{
						ptTop.x = m_ptStart.x;
					}
					if (m_ptCurr.y < m_ptStart.y)
						ptTop.y = m_ptCurr.y;
					else
						ptTop.y = m_ptStart.y;
					m_hSelected = ::CreateCompatibleBitmap(m_hMem, nDisX, nDisY);
					HDC hdc = ::CreateCompatibleDC(m_hMem);
					HBITMAP hOld = (HBITMAP)::SelectObject(hdc, m_hSelected);
					::StretchBlt(hdc, 0, 0, nDisX, nDisY, m_hMem, ptTop.x, ptTop.y, nDisX, nDisY, SRCCOPY);
					::SelectObject(hdc, hOld);
					::DeleteObject(hdc);
					m_Status = csSelected;
				}				
			}
			break;
		}
	case WM_MOUSEMOVE:
		{
			if (m_Status != csSelected)
			{
				m_ptCurr.y = lParam >> 16 & 0x0000FFFF;
				m_ptCurr.x = lParam & 0x0000FFFF;
			}
			DisplayInfo();
			break;
		}
	case WM_RBUTTONUP:
		{
			m_nModalResult = IDCANCEL;
			if (m_Status == csSelected)
				m_Status = csNormal;
			else
				Close();
			break;
		}
	case WM_PAINT:
		{
			if(m_hMem)
			{
				HDC hdc = ::GetDC(m_hWnd);
				::BitBlt(hdc, 0, 0, m_sizeScreen.cx, m_sizeScreen.cy, m_hMem, 0, 0, SRCCOPY);
				::ReleaseDC(m_hWnd, hdc);
			}
			break;
		}
	case WM_KEYUP:
		{
			WORD wCharCode = wParam & 0x0000FFFF;
			if (wCharCode == VK_ESCAPE)
			{
				m_nModalResult = IDCANCEL;
				if (m_Status == csSelected)
					m_Status = csNormal;
				else
					Close();
			}
			break;
		}
	}
	return ::CallWindowProc(m_OldWndProc, m_hWnd, uMsg, wParam, lParam);
}

void CScreenDialog::OnFinalMessage(HWND hWnd)
{
	delete this;
}

void CScreenDialog::DisplayInfo()
{
	RECT rc = {DISPLAY_INFO_RECT_OFFSET, DISPLAY_INFO_RECT_OFFSET, 
		       DISPLAY_INFO_RECT_OFFSET + DISPLAY_INFO_RECT_WIDTH,
		       DISPLAY_INFO_RECT_OFFSET + DISPLAY_INFO_RECT_HEIGHT};

	if ((m_ptCurr.x < DISPLAY_INFO_RECT_WIDTH + DISPLAY_INFO_RECT_OFFSET) &&
		(m_ptCurr.y < DISPLAY_INFO_RECT_HEIGHT + DISPLAY_INFO_RECT_OFFSET))
	{
		rc.left = m_sizeScreen.cx - DISPLAY_INFO_RECT_OFFSET - DISPLAY_INFO_RECT_WIDTH;
	    rc.top  = DISPLAY_INFO_RECT_OFFSET;
	    rc.right = m_sizeScreen.cx - DISPLAY_INFO_RECT_OFFSET;
		rc.bottom = DISPLAY_INFO_RECT_OFFSET + DISPLAY_INFO_RECT_HEIGHT;
	}

	HDC hdc = ::GetDC(m_hWnd);
	COLORREF crCurr = ::GetPixel(hdc, m_ptCurr.x, m_ptCurr.y);
    
	HDC hMem = ::CreateCompatibleDC(hdc); //内存DC
	HBITMAP hNewBitmap = ::CreateCompatibleBitmap(hdc, m_sizeScreen.cx, m_sizeScreen.cy);
	HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hMem, hNewBitmap);
	::BitBlt(hMem, 0, 0, m_sizeScreen.cx, m_sizeScreen.cy, m_hMem, 0, 0, SRCCOPY);
    
	//Draw background
	HBRUSH hBkBrush = ::CreatePatternBrush(m_hBkBitmap);
	HPEN hBkPen = ::CreatePen(PS_SOLID, 1, SCREEN_INFO_EDGECOLOR);
	HPEN hOldPen = (HPEN) ::SelectObject(hMem, hBkPen);
	HBRUSH hOldBrush = (HBRUSH) ::SelectObject(hMem, hBkBrush);
	::SetBrushOrgEx(hMem, rc.left, rc.top, NULL);
	::Rectangle(hMem, rc.left, rc.top, rc.right, rc.bottom);
	::SelectObject(hMem, hOldPen);
	::SelectObject(hMem, hOldBrush);
	::DeleteObject(hBkBrush);
	::DeleteObject(hBkPen);

	TCHAR szTemp1[128] = {0};
	TCHAR szTemp2[128] = {0};
	TCHAR szTemp3[128] = {0};
	switch(m_Status)
	{
		case csNormal:
			{
				::swprintf(szTemp1, 128, _T("当前颜色： 红:%d, 绿:%d, 蓝:%d"), 
					GetRValue(crCurr), GetGValue(crCurr), GetBValue(crCurr));
				::swprintf(szTemp2, 128, _T("按下鼠标左键开始选取截取区域"));
				::swprintf(szTemp3, 128, _T("按ESC键，或者鼠标右键退出选取"));
				break;
			}
		case csSelecting:
			{
				::swprintf(szTemp1, 128, _T("顶点：%d, %d 当前位置： %d, %d "), m_ptStart.x,
					m_ptStart.y, m_ptCurr.x, m_ptCurr.y);
				::swprintf(szTemp2, 128, _T("选取区域宽度：%d  高度：%d"), abs(m_ptCurr.x - m_ptStart.x), 
					abs(m_ptCurr.y - m_ptStart.y) );
				::swprintf(szTemp3, 128, _T("松开鼠标确定选取区域"));
				break;
			} 
		case csSelected:
			{
				::swprintf(szTemp1, 128, _T("ESC键取消当前选择"));
				::swprintf(szTemp2, 128, _T("点击鼠标右键取消当前选择"));
				::swprintf(szTemp3, 128, _T("双击鼠标左键选择当前图片"));
				break;
			}
	}
	HFONT hFont = ::CreateFont(-::MulDiv(8, GetDeviceCaps(hMem, LOGPIXELSY), 72), 0, 0, 0, FW_NORMAL, 0, 0, 0, 
		                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, 0, L"tahoma");
	HFONT hOldFont = (HFONT)::SelectObject(hMem, hFont);
	::SetBkMode(hMem, TRANSPARENT);
	::SetTextColor(hMem, RGB(255, 25, 255));
	::TextOut(hMem, DISPLAY_INFO_TEXT_OFFSET + rc.left, DISPLAY_INFO_TEXT_H_OFF, szTemp1, ::lstrlen(szTemp1));
	::TextOut(hMem, DISPLAY_INFO_TEXT_OFFSET + rc.left, DISPLAY_INFO_TEXT_H_OFF + DISPLAY_INFO_TEXT_HEIGHT, 
		szTemp2, ::lstrlen(szTemp2));
	::TextOut(hMem, DISPLAY_INFO_TEXT_OFFSET + rc.left, DISPLAY_INFO_TEXT_H_OFF + DISPLAY_INFO_TEXT_HEIGHT * 2, 
		szTemp3, ::lstrlen(szTemp3));
	::SelectObject(hMem, hOldFont);
	::DeleteObject(hFont);
	//绘制选择区域
	if (m_Status != csNormal)
	{
		HPEN hLinePen = ::CreatePen(PS_SOLID, 1, SELECT_RECTCOLOR);
		HPEN hOldPen = (HPEN)::SelectObject(hMem, hLinePen);
		::MoveToEx(hMem, m_ptStart.x, m_ptStart.y, NULL);
		::LineTo(hMem, m_ptCurr.x, m_ptStart.y);
		::LineTo(hMem, m_ptCurr.x, m_ptCurr.y);
		::LineTo(hMem, m_ptStart.x, m_ptCurr.y);
		::LineTo(hMem, m_ptStart.x, m_ptStart.y);
		::SelectObject(hMem, hOldPen);
		::DeleteObject(hLinePen);
	}
    
	::StretchBlt(hdc, 0, 0, m_sizeScreen.cx, m_sizeScreen.cy, hMem, 0, 0, m_sizeScreen.cx, 
		m_sizeScreen.cy, SRCCOPY);
	::SelectObject(hMem, hOldBitmap);
	::DeleteObject(hNewBitmap);
	::DeleteObject(hMem);
	::ReleaseDC(m_hWnd, hdc);
}

#pragma warning(default:4996)
