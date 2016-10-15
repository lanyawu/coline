#include <Commonlib/DebugLog.h>
#include <Commonlib/stringutils.h>
#include <Commonlib/systemutils.h>
#include <SmartSkin/smartskin.h>
#include <Core/common.h>
#include "LoginImpl.h"
#include "../IMCommonLib/InterfaceAnsiString.h"

#pragma warning(disable:4996)

#define DEFAULT_SERVER_PORT 9902

CLoginImpl::CLoginImpl(void):
            m_pCore(NULL),
			m_hWnd(NULL),
			m_bFromBal(FALSE),
			m_bLoginSucc(FALSE),
			m_bShowRealName(FALSE),
			m_pBalancerSocket(NULL)
{
	//
	m_strPresence = "online";
	m_strPresenceMemo = "在线"; 
}


CLoginImpl::~CLoginImpl(void)
{
	if (m_hWnd)
	{
		::SkinCloseWindow(m_hWnd);
	}
	m_hWnd = NULL;
	//
	if (m_pCore)
	{
		m_pCore->Release();
		m_pCore = NULL;
	}
	m_pCore = NULL;
	if (m_pBalancerSocket)
	{
		delete m_pBalancerSocket;
		m_pBalancerSocket = NULL;
	}
}

STDMETHODIMP CLoginImpl::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(ICoreLogin)))
	{
		*ppv = (ICoreLogin *) this;
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

void CLoginImpl::SetEnabled(BOOL bEnabled)
{
	if (m_hWnd != NULL)
	{
		::SkinSetControlEnable(m_hWnd, L"cb_username", bEnabled);
		::SkinSetControlEnable(m_hWnd, L"edit_password", bEnabled);
		::SkinSetControlEnable(m_hWnd, L"saveusername", bEnabled);
		::SkinSetControlEnable(m_hWnd, L"rememberpwd", bEnabled);
		::SkinSetControlEnable(m_hWnd, L"autologon", bEnabled);
		::SkinSetControlEnable(m_hWnd, L"cb_logonstatus", bEnabled);
		::SkinSetControlEnable(m_hWnd, L"settings", bEnabled);
		::SkinSetControlEnable(m_hWnd, L"helpbutton", bEnabled);
		::SkinSetControlEnable(m_hWnd, L"ok", bEnabled);
		::SkinSetControlEnable(m_hWnd, L"netsetting", bEnabled);
		::SkinSetControlEnable(m_hWnd, L"defatulnet", bEnabled);
		::SkinSetControlEnable(m_hWnd, L"deluser", bEnabled);
		if (bEnabled)
		{
			::SkinSetControlVisible(m_hWnd, L"logongiflayout", FALSE);
			::SkinSetControlVisible(m_hWnd, L"logonbtnlayout", TRUE);
		} else
		{
			::SkinSetControlVisible(m_hWnd, L"logongiflayout", TRUE);
			::SkinSetControlVisible(m_hWnd, L"logonbtnlayout", FALSE);
		}
	}
}

void CLoginImpl::GetClientVersioin()
{
	if (!m_strClientVer.empty())
		return ;
	IConfigure *pCfg = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		CInterfaceAnsiString strVer;
		if (SUCCEEDED(pCfg->GetParamValue(TRUE, "client", "version", &strVer)))
		{
			m_strClientVer = strVer.GetData();
		} else
		{
			m_strClientVer = "0";
			TiXmlDocument xmldoc;
			std::string strVerFile;
			char szAppPath[MAX_PATH] = {0};
			char szAppName[MAX_PATH] = {0};
			CSystemUtils::GetApplicationFileName(szAppName, MAX_PATH - 1);
			CSystemUtils::ExtractFilePath(szAppName, szAppPath, MAX_PATH - 1);
			strVerFile = szAppPath;
			strVerFile += "config.xml";
			if (xmldoc.LoadFile(strVerFile.c_str()))
			{
				TiXmlElement *pNode = xmldoc.FirstChildElement();
				if (pNode)
				{
					TiXmlElement *pChild = pNode->FirstChildElement("clientversion");
					if (pChild)
					{
						if (pChild->GetText())
							m_strClientVer = pChild->GetText();
						else
							m_strClientVer = "0.0.0.0";
						pCfg->SetParamValue(TRUE, "client", "version", m_strClientVer.c_str());
					} //end if (pChild && ...
				} //end if (pNode)
			} //end if (xmldoc.LoadFile(
		} //end else if (..
		pCfg->Release();
	} //end if (SUCCEEDED(m_pCore->QueryInterface(...
}

