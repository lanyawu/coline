#include <time.h>
#include <commonlib/DebugLog.h>
#include <Commonlib/systemutils.h>
#include <smartSkin/SmartSkin.h> 
#include <Crypto/crypto.h>
#include <Core/common.h>
#include "TrayMsgImpl.h"
#include "../IMCommonLib/InterfaceAnsiString.h" 

#define TRAY_PANEL_WIDTH  210
#define TRAY_PANEL_HEIGHT 140
#define TRAY_PANEL_BODER  10
#define MAX_TRAY_REF      50
#define TRAY_PER_MOVE_HEIGHT  
#define TIMER_INTERVAL    10000  //3秒

#pragma warning(disable:4996)

CTrayMsgImpl::CTrayMsgImpl(void):
              m_TrayIcon(NULL),
			  m_pCore(NULL),
			  m_hMain(NULL),
			  m_hWnd(NULL),
			  m_bFlashTray(TRUE),
			  m_hMsgBox(NULL),
			  m_ptrTimer(NULL),
			  m_uToolBarMsg(0),
			  m_hLogin(NULL)
{
	::srand((unsigned int)::time(NULL));
	m_nTimerId = ::rand();
	CSystemUtils::GetScreenRect(&m_rcScreen);
}


CTrayMsgImpl::~CTrayMsgImpl(void)
{
	HideTrayIcon();
	if (m_hWnd)
		::SkinCloseWindow(m_hWnd);
	m_hWnd = NULL;
	if (m_hHomePage)
		::SkinCloseWindow(m_hHomePage);
	if (m_hMsgBox)
		::SkinCloseWindow(m_hMsgBox);
	if (m_TrayIcon)
		delete m_TrayIcon;
	m_TrayIcon = NULL;
	if (m_pCore)
		m_pCore->Release();
	m_pCore = NULL;
}

//IUnknown
STDMETHODIMP CTrayMsgImpl::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(ITrayMsg)))
	{
		*ppv = (ITrayMsg *) this;
		_AddRef();
		return S_OK;
	} else if (IsEqualIID(riid, __uuidof(ICoreEvent)))
	{
		*ppv = (ICoreEvent *) this;
		_AddRef();
		return S_OK;
	} else if (IsEqualIID(riid, __uuidof(IProtocolParser)))
	{
		*ppv = (IProtocolParser *) this;
		_AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP CTrayMsgImpl::InitTrayMsg(HINSTANCE hInstance, HICON hDefaultIcon, const char *szTip)
{
	if (m_TrayIcon)
		delete m_TrayIcon;
	m_TrayIcon = new CTrayMsgSysIcon(hInstance, this);
	HICON h = hDefaultIcon;
	if (h == NULL)
	{		
		h = ::LoadIcon((HINSTANCE)::GetModuleHandle(L"TrayMsg.plg"), L"offline");	 
	}
	if (m_TrayIcon->SetDefaultData(h, szTip))
		return S_OK;
	return E_FAIL;
}

STDMETHODIMP CTrayMsgImpl::AddAnimateIcon(HICON hAnimIcon)
{
	if (m_TrayIcon)
	{
		m_TrayIcon->AddAnimateIcon(hAnimIcon);
		return S_OK;
	}
	return E_FAIL;
}

STDMETHODIMP CTrayMsgImpl::StartAnimate(const char *szTip)
{
	if (m_TrayIcon)
	{
		m_bFlashTray = TRUE;
		if (m_TrayIcon->Animate(szTip))
			return S_OK;
	}
	return E_FAIL;
}

STDMETHODIMP CTrayMsgImpl::StopAnimate()
{
	if (m_TrayIcon)
	{
		if (m_TrayIcon->StopAnimate())
			return S_OK;
	}
	return E_FAIL;
}

//
void CTrayMsgImpl::InitMsgTip(HWND hWnd, IUserPendMessageTipList *pList)
{
	static char MESSAGE_LIST_XML[] = "<Container xsi:type=\"HorizontalLayout\" height=\"20\"><Control xsi:type=\"PaddingPanel\" width=\"10\" /><Control xsi:type=\"TextPanel\" name=\"%s\"  text=\"%s\" border=\"false\" textColor=\"#0000FF\" horizonalign=\"left\" vertalign=\"vcenter\" singleline=\"true\" enablelinks=\"true\"/>\
                                      <Control xsi:type=\"PaddingPanel\" /> <Control xsi:type=\"TextPanel\" width=\"30\" text=\"%s\" border=\"false\" textColor=\"#0000FF\" horizonalign=\"left\" vertalign=\"vcenter\" singleline=\"true\" enablelinks=\"true\"/> <Control xsi:type=\"PaddingPanel\" width=\"10\"/></Container>";
	int nCount = pList->GetPendMsgTipCount();
	CUserMessageTip Tip;
	IContacts *pContact = NULL;
	int nSize = ::strlen(MESSAGE_LIST_XML) + 512;
	char *szXml = new char[nSize];
	CInterfaceAnsiString strTmp;
	CInterfaceAnsiString strRealName, strDept;
	std::string strLinkName;
	char szCount[32];
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
	{
		BOOL bSucc = FALSE;
		std::string strUserName;
		for (int i = 0; i < nCount; i ++)
		{
			if (SUCCEEDED(pList->GetFrontMessage(&Tip)))
			{
				bSucc = FALSE;
				strUserName = Tip.GetUserName();
				if (strUserName.find("@") != std::string::npos)
				{
					bSucc =  (SUCCEEDED(pContact->GetRealNameById(Tip.GetUserName(), NULL, &strRealName))
					         && SUCCEEDED(pContact->GetUserDeptPath(Tip.GetUserName(), NULL, &strDept)));
					strRealName.AppendString("(");
					strRealName.AppendString(strDept.GetData());
					strRealName.AppendString(")");
				} else
				{
					IGroupFrame *pFrame = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IGroupFrame), (void **)&pFrame)))
					{ 
						bSucc = SUCCEEDED(pFrame->GetGroupNameById(strUserName.c_str(), &strRealName));
						pFrame->Release();
					}
				}
				if (bSucc)
				{
					strLinkName = "chat_"; 
					strLinkName += Tip.GetUserName(); 
					memset(szCount, 0, 32);
					memset(szXml, 0, nSize);
					::itoa(Tip.GetMsgCount(), szCount, 10);
					sprintf(szXml, MESSAGE_LIST_XML, strLinkName.c_str(), strRealName.GetData(), szCount);
					::SkinAddChildControl(hWnd, L"msglist", szXml, NULL, NULL, 9999);
				}
			}
		} //end for 
		pContact->Release();
	} //end if (SUCCEEDED(
	delete []szXml;
}

