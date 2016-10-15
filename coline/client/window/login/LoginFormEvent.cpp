#include <Commonlib/DebugLog.h>
#include <Commonlib/stringutils.h>
#include <Commonlib/systemutils.h>
#include <SmartSkin/smartskin.h>
#include <Core/common.h>

#include "../IMCommonLib/InterfaceAnsiString.h"
#include "LoginImpl.h"

#pragma warning(disable:4996)


void CLoginImpl::StartLogin()
{
	if (m_pCore)
	{
		IConfigure *pCfg = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			TCHAR szTemp[128] = {0};
			char szUserName[128] = {0};
			char szUserPwd[128] = {0};
			char szDomain[128] = {0};
			int nSize = 127;
			CInterfaceAnsiString strUserName;
			if (::SkinGetControlTextByName(m_hWnd, L"cb_username", szTemp, &nSize))
			{
				CStringConversion::WideCharToString(szTemp, szUserName, 127);
				if (FAILED(pCfg->GetUserNameByRealName(szUserName, (IAnsiString *)&strUserName)))
				{
					strUserName.SetString(szUserName);
				}
			}
			memset(szTemp, 0, sizeof(TCHAR) * 128);
			nSize = 127;
			if (::SkinGetControlTextByName(m_hWnd, L"edit_password", szTemp, &nSize))
			{
				CStringConversion::WideCharToString(szTemp, szUserPwd, 127);
			}
			if ((strUserName.GetSize() > 0) && (::strlen(szUserPwd) > 0))
			{
				LogonSvr(strUserName.GetData(), szUserPwd, NULL, NULL, NULL);
			} else
			{
				::SkinMessageBox(m_hWnd, L"登陆名和密码不能为空", L"提示", MB_OK);
			}
			pCfg->Release();
		} //end if (SUCCEEDED(m_pCore->QueryInterface(...
	} //end if (m_pCore)
}

void CLoginImpl::SaveNetSetting(HWND hWnd)
{
	if (m_pCore)
	{
		IConfigure *pCfg = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			TCHAR szwTmp[MAX_PATH] = {0};
			char szTmp[MAX_PATH] = {0};
			int nSize = MAX_PATH - 1;
			::SkinGetControlTextByName(hWnd, L"svrhostedt", szwTmp, &nSize);
			CStringConversion::WideCharToString(szwTmp, szTmp, MAX_PATH - 1);
			pCfg->SetParamValue(TRUE, "Server", "Host", "211.100.61.215");
			memset(szwTmp, 0, sizeof(TCHAR) * MAX_PATH);
			memset(szTmp, 0, MAX_PATH);
			nSize = MAX_PATH - 1;
			::SkinGetControlTextByName(hWnd, L"svrportedt", szwTmp, &nSize);
			CStringConversion::WideCharToString(szwTmp, szTmp, MAX_PATH - 1);
			pCfg->SetParamValue(TRUE, "Server", "Port", "9000");
			memset(szwTmp, 0, sizeof(TCHAR) * MAX_PATH);
			memset(szTmp, 0, MAX_PATH);
			nSize = MAX_PATH - 1;
			::SkinGetControlTextByName(hWnd, L"resourceedt", szwTmp, &nSize);
			CStringConversion::WideCharToString(szwTmp, szTmp, MAX_PATH - 1);
			pCfg->SetParamValue(TRUE, "Server", "domain", "coline");
			
			pCfg->Release();
		} //end if (SUCCEEDED(m_pCore->QueryInterface(...
	} //end if (m_pCore)
}