void CLoginImpl::SaveUserInfo()
{
	if (m_pCore)
	{
		IConfigure *pCfg = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			BOOL bSaveName = ::SkinGetCheckBoxStatus(m_hWnd, L"saveusername") != 0;
			BOOL bSavePwd = ::SkinGetCheckBoxStatus(m_hWnd, L"rememberpwd") != 0;
			BOOL bAutoLogin = ::SkinGetCheckBoxStatus(m_hWnd, L"autologon") != 0;
			CInterfaceAnsiString strHost, strPort;
			CInterfaceAnsiString strDomain;
			m_pCore->GetUserDomain((IAnsiString *)&strDomain);
			if (bAutoLogin)
				pCfg->SetParamValue(TRUE, "normal", "autologin", "true");
			else
				pCfg->SetParamValue(TRUE, "normal", "autologin", "false");
			if (bSaveName)
				pCfg->SetParamValue(TRUE, "normal", "saveusername", "true");
			else
				pCfg->SetParamValue(TRUE, "normal", "saveusername", "false");
			if (bSavePwd)
				pCfg->SetParamValue(TRUE, "normal", "savepwd", "true");
			else
				pCfg->SetParamValue(TRUE, "normal", "savepwd", "false"); 

			pCfg->GetParamValue(TRUE, "Server", "Host", (IAnsiString *)&strHost);				
		    pCfg->GetParamValue(TRUE, "Server", "Port", (IAnsiString *)&strPort);
			if (bSaveName)
			{
				TCHAR szTemp[128] = {0};
			    char szUserName[128] = {0};
			    char szUserPwd[128] = {0};
			    
			    int nSize = 127;	
				CInterfaceAnsiString strUserName;
			    if (::SkinGetControlTextByName(m_hWnd, L"cb_username", szTemp, &nSize))
			    {
					CStringConversion::WideCharToString(szTemp, szUserName, 127);
					CStringConversion::WideCharToString(szTemp, szUserName, 127);
					if (FAILED(pCfg->GetUserNameByRealName(szUserName, (IAnsiString *)&strUserName)))
						strUserName.SetString(szUserName);
				}
				memset(szTemp, 0, sizeof(TCHAR) * 128);
				nSize = 127;
				if (bSavePwd && ::SkinGetControlTextByName(m_hWnd, L"edit_password", szTemp, &nSize))
				{
					CStringConversion::WideCharToString(szTemp, szUserPwd, 127);
				}
				pCfg->SetUserLoginInfo(strUserName.GetData(), szUserPwd, strDomain.GetData(), bSavePwd, 
					m_strPresence.c_str(), strHost.GetData(), atoi(strPort.GetData()));
				CInterfaceAnsiString strRealName;
				m_pCore->GetUserNickName((IAnsiString *)&strRealName);
				pCfg->SetUserRealName(strUserName.GetData(), strDomain.GetData(), strRealName.GetData());
			} else
			{
				TCHAR szTemp[128] = {0};
				int nSize = 127;
				char szUserName[128] = {0};
				CInterfaceAnsiString strUserName;
				if (::SkinGetControlTextByName(m_hWnd, L"cb_username", szTemp, &nSize))
			    {
					CStringConversion::WideCharToString(szTemp, szUserName, 127); 
					if (FAILED(pCfg->GetUserNameByRealName(szUserName, (IAnsiString *)&strUserName)))
						strUserName.SetString(szUserName);
				}
				pCfg->SetUserLoginInfo(strUserName.GetData(), NULL, strDomain.GetData(), FALSE, 
					m_strPresence.c_str(), strHost.GetData(), atoi(strPort.GetData()));
			}//end if (bSaveUserName)
		} //end if (SUCCEEDED(
		pCfg->Release();
	} //end if (m_pCore)
}