#define MESSAGE_TIP_BOX_HEIGHT 50
#define MESSAGE_TIP_PER_HEIGHT 30
//
void CTrayMsgImpl::ShowMsgTipBox()
{ 
	if (!m_bFlashTray)
		return ;
	CUserMessageTipList mtList;
	if (SUCCEEDED(m_pCore->GetUserPendMsgTipList(&mtList)))
	{
		if (mtList.GetPendMsgTipCount() > 0)
		{
			if (m_hMsgBox)
				::SkinCloseWindow(m_hMsgBox);
			m_hMsgBox = NULL;
			IUIManager *pUI = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
			{ 
				RECT rc = {0}; 
				rc.left = m_rcScreen.right - TRAY_PANEL_WIDTH - TRAY_PANEL_BODER;
				rc.top = m_rcScreen.bottom - MESSAGE_TIP_BOX_HEIGHT - MESSAGE_TIP_PER_HEIGHT * mtList.GetPendMsgTipCount()  - 20;
				rc.right = rc.left + TRAY_PANEL_WIDTH;
				rc.bottom = m_rcScreen.bottom - 20;
				if (SUCCEEDED(pUI->CreateUIWindow(NULL, "msgboxpanel", &rc, WS_POPUP | WS_MINIMIZEBOX, WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
					          L"消息盒子", &m_hMsgBox)))
				{
					InitMsgTip(m_hMsgBox, &mtList);
					::AnimateWindow(m_hMsgBox, 2000, AW_BLEND);
				}
				pUI->Release();
			} //end if (SUCCEEDED(m_pCore->
		} //end if(mtList->Get...
	} //end if (SUCCEEDED(m_pCore->
}

//
STDMETHODIMP CTrayMsgImpl::ShowTipPanel(const char *szCaption, const char *szContent)
{
	RECT rc = {0}; 
	rc.left = m_rcScreen.right - TRAY_PANEL_WIDTH - TRAY_PANEL_BODER;
	rc.top = m_rcScreen.bottom - TRAY_PANEL_HEIGHT - 20;
	rc.right = rc.left + TRAY_PANEL_WIDTH;
	rc.bottom = rc.top + TRAY_PANEL_HEIGHT;
	IUIManager *pUI = NULL;
	HWND hWnd = NULL;
	TCHAR szwCaption[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szCaption, szwCaption, MAX_PATH - 1);
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
	{ 
		if (SUCCEEDED(pUI->CreateUIWindow(NULL, "tippanel", &rc, WS_POPUP | WS_MINIMIZEBOX, WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
				          szwCaption, &hWnd)))
		{
			int n = strlen(szContent);
			TCHAR *szwContent = new TCHAR[n + 1];
			memset(szwContent, 0, sizeof(TCHAR) * (n + 1));
			CStringConversion::StringToWideChar(szContent, szwContent, n);
			::SkinSetControlAttr(hWnd, L"content", L"text", szwContent);
			::AnimateWindow(hWnd, 2000, AW_BLEND);
			delete [] szwContent;
		}
		pUI->Release();
	}
	return S_OK;
}

BOOL CTrayMsgImpl::InitTrayPanel(const TCHAR *szCaption, const TCHAR *szImage, 
	                      const TCHAR *szUrl, const TCHAR *szTipText, HWND *hWnd, BOOL bUserClosed)
{
	RECT rc = {0}; 
	rc.left = m_rcScreen.right - TRAY_PANEL_WIDTH - TRAY_PANEL_BODER;
	rc.top = m_rcScreen.bottom - TRAY_PANEL_HEIGHT - 20;
	rc.right = rc.left + TRAY_PANEL_WIDTH;
	rc.bottom = rc.top + TRAY_PANEL_HEIGHT;
	IUIManager *pUI = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
	{
		if (bUserClosed)
		{
			if (SUCCEEDED(pUI->CreateUIWindow(NULL, "richtraypanel", &rc, WS_POPUP | WS_MINIMIZEBOX, WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
				          szCaption, hWnd)))
			{
				::SkinSetControlTextByName(*hWnd, L"tipText", szTipText);
				if (szUrl)
				{
					::SkinSetControlVisible(m_hWnd, L"notifysetting", TRUE);
					::SkinSetControlTextByName(*hWnd, L"lblurl", szUrl);
				} else
				{
					::SkinSetControlVisible(*hWnd, L"notifyType", FALSE);
					::SkinSetControlVisible(m_hWnd, L"notifysetting", FALSE);
				}
				::SetWindowText(*hWnd, szCaption); 
			}
		} else
		{
			if (SUCCEEDED(pUI->CreateUIWindow(NULL, "traypanel", &rc, WS_POPUP | WS_MINIMIZEBOX, WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
				szCaption, hWnd)))
			{ 				
				pUI->OrderWindowMessage("traypanel", *hWnd, WM_TIMER, (ICoreEvent *) this);
				::SkinSetControlTextByName(*hWnd, L"notifypanel", szTipText);
				if (szUrl)
				{
					::SkinSetControlVisible(m_hWnd, L"notifysetting", TRUE);
					::SkinSetControlTextByName(*hWnd, L"labelUrl", szUrl);
				} else
				{
					::SkinSetControlVisible(*hWnd, L"notifyType", FALSE);
					::SkinSetControlVisible(m_hWnd, L"notifysetting", FALSE);
				}
				::SetWindowText(*hWnd, szCaption);
			}
		}
		pUI->Release();
		return TRUE;
	}
	return FALSE;
}

