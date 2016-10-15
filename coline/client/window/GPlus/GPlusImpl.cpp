#include <SmartSkin/smartskin.h>
#include <Commonlib/systemutils.h>
#include <Commonlib/DebugLog.h>
#include "GPlusImpl.h"
#include "../IMCommonLib/InterfaceUserList.h"
#include "../IMCommonLib/InterfaceAnsiString.h"
#include <Core/common.h>

#pragma warning(disable:4996)

CGPlusImpl::CGPlusImpl(void):
            m_pCore(NULL),
			m_hSearchBar(NULL),
			m_hDesktop(NULL),
			m_hAppDesktop(NULL),
			m_hMainToolBar(NULL)
{
}


CGPlusImpl::~CGPlusImpl(void)
{
	if (m_hSearchBar)
		::SkinCloseWindow(m_hSearchBar);
	if (m_hMainToolBar)
		::SkinCloseWindow(m_hMainToolBar);
	if (m_hDesktop)
		::SkinCloseWindow(m_hDesktop);
	if (m_hAppDesktop)
		::SkinCloseWindow(m_hAppDesktop);
	m_hAppDesktop = NULL;
	m_hDesktop = NULL;
	m_hMainToolBar = NULL;
	m_hSearchBar = NULL;
	if (m_pCore)
		m_pCore->Release();
	m_pCore = NULL;
}

STDMETHODIMP CGPlusImpl::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IGPlus)))
	{
		*ppv = (IGPlus *) this;
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

//设置透明
BOOL SetWindowTransparent(HWND hWnd, COLORREF crKey, BYTE Alpha, int FLAG)
{
	if ((hWnd != NULL) && ::IsWindow(hWnd))
	{
		LONG lExStyle = ::GetWindowLong(hWnd, GWL_EXSTYLE);
		lExStyle |= WS_EX_LAYERED;
		::SetWindowLong(hWnd, GWL_EXSTYLE, lExStyle);
		::SetLayeredWindowAttributes(hWnd, crKey, Alpha, FLAG); 
	}
	return FALSE;
}

BOOL CGPlusImpl::AddWidgetToUI(HWND hWnd, const char *szTabId, const char *szCaption, const char *szUrl, 
		const char *szTip, const int nImageId, const char *szImageFile)
{
	if (szUrl == NULL)
		return FALSE;
	TCHAR szwTabId[MAX_PATH] = {0}, szwCaption[MAX_PATH] = {0}, szwUrl[MAX_PATH] = {0}, szwImageFile[MAX_PATH] = {0};
	TCHAR szwTip[MAX_PATH] = {0}, szwFlag[MAX_PATH] = {0};

	if (szTabId)
		CStringConversion::StringToWideChar(szTabId, szwTabId, MAX_PATH - 1);
	if (szCaption)
		CStringConversion::StringToWideChar(szCaption, szwCaption, MAX_PATH - 1); 
	if (szImageFile)
		CStringConversion::StringToWideChar(szImageFile, szwImageFile, MAX_PATH - 1);
	if (szTip)
		CStringConversion::StringToWideChar(szTip, szwTip, MAX_PATH - 1);
 
	if (::SkinAddAutoShortCut(hWnd, szwTabId, szwCaption, szwImageFile, nImageId, szwTip, szwFlag))
	{
		char szFlag[MAX_PATH] = {0};
		CStringConversion::WideCharToString(szwFlag, szFlag, 127);
		m_Plugins[szFlag] = szUrl;
		return TRUE;
	}
	return FALSE;
}

void CGPlusImpl::LoadRecentlyUsers()
{
	IConfigure *pCfg = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		CInterfaceUserList recentlyList;
		if (SUCCEEDED(pCfg->GetRecentlyList((IUserList *)&recentlyList)))
		{
			void *pSaveNode = NULL;
			std::string strOrderXml;
			//add user node
			char szDisplayText[512] = {0};
	        LPORG_TREE_NODE_DATA pData = NULL;
			CInterfaceAnsiString strStatus;
		    CInterfaceAnsiString strSign;
			IContacts *pContacts = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContacts)))
		    {			 
				CInterfaceAnsiString strHeaderFile; 
				std::string strUrl;
				while (SUCCEEDED(recentlyList.PopFrontUserInfo(&pData)))
				{
					memset(szDisplayText, 0, 512);
					CStringConversion::UTF8ToString(pData->szDisplayName, szDisplayText, 511);
					strUrl = "chat://";
					strUrl += pData->szUserName;
					if (SUCCEEDED(pContacts->GetContactHead(pData->szUserName, &strHeaderFile, FALSE)))
					{ 
						AddWidgetToUI(m_hDesktop, "oftenusing", szDisplayText, strUrl.c_str(), szDisplayText, 0, strHeaderFile.GetData());
					} else
						AddWidgetToUI(m_hDesktop, "oftenusing", szDisplayText, strUrl.c_str(), szDisplayText, 35, ""); 
					delete pData; 
					 
				} //end while (SUCCEEDED( 
				pContacts->Release();
			}  
		} 
		pCfg->Release();
	}
}