//广播消息
STDMETHODIMP CLoginImpl::DoBroadcastMessage(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData)
{
	if ((::stricmp(szFromWndName, "contacts") == 0) && (::stricmp(szType, "downorg") == 0))		
	{  
		if (m_pBalancerSocket)
			delete m_pBalancerSocket;
		m_pBalancerSocket = NULL;
		if (::stricmp(pContent, "true") == 0) 
		{		
			SaveUserInfo();
			m_bLoginSucc = TRUE;
			if (m_hWnd)
				::PostMessage(m_hWnd, WM_ORGDL_PROGRESS, 1, 0);
		} else
		{
			::SkinMessageBox(m_hWnd, L"下载联系人列表错误", L"提示", MB_OK);
			SetEnabled(TRUE); 
			ITrayMsg *pTrayMsg = NULL;
			if (m_pCore && (SUCCEEDED(m_pCore->QueryInterface(__uuidof(ITrayMsg), (void **)&pTrayMsg))))
			{
				pTrayMsg->StopAnimate();
			}
		} 
	} else if ((::stricmp(szFromWndName, "CoreFrame") == 0) && (::stricmp(szType, "connectsvr") == 0)
		&& (m_hWnd != NULL))
	{
		if (::stricmp(pContent, "succ") == 0)
		{
			//
			m_pCore->AuthUser(NULL, NULL, NULL, NULL);
		} else if (::stricmp(pContent, "failed") == 0)
		{
			::SkinMessageBox(m_hWnd, L"连接服务器失败", L"提示", MB_OK);
			SetEnabled(TRUE); 
			ITrayMsg *pTrayMsg = NULL;
			if (m_pCore && (SUCCEEDED(m_pCore->QueryInterface(__uuidof(ITrayMsg), (void **)&pTrayMsg))))
			{
				pTrayMsg->StopAnimate();
			}
		}
	} else if ((::stricmp(szFromWndName, "downorg") == 0) && (::stricmp(szType, "progress") == 0))
	{
		if (m_hWnd)
		{
			TCHAR *szTmp = NULL;
			if (pContent)
			{
				int nLen = ::strlen(pContent);
				szTmp = new TCHAR[nLen + 1];
				memset(szTmp, 0, sizeof(TCHAR) * (nLen + 1));
				CStringConversion::StringToWideChar(pContent, szTmp, nLen);
			}
			::PostMessage(m_hWnd, WM_ORGDL_PROGRESS, 0, (LPARAM)szTmp);
		} //end if (m_hWnd)
	} else if ((::stricmp(szFromWndName, "mainwindow") == 0) && (::stricmp(szType, "showcontacts") == 0)
		       && (::stricmp(pContent, "complete") == 0))
	{
		ITrayMsg *pTrayMsg = NULL;
		if (m_pCore && (SUCCEEDED(m_pCore->QueryInterface(__uuidof(ITrayMsg), (void **)&pTrayMsg))))
		{
			pTrayMsg->StopAnimate();
		}
		::SkinCloseWindow(m_hWnd); 	
	}
	return E_NOTIMPL;
}

STDMETHODIMP CLoginImpl::DoCoreEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
	                     LPARAM lParam, HRESULT *hResult)
{
	if (::stricmp(szType, "click") == 0)
	{
		*hResult = ClickEvent(hWnd, szType, szName, wParam, lParam);
	} else if (::stricmp(szType, "link") == 0)  
	{
		*hResult = LinkEvent(hWnd, szType, szName, wParam, lParam);
	} else if (::stricmp(szType, "changed") == 0)
	{
		*hResult = ChangeEvent(hWnd, szType, szName, wParam, lParam);
	} else if (::stricmp(szType, "itemselect") == 0)
	{
		*hResult = ItemSelectEvent(hWnd, szType, szName, wParam, lParam);
	} else if (::stricmp(szType, "menucommand") == 0)
	{
		*hResult = MenuCommandEvent(hWnd, szType, szName, wParam, lParam);
    } else if (::stricmp(szType, "afterinit") == 0)
	{ 
		*hResult = WindowAfterInit(hWnd, szType, szName, wParam, lParam);
	}  else //
	{
		//PRINTDEBUGLOG(dtInfo, "Logon Event, Type:%s Name:%s wParam:%d lParam:%d", szType, szName, wParam, lParam);
	}
	return E_NOTIMPL;
}

STDMETHODIMP CLoginImpl::SetCoreFrameWork(ICoreFrameWork *pCore)
{
	if (m_pCore)
	{
		m_pCore->Release();
	}
	m_pCore = pCore;
	if (m_pCore)
	{
		m_pCore->AddRef();
		//order event
		m_pCore->AddOrderEvent((ICoreEvent *) this, "LogonWindow", NULL, NULL);
		//
		//order protocol
		m_pCore->AddOrderProtocol((IProtocolParser *) this, "sys", "login");
	}
	return S_OK;
}

STDMETHODIMP CLoginImpl::GetSkinXmlString(IAnsiString *szXmlString)
{
	HRESULT hr = E_FAIL;
	IConfigure *pCfg = NULL; 
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{ 
		hr = pCfg->GetSkinXml("loginframe.xml",szXmlString); 
		pCfg->Release();
	}
	return hr;  
}

void CLoginImpl::AddUserInfoToUI(TiXmlElement *pNode, BOOL bShowRealName)
{
	if (pNode->Attribute("username") != NULL)
	{
		//::AddTreeChildNode
		TCHAR szwTmp[MAX_PATH] = {0};
		if (bShowRealName && (pNode->Attribute("realname") != NULL))
			CStringConversion::UTF8ToWideChar(pNode->Attribute("realname"), szwTmp, MAX_PATH - 1);
		else
			CStringConversion::StringToWideChar(pNode->Attribute("username"), szwTmp, MAX_PATH - 1);
		if (::lstrlen(szwTmp) > 0)
			::SkinSetDropdownItemString(m_hWnd, L"cb_username", 9999, szwTmp, NULL);
	}
}

