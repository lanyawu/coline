#include <SmartSkin/SmartSkin.h>
#include <Commonlib/stringutils.h>
#include <Commonlib/systemutils.h>
#include <Commonlib/DebugLog.h>
#include "UIManagerImpl.h"


CUIManagerImpl::CUIManagerImpl(void):
                m_pCore(NULL) 
{
}


CUIManagerImpl::~CUIManagerImpl(void)
{
	ClearOrderAllMessage();
	if (m_pCore)
		m_pCore->Release();
	m_pCore = NULL;
}

//IUnknown
STDMETHODIMP CUIManagerImpl::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IUIManager)))
	{
		*ppv = (IUIManager *) this;
		_AddRef();
		return S_OK;
	} else if (IsEqualIID(riid, __uuidof(ICoreEvent)))
	{
		*ppv = (ICoreEvent *) this;
		_AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

//ICoreEvent
STDMETHODIMP CUIManagerImpl::DoCoreEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
	                     LPARAM lParam, HRESULT *hResult)
{
	return E_FAIL;
}

STDMETHODIMP CUIManagerImpl::SetCoreFrameWork(ICoreFrameWork *pCore)
{
	if (m_pCore)
		m_pCore->Release();
	m_pCore = pCore;
	if (m_pCore)
		m_pCore->AddRef();
	return S_OK;
}


STDMETHODIMP CUIManagerImpl::GetSkinXmlString(IAnsiString *szXmlString)
{
	return E_FAIL;
}

	//广播消息
STDMETHODIMP CUIManagerImpl::DoBroadcastMessage(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData)
{
	return E_NOTIMPL;
}

STDMETHODIMP CUIManagerImpl::CoreFrameWorkError(int nErrorNo, const char *szErrorMsg)
{
	return E_NOTIMPL;
}

STDMETHODIMP CUIManagerImpl::DoWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, 
	                              LPARAM lParam, LRESULT *lRes)
{
	return E_NOTIMPL;
}

//IUIManager
STDMETHODIMP CUIManagerImpl::AddPluginSkin(const char *szXmlString)
{
	if (::SkinAddPluginXML(szXmlString))
		return S_OK;
	return E_FAIL;
}

STDMETHODIMP CUIManagerImpl::CreateSkinByXmlStream(const char *szXml, const int nXmlSize, const char *szSkinPath)
{
	if (::SkinCreateFromStream(szXml, nXmlSize, szSkinPath))
		return S_OK;
	return E_FAIL;
}

STDMETHODIMP CUIManagerImpl::InitSkinXmlFile(const char *szXmlFile)
{
	if (::SkinCheckRunOption())
	{
		if (::SkinCreateFromFile(szXmlFile) == 0)
		{
			return S_OK;
		}
	}
	return E_FAIL;
}

STDMETHODIMP CUIManagerImpl::CreateUIWindow(HWND hParent, const char *szWindowName, const PRECT lprc, DWORD dwStyle,
		       DWORD dwExStyle, const TCHAR *szCaption, HWND *hWnd)
{
	HWND hTemp = ::SkinCreateWindowByName(szWindowName, szCaption, hParent, dwStyle, dwExStyle, lprc, 
		                 TRUE, WindowEventCallback, WindowMessageCallback, this);
	if (hTemp != NULL)
	{
		if (hWnd)
			*hWnd = hTemp;
		m_WinList.insert(std::pair<CAnsiString_, HWND>(szWindowName, hTemp));
		//
		if (m_pCore)
		{
			HRESULT hr = 0;
			m_pCore->DoCoreEvent(hTemp, szWindowName, "afterinit", szWindowName, 0, 0, &hr);
		}
		return S_OK;
	}
	
	return E_FAIL;
}