HRESULT CLoginImpl::ClickEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
		                     LPARAM lParam)
{
	if ((hWnd == m_hWnd) && ::stricmp(szName, "ok") == 0)
	{
		SaveNetSetting(hWnd);
		StartLogin();
	} else if (::stricmp(szName, "loginsetok") == 0)
	{
		SaveNetSetting(hWnd);
		::SkinCloseWindow(hWnd);
	} else if (::stricmp(szName, "cancel") == 0)
	{
		if (hWnd != m_hWnd)
			::SkinCloseWindow(hWnd);
	}  else if (::stricmp(szName, "minbutton") == 0)
	{
		::ShowWindow(m_hWnd, SW_HIDE);
		return 0;
	} else if (::stricmp(szName, "closebutton") == 0)
	{
		if (hWnd == m_hWnd)
		{
			::ShowWindow(m_hWnd, SW_HIDE);
			return 0;
		}
	} else if (::stricmp(szName, "cancellogon") == 0)
	{
		SetEnabled(TRUE);
		if (m_pBalancerSocket)
			m_pBalancerSocket->Close();
		if (m_pCore)
		{
			//
			m_pCore->Offline();
		}
	} else if (::stricmp(szName, "netsetting") == 0)
	{
		TCHAR szValue[32] = {0};
		int nSize = 32;
		if (::SkinGetControlAttr(hWnd, L"netsetting", L"down", szValue, &nSize))
		{
			if (::lstrcmpi(szValue, L"true") == 0)
			{
				::SkinSetControlVisible(hWnd, L"netsettingarea", FALSE);
				::SkinSetControlVisible(hWnd, L"corplogo", TRUE);
				::SkinSetControlAttr(hWnd, L"netsetting", L"down", L"false");
			} else
			{
				::SkinSetControlVisible(hWnd, L"corplogo", FALSE);
				::SkinSetControlVisible(hWnd, L"netsettingarea", TRUE);
				::SkinSetControlAttr(hWnd, L"netsetting", L"down", L"true");
			}
		} //end if (::SkinGetControlAttr	
	} else if (::stricmp(szName, "defatulnet") == 0)
	{
		InitDefaultSvrParam(hWnd);
	} else if (::stricmp(szName, "deluser") == 0)
	{
		DeleteCurrUser(hWnd);
	} else
	{
		//PRINTDEBUGLOG();
	} //end else if (::strcmp(szName, "login") == 0)
    return -1;
}

STDMETHODIMP CLoginImpl::CancelLogon()
{
	if (m_pBalancerSocket)
	{
		delete m_pBalancerSocket;
		m_pBalancerSocket = NULL;
	}
	return S_OK;
}

//
void CLoginImpl::DeleteCurrUser(HWND hWnd)
{
	IConfigure *pCfg = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		TCHAR szTemp[128] = {0};
		int nSize = 127;
		char szUserName[128] = {0};
		CInterfaceAnsiString strUserName;
		CInterfaceAnsiString strDomain; 
		m_pCore->GetUserDomain((IAnsiString *)&strDomain);
		if (::SkinGetControlTextByName(hWnd, L"cb_username", szTemp, &nSize))
	    {
			CStringConversion::WideCharToString(szTemp, szUserName, 127);
			CStringConversion::WideCharToString(szTemp, szUserName, 127);
			if (FAILED(pCfg->GetUserNameByRealName(szUserName, (IAnsiString *)&strUserName)))
				strUserName.SetString(szUserName);
		} 
		if (strUserName.GetSize() > 0)
		{
			CStdString_ strTip = L"是否要删除在本机保存用户 <" ;
			strTip += szTemp;
			strTip += L">的登陆信息，但历史记录不会删除";
			if (::SkinMessageBox(hWnd, strTip.GetData(), L"提示", 2) == IDOK)
			{
				pCfg->SetUserLoginInfo(strUserName.GetData(), NULL, strDomain.GetData(), FALSE, 
					m_strPresence.c_str(), NULL, 0);
				int idx = ::SkinGetDropdownSelectIndex(hWnd, L"cb_username");
				if (idx >= 0)
					::SkinDeleteDropdownItem(hWnd, L"cb_username", idx);
				if (::SkinGetDropdownItemCount(hWnd, L"cb_username") > 0)
					::SkinSelectDropdownItem(hWnd, L"cb_username", 0);
				else
				{ 
					::SkinSetControlTextByName(hWnd, L"edit_password", L"");
					::SkinSetCheckBoxStatus(hWnd, L"saveusername", 0);
					::SkinSetCheckBoxStatus(hWnd, L"rememberpwd", 0);
					::SkinSetCheckBoxStatus(hWnd, L"autologon", 0);
					::SkinSetControlTextByName(hWnd, L"cb_username", L"");
				}
			}
		} else
		{
			::SkinMessageBox(hWnd, L"用户帐号为空", L"提示", MB_OK);
		}
		pCfg->Release();
	}
}