void CLoginImpl::InitLogonWindow()
{ 
	if (m_pCore)
	{
		IConfigure *pCfg = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			CInterfaceAnsiString strXml;
			if (SUCCEEDED(pCfg->GetUserLoginUserList((IAnsiString *)&strXml)))
			{
				TiXmlDocument xml;
				if (xml.Load(strXml.GetData(), strXml.GetSize()))
				{					
					TiXmlElement *pRoot = xml.FirstChildElement(); //users node
					TiXmlElement *pChild = NULL;
					if (pRoot)
						pChild = pRoot->FirstChildElement();
					while (pChild)
					{
						AddUserInfoToUI(pChild, m_bShowRealName);
						pChild = pChild->NextSiblingElement();
					} //end while (pRoot)
					::SkinSelectDropdownItem(m_hWnd, L"cb_username", 0);
				} //end if (xml.Load(...
			} //end if (SUCCEEDED(pCfg->GetUserLoginUserList(...
			pCfg->Release();
		} //end if (SUCCEEDED(m_pCore->QueryInterface(...
	} //end if (m_pCore)
}

STDMETHODIMP CLoginImpl::ShowLogonWindow(const char *szInitUserName, const char *szInitUserPwd, BOOL bAutoLogin)
{
	HRESULT hr = E_FAIL;
	BOOL bAuto = bAutoLogin;
	if (m_pCore)
	{
		IUIManager *pUI = NULL;
		IConfigure *pCfg = NULL;		
		hr = m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg);
		if (SUCCEEDED(hr) && pCfg)
		{  
			RECT rc = {0};;
			CInterfaceAnsiString strPos;
			if (SUCCEEDED(pCfg->GetParamValue(TRUE, "Position", "LoginFrame", (IAnsiString *)&strPos)))
			{
				CSystemUtils::StringToRect(&rc, strPos.GetData());
			}
			CInterfaceAnsiString szValue;
			pCfg->GetParamValue(TRUE, "logonwindow", "showrealname", &szValue);
			m_bShowRealName = (::stricmp(szValue.GetData(), "true") == 0);
			if (::IsRectEmpty(&rc)) 
			{
				CSystemUtils::GetScreenRect(&rc);
				rc.left = (rc.right - rc.left - 486) / 2;
				rc.top = (rc.bottom - rc.top - 490) / 2;
				rc.right = rc.left + 486;
				rc.bottom = rc.top + 490;
				//rc.right -= 20;
				//rc.left = rc.right - 330;
				//rc.top = 100;
				//rc.bottom = rc.top + 530;
			}
			hr = m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI);
			if (SUCCEEDED(hr) && pUI)
			{
				pUI->CreateUIWindow(NULL, "LogonWindow", &rc, WS_POPUP | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX,
				                 0, L"", &m_hWnd);	
				InitLogonWindow();
				if (::IsWindow(m_hWnd))
				{
					if (!CSystemUtils::Show2ToolBar(m_hWnd, FALSE))
					{
						PRINTDEBUGLOG(dtInfo, "Login Window Show2Toolbar Failed");
					}
					pUI->OrderWindowMessage("LogonWindow", m_hWnd, WM_ORGDL_PROGRESS, (ICoreEvent *) this);
					::ShowWindow(m_hWnd, SW_SHOW); 
				    CSystemUtils::BringToFront(m_hWnd);
				}
				pUI->Release();
				pUI = NULL;
			}
			if ((szInitUserName != NULL) && (::strlen(szInitUserName) > 0))
				ChangeLoginUserName(m_hWnd, szInitUserName, m_bShowRealName);
			if ((szInitUserName == NULL) || (::strlen(szInitUserName) == 0))
			{ 
				CInterfaceAnsiString strTmp;
				if (SUCCEEDED(pCfg->GetParamValue(TRUE, "normal", "autologin", &strTmp)))
				{
					if (::stricmp(strTmp.GetData(), "true") == 0)
					{
						::SkinSetCheckBoxStatus(m_hWnd, L"autologon", 1);
						bAutoLogin = TRUE;
					}
				}
			}
			pCfg->Release();
			pCfg = NULL;
		} //end if (SUCCEEDED(hr)...
	} //end if (m_pCore)
	if (bAutoLogin)
		StartLogin();
	return hr;
}

STDMETHODIMP CLoginImpl::DoPresenceChange(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder)
{
	return E_NOTIMPL;
}