STDMETHODIMP CUIManagerImpl::ShowModalWindow(HWND hParent, const char *szWindowName, const TCHAR *szCaption, 
		           const int X, const int Y, const int nWidth, const int nHeight, int *nModalResult)
{
	RECT rc = {0};
	if ((X == 0) && (Y == 0))
	{
		RECT rcParent = {0};
		if (!::GetWindowRect(hParent, &rcParent))
		{
			CSystemUtils::GetScreenRect(&rcParent);
		} 
		rc.left = (rcParent.left + rcParent.right - nWidth) / 2;
		rc.top = (rcParent.top + rcParent.bottom - nHeight) / 2;
		if (rc.left < 0)
		{
			rc.left = 0;
		} else if (rc.top < 0)
		{
			rc.top = 0;
		} else
		{
			CSystemUtils::GetScreenRect(&rcParent);
			if ((rc.left + nWidth) > rcParent.right)
				rc.left = rcParent.right - nWidth - 10;
			if ((rc.top + nHeight) > rcParent.bottom)
				rc.top = rcParent.bottom - nHeight - 10;
		}
	} else
	{
		rc.left = X;
		rc.top = Y;
	}
	rc.right = rc.left + nWidth;
	rc.bottom = rc.top + nHeight;
	HWND h = NULL;
	HRESULT hr = CreateUIWindow(hParent, szWindowName, &rc, WS_POPUP | WS_SYSMENU,
				                WS_EX_TOOLWINDOW,  szCaption, &h);
	if (SUCCEEDED(hr))
	{
		DWORD dwResult = (int)::SkinShowModal(h);
		if (nModalResult)
			*nModalResult = (int) dwResult;
	}
	return hr;
}

STDMETHODIMP CUIManagerImpl::SendMessageToWindow(const char *szWndName, UINT uMsg, WPARAM wParam, 
		                            LPARAM lParam, LRESULT *hr)
{
	std::multimap<CAnsiString_, HWND>::iterator it = m_WinList.find(szWndName);
	for (;it != m_WinList.end(); it ++)
	{
		//只发一个
		*hr = SendMessage(it->second, uMsg, wParam, lParam);
		return S_OK;
	}
	return E_FAIL;
}

//订制消息
STDMETHODIMP CUIManagerImpl::OrderWindowMessage(const char *szWndName, HWND hWnd, UINT uMsg, ICoreEvent *pCore)
{
	std::multimap<CAnsiString_, HWND>::iterator it = m_WinList.find(szWndName);
	for (;it != m_WinList.end(); it ++)
	{
		if ((hWnd == NULL) || (hWnd == it->second))
		{
			::SkinOrderWindowMessage(it->second, uMsg);
			std::map<HWND, COrderWinMsgList *>::iterator OrderIt = m_OrderList.find(it->second);
			if (OrderIt != m_OrderList.end())
				OrderIt->second->AddOrderEvent(uMsg, pCore);
			else
			{
				COrderWinMsgList *pItem = new COrderWinMsgList();
				pItem->AddOrderEvent(uMsg, pCore);
				m_OrderList.insert(std::pair<HWND, COrderWinMsgList *>(it->second, pItem));
			} //end else if (OrderIt != m_OrderList.end())
		} //end if ((hWnd == NULL ...
	} //end for (;
	return S_OK;
}

//删除订制
STDMETHODIMP CUIManagerImpl::DeleteWindowMessage(const char *szWndName, UINT uMsg, ICoreEvent *pCore)
{
	return E_NOTIMPL;
}

STDMETHODIMP CUIManagerImpl::BlendSkinStyle(COLORREF cr)
{ 
	int r, g, b;
	r =  GetRValue(cr);
	g =  GetGValue(cr);
	b =  GetBValue(cr);
	if (::SkinBlendSkinStyle(r, g, b))
		return S_OK; 
	return E_FAIL;
}

STDMETHODIMP CUIManagerImpl::AlphaBackImage(const char *szFileName)
{
	if (::SkinMixSkinBackGround(szFileName))
		return S_OK;
	return E_FAIL;
}