STDMETHODIMP CTrayMsgImpl::ShowTipInfo(const TCHAR *szImage, const TCHAR *szTipText, 
		                    const TCHAR *szCaption, const TCHAR *szUrl, BOOL bUserClosed)
{
	if (bUserClosed)
	{
		HWND hWnd = NULL;
		InitTrayPanel(szCaption, szImage, szUrl, szTipText, &hWnd, bUserClosed);
		AnimateWindow(hWnd, 2000, AW_BLEND);
		::ShowWindow(hWnd, SW_SHOW);
		::InvalidateRect(hWnd, NULL, TRUE);
	} else
	{
		if ((m_hWnd == NULL) || (!::IsWindow(m_hWnd)))
		{
			InitTrayPanel(szCaption, szImage, szUrl, szTipText, &m_hWnd, FALSE);
		}
		if (m_hWnd && ::IsWindow(m_hWnd))
		{ 
			::SetWindowText(m_hWnd, szCaption);
			::SkinSetControlAttr(m_hWnd, L"trayimage", L"filename", szImage);
			::SkinSetControlTextByName(m_hWnd, L"notifypanel", szTipText);
			if (szUrl)
			{
				::SkinSetControlVisible(m_hWnd, L"notifyType", TRUE);
				::SkinSetControlVisible(m_hWnd, L"notifysetting", TRUE);
				::SkinSetControlTextByName(m_hWnd, L"labelUrl", szUrl);
			} else
			{
				::SkinSetControlVisible(m_hWnd, L"notifyType", FALSE);
				::SkinSetControlVisible(m_hWnd, L"notifysetting", FALSE);
			}
			::InterlockedExchange(&m_lRef, 0);
			if (m_ptrTimer)
				::KillTimer(m_hWnd, m_ptrTimer);
			m_ptrTimer = ::SetTimer(m_hWnd, m_nTimerId, TIMER_INTERVAL, NULL);
			AnimateWindow(m_hWnd, 2000, AW_BLEND);
			::ShowWindow(m_hWnd, SW_SHOW);
			::InvalidateRect(m_hWnd, NULL, TRUE);
			//::MoveWindow(m_hWnd, m_rcScreen.right - TRAY_PANEL_WIDTH - TRAY_PANEL_BODER, m_rcScreen.bottom - TRAY_PANEL_HEIGHT / 2, 
			//	    TRAY_PANEL_WIDTH, TRAY_PANEL_HEIGHT, TRUE);
			return S_OK;
		}
	}
	return E_FAIL;
}

STDMETHODIMP CTrayMsgImpl::ShowTrayIcon()
{
	if (m_TrayIcon)
	{
		m_TrayIcon->Show();
		CInterfaceAnsiString strPresence, strMemo;
		if (SUCCEEDED(m_pCore->GetPresence(NULL, &strPresence, &strMemo)))
			return RefreshPresence(strPresence.GetData(), strMemo.GetData()); 
	}
	return E_FAIL;
}

STDMETHODIMP CTrayMsgImpl::HideTrayIcon()
{
	if (m_TrayIcon)
	{
		if (m_TrayIcon->Hide())
			return S_OK;
	}
	return E_FAIL;
}