void ParserVersion(const char *szVer, int &nMajor, int &nMajor2, int &ver1, int &beta)
{
	std::string strVer = szVer;
	std::string strTmp;
	int nPos = strVer.find(".");
	nMajor = 0;
	nMajor2 = 0;
	ver1 = 0;
	beta = 0;
	if (nPos != std::string::npos)
	{
		strTmp = strVer.substr(0, nPos);
		nMajor = ::atoi(strTmp.c_str());
		strVer = strVer.substr(nPos + 1);
	}
	nPos = strVer.find(".");
	if (nPos != std::string::npos)
	{
		strTmp = strVer.substr(0, nPos);
		nMajor2 = ::atoi(strTmp.c_str());
		strVer = strVer.substr(nPos + 1);
	}
	nPos = strVer.find(".");
	if (nPos != std::string::npos)
	{
		strTmp = strVer.substr(0, nPos);
		ver1 = ::atoi(strTmp.c_str());
		strVer = strVer.substr(nPos + 1);
	}
	beta = ::atoi(strVer.c_str()); 
}

BOOL NewVersionHighCurrent(const char *szCurr, const char *szNew)
{
	PRINTDEBUGLOG(dtInfo, "CurrentVersion:%s   Newest Version:%s", szCurr, szNew);
	if (szCurr && szNew)
	{
		//
		if (::stricmp(szCurr, szNew) != 0)
		{
			int nCurrMajor, nCurrMajor2, nCurrVer, nCurrBeta;
			int nNewMajor, nNewMajor2, nNewVer, nNewBeta;
			ParserVersion(szCurr, nCurrMajor, nCurrMajor2, nCurrVer, nCurrBeta);
			ParserVersion(szNew, nNewMajor, nNewMajor2, nNewVer, nNewBeta);
			if (nNewMajor == nCurrMajor)
			{
				if (nNewMajor2 == nCurrMajor2)
				{
					if (nNewVer == nCurrVer)
					{
						if (nNewBeta > nCurrBeta)
							return TRUE;
					} else if (nNewVer > nCurrVer)
						return TRUE;
				} else if (nNewMajor2 > nCurrMajor2)
					return TRUE;
			} else if (nNewMajor > nCurrMajor)
				return TRUE;
		}
	}
	return FALSE;
}

BOOL CLoginImpl::OnRecvProtocol(const char *szBuf, const int nBufSize)
{
	static char SYS_GET_BALANCER_SVR_XML[] = "<sys type=\"b_getgocomserver\"/>";
	TiXmlDocument xml;
	if (xml.Load(szBuf, nBufSize))
	{
		TiXmlElement *pNode = xml.FirstChildElement();
		if (pNode)
		{
			const char *pValue = pNode->Value();
			const char *pType = pNode->Attribute("type");
			if (pValue && pType)
			{
				if (::stricmp(pValue, "sys") == 0)
				{
					if (::stricmp(pType, "challenge2010") == 0)
					{ 
						if (pNode->Attribute("from") != NULL)
						{
							if (::stricmp(pNode->Attribute("from"), "balance") == 0)
								m_bFromBal = TRUE;
						}

						if (m_pBalancerSocket)
						{
							GetClientVersioin();
							std::string strXml = "<sys type=\"version2010\" domain=\"GoCom\" version=\""; 
							if (!m_strClientVer.empty())
								strXml += m_strClientVer;
							strXml += "\"/>";
							m_pBalancerSocket->SendRawData(strXml.c_str(), (int) strXml.size());
						}
					} else if (::stricmp(pType, "version2010") == 0)
					{
						//check version <sys type="version2010" domain="" result="ok" newversion="" url="" memo=""  httpip=""  httpport=""/>;

						//
						if (pNode->Attribute("result"))
						{
							if (::stricmp(pNode->Attribute("result"), "ok") != 0)
							{
								if ((pNode->Attribute("newversion") != NULL)
									&& (NewVersionHighCurrent(m_strClientVer.c_str(), pNode->Attribute("newversion"))))
								{
									if ((pNode->Attribute("httpip") != NULL) && (pNode->Attribute("httpport") != NULL)
										|| (pNode->Attribute("url") != NULL))
									{
										std::string strUrl;
										std::string strHttpSvr = pNode->Attribute("httpip");

										if (stricmp(strHttpSvr.c_str(), "127.0.0.1") == 0)
										{
											CInterfaceAnsiString strTmp;
											IConfigure *pCfg = NULL;
											if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
											{
												pCfg->GetParamValue(TRUE, "Server", "Host", &strTmp);
												pCfg->Release();
											} 
											if (strTmp.GetSize() > 0)
											{ 
												strHttpSvr = strTmp.GetData();
											}
										} 
										strUrl = "http://"; 
										strUrl += strHttpSvr;
										strUrl += ":";
										strUrl += pNode->Attribute("httpport"); 
										strUrl += "/update/";
										strUrl += pNode->Attribute("url");
										if (pNode->Attribute("newversion") != NULL)
										{
											//IConfigure *pCfg = NULL;
											//if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
											//{
											//	pCfg->SetParamValue(TRUE, "client", "version", pNode->Attribute("newversion"));
											//	pCfg->Release();
											//}
										}
										UpdateClientVersion(strHttpSvr.c_str(), pNode->Attribute("httpport"),
											strUrl.c_str(), m_strClientVer.c_str());
									} //end if (pNode->Attribute(..
								}  
							} //
							if (m_pBalancerSocket && m_bFromBal)
							{
								m_pBalancerSocket->SendRawData(SYS_GET_BALANCER_SVR_XML, (int) ::strlen(SYS_GET_BALANCER_SVR_XML));
							}  else
							{ 
								m_pCore->EstablishSafeSocket(m_strLoginHost.c_str(), m_wLoginPort);  
							} 
						} //end if (pNode->Attribute(..
					} else if (::stricmp(pType, "b_getgocomserver") == 0)
					{
						const char *szOk = pNode->Attribute("result");
						const char *szIp = pNode->Attribute("ip");
						const char *szPort = pNode->Attribute("port");
						if (::stricmp(szOk, "ok") == 0)
						{
							//
							if (m_pCore)
								m_pCore->EstablishSafeSocket(szIp, ::atoi(szPort));
						} else
						{
							 ::SkinMessageBox(m_hWnd, L"没有可用的连接服务器", L"提示", MB_OK);
							 SetEnabled(TRUE);
						}
					} //end else if (::stricmp(pType, "b_getgocomserver") == 0)
				} //end if (::stricmp(pValue, "sys") == 0)
			} // end if (pValue && pType)
		} //end if (pNode)
		return TRUE;
	} else //else if (xml.Load(szBuf, nBufSize))
		return FALSE;
}