STDMETHODIMP CUIManagerImpl::UIManagerRun()
{
	::SkinReInitApplicationRun();
	::SkinApplicationRun();
	return S_OK;
}

STDMETHODIMP CUIManagerImpl::GetWindowHWNDByName(const char *szWndName, HWND *hWnd)
{
	std::multimap<CAnsiString_, HWND>::iterator it = m_WinList.find(szWndName);
	if (it != m_WinList.end())
	{
		(*hWnd) = it->second;
		return S_OK;
	}
	return E_FAIL;
}

STDMETHODIMP CUIManagerImpl::ClearOrderAllMessage()
{
	std::map<HWND, COrderWinMsgList *>::iterator it;
	for (it = m_OrderList.begin(); it != m_OrderList.end(); it ++)
	{
		delete it->second;
	}
	m_OrderList.clear();
	return S_OK;
}


BOOL CALLBACK CUIManagerImpl::WindowEventCallback(HWND hWnd, LPCTSTR pstrEvent, LPCTSTR pstrWndName, LPCTSTR pstrControlName,
	                POINT *ptMouse, WPARAM wParam, LPARAM lParam, void *pOverlapped)
{
	CUIManagerImpl *pThis = (CUIManagerImpl *)pOverlapped;
	if (pThis->m_pCore)
	{
		HRESULT hr = -1;
		char szCtrlName[128] = {0};
		char szWndName[128] = {0};
		char szEvent[32] = {0};
		CStringConversion::WideCharToString(pstrControlName, szCtrlName, 127);
		CStringConversion::WideCharToString(pstrWndName, szWndName, 127);
		CStringConversion::WideCharToString(pstrEvent, szEvent, 31);
		pThis->m_pCore->DoCoreEvent(hWnd, szWndName, szEvent, szCtrlName, wParam, lParam, &hr); 
		if (hr == 0)
			return TRUE;
	}
	return FALSE;
}

BOOL CALLBACK CUIManagerImpl::WindowMessageCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, 
		                     LRESULT *lRes, void *pOverlapped)
{
	CUIManagerImpl *pThis = (CUIManagerImpl *)pOverlapped;

	//
	BOOL bDid = FALSE;
	std::map<HWND, COrderWinMsgList *>::iterator it = pThis->m_OrderList.find(hWnd);
	if (it != pThis->m_OrderList.end())
		bDid = it->second->DoWinMessage(hWnd, uMsg, wParam, lParam, lRes);
	 
	if (uMsg == WM_DESTROY)
	{
		PRINTDEBUGLOG(dtError, "start destoy windows");
		std::multimap<CAnsiString_, HWND>::iterator it;
		for (it = pThis->m_WinList.begin(); it != pThis->m_WinList.end();)
		{
			if (it->second == hWnd)
			{	
				it = pThis->m_WinList.erase(it);
				break;
			} else
				it ++;
		} //end for (it = pThis->m_WinList.begin();
		PRINTDEBUGLOG(dtError, "destroy window from winlist");
		std::map<HWND, COrderWinMsgList *>::iterator OrderIt = pThis->m_OrderList.find(hWnd);
		if (OrderIt != pThis->m_OrderList.end())
		{
			delete OrderIt->second;
			pThis->m_OrderList.erase(OrderIt);
		} //
		PRINTDEBUGLOG(dtError, "destroy window succ");
	} //end if (uMsg == WM_DESTROY)
	return bDid;
}

STDMETHODIMP CUIManagerImpl::GetControlText(HWND hWnd, const TCHAR *szCtrlName, TCHAR *szText, int *nSize)
{
	if (::SkinGetControlTextByName(hWnd, szCtrlName, szText, nSize))
		return S_OK;
	return E_FAIL;
}

STDMETHODIMP CUIManagerImpl::SetControlText(HWND hWnd, const TCHAR *szCtrlName, const TCHAR *szText)
{
	if (::SkinSetControlTextByName(hWnd, szCtrlName, szText))
		return S_OK;
	return E_FAIL;
}