HRESULT CTrayMsgImpl::DoLinkEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szName, "notifypanel") == 0)
	{
		TCHAR szUrl[MAX_PATH] = {0};
		int nSize = MAX_PATH - 1;
		if (::SkinGetControlTextByName(hWnd, L"labelUrl", szUrl, &nSize))
		{
			char szTmp[MAX_PATH] = {0};
			CStringConversion::WideCharToString(szUrl, szTmp, MAX_PATH - 1);
			std::string strUrl = szTmp;
			//chat://  
			if (strUrl.find("chat://") == 0)
			{
				std::string strUserName = strUrl.substr(strlen("chat://"));
				IChatFrame *pFrame = NULL;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pFrame)))
				{
					pFrame->ShowChatFrame(hWnd, strUserName.c_str(), NULL);
					pFrame->Release();
				} //end if (m_pCore &&
			} //end if (strUrl.find(...
		} //end if (::SkinGetControlTextByName(...
	} else if (::stricmp(szName, "notifysetting") == 0) //设置
	{
		IConfigureUI *pUI = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigureUI), (void **)&pUI)))
		{
			pUI->Navegate2Frame(NULL, "soundpage");
			pUI->Release();
		}
	} else if ((::stricmp(szName, "viewdetail") == 0) || (::stricmp(szName, "tipText") == 0)) //查看
	{
		TCHAR szUrl[MAX_PATH] = {0};
		int nSize = MAX_PATH - 1;
		if (::SkinGetControlTextByName(hWnd, L"lblurl", szUrl, &nSize))
		{
			char szTmp[MAX_PATH] = {0};
			CStringConversion::WideCharToString(szUrl, szTmp, MAX_PATH - 1); 
			CSystemUtils::OpenURL(szTmp);
		}
	} else if (::stricmp(szName, "closebox") == 0)
	{
		::SkinCloseWindow(hWnd);
		return 0;
	} else if (::stricmp(szName, "ignorebox") == 0)
	{
		m_bFlashTray = FALSE;
		StopAnimate();
		::SkinCloseWindow(hWnd);
		return 0;
	} else if (::strnicmp(szName, "chat_", 5) == 0)
	{ 
		std::string strUserName = szName;
		strUserName = strUserName.substr(5);
		if (strUserName.find("@") != std::string::npos)
		{
			IChatFrame *pFrame = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pFrame)))
			{
				pFrame->ShowChatFrame(hWnd, strUserName.c_str(), NULL);
				pFrame->Release();
			}
		} else
		{
			IGroupFrame *pFrame = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IGroupFrame), (void **)&pFrame)))
			{
				pFrame->ShowGroupFrame(strUserName.c_str(), NULL);
				pFrame->Release();
			}
		}
		CInterfaceAnsiString strFrom, strType;
		if (SUCCEEDED(m_pCore->GetLastPendingMsg(&strFrom, &strType)))
		{
			StartAnimate(NULL);
		}
		::SkinCloseWindow(hWnd);
		return 0;
	}
	return -1;
}

//ICoreEvent
STDMETHODIMP CTrayMsgImpl::DoCoreEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
	                     LPARAM lParam, HRESULT *hResult)
{
	if (::stricmp(szType, "link") == 0)
	{
		*hResult = DoLinkEvent(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "afterinit") == 0)
	{
		if (::stricmp(szName, "mainwindow") == 0)
		{
			IUIManager *pUI = NULL;
			if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
			{  				
				pUI->OrderWindowMessage("mainwindow", hWnd, WM_SHOWTIPPANEL, (ICoreEvent *) this); 
				pUI->OrderWindowMessage("mainwindow", hWnd, WM_SHOWHOMEPAGE, (ICoreEvent *) this);
				pUI->Release();
			}
		}
	} else if (::stricmp(szType, "beforemenupop") == 0)
	{
		if (::stricmp(szName, "maintraymenu") == 0)
		{
			//PRINTDEBUGLOG(dtInfo, "MainWindow beforepopmenu, WPARAM:%d lParam:%d", wParam, lParam);
		} else if (::stricmp(szName, "logontraymenu") == 0)
		{
			//PRINTDEBUGLOG(dtInfo, "LogonWindow beforepopmenu, WPARAM:%d lParam:%d", wParam, lParam);
		}
	} else if (::stricmp(szType, "menucommand") == 0)
	{
		if (::stricmp(szName, "maintraymenu") == 0)
		{
			HWND h = NULL;
			GetWindowHWND("mainwindow", &h);
			if (h != NULL)
			{
				switch(wParam)
				{
				case 20001: //显示主窗口
					CSystemUtils::BringToFront(h);
					break;
				case 20004: //离线
					if (m_pCore)
						m_pCore->ChangePresence("offline", "离线");
					break;
				case 20005: //显示为脱机
					if (m_pCore)
						m_pCore->ChangePresence("appearoffline", "隐身");
					break;
				case 20006: //忙碌
					if (m_pCore)
						m_pCore->ChangePresence("busy", "忙碌");
					break;
				case 20007: //离开
					if (m_pCore)
						m_pCore->ChangePresence("away", "离开");
					break;
				case 20008: //上线
					if (m_pCore)
						m_pCore->ChangePresence("online", "在线");
					break;
				case 20003: //注销
					if (m_pCore)
						m_pCore->Logout();
					break;
				case 20002: //退出
					::SkinCloseWindow(h);
					//::PostMessage(h, WM_QUIT, 0, 0);
					break; 
				} //end switch(wParam)
			} //end if (h != NULL)
		} else if (::stricmp(szName, "logontraymenu") == 0)
		{
			HWND h = NULL;
			GetWindowHWND("LogonWindow", &h);
			if (h != NULL)
			{
				switch(wParam)
				{
				case 10001:
					CSystemUtils::BringToFront(h);
					break;
				case 10002:
					{
						::SkinCloseWindow(h);
						HWND hMain = NULL;
				        GetWindowHWND("mainwindow", &hMain);
						if (hMain)
						{
							::SkinCloseWindow(hMain);
							//::PostMessage(hMain, WM_QUIT, 0, 0);
						}
					}
					break;
				} //end switch(wParam)
			} //end if (h != NULL)
		} //else if (::stricmp(szName, "logontraymenu") == 0)
	} else if (::stricmp(szType, "afterinit") == 0)
	{
		 if (::stricmp(szName, "MainWindow") == 0)
		 {
			 HWND h = NULL;
			 GetWindowHWND("mainwindow", &h);
			 if (h != NULL)
				 ::SkinCreateMenu(h, L"maintraymenu");
			 m_uToolBarMsg = ::RegisterWindowMessage(_T("TaskbarCreated"));
			 if (m_uToolBarMsg > 0)
			 {
				 IUIManager *pUI = NULL;
				 if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
				 {
					 pUI->OrderWindowMessage(UI_MAIN_WINDOW_NAME, NULL, m_uToolBarMsg, (ICoreEvent *) this);
					 pUI->OrderWindowMessage(UI_MAIN_WINDOW_NAME, NULL, WM_SHOWHOMEPAGE, (ICoreEvent *) this);
					 pUI->Release();
				 }
			 }
		 } else if (::stricmp(szName, "LogonWindow") == 0)
		 {
			 HWND h = NULL;
			 GetWindowHWND("LogonWindow", &h);
			 if (h != NULL)
				 ::SkinCreateMenu(h, L"logontraymenu");
			 //
		 }
	} else
	{
		//PRINTDEBUGLOG(dtInfo, "traymsg Invalid Event:%s Name:%s", szType, szName);
	} //end else if (::stricmp(szType, "beforemenupop") == 0
	return E_NOTIMPL;
}