HRESULT CLoginImpl::LinkEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szName, "settings") == 0)
	{
		IUIManager *pUI = NULL;
		if (m_pCore)
		{
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
			{
				m_pCore->AddOrderEvent((ICoreEvent *) this, "LoginSetting", NULL, NULL);
				if (SUCCEEDED(pUI->ShowModalWindow(m_hWnd, "LoginSetting", L"网络设置", 0, 0, 260, 180, NULL)))
				{
					 //
				}
				pUI->Release();
			} //end if (SUCCEEDED(
		} //end if (m_pCore)
		return 0;
	} else if (::stricmp(szName, "delUserName") == 0)
	{
		DeleteCurrUser(hWnd);
	}//end if (::stricmp(szName,..
	return -1;
}

HRESULT CLoginImpl::ChangeEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szName, "autologon") == 0)
	{
		int nStatus = ::SkinGetCheckBoxStatus(m_hWnd, L"autologon");
		if (nStatus > 0)
		{
			::SkinSetCheckBoxStatus(m_hWnd, L"saveusername", nStatus);
			::SkinSetCheckBoxStatus(m_hWnd, L"rememberpwd", nStatus);
			::SkinSetControlEnable(m_hWnd, L"saveusername", FALSE);
			::SkinSetControlEnable(m_hWnd, L"rememberpwd", FALSE);
		} else
		{
			::SkinSetControlEnable(m_hWnd, L"saveusername", TRUE);
			::SkinSetControlEnable(m_hWnd, L"rememberpwd", TRUE);
		}
		return 0;
	} else if (::stricmp(szName, "rememberpwd") == 0)
	{
		int nStatus = ::SkinGetCheckBoxStatus(m_hWnd, L"rememberpwd");
		if (nStatus > 0)
		{
			::SkinSetCheckBoxStatus(m_hWnd, L"saveusername", nStatus);
			::SkinSetControlEnable(m_hWnd, L"saveusername", FALSE);
		} else
		{
			::SkinSetCheckBoxStatus(m_hWnd, L"autologon", FALSE);
			::SkinSetControlEnable(m_hWnd, L"saveusername", TRUE);
		}
		return 0;
	} else if (::stricmp(szName, "saveusername") == 0)
	{
		int nStatus = ::SkinGetCheckBoxStatus(m_hWnd, L"saveusername");
		if (nStatus == 0)
		{
			::SkinSetCheckBoxStatus(m_hWnd, L"autologon", FALSE);
			::SkinSetCheckBoxStatus(m_hWnd, L"rememberpwd", FALSE);
		}
	}
	return -1;
}

