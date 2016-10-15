#include <Commonlib/systemutils.h>
#include <SmartSkin/smartskin.h>
#include "BMCFrameImpl.h"

#include "../IMCommonLib/InterfaceAnsiString.h"

#pragma warning(disable:4996)

CBMCFrameImpl::CBMCFrameImpl(void):
               m_pCore(NULL),
			   m_hWnd(NULL)
{
}


CBMCFrameImpl::~CBMCFrameImpl(void)
{
	if (m_hWnd)
		::SkinCloseWindow(m_hWnd);
	m_hWnd = NULL;
	if (m_pCore)
		m_pCore->Release();
	m_pCore = NULL;
}

//IUnknown
STDMETHODIMP CBMCFrameImpl::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if (::IsEqualGUID(riid, IID_IUnknown) || (::IsEqualGUID(riid, __uuidof(IBMCFrame))))
	{
		*ppv = (IBMCFrame *) this;
		_AddRef();
		return S_OK;
	} else if (::IsEqualGUID(riid, __uuidof(ICoreEvent)))
	{
		*ppv = (ICoreEvent *) this;
		_AddRef();
		return S_OK;
	} else if (::IsEqualGUID(riid, __uuidof(IProtocolParser)))
	{
		*ppv = (IProtocolParser *) this;
		_AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

HRESULT CBMCFrameImpl::DoMenuCommand(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szName, "systemmenu") == 0)
	{
		switch(wParam)
		{
		case 90006:
			 ShowBMCFrame(NULL);
			 break;
		}
	}
	return -1;
}

//ICoreEvent
STDMETHODIMP CBMCFrameImpl::DoCoreEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
	                     LPARAM lParam, HRESULT *hResult)
{
	if (::stricmp(szType, "menucommand") == 0)
	{
		*hResult = DoMenuCommand(hWnd, szName, wParam, lParam);
	} else
	{

	}
	return E_NOTIMPL;
}

STDMETHODIMP CBMCFrameImpl::DoBroadcastMessage(const char *szFromWndName, HWND hWnd, const char *szType, 
	                                           const char *szContent, void *pData)
{
	return E_NOTIMPL;
}

STDMETHODIMP CBMCFrameImpl::SetCoreFrameWork(ICoreFrameWork *pCore)
{
	if (m_pCore)
		m_pCore->Release();
	m_pCore = pCore;
	if (m_pCore)
	{
		m_pCore->AddRef();

		//Order 
		m_pCore->AddOrderEvent((ICoreEvent *) this, "BMCFrame", NULL, NULL);
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "systemmenu", "menucommand");
	}
	return S_OK;
}

STDMETHODIMP CBMCFrameImpl::GetSkinXmlString(IAnsiString *szXmlString)
{
	HRESULT hr = E_FAIL;
	IConfigure *pCfg = NULL; 
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{ 
		hr = pCfg->GetSkinXml("BMCFrame.xml",szXmlString); 
		pCfg->Release();
	}
	return hr;  
}

//
STDMETHODIMP CBMCFrameImpl::CoreFrameWorkError(int nErrorNo, const char *szErrorMsg)
{
	return E_NOTIMPL;
}

//
STDMETHODIMP CBMCFrameImpl::DoWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes)
{
	return E_NOTIMPL;
}

//IProtocolParser
STDMETHODIMP CBMCFrameImpl::DoRecvProtocol(const BYTE *pData, const LONG lSize)
{
	return E_NOTIMPL;
}

STDMETHODIMP CBMCFrameImpl::DoPresenceChange(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder)
{
	return E_NOTIMPL;
}

//IBMCFrame
STDMETHODIMP CBMCFrameImpl::ShowBMCFrame(LPRECT lprc)
{
	if (m_hWnd && ::IsWindow(m_hWnd))
	{
		if (::ShowWindow(m_hWnd, SW_SHOW))
		{
			CSystemUtils::BringToFront(m_hWnd); 
			return S_OK;
		} else
			return E_FAIL;
	} else
	{
		if (m_hWnd)
			::SkinCloseWindow(m_hWnd);
		m_hWnd = NULL;
		HRESULT hr = E_FAIL;
		if (m_pCore)
		{
			IUIManager *pUI = NULL;
			IConfigure *pCfg = NULL;		
			hr = m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg);
			if (SUCCEEDED(hr) && pCfg)
			{
				RECT rc = {100, 100, 700, 730};
				if (lprc)
					rc = *lprc;
				else
				{
					RECT rcSave = {0};
					CInterfaceAnsiString strPos;
					if (SUCCEEDED(pCfg->GetParamValue(FALSE, "Position", "BMCFrame", (IAnsiString *)&strPos)))
					{
						CSystemUtils::StringToRect(&rcSave, strPos.GetData());
					}
					if (!::IsRectEmpty(&rcSave))
						rc = rcSave;
				}
				hr = m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI);
				if (SUCCEEDED(hr) && pUI)
				{
					pUI->CreateUIWindow(NULL, "BMCFrame", &rc, WS_POPUP | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX,
					                0, L"", &m_hWnd);				
					if (::IsWindow(m_hWnd))
					{
						 ::ShowWindow(m_hWnd, SW_SHOW);
					}
					//pUI->OrderWindowMessage("ContactPanel", NULL, WM_DESTROY, (ICoreEvent *) this);
					//  
					pUI->Release();
					pUI = NULL;
				}
				pCfg->Release();
				pCfg = NULL;
			} //end if (SUCCEEDED(hr)... 
		} //end if (m_pCore)
		return hr;
	}
	return E_FAIL;
}

#pragma warning(default:4996)