//广播消息
STDMETHODIMP CTrayMsgImpl::DoBroadcastMessage(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData)
{
	if ((::stricmp(szFromWndName, "mainwindow") == 0) && (::stricmp(szType, "showcontacts") == 0)
		&& (::stricmp(pContent, "complete") == 0))
	{
		CInterfaceAnsiString strPresence, strMemo;
		if (SUCCEEDED(m_pCore->GetPresence(NULL, &strPresence, &strMemo)))
			RefreshPresence(strPresence.GetData(), strMemo.GetData());
		return S_OK;
	} else if ((::stricmp(szFromWndName, "coreframe") == 0) && (::stricmp(szType, "getsrvar") == 0))
	{
		if (::stricmp(pContent, "homepage") == 0)
		{
			char *szValue = (char *)pData;
			if (szValue)
			{
				m_strHomePage = szValue;
				CInterfaceAnsiString strTmp;
				//replace username
				if (SUCCEEDED(m_pCore->GetUserName(&strTmp)))
				{
					int nPos = m_strHomePage.find("%gcusername%");
					if (nPos != std::string::npos)
						m_strHomePage.replace(nPos, ::strlen("%gcusername%"), strTmp.GetData());
				} //end if (SUCCEEDED(
				//replace password
				if (SUCCEEDED(m_pCore->GetUserPassword(&strTmp)))
				{
					int nPos = m_strHomePage.find("%gcpassword%");
					if (nPos != std::string::npos)
						m_strHomePage.replace(nPos, ::strlen("%gcpassword%"), strTmp.GetData());
				} //end if (SUCCEEDED(..
				//替换MD5
				{
					int nPos = m_strHomePage.find("%gcmd5password%");
					char szMd5[60] = {0};
					::md5_encode(strTmp.GetData(), strTmp.GetSize(), szMd5);
					if (nPos != std::string::npos)
						m_strHomePage.replace(nPos, ::strlen("%gcmd5password%"), szMd5);
				}
				if (m_pCore->GetIsOnline())
				{
					if (m_hMain == NULL)
						GetWindowHWND("MainWindow", &m_hMain);
					if (m_hMain) 
						::PostMessage(m_hMain, WM_SHOWHOMEPAGE, 0, 0);
				} //end if (m_pCore->GetIsOnline())
			} //end if (szValue)
		} //end if (::stricmp(
	}
	return E_FAIL;
}

STDMETHODIMP CTrayMsgImpl::SetCoreFrameWork(ICoreFrameWork *pCore)
{
	if (m_pCore)
		m_pCore->Release();
	m_pCore = pCore;
	if (m_pCore)
	{
		m_pCore->AddRef();

		//order 
		m_pCore->AddOrderEvent((ICoreEvent *) this, "MainWindow", "maintraymenu", "beforemenupop");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "LogonWindow", "logontraymenu", "beforemenupop");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "MainWindow", "maintraymenu", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "LogonWindow", "logontraymenu", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "MainWindow", "MainWindow", "afterinit");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "LogonWindow", "LogonWindow", "afterinit");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "traypanel", NULL, NULL);
		m_pCore->AddOrderEvent((ICoreEvent *) this, "richtraypanel", NULL, NULL);
		m_pCore->AddOrderEvent((ICoreEvent *) this, "msgboxpanel", NULL, NULL);

		//
		m_pCore->AddOrderProtocol((IProtocolParser *) this, "sys", "presence");
		//
		m_pCore->AddOrderProtocol((IProtocolParser *) this, "sys", "getsrvar");
	}
	return S_OK;
}

STDMETHODIMP CTrayMsgImpl::GetSkinXmlString(IAnsiString *szXmlString)
{
	HRESULT hr = E_FAIL;
	IConfigure *pCfg = NULL; 
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{ 
		hr = pCfg->GetSkinXml("traymsg.xml",szXmlString); 
		pCfg->Release();
	}
	return hr;   
}

//
STDMETHODIMP CTrayMsgImpl::CoreFrameWorkError(int nErrorNo, const char *szErrorMsg)
{
	switch(nErrorNo)
	{
	case CORE_ERROR_SOCKET_CLOSED:
	case CORE_ERROR_KICKOUT:
		 DoPresenceChange(m_strUserName.c_str(), "offline", "离线", FALSE);
		 break;
	case CORE_ERROR_LOGOUT:
		 if (m_TrayIcon)
		 {
			HICON h = ::LoadIcon((HINSTANCE)::GetModuleHandle(L"TrayMsg.plg"), L"offline");
			m_TrayIcon->SetDefaultData(h, "CoLine 未登陆");
		 } 
		 m_strUserName.clear();
		 m_strRealName.clear();
		 break;
	}
	return S_OK;
}