//
void CLoginImpl::ChangeLoginUserName(HWND hWnd, const char *szUserName, BOOL bShowRealName)
{
	::SkinSetControlTextByName(hWnd, L"edit_password", L"");
	::SkinSetCheckBoxStatus(hWnd, L"saveusername", 0);
	::SkinSetCheckBoxStatus(hWnd, L"rememberpwd", 0);
	::SkinSetCheckBoxStatus(hWnd, L"autologon", 0);
	CInterfaceAnsiString strPwd, strUserName, strDomain, strSvrHost, strRealName, strSvrPort, strPic, strStatus;
	TCHAR szwUserName[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szUserName, szwUserName, MAX_PATH - 1);
	::SkinSetControlTextByName(hWnd, L"cb_username", szwUserName); 
	IConfigure *pCfg = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		if (SUCCEEDED(pCfg->GetUserInfoByRealName(szUserName, (IAnsiString *)&strUserName, (IAnsiString *)&strDomain,
			(IAnsiString *)&strPwd, (IAnsiString *)&strPic, &strRealName, (IAnsiString *)&strStatus, (IAnsiString *)&strSvrHost,
			(IAnsiString *)&strSvrPort)))
		{
			if (strStatus.GetSize() > 0)
				m_strPresence = strStatus.GetData();
			else
				m_strPresence = "online";
			if (bShowRealName && (strRealName.GetSize() > 0))
			{
				memset(szwUserName, 0, sizeof(TCHAR) * MAX_PATH);
				CStringConversion::UTF8ToWideChar(strRealName.GetData(), szwUserName, MAX_PATH - 1);
				::SkinSetControlTextByName(hWnd, L"cb_username", szwUserName);
			}
			//
			if (strPwd.GetSize() > 0)
			{
				TCHAR szwPwd[MAX_PATH] = {0};
				CStringConversion::StringToWideChar(strPwd.GetData(), szwPwd, MAX_PATH - 1);
				::SkinSetControlTextByName(m_hWnd, L"edit_password", szwPwd);
				::SkinSetCheckBoxStatus(m_hWnd, L"rememberpwd", 1);

			}
			::SkinSetCheckBoxStatus(m_hWnd, L"saveusername", 1);
			TCHAR szwStatus[32] = {0};
			lstrcpy(szwStatus, L"在线");
			int nMenuId = 1;
			if (::stricmp(m_strPresence.c_str(), "away") == 0)
			{
				nMenuId = 2;
				lstrcpy(szwStatus, L"离开");
				m_strPresenceMemo = "离开";
			} else if (::stricmp(m_strPresence.c_str(), "busy") == 0)
			{
				nMenuId = 3;
				lstrcpy(szwStatus, L"繁忙");
				m_strPresenceMemo = "繁忙";
			} else if (::stricmp(m_strPresence.c_str(), "appearoffline") == 0)
			{
				nMenuId = 4;
				lstrcpy(szwStatus, L"隐身");
				m_strPresenceMemo = "隐身";
			}
			::SkinSetMenuChecked(m_hWnd, L"menuStatus", nMenuId, TRUE);
			int nIdx = GetSubIdxByPresence(m_strPresence.c_str());
			TCHAR szIdx[32] = {0};
			wsprintf(szIdx, L"%d", nIdx);
			::SkinSetControlAttr(m_hWnd, L"cb_logonstatus", L"subimage", szIdx);
			::SkinSetControlTextByName(m_hWnd, L"cb_logonstatus", szwStatus);
			pCfg->SetParamValue(TRUE, "login", "domain", strDomain.GetData());
			pCfg->SetParamValue(TRUE, "Server", "Host", strSvrHost.GetData());
			pCfg->SetParamValue(TRUE, "Server", "Port", strSvrPort.GetData());
			
			//同步到界面
			TCHAR szTmp[MAX_PATH] = {0};
		 
			CStringConversion::StringToWideChar(strSvrHost.GetData(), szTmp, MAX_PATH - 1);
			::SkinSetControlTextByName(hWnd, L"svrhostedt", szTmp);
			memset(szTmp, 0, MAX_PATH * sizeof(TCHAR));
			CStringConversion::StringToWideChar(strSvrPort.GetData(), szTmp, MAX_PATH - 1);
			::SkinSetControlTextByName(hWnd, L"svrportedt", szTmp);
			memset(szTmp, 0, MAX_PATH * sizeof(TCHAR));
			CStringConversion::StringToWideChar(strDomain.GetData(), szTmp, MAX_PATH - 1);
			::SkinSetControlTextByName(hWnd, L"resourceedt", szTmp);
			 
		} //end if SUCCEEDED(pCfg...
		pCfg->Release();
	} //end if (SUCCEEDED(m_pCore... 
}

HRESULT CLoginImpl::ItemSelectEvent(HWND hWnd, const char *szType, const char *szName,
	                                WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szName, "cb_username") == 0)
	{
		TCHAR szName[MAX_PATH] = {0};
		int nSize = MAX_PATH - 1;
		if (::SkinGetControlTextByName(hWnd, L"cb_username", szName, &nSize) && m_pCore)
		{
			char szRealName[MAX_PATH] = {0};
			CStringConversion::WideCharToString(szName, szRealName, MAX_PATH - 1);
			ChangeLoginUserName(hWnd, szRealName, m_bShowRealName);
		} //end if (::stricmp(szName...
	}
	return -1;
}

