#include "SmartTipWnd.h"
#include "SmartUIResource.h"

const TCHAR TIP_SHOW_CONTROL_NAME[] = L"tipshow";

const int TOOLTIP_MIN_WIDTH = 20;
const int TOOLTIP_MIN_HEIGHT = 10;
const int TOOLTIP_MAX_WIDTH  = 300;
const int TOOLTIP_MAX_HEIGHT = 200;

CSmartTipWnd::CSmartTipWnd(void):
              CSmartWindow(NULL, SMART_TOOLTIP_WINDOW_NODE_NAME),
              m_edtShow(NULL)    
{
	//ÔØÈë±³¾°Í¼Æ¬
	if (m_paintMgr.GetHintWindowBkgImageId() > 0)
	{
		LPUI_IMAGE_ITEM pImage;
		if (CSmartUIResource::Instance()->GetImageById(m_paintMgr.GetHintWindowBkgImageId(), &pImage))
		{
			m_graphBkgnd.LoadFromGraphic(pImage->pGraphic);
		}
	} 
}


CSmartTipWnd::~CSmartTipWnd(void)
{
	//
}

//INotifyUI overridable
void CSmartTipWnd::Notify(TNotifyUI& msg)
{
	//
}

void CSmartTipWnd::SetParent(CWindowWnd *pParent)
{
	//
}

void CSmartTipWnd::SetText(LPCTSTR szTitle)
{
	//
}

std::string CSmartTipWnd::GetWindowName()
{
	return SMART_TOOLTIP_WINDOW_NODE_NAME;
}

//CWindowWnd overridable
LPCTSTR CSmartTipWnd::GetWindowClassName() const
{
	return SMART_TOOLTIP_WINDOW_NAME;
}
 

BOOL CSmartTipWnd::OnTrackActivate(WPARAM wParam, LPARAM lParam)
{
	BOOL bShow = (BOOL) wParam;
	LPTOOLINFO pInfo = (LPTOOLINFO)lParam;
	if (bShow)
	{
		//calculate hint window position
		POINT poMouse = {0};
		::GetCursorPos(&poMouse); 
		::MoveWindow(m_hWnd, poMouse.x, poMouse.y, pInfo->rect.right - pInfo->rect.left,
			pInfo->rect.bottom - pInfo->rect.top, FALSE);
		if (m_edtShow)
		{
			m_edtShow->SetText(pInfo->lpszText);
		}
		if (!::ShowWindow(m_hWnd, SW_SHOWNORMAL))
		{
			//debug
			DWORD dwError = ::GetLastError();
			printf("%d", dwError);
		}
	} else
	{
		::ShowWindow(m_hWnd, SW_HIDE);
	}
	return TRUE;
}

 

LRESULT CSmartTipWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL bHandled = FALSE;
	LRESULT lRes = 0;
	switch(uMsg)
	{
    	case TTM_TRACKACTIVATE:
			bHandled = OnTrackActivate(wParam, lParam);
			break;
		default:
			break;
	}
	//default message handling
	if (bHandled) 
	{
		return lRes;
	} else  
		return CSmartWindow::HandleMessage(uMsg, wParam, lParam);
}
 
BOOL CSmartTipWnd::Init()
{
	m_edtShow = dynamic_cast<CRichEditUI *>(m_paintMgr.FindControl(TIP_SHOW_CONTROL_NAME));
	if (m_edtShow)
	{
		//init richedit
		m_edtShow->SetReadOnly(TRUE);
		m_edtShow->SetAutoDetectLink(true);
		m_edtShow->SetBkgndColor(0);
		m_edtShow->SetEnabled(false);
	}
	return TRUE;
}

 