//
STDMETHODIMP CTrayMsgImpl::DoWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes)
{
	if ((uMsg == WM_TIMER) && (hWnd == m_hWnd) && (wParam == m_nTimerId))
	{
		::KillTimer(m_hWnd, m_ptrTimer);
		m_ptrTimer = NULL;
		::ShowWindow(m_hWnd, SW_HIDE);
		return S_OK;
		//
		switch (::InterlockedIncrement(&m_lRef))
		{
		case 1:
		case 49:
			::MoveWindow(m_hWnd, m_rcScreen.right - TRAY_PANEL_WIDTH - TRAY_PANEL_BODER, m_rcScreen.bottom - TRAY_PANEL_HEIGHT / 10 * 6, 
			    TRAY_PANEL_WIDTH, TRAY_PANEL_HEIGHT, TRUE);
			break;
		case 2:
		case 48:
			::MoveWindow(m_hWnd, m_rcScreen.right - TRAY_PANEL_WIDTH - TRAY_PANEL_BODER, m_rcScreen.bottom - TRAY_PANEL_HEIGHT / 10 * 7, 
			    TRAY_PANEL_WIDTH, TRAY_PANEL_HEIGHT, TRUE);
			break;
		case 3:
		case 47:
			::MoveWindow(m_hWnd, m_rcScreen.right - TRAY_PANEL_WIDTH - TRAY_PANEL_BODER, m_rcScreen.bottom - TRAY_PANEL_HEIGHT / 10 * 8, 
			    TRAY_PANEL_WIDTH, TRAY_PANEL_HEIGHT, TRUE);
			break;
		case 4:
		case 46:
			::MoveWindow(m_hWnd, m_rcScreen.right - TRAY_PANEL_WIDTH - TRAY_PANEL_BODER, m_rcScreen.bottom - TRAY_PANEL_HEIGHT / 10 * 9, 
			    TRAY_PANEL_WIDTH, TRAY_PANEL_HEIGHT, TRUE);
			break;
		case 5: 
		case 45:
			::MoveWindow(m_hWnd, m_rcScreen.right - TRAY_PANEL_WIDTH - TRAY_PANEL_BODER, m_rcScreen.bottom - TRAY_PANEL_HEIGHT, 
			    TRAY_PANEL_WIDTH, TRAY_PANEL_HEIGHT, TRUE);
			break;
		case 6: 
		case 44:
			::MoveWindow(m_hWnd, m_rcScreen.right - TRAY_PANEL_WIDTH - TRAY_PANEL_BODER, m_rcScreen.bottom - TRAY_PANEL_HEIGHT / 10 * 12, 
			    TRAY_PANEL_WIDTH, TRAY_PANEL_HEIGHT, TRUE);
			break;
		case MAX_TRAY_REF:
			::KillTimer(m_hWnd, m_ptrTimer);
			m_ptrTimer = NULL;
			::ShowWindow(m_hWnd, SW_HIDE);
			break;
		}
	} else if ((uMsg == m_uToolBarMsg) && (m_uToolBarMsg != 0))
	{
		if (m_TrayIcon)
		{
			m_TrayIcon->Show();
		}
		InitSelfUserName();
		if (!m_strUserName.empty())
		{
			//
			CInterfaceAnsiString strPresence, strMemo;
			if (SUCCEEDED(m_pCore->GetPresence(NULL, &strPresence, &strMemo)))
			{
				DoPresenceChange(m_strUserName.c_str(), strPresence.GetData(), strMemo.GetData(), FALSE);
			}
		} else
		{
			InitTrayMsg((HINSTANCE)::GetModuleHandle(NULL), NULL, NULL);
		} //end if (!
	} else if (uMsg == WM_SHOWHOMEPAGE)
	{ 
		ShowHomePage(); 
	} else if (uMsg == WM_SHOWTIPPANEL)
	{
		char *szCaption = (char *)wParam;
		char *szContent = (char *)lParam;
		ShowTipPanel(szCaption, szContent);
	}
	return E_NOTIMPL;
}

#define HOMEPAGE_WINDOW_WIDTH  600
#define HOMEPAGE_WINDOW_HEIGHT 600
void CTrayMsgImpl::ShowHomePage()
{
	//
	 
	if (!m_strHomePage.empty())
	{
		if (m_hMain == NULL)
		{
			GetWindowHWND("mainwindow", &m_hMain);
		}
		if (m_hMain)
		{
			TCHAR szwTmp[1024] = {0};
			CStringConversion::StringToWideChar(m_strHomePage.c_str(), szwTmp, 1023);
			::SkinSetControlAttr(m_hMain, L"homepage", L"url", szwTmp);
		}
		return;
		IUIManager *pUI = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
		{ 
			if ((m_hHomePage == NULL) || (!::IsWindow(m_hHomePage)))
			{
				RECT rcScreen = {0};
				CSystemUtils::GetScreenRect(&rcScreen);
				RECT rc = {0};
				rc.left = (rcScreen.right - HOMEPAGE_WINDOW_WIDTH) / 2;
				rc.top = (rcScreen.bottom - HOMEPAGE_WINDOW_HEIGHT) / 2;
				rc.right = rc.left + HOMEPAGE_WINDOW_WIDTH;
				rc.bottom = rc.top + HOMEPAGE_WINDOW_HEIGHT;
				if (SUCCEEDED(pUI->CreateUIWindow(NULL, "homepanel", &rc, WS_POPUP | WS_MINIMIZEBOX, 0,
					          L"首页", &m_hHomePage)))
				{ 
					TCHAR *szUrl = new TCHAR[m_strHomePage.size() + 1];
					memset(szUrl, 0, sizeof(TCHAR) * (m_strHomePage.size() + 1));
					CStringConversion::StringToWideChar(m_strHomePage.c_str(), szUrl, m_strHomePage.size());
					::SkinSetControlAttr(m_hHomePage, L"homeNav", L"url", szUrl);
					CInterfaceAnsiString strTmp;
					if (SUCCEEDED(m_pCore->GetUserNickName(&strTmp)))
					{
						memset(szUrl, 0, sizeof(TCHAR) * (m_strHomePage.size() + 1));
						CStringConversion::UTF8ToWideChar(strTmp.GetData(), szUrl, m_strHomePage.size());
						::SkinSetControlTextByName(m_hHomePage, L"lblUserName", szUrl);
					}
					delete []szUrl;
				}
			}  
			::ShowWindow(m_hHomePage, SW_SHOW);
			 
		} //end if (SUCCEEDED(m_pCore->QueryInterface(...
	}
}