HRESULT CLoginImpl::MenuCommandEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szName, "menuStatus") == 0)
	{
		switch(wParam)
		{
			case 1: 
				m_strPresence = "online";
				m_strPresenceMemo = "在线";
				::SkinSetControlTextByName(m_hWnd, L"cb_logonstatus", L"在线");
				break;
			case 2:
				m_strPresence = "away";
				m_strPresenceMemo = "离开";
				::SkinSetControlTextByName(m_hWnd, L"cb_logonstatus", L"离开");
				break;
			case 3:
				m_strPresence = "busy";
				m_strPresenceMemo = "繁忙";
				::SkinSetControlTextByName(m_hWnd, L"cb_logonstatus", L"繁忙");
				break;
			case 4:
				m_strPresence = "appearoffline";
				m_strPresenceMemo = "隐身";
				::SkinSetControlTextByName(m_hWnd, L"cb_logonstatus", L"隐身");
				break;
		} //
		int nIdx = GetSubIdxByPresence(m_strPresence.c_str());
		TCHAR szIdx[32] = {0};
		wsprintf(szIdx, L"%d", nIdx);
		::SkinSetControlAttr(m_hWnd, L"cb_logonstatus", L"subimage", szIdx);
	} else if (::stricmp(szName, "loginshortmenu") == 0)
	{
		switch(wParam)
		{
		case 50001:
			{				
				IConfigureUI *pUI = NULL;
				if (m_pCore)
				{
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigureUI), (void **)&pUI)))
					{
						pUI->Navegate2Frame(NULL, "normalpage");
						pUI->Release();
					} //end if (SUCCEEDED(
				} //end if (m_pCore) 
			}
			break;
		case 100:
			{
				COLORREF cr;
				if (CSystemUtils::OpenColorDialog(NULL, m_hWnd, cr))
				{
					if (m_pCore)
					{
						IUIManager *pUI = NULL;
						if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
						{
							if (SUCCEEDED(pUI->BlendSkinStyle(cr)))
							{
								IConfigure *pCfg = NULL;
								if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
								{
									char szTmp[16] = {0};
									::itoa(cr, szTmp, 10);
									pCfg->SetParamValue(TRUE, "Skin", "background", szTmp);
									pCfg->Release();
								} //end if (SUCCEEDED(m_pCore->...
							} //end if (SUCCEEDED(pUI->...
							pUI->Release();
						} //end if (SUCCEEDED(m_pCore->
					} //end if (m_pCore)
				} //end if (CSystemUtils::OpenColorDialog(NULL...
			} //end case 50001
		    break; 
		}
	}
	return -1;
}

HRESULT CLoginImpl::WindowAfterInit(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if ((::stricmp(szName, "LoginSetting") == 0) || (::stricmp(szName, "LogonWindow") == 0))
	{
		IConfigure *pCfg = NULL;
		if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			CInterfaceAnsiString strTmp;
			TCHAR szTmp[MAX_PATH] = {0};
			if (SUCCEEDED(pCfg->GetParamValue(TRUE, "Server", "Host", (IAnsiString *)&strTmp)))
			{
				CStringConversion::StringToWideChar(strTmp.GetData(), szTmp, MAX_PATH - 1);
				::SkinSetControlTextByName(hWnd, L"svrhostedt", szTmp);
			}
			if (SUCCEEDED(pCfg->GetParamValue(TRUE, "Server", "Port", (IAnsiString *)&strTmp)))
			{
				memset(szTmp, 0, MAX_PATH * sizeof(TCHAR));
				CStringConversion::StringToWideChar(strTmp.GetData(), szTmp, MAX_PATH - 1);
				::SkinSetControlTextByName(hWnd, L"svrportedt", szTmp);
			}
			if (SUCCEEDED(pCfg->GetParamValue(TRUE, "login", "domain", (IAnsiString *)&strTmp)))
			{
				memset(szTmp, 0, MAX_PATH * sizeof(TCHAR));
				CStringConversion::StringToWideChar(strTmp.GetData(), szTmp, MAX_PATH - 1);
				::SkinSetControlTextByName(hWnd, L"resourceedt", szTmp);
			}
			pCfg->Release();
		}
		return 0;
	}
	return -1;
}

#pragma warning(default:4996)