void CGPlusImpl::ShowAppDesktop()
{
	HideAllDesktop();
	IUIManager *pUI = NULL; 
	RECT rcScreen = {0};
	SystemParametersInfo(SPI_GETWORKAREA,0,&rcScreen,0); 
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
	{
		//创建main
		if (m_hAppDesktop == NULL)
	    {   
			rcScreen.top += 40; 
			HWND hDesktop = CSystemUtils::FindDesktopWindow();
			if (SUCCEEDED(pUI->CreateUIWindow(hDesktop, "appdesktop", &rcScreen, WS_POPUP , WS_EX_TOOLWINDOW,
					L"", &m_hAppDesktop)))
			{ 
				//SetWindowTransparent(m_hAppDesktop, RGB(255,255,255), 32, 1/*LWA_ALPHA */);
				ShowWindow(m_hAppDesktop, SW_SHOW); 
				LoadUserApplications();
			} else
			{
				PRINTDEBUGLOG(dtInfo, "Create GoComDesktop Failed");
			}
		} else
		{
			ShowWindow(m_hAppDesktop, SW_SHOW);
		}
		pUI->Release();
	}
}

void CGPlusImpl::HideAllDesktop()
{
	if (m_hDesktop)
		::ShowWindow(m_hDesktop, SW_HIDE);
	if (m_hAppDesktop)
		::ShowWindow(m_hAppDesktop, SW_HIDE);
}

void CGPlusImpl::LoadUserApplications()
{
	char szAppPath[MAX_PATH] = {0};
	CSystemUtils::GetSystemDirectory(szAppPath, MAX_PATH - 1);
	std::string strTmp = szAppPath;
	strTmp += "\\notepad.exe";
	AddWidgetToUI(m_hAppDesktop, "oftenusing", "记事本", strTmp.c_str(), "记事本", 0, strTmp.c_str());
	strTmp = szAppPath;
	strTmp += "\\calc.exe";
	AddWidgetToUI(m_hAppDesktop, "oftenusing", "计算器", strTmp.c_str(), "计算器", 0, strTmp.c_str());
}
 
BOOL CALLBACK EnumUserWindowsCB_(HWND hWnd, LPARAM lParam)
{
	LONG lFlags = ::GetWindowLong(hWnd, GWL_STYLE);
	if ((lFlags & WS_VISIBLE) == 0)
		return TRUE;
 
	HWND hSndWnd = ::FindWindowEx(hWnd, NULL, L"SHELLDLL_DefView", NULL);
	if (hSndWnd == NULL)
		return TRUE;
	HWND hTargetWnd = ::FindWindowEx(hSndWnd, NULL, L"SysListView32", L"FolderView");
	if (hTargetWnd == NULL)
		return TRUE;
	HWND *h = (HWND *)lParam;
	*h = hSndWnd;
	return FALSE;
}


HWND FindDesktopWindow()
{ 
	HWND hWnd = NULL;
	EnumWindows(EnumUserWindowsCB_, (LPARAM) &hWnd);
	return hWnd; 
}

void CGPlusImpl::ShowDesktop()
{
	HideAllDesktop();
	IUIManager *pUI = NULL; 
	RECT rcScreen = {0};
	SystemParametersInfo(SPI_GETWORKAREA,0,&rcScreen,0); 
		
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
	{
		//创建main
		if (m_hDesktop == NULL)
	    {   
			rcScreen.top += 40; 
			HWND hDesktop = FindDesktopWindow();
			if (SUCCEEDED(pUI->CreateUIWindow(hDesktop, "GoComDesktop", &rcScreen, WS_POPUP , WS_EX_TOOLWINDOW,
					L"", &m_hDesktop)))
			{ 
				//SetWindowTransparent(m_hDesktop, RGB(255,255,255), 32, 1/*LWA_ALPHA */);
				ShowWindow(m_hDesktop, SW_SHOW); 
				LoadRecentlyUsers();
			} else
			{
				PRINTDEBUGLOG(dtInfo, "Create GoComDesktop Failed");
			}
		} else
		{
			ShowWindow(m_hDesktop, SW_SHOW);
		}
		pUI->Release();
	}
}

//ICoreEvent
STDMETHODIMP CGPlusImpl::DoCoreEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
		                    LPARAM lParam, HRESULT *hResult)
{
	if (::stricmp(szType, "click") == 0)
	{
		if (::stricmp(szName, "btnDesktop") == 0)
		{
			ShowDesktop();
		} else if (::stricmp(szName, "btnApp") == 0)
		{
			ShowAppDesktop();
		} else
		{
			std::map<std::string, std::string>::iterator it = m_Plugins.find(szName);
			if (it != m_Plugins.end())
			{
				if (::strnicmp(it->second.c_str(), "chat://", strlen("chat://")) == 0)
				{
					 std::string strUserName = it->second.substr(strlen("chat://"));
					 if (strUserName.find('@') != std::string::npos) //P2P聊天
					{
						IChatFrame *pChat = NULL;
						if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pChat)))
						{
							pChat->ShowChatFrame(NULL, strUserName.c_str(), NULL);
							pChat->Release();
						}
					} else //分组
					{
						IGroupFrame *pFrame = NULL;
						if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IGroupFrame), (void **)&pFrame)))
						{
							pFrame->ShowGroupFrame(strUserName.c_str(), NULL);
							pFrame->Release();
						} //end if (m_pCore
					} //end if (strId.find(...
				} else
				{
					TCHAR szwTmp[MAX_PATH] = {0};
					CStringConversion::StringToWideChar(it->second.c_str(), szwTmp, MAX_PATH - 1);
					::ShellExecute(NULL, L"open", szwTmp, NULL, NULL, SW_SHOW);
				}//end else if (::stricmp(it->second... 
			} //end if (it != m_Plugins.end())
		}
	}
	return E_NOTIMPL;
}