//左键单击
void CTrayMsgImpl::OnLButtonClick(int nShiftState,int nX, int nY)
{ 
	//
}

//
void CTrayMsgImpl::OnBalloonShow(int nShiftState, int nX, int nY)
{
	 //
}

//
void CTrayMsgImpl::OnToolTipShow(BOOL bActived)
{
	if (bActived)
		ShowMsgTipBox();
	else
	{
		if (m_hMsgBox)
		{
			POINT pt = {0};
			RECT rc = {0};
			::GetCursorPos(&pt);
			::GetWindowRect(m_hMsgBox, &rc);
			if (!::PtInRect(&rc, pt))
				::SkinCloseWindow(m_hMsgBox);
		}
	}
}
//
void CTrayMsgImpl::OnBalloonHide(int nShiftState, int nX, int nY)
{
	if (m_hMsgBox)
		::SkinCloseWindow(m_hMsgBox);
	m_hMsgBox = NULL;
}

//右键单击 弹出托盘菜单
void CTrayMsgImpl::OnRButtonClick(int nShiftState,int nX, int nY)
{
	HWND h = NULL; 
	if (m_pCore->GetIsOnline())
	{
		if (m_hMain == NULL)
			GetWindowHWND("MainWindow", &m_hMain);
		if (m_hMain)
			::SkinPopTrackMenu(m_hMain, L"maintraymenu", TPM_LEFTALIGN, nX, nY);
	} else
	{
		//登陆窗口
		if ((m_hLogin == NULL) || (!::IsWindow(m_hLogin)))
			GetWindowHWND("LogonWindow", &m_hLogin);
		if (m_hLogin)
			::SkinPopTrackMenu(m_hLogin, L"logontraymenu", TPM_LEFTALIGN, nX, nY);
		else
		{
			if ((m_hMain == NULL) || (!::IsWindow(m_hMain)))
				GetWindowHWND("MainWindow", &m_hMain);
			if (m_hMain)
				::SkinPopTrackMenu(m_hMain, L"maintraymenu", TPM_LEFTALIGN, nX, nY);
		} //end else if (m_hLogin)
	} //end else if (nStatus > 0)
}

//左键双击
void CTrayMsgImpl::OnLButtonDblClick(int nShiftState,int nX, int nY)
{
	//
	if (m_pCore)
	{
		//先读取未显示消息

		//再弹出窗口 
		HWND h = NULL; 
		if (m_pCore->GetIsOnline())
		{
			if (FAILED(m_pCore->PickPendingMessage()))
			{
				if (m_hMain == NULL)
					GetWindowHWND("MainWindow", &m_hMain);
				h = m_hMain;
			} else
			{
				CInterfaceAnsiString strFrom, strType;
				if (SUCCEEDED(m_pCore->GetLastPendingMsg(&strFrom, &strType)))
				{
					StartAnimate(NULL);
				} //end if (SUCCEEDED(
				if (m_hMsgBox)
				{
					::SkinCloseWindow(m_hMsgBox);
					m_hMsgBox = NULL;
				}
			} //end else if (FAILED(
		} else
		{
			//登陆窗口
			if (m_hLogin == NULL)
				GetWindowHWND("LogonWindow", &m_hLogin);
			
			h = m_hLogin;
			if (h == NULL)
			{
				if (m_hMain == NULL)
					GetWindowHWND("MainWindow", &m_hMain);
				h = m_hMain;
			}
		}
		if (h != NULL)
			CSystemUtils::BringToFront(h);
	 
	} //end if (m_pCore)
}

//右键双击
void CTrayMsgImpl::OnRButtonDblClick(int nShiftState, int nX, int nY)
{
	//
}

BOOL CTrayMsgImpl::GetWindowHWND(const char *szName, HWND *h)
{
	BOOL bSucc = FALSE;
	IUIManager *pUI = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
	{
		bSucc = SUCCEEDED(pUI->GetWindowHWNDByName(szName, h));
		pUI->Release();
	}
	return bSucc;
}

STDMETHODIMP CTrayMsgImpl::DoRecvProtocol(const BYTE *pData, const LONG lSize)
{
	return E_NOTIMPL;
}