BOOL CALLBACK UpdateFilesFromSvr(const char *szVerFile, const char *szUrl, const char *szTempPath);

 

void  CLoginImpl::UpdateClientVersion(const char *szHttpIp, const char *szPort, const char *szUrl, const char *szCurVer)
{
	//
	char szPath[MAX_PATH] = {0};
	CSystemUtils::GetLocalAppPath(szPath, MAX_PATH - 1);
	strcat(szPath, "\\CoLine\\Updater\\");
	CSystemUtils::ForceDirectories(szPath);
	std::string strVerFile;
	char szAppPath[MAX_PATH] = {0};
	char szAppName[MAX_PATH] = {0};
	CSystemUtils::GetApplicationFileName(szAppName, MAX_PATH - 1);
 
	std::string strParams = "\"";
	strParams += szAppName;
	CSystemUtils::ExtractFilePath(szAppName, szAppPath, MAX_PATH - 1);
	strVerFile = szAppPath;
	strVerFile += "atoupdex.exe";
	if (CSystemUtils::FileIsExists(strVerFile.c_str()))
	{
		strParams += "\" \""; 
		strParams += szUrl;
		strParams += "\" ";
		strParams += szHttpIp;
		strParams += " ";
		strParams += szPort;
		strParams += " ";
		strParams += szCurVer;  
		PRINTDEBUGLOG(dtInfo, "update client params:%s", strParams.c_str());
		if (CSystemUtils::StartShellProcessor(strVerFile.c_str(), strParams.c_str(), NULL, FALSE))
		{ 
			//exit application
			::SkinCloseWindow(m_hWnd);
			IUIManager *pUI = NULL;
			HWND h = NULL; 
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
			{
				pUI->GetWindowHWNDByName("mainwindow", &h);
				pUI->Release();
			}
			if (h) 
				::PostMessage(h, WM_QUIT, 0, 0);
		}
	} //end if (CSystemUtils::
}

void CLoginImpl::OnSocketClose(CAsyncNetIoSocket *pSocket, const int nErrorNo)
{
	//
	if (pSocket == m_pBalancerSocket)
	{
		//
		 
	}
}