STDMETHODIMP CGPlusImpl::SetCoreFrameWork(ICoreFrameWork *pCore)
{
	if (m_pCore)
		m_pCore->Release();
	m_pCore = pCore;
	if (m_pCore)
	{
		m_pCore->AddRef();
        //
		m_pCore->AddOrderEvent((ICoreEvent *) this, "GPlusToolBar", NULL, NULL);
		m_pCore->AddOrderEvent((ICoreEvent *) this, "GoComDesktop", NULL, NULL);
		m_pCore->AddOrderEvent((ICoreEvent *) this, "appdesktop", NULL, NULL);
	}
	return E_FAIL;
}

STDMETHODIMP CGPlusImpl::GetSkinXmlString(IAnsiString *szXmlString)
{
	HRESULT hr = E_FAIL;
	IConfigure *pCfg = NULL; 
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{ 
		hr = pCfg->GetSkinXml("GPlus.xml",szXmlString); 
		pCfg->Release();
	}
	return hr;  
}

//
STDMETHODIMP CGPlusImpl::CoreFrameWorkError(int nErrorNo, const char *szErrorMsg)
{
	return E_NOTIMPL;
}

//广播消息
STDMETHODIMP CGPlusImpl::DoBroadcastMessage(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                    const char *pContent, void *pData)
{
	if ((::stricmp(szFromWndName, "mainwindow") == 0) && (::stricmp(szType, "showcontacts") == 0)
		&& (::stricmp(pContent, "complete") == 0))
	{
		//
		ShowGPlusToolBar();
	}
	return E_NOTIMPL;
}

//
STDMETHODIMP CGPlusImpl::DoWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes)
{
 
	return E_NOTIMPL;
}

#define GPLUSTOOLBAR_TOP     10
#define GPLUSTOOLBAR_HEIGHT  30
#define GPLUSTOOLBAR_WIDTH   180
#define GPLUSSEARCHBAR_WIDTH 150

STDMETHODIMP CGPlusImpl::ShowGPlusToolBar()
{
	
	IUIManager *pUI = NULL;
	RECT rc = {0};
	RECT rcScreen = {0};
	CSystemUtils::GetScreenRect(&rcScreen);
		
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
	{
		//创建main
		if (m_hMainToolBar == NULL)
	    { 
			rc.left = (rcScreen.right - GPLUSTOOLBAR_WIDTH) / 2;
			rc.top = GPLUSTOOLBAR_TOP;
			rc.bottom = rc.top + GPLUSTOOLBAR_HEIGHT;
			rc.right = rc.left + GPLUSTOOLBAR_WIDTH;
			HWND hWnd = CSystemUtils::FindDesktopWindow();
			if (SUCCEEDED(pUI->CreateUIWindow(hWnd, "GPlusToolBar", &rc, WS_POPUP , WS_EX_TOOLWINDOW,
					L"", &m_hMainToolBar)))
			{
				SetWindowTransparent(m_hMainToolBar, RGB(0,0,0), 255, 3/*LWA_ALPHA */);
				ShowWindow(m_hMainToolBar, SW_SHOW);
			} else
			{
				PRINTDEBUGLOG(dtInfo, "Create GPlusToolBar Failed");
			}
		}
		//
		if (m_hSearchBar ==NULL)
		{
			rc.left = (rcScreen.right - GPLUSSEARCHBAR_WIDTH) - 20;
			rc.top = GPLUSTOOLBAR_TOP;
			rc.bottom = rc.top + GPLUSTOOLBAR_HEIGHT;
			rc.right = rc.left + GPLUSSEARCHBAR_WIDTH;
			HWND hWnd = CSystemUtils::FindDesktopWindow();
			//创建search
			if (SUCCEEDED(pUI->CreateUIWindow(hWnd, "GPlusSearchToolBar", &rc, WS_POPUP, WS_EX_TOOLWINDOW,
					L"", &m_hSearchBar)))
			{
				//
				ShowWindow(m_hSearchBar, SW_SHOW);
			} else
			{
				PRINTDEBUGLOG(dtInfo, "creat GplusSearchToolBar Failed");
			}
		} //end if (m_hSearchBar == NULL)
		pUI->Release();
	} //end if (SUCCEEDED(m_pCore->...
	return S_OK;
}

#pragma warning(default:4996)