STDMETHODIMP CTrayMsgImpl::RefreshPresence(const char *szPresence, const char *szMemo)
{
	if (!m_pCore->GetIsOnline() && (::stricmp(szPresence, "offline") != 0))
		return E_FAIL;

	if (m_strRealName.empty())
	{
		CInterfaceAnsiString strRealName;
		if (m_pCore && SUCCEEDED(m_pCore->GetUserNickName(&strRealName)))
		{
			char szTmp[128] = {0};
			CStringConversion::UTF8ToString(strRealName.GetData(), szTmp, 127);
			m_strRealName = szTmp;
		}
	}
	if (m_TrayIcon)
	{
		std::string strTip; 
		HICON h = NULL;
		if (!m_strRealName.empty())
		{ 
			strTip = "CoLine:";
			strTip += m_strRealName;
			strTip += "\n";
		}
		strTip += "状态:";
		UINT uMenuId = 0; 
		//modify status
		if (::stricmp(szPresence, "online") == 0)
		{
			h = ::LoadIcon((HINSTANCE)::GetModuleHandle(L"TrayMsg.plg"), L"online");
			if (szMemo)
				strTip += szMemo;
			else
				strTip += "在线";
			uMenuId = 20008;
		} else if (::stricmp(szPresence, "away") == 0)
		{
			h = ::LoadIcon((HINSTANCE)::GetModuleHandle(L"TrayMsg.plg"), L"away");
			if (szMemo)
				strTip += szMemo;
			else
				strTip += "离开";
			uMenuId = 20007;
		} else if (::stricmp(szPresence, "busy") == 0)
		{
			h = ::LoadIcon((HINSTANCE)::GetModuleHandle(L"TrayMsg.plg"), L"busy");
			if (szMemo)
				strTip += szMemo;
			else
				strTip += "忙碌";
			uMenuId = 20006;
		} else if (::stricmp(szPresence, "appearoffline") == 0)
		{
			h = ::LoadIcon((HINSTANCE)::GetModuleHandle(L"TrayMsg.plg"), L"hidden");
			if (szMemo)
				strTip += szMemo;
			else
				strTip += "隐身";
			uMenuId = 20005;
		} else if (::stricmp(szPresence, "offline") == 0)
		{
			h = ::LoadIcon((HINSTANCE)::GetModuleHandle(L"TrayMsg.plg"), L"offline");
			if (szMemo)
				strTip += szMemo;
			else
				strTip += "离线";
			uMenuId = 20004;
		} else
		{
			h = ::LoadIcon((HINSTANCE)::GetModuleHandle(L"TrayMsg.plg"), L"away");
			if (szMemo)
				strTip += szMemo;
			else
				strTip += "离开";
			uMenuId = 20007;
		}
		if (uMenuId != 0)
		{
			if (m_hMain == NULL)
				GetWindowHWND("MainWindow", &m_hMain);
			if (m_hMain)
				::SkinSetMenuChecked(m_hMain, L"maintraymenu", uMenuId, TRUE);
		}
		if (m_TrayIcon->SetDefaultData(h, strTip.c_str()))
			return S_OK;
	}
	return E_FAIL;
}

void CTrayMsgImpl::InitSelfUserName()
{
	//
	if (m_strUserName.empty())
	{
		CInterfaceAnsiString strTmp;
		CInterfaceAnsiString strDomain;
		if (m_pCore && SUCCEEDED(m_pCore->GetUserName(&strTmp))
			&& SUCCEEDED(m_pCore->GetUserDomain(&strDomain)))
		{
			m_strUserName = strTmp.GetData();
			m_strUserName += "@";
			m_strUserName += strDomain.GetData();
		} //end if (m_pCore 
	} //end if (m_strUserName...
}

STDMETHODIMP CTrayMsgImpl::DoPresenceChange(const char *szUserName, const char *szNewPresence,
	                                 const char *szMemo, BOOL bOrder)
{
	InitSelfUserName();
	if (!m_strUserName.empty())
	{
		if ((::stricmp(szUserName, m_strUserName.c_str()) == 0) && m_TrayIcon)
		{
			return RefreshPresence(szNewPresence, szMemo);
		}  else if (!bOrder)
		{
			IConfigure *pCfg = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
			{
				if (pCfg->IsContactOnlineTip(szUserName))
				{
					if (::stricmp(szNewPresence, "online") == 0) //判断是否在关注人中
					{
						IContacts *pContact = NULL;
						if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
						{
							TCHAR szTmp[MAX_PATH] = {0};
							CInterfaceAnsiString szValue;
							if (SUCCEEDED(pContact->GetContactUserValue(szUserName, "realname", &szValue)))
							{
								CStringConversion::UTF8ToWideChar(szValue.GetData(), szTmp, MAX_PATH - 1);
							} else
							{
								CStringConversion::StringToWideChar(szUserName, szTmp, MAX_PATH - 1);
							} 
							lstrcat(szTmp, L"  ");
							if (m_pCore)
							{
								TCHAR szUrl[MAX_PATH] = {0}; 
								::lstrcpy(szUrl, L"chat://");
								CStringConversion::StringToWideChar(szUserName, szUrl + (::lstrlen(L"chat://")), MAX_PATH - 1);	 
								CInterfaceAnsiString strTmp;
								if (SUCCEEDED(pContact->GetContactHead(szUserName, &strTmp, FALSE)))
								{
									TCHAR szImageFile[MAX_PATH] = {0};
									CStringConversion::StringToWideChar(strTmp.GetData(), szImageFile, MAX_PATH - 1);
									m_pCore->ShowTrayTipInfo(szImageFile, szTmp, szUrl, L"上线通知", NULL);
								} else
									m_pCore->ShowTrayTipInfo(NULL, szTmp, szUrl, L"上线通知", NULL);
							}
							pContact->Release();
						} //end if (SUCCEEDED(m_pCore->..
						//播放声音
						IConfigure *pCfg = NULL;
						if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
						{
							pCfg->PlayMsgSound("online", szUserName, FALSE);
							pCfg->Release();
						}
					} //end if (::stricmp(szNewPresence,
				} //end if (pCfg->IsContactOnlineTip(szUserName))
				pCfg->Release();
			}  //end if (SUCCEEDED(m_pCore->QueryInterface(
		} //end else if (
	} //end if (!m_strUserName.empty())
	return E_FAIL;
}

#pragma warning(default:4996)