void CLoginImpl::OnSocketConnect(CAsyncNetIoSocket *pSocket, const int nErrorNo)
{
	if ((pSocket == m_pBalancerSocket) && (nErrorNo == 0))
	{
		std::string strXml = "<sys type=\"challenge2010\"/>";
		m_pBalancerSocket->SendRawData(strXml.c_str(), (int) strXml.size());
	} else
	{
		if (m_hWnd && ::IsWindow(m_hWnd))
		{
			//
			SetEnabled(TRUE);
			::SkinMessageBox(m_hWnd, L"连接服务器失败", L"提示", MB_OK);
		} else
			m_pCore->BroadcastMessage("LogonWindow", NULL, "login", "failed", NULL);
		ITrayMsg *pTrayMsg = NULL;
		if (m_pCore && (SUCCEEDED(m_pCore->QueryInterface(__uuidof(ITrayMsg), (void **)&pTrayMsg))))
		{
			pTrayMsg->StopAnimate();
		}
	}
}

STDMETHODIMP CLoginImpl::CoreFrameWorkError(int nErrorNo, const char *szErrorMsg)
{
	if (szErrorMsg)
	{
		if (m_hWnd && ::IsWindow(m_hWnd) && ::IsWindowVisible(m_hWnd) && (!m_bLoginSucc))
		{
			int nSize = ::strlen(szErrorMsg);
			TCHAR *szTmp = new TCHAR[nSize + 1];
			memset(szTmp, 0, sizeof(TCHAR) * (nSize + 1));
			CStringConversion::StringToWideChar(szErrorMsg, szTmp, nSize);
			::SkinMessageBox(m_hWnd, szTmp, L"提示", MB_OK);
			SetEnabled(TRUE);
			delete []szTmp;
		}
	}
	return E_FAIL;
}

STDMETHODIMP CLoginImpl::DoWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes)
{
	if (uMsg == WM_DESTROY && m_hWnd == hWnd)
	{
		RECT rc = {0};
		if (::GetWindowRect(hWnd, &rc))
		{
			std::string strRect;
			if (CSystemUtils::RectToString(rc, strRect))
			{
				if (m_pCore)
				{
					IConfigure *pCfg = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
					{
						pCfg->SetParamValue(TRUE, "Position", "LoginFrame", strRect.c_str());
						pCfg->Release();
					} //end if (SUCCEEDED(...
				} //end if (m_pCore)
			} //end if (CSystemUtils::RectToString(...
		} //end if (::GetWindowRect(hWnd...
		m_hWnd = NULL;
		return S_OK;
	} else if ((uMsg == WM_ORGDL_PROGRESS) && (hWnd == m_hWnd))
	{
		if (wParam == 0)
		{
			if (lParam != 0)
			{
				TCHAR *szTmp = (TCHAR *)lParam;
				::SkinSetControlTextByName(hWnd, L"lblprogress", szTmp);
				::SkinUpdateControlUI(hWnd, L"lblprogress");
			    delete []szTmp;
			} else
			{
				::SkinSetControlVisible(hWnd, L"lblprogress", TRUE);
				::SkinSetControlTextByName(hWnd, L"logintip", L"正在下载组织结构");
				::SkinUpdateControlUI(hWnd, L"logintip");
			}
		} else if (wParam == 1)
		{
			::SkinSetControlVisible(hWnd, L"lblprogress", FALSE);
			::SkinSetControlTextByName(hWnd, L"logintip", L"正在加载组织结构");
			::SkinUpdateControlUI(hWnd, L"logintip");
		}
	}
	return E_FAIL;
}

STDMETHODIMP CLoginImpl::LogonSvr(const char *szUserName, const char *szUserPwd, const char *szDomain, 
	                              const char *szPresence, const char *szPresenceMemo)
{
	m_bLoginSucc = FALSE;
	IConfigure *pCfg = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{ 	 
		SetEnabled(FALSE);
		m_pCore->InitSafeSocket();
		if (szUserName && szUserPwd)
		{
			if (szPresence)
				m_strPresence = szPresence;
			if (szPresenceMemo)
				m_strPresenceMemo = szPresenceMemo;
			m_pCore->AuthUser(szUserName, szUserPwd, m_strPresence.c_str(), m_strPresenceMemo.c_str());
		} else
			m_pCore->AuthUser(NULL, NULL, NULL, NULL);
        //
		ITrayMsg *pTrayMsg = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(ITrayMsg), (void **)&pTrayMsg)))
		{
			HICON h = ::LoadIcon((HINSTANCE)::GetModuleHandle(L"login.plg"), L"LOGIN_1");
			pTrayMsg->AddAnimateIcon(h);
			h = ::LoadIcon((HINSTANCE)::GetModuleHandle(L"login.plg"), L"LOGIN_2");
			pTrayMsg->AddAnimateIcon(h);
			h = ::LoadIcon((HINSTANCE)::GetModuleHandle(L"login.plg"), L"LOGIN_3");
			pTrayMsg->AddAnimateIcon(h);
			pTrayMsg->StartAnimate("CoLine 正在登录");
		}

		m_wLoginPort = DEFAULT_SERVER_PORT;

		CInterfaceAnsiString strTmp;
		if (SUCCEEDED(pCfg->GetParamValue(TRUE, "Server", "Host", (IAnsiString *)&strTmp)))
			m_strLoginHost = strTmp.GetData();
		if (SUCCEEDED(pCfg->GetParamValue(TRUE, "Server", "Port", (IAnsiString *)&strTmp)))
			m_wLoginPort = ::atoi(strTmp.GetData());
			
		if (m_strLoginHost.empty())
			m_strLoginHost = "127.0.0.1";
		if (m_wLoginPort == 0)
			m_wLoginPort = DEFAULT_SERVER_PORT;

		
		//登陆
		if (m_pBalancerSocket)
			delete m_pBalancerSocket;
		m_pBalancerSocket = NULL;
		m_pBalancerSocket = new CProtoInterfaceSocket(this);
		if (!m_pBalancerSocket->Connect(m_strLoginHost.c_str(), m_wLoginPort))
		{
			if (m_hWnd && ::IsWindow(m_hWnd))
			{
				SetEnabled(TRUE);
				::SkinMessageBox(m_hWnd, L"连接服务器失败", L"提示", MB_OK);
			} else
			{
				m_pCore->BroadcastMessage("LogonWindow", NULL, "login", "failed", NULL);
			}
		}
		
		pCfg->Release();
		return S_OK;
	}
	return E_FAIL;
}

//IProtocolParser
STDMETHODIMP CLoginImpl::DoRecvProtocol(const BYTE *pData, const LONG lSize)
{
	//
	TiXmlDocument xml;
	if (xml.Load((char *)pData, lSize))
	{
		TiXmlElement *pNode = xml.FirstChildElement();
		const char *pValue = pNode->Value();
		const char *pType = pNode->Attribute("type");
		if (pValue && pType)
		{
			if (::stricmp(pValue, "sys") == 0)
			{
				if (::stricmp(pType, "login") == 0)
				{
					const char *szResult = pNode->Attribute("result");
					if (szResult && (::stricmp(szResult, "ok") != 0))
					{
						SetEnabled(TRUE);
						::SkinMessageBox(m_hWnd, L"登陆失败\n请确认用户名和密码是否正确", L"提示", MB_OK);
					} else
					{
						//::SendMessage(m_hWnd, WM_CLOSE, 0, 0);
					}//end else if (szResult..
				} //end if (::stricmp(pType..
			} //end if (::stricmp(pValue ....
		} //end if (pValue && pType)
		return S_OK;
	} //end if (xml.Load(...
	return E_FAIL;
}

void CLoginImpl::InitDefaultSvrParam(HWND hWnd)
{
	char szAppPathFileName[MAX_PATH] = {0};
	char szAppPath[MAX_PATH] = {0};
	CSystemUtils::GetApplicationFileName(szAppPathFileName, MAX_PATH - 1);
	CSystemUtils::ExtractFilePath(szAppPathFileName, szAppPath, MAX_PATH - 1);
    
    TiXmlDocument xml;
	std::string strConfigXmlFile = szAppPath; 
	strConfigXmlFile += GLOBAL_CONFIG_FILE_NAME;
	if (xml.LoadFile(strConfigXmlFile.c_str()))
	{
		TiXmlElement *pNode = xml.FirstChildElement("config");
		if (pNode)
		{ 
		  
			TiXmlElement *pChild = pNode->FirstChildElement("serverip");

			TCHAR szTmp[MAX_PATH] = {0};
			if (pChild && pChild->GetText() != NULL)
			{
				CStringConversion::StringToWideChar(pChild->GetText(), szTmp, MAX_PATH - 1);
		        ::SkinSetControlTextByName(hWnd, L"svrhostedt", szTmp); 
			}
			pChild = pNode->FirstChildElement("serverport");
			if (pChild && pChild->GetText() != NULL)
			{
				memset(szTmp, 0, MAX_PATH * sizeof(TCHAR));
				CStringConversion::StringToWideChar(pChild->GetText(), szTmp, MAX_PATH - 1);
				::SkinSetControlTextByName(hWnd, L"svrportedt", szTmp);
			}
			pChild = pNode->FirstChildElement("defaultdomain");
			if (pChild && pChild->GetText() != NULL)
			{
				memset(szTmp, 0, MAX_PATH * sizeof(TCHAR));
				CStringConversion::StringToWideChar(pChild->GetText(), szTmp, MAX_PATH - 1);
				::SkinSetControlTextByName(hWnd, L"resourceedt", szTmp);
			} 
			 
		}
	}
}
#pragma warning(default:4996)
