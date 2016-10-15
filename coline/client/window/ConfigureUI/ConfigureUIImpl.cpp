#include <Commonlib/systemutils.h>
#include <Commonlib/stringutils.h>
#include <SmartSkin/smartskin.h>
#include <Commonlib/DebugLog.h>
#include "../IMCommonLib/InterfaceAnsiString.h"
#include "../IMCommonLib/MessageList.h"
#include "../IMCommonLib/InterfaceUserList.h"
#include "../IMCommonLib/InstantUserInfo.h"
#include "ConfigureUIImpl.h"
#include <Core/treecallbackfun.h>
#include <Core/common.h>
#pragma warning(disable:4996)

//window启动项
#define REGISTER_START_RUN_KEY "Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define REGISTER_START_RUN_NAME "CoLine"

static const char SOUND_ITEM_NAME[8][32] = {"system", "friend", "group", "online", "video", "audio", "shake", "sms"};
CConfigureUIImpl::CConfigureUIImpl(void):
                  m_hWnd(NULL),
				  m_hWndInfo(NULL),
				  m_bChanged(FALSE),
				  m_pCore(NULL)
{
	//
}


CConfigureUIImpl::~CConfigureUIImpl(void)
{
	if (::IsWindow(m_hWnd))
		::SkinCloseWindow(m_hWnd);
	if (::IsWindow(m_hWndInfo))
		::SkinCloseWindow(m_hWndInfo);
	if (m_pCore)
		m_pCore->Release();
	m_pCore = NULL;
}


//IUnknown
STDMETHODIMP CConfigureUIImpl::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IConfigureUI)))
	{
		*ppv = (IConfigureUI *) this;
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

#define PERSON_USER_INFO_WIDTH  420
#define PERSON_USER_INFO_HEIGHT 320
//
STDMETHODIMP CConfigureUIImpl::ViewUserInfo(const char *szUserName)
{ 
	if (m_hWndInfo && ::IsWindow(m_hWndInfo))
	{
		::ShowWindow(m_hWndInfo, SW_SHOW);
		CSystemUtils::BringToFront(m_hWndInfo);
	} else
	{
		IUIManager *pUI = NULL; 		
		HRESULT hr = m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI);
		if (SUCCEEDED(hr) && pUI)
		{
			RECT rc = {0};
			CSystemUtils::GetScreenRect(&rc);
			rc.left = (rc.right - PERSON_USER_INFO_WIDTH) / 2;
			rc.top = (rc.bottom - PERSON_USER_INFO_HEIGHT) / 2;
			rc.right = rc.left + PERSON_USER_INFO_WIDTH;
			rc.bottom = rc.top + PERSON_USER_INFO_HEIGHT;
 
			HWND hTemp; 
			pUI->CreateUIWindow(NULL, "personconfig", &rc, WS_POPUP | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX
				                | WS_MINIMIZEBOX, 0, L"用户详细资料", &hTemp);
			if (hTemp && ::IsWindow(hTemp))
			{
				m_hWndInfo = hTemp;
				::ShowWindow(m_hWndInfo, SW_SHOW);
				CSystemUtils::BringToFront(m_hWndInfo); 
			} //end if (hTemp && 
			pUI->Release();
		}
	}
	if (::IsWindow(m_hWndInfo))
	{
		InitPersonInfo(szUserName);
	}
	return S_OK;
}

//
void CConfigureUIImpl::SetPersonEnable(BOOL bEnable)
{
	if (::IsWindow(m_hWndInfo))
	{
		::SkinSetControlEnable(m_hWndInfo, L"edtusername", FALSE);
		::SkinSetControlEnable(m_hWndInfo, L"edtrealname", FALSE);
		::SkinSetControlVisible(m_hWndInfo, L"person_ok", bEnable);
		::SkinSetControlVisible(m_hWndInfo, L"person_refresh", !bEnable);
		::SkinSetControlEnable(m_hWndInfo, L"edtsign", bEnable);
		::SkinSetControlEnable(m_hWndInfo, L"edtcell", bEnable);
		::SkinSetControlVisible(m_hWndInfo, L"changeheaderarea", bEnable);
		::SkinSetControlVisible(m_hWndInfo, L"uploadarea", bEnable);
		::SkinSetControlEnable(m_hWndInfo, L"edtmobilephone", bEnable);
		::SkinSetControlEnable(m_hWndInfo, L"edttel", bEnable);
        ::SkinSetControlEnable(m_hWndInfo, L"edtEmail", bEnable);
        ::SkinSetControlEnable(m_hWndInfo, L"edtfax", bEnable);
        ::SkinSetControlEnable(m_hWndInfo, L"edtprovince", bEnable);
		::SkinSetControlEnable(m_hWndInfo, L"edtcity", bEnable);
		::SkinSetControlEnable(m_hWndInfo, L"edtfamilytel", bEnable);
		::SkinSetControlEnable(m_hWndInfo, L"edtpostcode", bEnable);
		::SkinSetControlEnable(m_hWndInfo, L"edtaddress", bEnable);
		::SkinSetControlEnable(m_hWndInfo, L"edthomepage", bEnable); 
	}
}

//
void CConfigureUIImpl::InitPersonInfo(const char *szUserName)
{
	CInterfaceAnsiString strTmp;
	std::string strMyName;
	m_pCore->GetUserName(&strTmp);
	strMyName = strTmp.GetData();
	strMyName += "@";
	m_pCore->GetUserDomain(&strTmp);
	strMyName += strTmp.GetData();
	IContacts *pContact = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
	{ 
		std::string strTmpUserName;
		if (szUserName)
		{
			strTmpUserName = szUserName;
			pContact->GetRealNameById(szUserName, NULL, &strTmp);
		} else
		{
			strTmpUserName = strMyName;
			pContact->GetRealNameById(strMyName.c_str(), NULL, &strTmp);
		}
		TCHAR szName[MAX_PATH] = {0};
		CStringConversion::UTF8ToWideChar(strTmp.GetData(), szName, MAX_PATH - 1);
		::SkinSetControlTextByName(m_hWndInfo, L"edtrealname", szName);

		CStdString_ strCaption = L"用户详细资料——";
		strCaption += szName;
		::SetWindowText(m_hWndInfo, strCaption.GetData());
		//头像
		if (SUCCEEDED(pContact->GetContactHead(strTmpUserName.c_str(), &strTmp, FALSE)))
		{
			TCHAR szFileName[MAX_PATH] = {0};
			CStringConversion::StringToWideChar(strTmp.GetData(), szFileName, MAX_PATH - 1);
			::SkinSetControlAttr(m_hWndInfo, L"headerimage", L"filename", szFileName);
		} 
		std::string strUserName;
		int nPos = strTmpUserName.find('@');
		if (nPos != std::string::npos)
			strUserName = strTmpUserName.substr(0, nPos);
		else
			strUserName = szUserName;
		memset(szName, 0, sizeof(TCHAR) * MAX_PATH);
		CStringConversion::StringToWideChar(strUserName.c_str(), szName, MAX_PATH - 1);
		::SkinSetControlTextByName(m_hWndInfo, L"edtusername", szName);

		if ((szUserName == NULL) || (::stricmp(szUserName, strMyName.c_str()) == 0))
		{
			SetPersonEnable(TRUE);
		} else
		{
			SetPersonEnable(FALSE);
		}
		pContact->Release();
	}
	UpdateFromServer(szUserName);
}

STDMETHODIMP CConfigureUIImpl::Navegate2Frame(const LPRECT lprc, const char *szItem)
{
	m_strHeaderFileName.clear();
	if (m_hWnd && ::IsWindow(m_hWnd))
	{
		::ShowWindow(m_hWnd, SW_SHOW);
		CSystemUtils::BringToFront(m_hWnd);
	} else
	{
		IUIManager *pUI = NULL; 		
		HRESULT hr = m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI);
		if (SUCCEEDED(hr) && pUI)
		{
			RECT rc = {0};
			CSystemUtils::GetScreenRect(&rc);
			rc.left = (rc.right - 600) / 2;
			rc.top = (rc.bottom - 400) / 2;
			rc.right = rc.left + 600;
			rc.bottom = rc.top + 500;
			if (!lprc)
			{
				IConfigure *pCfg = NULL;		
				hr = m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg);
				if (SUCCEEDED(hr) && pCfg)
				{
					RECT rcSave = {0};
					CInterfaceAnsiString strPos;
					if (SUCCEEDED(pCfg->GetParamValue(FALSE, "Position", "ConfigFrame", (IAnsiString *)&strPos)))
					{
						CSystemUtils::StringToRect(&rcSave, strPos.GetData());
					}
					if (!::IsRectEmpty(&rcSave))
						rc = rcSave;
					pCfg->Release();
				}
			} else
				rc = *lprc;
			HWND hTemp; 
			pUI->CreateUIWindow(NULL, "configwindow", &rc, WS_POPUP | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX
				                | WS_MINIMIZEBOX, 0, L"系统设置", &hTemp);
			if (hTemp && ::IsWindow(hTemp))
			{
				m_hWnd = hTemp;
				::ShowWindow(m_hWnd, SW_SHOW);
				CSystemUtils::BringToFront(m_hWnd);
				::SkinSetGetKeyFun(hTemp, L"contacttiptree", GetTreeNodeKey);
			    ::SkinSetFreeNodeDataCallBack(hTemp, L"contacttiptree", FreeTreeNodeData);
				pUI->OrderWindowMessage("configwindow", m_hWnd, WM_DESTROY, (ICoreEvent *) this);
				if (szItem)
				{
					TCHAR szwTmp[MAX_PATH] = {0};
					CStringConversion::StringToWideChar(szItem, szwTmp, MAX_PATH - 1);
					::SkinSetControlAttr(m_hWnd, L"syscfgfolder", L"currentpage", szwTmp);
					if (::stricmp(szItem, "normalpage") == 0)
					{
						::SkinSetControlEnable(m_hWnd, L"personpage", FALSE);
						::SkinSetControlEnable(m_hWnd, L"ackpage", FALSE);
						::SkinSetControlEnable(m_hWnd, L"soundpage", FALSE);
						::SkinSetControlEnable(m_hWnd, L"filepage", FALSE);
						::SkinSetControlEnable(m_hWnd, L"hotkeypage", FALSE);
						::SkinSetControlEnable(m_hWnd, L"passwordpage", FALSE); 
					}
				}
			} //end if (hTemp && 
			pUI->Release();
		}
	}
	return E_NOTIMPL;
}

void CConfigureUIImpl::ShowCurrContactCard(HWND hWnd, const TCHAR *szCtrlName)
{
	RECT rc = {0};
	POINT pt = {0};
	::GetWindowRect(hWnd, &rc);
	::GetCursorPos(&pt);
	CTreeNodeType tnType;
	void *pSelNode = NULL;
	LPORG_TREE_NODE_DATA pData = NULL;
	TCHAR szwNodeName[MAX_PATH] = {0};	
	int nNameSize = MAX_PATH - 1;
	if (::SkinGetSelectTreeNode(hWnd, szCtrlName, szwNodeName, &nNameSize, &pSelNode, &tnType, (void **)&pData))
	{
		if (pData && (tnType == TREENODE_TYPE_LEAF))
		{
			char *p = pData->szUserName;
			BOOL bGrp = TRUE;
			while (*p != '\0')
			{
				if (*p == '@')
				{
					bGrp = FALSE;
					break;
				}
				p ++;
			}
			if (!bGrp)
				ViewUserInfo(pData->szUserName); 
		}
	}
}

HRESULT CConfigureUIImpl::DoMenuCommand(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szName, "treeleaf") == 0)
	{
		if (wParam == 5) //查看详细资料
		{
			ShowCurrContactCard(hWnd, L"colleaguetree");
		}
	} else if (::stricmp(szName, "recentlymenu") == 0)
	{
		if (wParam == 5) //详细资料
		{
			ShowCurrContactCard(hWnd, L"recentlytree");
		}
	} else if (::stricmp(szName, "freleafmenu") == 0)
	{
		if (wParam == 5) //详细资料
		{
			ShowCurrContactCard(hWnd, L"frecontacttree");
		}
	} else 
	{
	   switch(wParam)
	   {
		case 6:
			{
				std::string strUserName;
				CInterfaceAnsiString strTmp;
				m_pCore->GetUserName(&strTmp);
				strUserName = strTmp.GetData();
				m_pCore->GetUserDomain(&strTmp);
				strUserName += "@";
				strUserName += strTmp.GetData();
				ViewUserInfo(strUserName.c_str());
				break;
			}
		case 1000:
			{ 
				Navegate2Frame(NULL, "");
				break;
			}
		case 2000:
			{
				Navegate2Frame(NULL, "personpage");
				break;
			}
		}
	}
	return -1;
}

void GetHotKeyStr(CStdString_ &strKey, WPARAM wParam)
{ 
	if (::GetKeyState(VK_SHIFT) &0xF000)
		strKey += _T("SHIFT+");
	if (::GetKeyState(VK_CONTROL) & 0xF000)
		strKey += _T("CTRL+");
	if (::GetKeyState(VK_MENU) & 0xF000)
		strKey += _T("ALT+");
	if (!strKey.IsEmpty())
	{
		if ((wParam > 0x20) && (wParam< 0xF0))
			strKey += ('A' + (TCHAR)wParam - 0x41);
	} else if ((wParam == VK_DELETE) || (wParam == VK_BACK))
		strKey = L"无";	  
}

void ClearLNChar(char *szValue)
{
	//去除回车符
	int nLen = ::strlen(szValue);
	if (nLen > 0)
	{
	for (int i = nLen - 1; i >= 0; i --)
	{
		if (szValue[i] == 13)
			szValue[i] = '\0';
	}
	}
}

//ICoreEvent
STDMETHODIMP CConfigureUIImpl::DoCoreEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
		                     LPARAM lParam, HRESULT *hResult)
{
	if (::stricmp(szType, "menucommand") == 0)
	{
		*hResult = DoMenuCommand(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "afterinit") == 0)
	{
		//
		if (::stricmp(szName, "configwindow") == 0)
		{
			m_hWnd = hWnd;
		    InitUI();
		} else if (::stricmp(szName, "mainwindow") == 0)
		{
			m_hWndMain = hWnd;
			IUIManager *pUI = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
			{
				pUI->OrderWindowMessage("mainwindow", NULL, WM_SHOW_DETAIL_INFO, (ICoreEvent *) this);
				pUI->Release();
			}
		}
	} else if (::stricmp(szType, "keydown") == 0)
	{
		if (::stricmp(szName, "edit_pickhotkey") == 0)
		{
			CStdString_ strKey;
			GetHotKeyStr(strKey, wParam);
			if (!strKey.IsEmpty())
			{
				::SkinSetControlTextByName(m_hWnd, L"edit_pickhotkey", strKey.GetData());
				SetChanged(TRUE);
			}
		} else if (::stricmp(szName, "edit_screenhotkey") == 0)
		{
			CStdString_ strKey;
			GetHotKeyStr(strKey, wParam);
			if (!strKey.IsEmpty())
			{
				::SkinSetControlTextByName(m_hWnd, L"edit_screenhotkey", strKey.GetData());
				SetChanged(TRUE);
			}
		}
	} else if (::stricmp(szType, "changed") == 0)
	{
		if (::stricmp(szName, "check_autochangestate") == 0)
		{
			int nStatus = ::SkinGetCheckBoxStatus(m_hWnd, L"check_autochangestate");
			if (nStatus == 0)
				::SkinSetControlEnable(m_hWnd, L"edit_timetoaway", FALSE);
			else
				::SkinSetControlEnable(m_hWnd, L"edit_timetoaway", TRUE);
			SetChanged(TRUE);
		} else if (::stricmp(szName, "check_autoreply") == 0)
		{
			int nStatus = ::SkinGetCheckBoxStatus(m_hWnd, L"check_autoreply");
			if (nStatus == 0)
			{
				::SkinSetControlEnable(m_hWnd, L"cb_selautoreply", FALSE);
				::SkinSetControlEnable(m_hWnd, L"edit_setautoreply", FALSE);
			} else
			{
				::SkinSetControlEnable(m_hWnd, L"cb_selautoreply", TRUE);
				::SkinSetControlEnable(m_hWnd, L"edit_setautoreply", TRUE);
			}
			SetChanged(TRUE);
		} else if (::stricmp(szName, "check_playnotify") == 0)
		{
			int nStatus = ::SkinGetCheckBoxStatus(m_hWnd, L"check_playnotify");
			if (nStatus == 0)
			{
				::SkinSetControlEnable(m_hWnd, L"cb_selsound", FALSE);
				::SkinSetControlEnable(m_hWnd, L"edt_filename", FALSE);
				::SkinSetControlEnable(m_hWnd, L"btn_playsound", FALSE);
				::SkinSetControlEnable(m_hWnd, L"btn_browse", FALSE);
			} else
			{
				::SkinSetControlEnable(m_hWnd, L"cb_selsound", TRUE);
				::SkinSetControlEnable(m_hWnd, L"edt_filename", TRUE);
				::SkinSetControlEnable(m_hWnd, L"btn_playsound", TRUE);
				::SkinSetControlEnable(m_hWnd, L"btn_browse", TRUE);
			} 
		} else if (::stricmp(szName, "check_resumetransfer") == 0)
		{
			int nStatus = ::SkinGetCheckBoxStatus(m_hWnd, L"check_resumetransfer");
			if (nStatus == 0)
			{
				::SkinSetControlEnable(m_hWnd, L"edit_cachepath", FALSE);
				::SkinSetControlEnable(m_hWnd, L"btn_browsesavepath", FALSE);
				::SkinSetControlEnable(m_hWnd, L"btn_managerpath", FALSE);
			} else
			{
				::SkinSetControlEnable(m_hWnd, L"edit_cachepath", TRUE);
				::SkinSetControlEnable(m_hWnd, L"btn_browsesavepath", TRUE);
				::SkinSetControlEnable(m_hWnd, L"btn_managerpath", TRUE);
			} 
		} else if (::stricmp(szName, "radio_pickcustom") == 0)
		{
			BOOL bChecked = ::SkinGetRadioChecked(m_hWnd, L"radio_pickcustom");
			if (bChecked)
				::SkinSetControlEnable(m_hWnd, L"edit_pickhotkey", TRUE);
			else
				::SkinSetControlEnable(m_hWnd, L"edit_pickhotkey", FALSE); 
		} else if (::stricmp(szName, "radio_screencustom") == 0)
		{
			BOOL bChecked = ::SkinGetRadioChecked(m_hWnd, L"radio_screencustom");
			if (bChecked)
				::SkinSetControlEnable(m_hWnd, L"edit_screenhotkey", TRUE);
			else
				::SkinSetControlEnable(m_hWnd, L"edit_screenhotkey", FALSE); 
		} else if (::stricmp(szName, "check_recently") == 0)
		{
			int nStatus = ::SkinGetCheckBoxStatus(m_hWnd, L"check_recently");
			if (nStatus == 0)
			{
				::SkinSetControlEnable(m_hWnd, L"edit_recentlycount", FALSE); 
			} else
			{
				::SkinSetControlEnable(m_hWnd, L"edit_recentlycount", TRUE); 
			} 
		} else if (::stricmp(szName, "check_msgTransparent") == 0)
		{
			BOOL bChecked = ::SkinGetCheckBoxStatus(m_hWnd, L"check_msgTransparent");
			if (bChecked)
			{
				::SkinSetControlEnable(m_hWnd, L"edttransparentimage", TRUE);
				::SkinSetControlEnable(m_hWnd, L"btn_seltransimage", TRUE);
			} else
			{
				::SkinSetControlEnable(m_hWnd, L"edttransparentimage", FALSE);
				::SkinSetControlEnable(m_hWnd, L"btn_seltransimage", FALSE);
			} 
		} else if (::stricmp(szName, "check_savepwd") == 0)
		{
			int nStatus = ::SkinGetCheckBoxStatus(m_hWnd, L"check_savepwd");
			if (nStatus == 1)
				::SkinSetCheckBoxStatus(m_hWnd, L"check_saveusername", 1);
			else
			{
				::SkinSetCheckBoxStatus(m_hWnd, L"check_autologin", 0);
			} 
		} else if (::stricmp(szName, "check_autologin") == 0)
		{
			int nStatus = ::SkinGetCheckBoxStatus(m_hWnd, L"check_autologin");
			if (nStatus == 1)
			{
				::SkinSetCheckBoxStatus(m_hWnd, L"check_saveusername", 1);
				::SkinSetCheckBoxStatus(m_hWnd, L"check_savepwd", 1);
			} else
			{

			} 
		} else if (::stricmp(szName, "check_saveusername") == 0)
		{
			int nStatus = ::SkinGetCheckBoxStatus(m_hWnd, L"check_saveusername");
			if (nStatus == 0)
			{
				::SkinSetCheckBoxStatus(m_hWnd, L"check_savepwd", 0);
				::SkinSetCheckBoxStatus(m_hWnd, L"check_autologin", 0);
			}
		} 
		SetChanged(TRUE);
	} else if (::stricmp(szType, "itemselect") == 0)
	{
		if (::stricmp(szName, "cb_selsound") == 0)
		{
			SelectSoundListChanged();
			SetChanged(TRUE);
		} else if (::stricmp(szName, "cb_selshortreply") == 0)
		{
			TCHAR szTmp[MAX_PATH] = {0};
			int nSize = MAX_PATH - 1;
			if (::SkinGetControlTextByName(m_hWnd, L"cb_selshortreply", szTmp, &nSize))
			{
				::SkinSetControlTextByName(m_hWnd, L"edit_setshortreply", szTmp);
				IConfigure *pCfg = NULL;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
				{
					int idx = ::SkinGetDropdownSelectIndex(m_hWnd, L"cb_selshortreply");
					char szValue[16] = {0};
					::itoa(idx, szValue, 10);
					pCfg->SetParamValue(FALSE, "replysetting", "shortreply", szValue);
					pCfg->Release();
				}
			}
			SetChanged(TRUE);
		} else if (::stricmp(szName, "cb_selautoreply") == 0)
		{
			TCHAR szTmp[MAX_PATH] = {0};
			int nSize = MAX_PATH - 1;
			if (::SkinGetControlTextByName(m_hWnd, L"cb_selautoreply", szTmp, &nSize))
			{
				::SkinSetControlTextByName(m_hWnd, L"edit_setautoreply", szTmp);
								IConfigure *pCfg = NULL;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
				{
					char szValue[MAX_PATH] = {0};
					CStringConversion::WideCharToString(szTmp, szValue, MAX_PATH - 1);
					pCfg->SetParamValue(FALSE, "replysetting", "autoreply", szValue);
					pCfg->Release();
				}
			} 
		    SetChanged(TRUE);
		} 
	} else if (::stricmp(szType, "click") == 0)
	{
		if (::stricmp(szName, "setting_ok") == 0)
		{
			SaveUI();
			::SkinCloseWindow(m_hWnd);
		} else if (::stricmp(szName, "setting_cancel") == 0)
		{
			::SkinCloseWindow(m_hWnd);
		} else if (::stricmp(szName, "setting_apply") == 0)
		{
			SaveUI();
		} else if (::stricmp(szName, "person_ok") == 0)
		{
			SavePersonCfg();
		} else if (::stricmp(szName, "person_cancel") == 0)
		{
			::SkinCloseWindow(hWnd);
		} else if (::stricmp(szName, "person_refresh") == 0)
		{
			TCHAR szName[MAX_PATH] = {0};
			
		} else if (::stricmp(szName, "btn_managerpath") == 0)
		{
			TCHAR szPath[MAX_PATH] = {0};
			int nSize = MAX_PATH - 1;
			if (::SkinGetControlTextByName(m_hWnd, L"edit_cachepath", szPath, &nSize))
			{
				char szTmp[MAX_PATH] = {0};
				CStringConversion::WideCharToString(szPath, szTmp, MAX_PATH - 1);
				CSystemUtils::OpenURL(szTmp);
			}
		} else if (::stricmp(szName, "btn_addshortreplay") == 0)
		{
			IConfigure *pCfg = NULL;
			if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
			{
				TCHAR szTmp[MAX_PATH] = {0};
				int nSize = MAX_PATH - 1;
				if (::SkinGetControlTextByName(m_hWnd, L"edit_setshortreply", szTmp, &nSize))
				{
					char szValue[MAX_PATH] = {0};
					CStringConversion::WideCharToString(szTmp, szValue, MAX_PATH - 1);
					ClearLNChar(szValue);
					int nIdx = pCfg->AddReplyMessage(0, 1, szValue);
					nIdx = ::SkinSetDropdownItemString(m_hWnd, L"cb_selshortreply", 9999, szTmp, (void *)nIdx);
					::SkinSelectDropdownItem(m_hWnd, L"cb_selshortreply", nIdx);
					SetChanged(TRUE);
				}
				pCfg->Release();
			} //end if (m_pCore..
		} else if (::stricmp(szName, "btn_addautoreplay") == 0)
		{
			IConfigure *pCfg = NULL;
			if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
			{
				TCHAR szTmp[MAX_PATH] = {0};
				int nSize = MAX_PATH - 1;
				if (::SkinGetControlTextByName(m_hWnd, L"edit_setautoreply", szTmp, &nSize))
				{
					char szValue[MAX_PATH] = {0};
					CStringConversion::WideCharToString(szTmp, szValue, MAX_PATH - 1);
					ClearLNChar(szValue);
					int nIdx = pCfg->AddReplyMessage(0, 2, szValue);
					nIdx = ::SkinSetDropdownItemString(m_hWnd, L"cb_selautoreply", 9999, szTmp, (void *)nIdx);
					::SkinSelectDropdownItem(m_hWnd, L"cb_selautoreply", nIdx);
					SetChanged(TRUE);
				}
				pCfg->Release();
			} //end if (m_pCore..
		} else if (::stricmp(szName, "btn_deleteshortreply") == 0)
		{
			//
			int idx = ::SkinGetDropdownSelectIndex(m_hWnd, L"cb_selshortreply");
			void *p = ::SkinGetDropdownItemData(m_hWnd, L"cb_selshortreply", idx);
			if (p)
			{ 
				IConfigure *pCfg = NULL;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
				{
					pCfg->DelReplyMessage((int)p, 1);
					pCfg->Release();
				}
				::SkinDeleteDropdownItem(m_hWnd, L"cb_selshortreply", idx); 
				::SkinSelectDropdownItem(m_hWnd, L"cb_selshortreply", 0);
				SetChanged(TRUE);
			} //end if (p)
		} else if (::stricmp(szName, "btn_editshortreply") == 0)
		{
			int idx = ::SkinGetDropdownSelectIndex(m_hWnd, L"cb_selshortreply");
			void *p = ::SkinGetDropdownItemData(m_hWnd, L"cb_selshortreply", idx);
			if (p)
			{
				IConfigure *pCfg = NULL;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
				{
					TCHAR szTmp[MAX_PATH] = {0};
					int nSize = MAX_PATH - 1;
					if (::SkinGetControlTextByName(m_hWnd, L"edit_setshortreply", szTmp, &nSize))
					{
						::SkinSetDropdownItemString(m_hWnd, L"cb_selshortreply", idx, szTmp, NULL);
						::SkinSelectDropdownItem(m_hWnd, L"cb_selshortreply", idx);
						char szValue[MAX_PATH] = {0};
						CStringConversion::WideCharToString(szTmp, szValue, MAX_PATH - 1);
						ClearLNChar(szValue);
						pCfg->AddReplyMessage((int)p, 1, szValue);
						SetChanged(TRUE);
					}
					pCfg->Release();
				} // end if (m_pCore && ...
			} //end if (p)
		} else if (::stricmp(szName, "btn_delautoreply") == 0)
		{
			int idx = ::SkinGetDropdownSelectIndex(m_hWnd, L"cb_selautoreply");
			void *p = ::SkinGetDropdownItemData(m_hWnd, L"cb_selautoreply", idx);
			if (p)
			{
				IConfigure *pCfg = NULL;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
				{
					pCfg->DelReplyMessage((int)p, 2);
					pCfg->Release();
				}
				::SkinDeleteDropdownItem(m_hWnd, L"cb_selautoreply", idx);
				::SkinSelectDropdownItem(m_hWnd, L"cb_selautoreply", 0);
				SetChanged(TRUE);
			}
		} else if (::stricmp(szName, "btn_editautoreply") == 0)
		{
			int idx = ::SkinGetDropdownSelectIndex(m_hWnd, L"cb_selautoreply");
			void *p = ::SkinGetDropdownItemData(m_hWnd, L"cb_selautoreply", idx);
			if (p)
			{
				IConfigure *pCfg = NULL;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
				{
					TCHAR szTmp[MAX_PATH] = {0};
					int nSize = MAX_PATH - 1;
					if (::SkinGetControlTextByName(m_hWnd, L"edit_setautoreply", szTmp, &nSize))
					{
						::SkinSetDropdownItemString(m_hWnd, L"cb_selautoreply", idx, szTmp, NULL);
						::SkinSelectDropdownItem(m_hWnd, L"cb_selautoreply", idx);
						char szValue[MAX_PATH] = {0};
						CStringConversion::WideCharToString(szTmp, szValue, MAX_PATH - 1);
						ClearLNChar(szValue);
						pCfg->AddReplyMessage((int)p, 2, szValue);
						SetChanged(TRUE);
					}
					pCfg->Release();
				} // end if (m_pCore && ...
			} //end if (p)
		} else if (::stricmp(szName, "btn_browsesavepath") == 0)
		{
			TCHAR szTmp[MAX_PATH] = {0};
			int nSize = MAX_PATH - 1;
			::SkinGetControlTextByName(m_hWnd, L"edit_cachepath", szTmp, &nSize);
			char szCachePath[MAX_PATH] = {0};
			if (CSystemUtils::SelectFolder(m_hWnd, szCachePath))
			{
				memset(szTmp, 0, sizeof(TCHAR) * MAX_PATH);
				CStringConversion::StringToWideChar(szCachePath, szTmp, MAX_PATH - 1);
				::SkinSetControlTextByName(m_hWnd, L"edit_cachepath", szTmp);
				SetChanged(TRUE);
			}
		} else if (::stricmp(szName, "btn_browsesavepath") == 0)
		{
			TCHAR szTmp[MAX_PATH] = {0};
			int nSize = MAX_PATH - 1;
			::SkinGetControlTextByName(m_hWnd, L"edit_cachepath", szTmp, &nSize);
			char szCachePath[MAX_PATH] = {0};
			CStringConversion::WideCharToString(szTmp, szCachePath, MAX_PATH - 1);
			CSystemUtils::OpenURL(szCachePath);
		} else if (::stricmp(szName, "btn_browsedefpath") == 0)
		{
			TCHAR szTmp[MAX_PATH] = {0};
			int nSize = MAX_PATH - 1;
			::SkinGetControlTextByName(m_hWnd, L"edit_defpath", szTmp, &nSize);
			char szDefaultPath[MAX_PATH] = {0};
			if (CSystemUtils::SelectFolder(m_hWnd, szDefaultPath))
			{
				memset(szTmp, 0, sizeof(TCHAR) * MAX_PATH);
				CStringConversion::StringToWideChar(szDefaultPath, szTmp, MAX_PATH - 1);
				::SkinSetControlTextByName(m_hWnd, L"edit_defpath", szTmp);
				SetChanged(TRUE);
			}
		} else if (::stricmp(szName, "btn_seltransimage") == 0)
		{
			CStringList_ FileList;
			if (CSystemUtils::OpenFileDialog(NULL, m_hWnd, "选择背景图片文件", "图片文件(*.bmp,*.jpg)|*.jpg;*.bmp", NULL, FileList, FALSE))
			{ 
				if (CSystemUtils::FileIsExists(FileList.back().c_str()))
				{
					TCHAR szwTmp[MAX_PATH] = {0};
					CStringConversion::StringToWideChar(FileList.back().c_str(), szwTmp, MAX_PATH - 1);
					::SkinSetControlTextByName(m_hWnd, L"edttransparentimage", szwTmp);
					SetChanged(TRUE);
				}
			}
		} else if (::stricmp(szName, "changeheader") == 0)
		{
			CStringList_ FileList;
			if (CSystemUtils::OpenFileDialog(NULL, hWnd, "选择头像图片文件", "图片文件(*.bmp,*.jpg)|*.jpg;*.bmp", NULL, FileList, FALSE))
			{
				m_strHeaderFileName = FileList.back();
				if (CSystemUtils::FileIsExists(m_strHeaderFileName.c_str()))
				{
					if (CSystemUtils::GetFileSize(m_strHeaderFileName.c_str()) > 102400) //100K
					{
						if (::SkinMessageBox(hWnd, L"头像文件过大，系统将自动压缩至100 *100，有可能导致图片失真，是否确定继续使用此图片？",
							L"提示", 2) != IDOK)
							m_strHeaderFileName.clear();
						else
						{
							char szTmpFileName[MAX_PATH] = {0};
							if (CSystemUtils::GetSystemTempFileName(szTmpFileName, MAX_PATH - 1))
							{
								if (::SkinTransImage(m_strHeaderFileName.c_str(), szTmpFileName, 100, 100))
									m_strHeaderFileName = szTmpFileName;
							}
						}
					}
					if (!m_strHeaderFileName.empty())
					{
						TCHAR szwTmp[MAX_PATH] = {0};
						CStringConversion::StringToWideChar(m_strHeaderFileName.c_str(), szwTmp, MAX_PATH - 1);
						::SkinSetControlAttr(hWnd, L"headerimage", L"filename", szwTmp);
						//上传至服务器
						UploadPictureToSvr(hWnd);
						SetChanged(TRUE);
					}
				}
			}
		} else if (::stricmp(szName, "uploadheader") == 0)
		{
			UploadPictureToSvr(hWnd);
			SetChanged(TRUE);
		} else if (::stricmp(szName, "modifypassword") == 0)
		{
			TCHAR szOldPwd[MAX_PATH] = {0};
			int nOldSize = MAX_PATH - 1;
			TCHAR szNewPwd[MAX_PATH] = {0};
			int nNewSize = MAX_PATH - 1;
			TCHAR szAgainPwd[MAX_PATH] = {0};
			int nAgainSize = MAX_PATH - 1;
			if ((::SkinGetControlTextByName(m_hWnd, L"edit_pwd", szOldPwd, &nOldSize))
				&& (::SkinGetControlTextByName(m_hWnd, L"edit_newpwd", szNewPwd, &nNewSize))
				&& (::SkinGetControlTextByName(m_hWnd, L"edit_newpwdagain", szAgainPwd, &nAgainSize)))
			{ 
				if (::lstrcmpi(szNewPwd, szAgainPwd) == 0)
				{
					char szSrcPwd[MAX_PATH] = {0};
					char szDestPwd[MAX_PATH] = {0};
					CStringConversion::WideCharToString(szOldPwd, szSrcPwd, MAX_PATH - 1);
					CStringConversion::WideCharToString(szNewPwd, szDestPwd, MAX_PATH - 1);
					CInterfaceAnsiString strTmp;
					if (SUCCEEDED(m_pCore->GetUserPassword(&strTmp)))
					{
						if (::stricmp(szSrcPwd, strTmp.GetData()) == 0)
						{
							std::string strXml = "<sys type=\"changepass\" pass=\"";
							strXml += szDestPwd;
							strXml += "\"/>";
							if (SUCCEEDED(m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG)strXml.size(), 0)))
								::SkinMessageBox(m_hWnd, L"修改密码成功，下次登陆时请使用新密码", L"提示", MB_OK);
							else
								::SkinMessageBox(m_hWnd, L"修改密码失败", L"提示", MB_OK);
						} else
						{
							::SkinMessageBox(m_hWnd, L"原始密码不匹配", L"提示", MB_OK);
						}
					} else
					{
						::SkinMessageBox(m_hWnd, L"修改密码失败", L"提示", MB_OK);
					}
				} else
				{
					::SkinMessageBox(m_hWnd, L"新密码与确认密码不匹配", L"提示", MB_OK);
				}
			} else
				::SkinMessageBox(m_hWnd, L"修改密码失败", L"提示", MB_OK);
		} else if (::stricmp(szName, "btn_browse") == 0)
		{
			TCHAR szTmp[MAX_PATH] = {0};
			int nSize = MAX_PATH - 1;
			::SkinGetControlTextByName(m_hWnd, L"edt_filename", szTmp, &nSize);
			char szFileName[MAX_PATH] = {0};
			CStringConversion::WideCharToString(szTmp, szFileName, MAX_PATH - 1);
			if (!CSystemUtils::FileIsExists(szFileName))
				memset(szFileName, 0, MAX_PATH);
			CStringList_ FileList;
			if (CSystemUtils::OpenFileDialog(NULL, m_hWnd, "选择声音文件", "声音文件(*.wav)|*.wav", szFileName, FileList, FALSE))
			{
				memset(szTmp, 0, sizeof(TCHAR) * MAX_PATH);
				std::string strFileName = FileList.back();
				CStringConversion::StringToWideChar(strFileName.c_str(), szTmp, MAX_PATH - 1);
				::SkinSetControlTextByName(m_hWnd, L"edt_filename", szTmp);
				IConfigure *pCfg = NULL;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
				{ 
					int idx = ::SkinGetDropdownSelectIndex(m_hWnd, L"cb_selsound"); 
					EditMapUIToCfg(pCfg, FALSE, "sound", SOUND_ITEM_NAME[idx], L"edt_filename");
					pCfg->Release();
					SetChanged(TRUE);
				}//end if (m_pCore...
			} //end if (CSystemUtils::..
		} else if (::stricmp(szName, "btn_playsound") == 0)
		{
			TCHAR szTmp[MAX_PATH] = {0};
			int nSize = MAX_PATH - 1;
			if (::SkinGetControlTextByName(m_hWnd, L"edt_filename", szTmp, &nSize))
			{
				char szFileName[MAX_PATH] = {0};
				CStringConversion::WideCharToString(szTmp, szFileName, MAX_PATH - 1);
				CSystemUtils::PlaySoundFile(szFileName, FALSE);
			} //end if (::SkinGetControlTextByName(...
		} else if (::stricmp(szName, "addContactTip") == 0)
		{
			AddContactTip();
			SetChanged(TRUE);
		} else if (::stricmp(szName, "delContactTip") == 0)
		{
			DelContactTip();
			SetChanged(TRUE);
		} else if (::stricmp(szName, "peerlogo") == 0)
		{
			IChatFrame *pFrame = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pFrame)))
			{
				CInterfaceAnsiString strUserName;
				if (SUCCEEDED(pFrame->GetUserNameByHWND(hWnd, &strUserName)))
				{
					ViewUserInfo(strUserName.GetData());
				}
				pFrame->Release();
			}
		}
	} else if (::stricmp(szType, "editchanged") == 0)
	{
		if (::stricmp(szName, "edit_timetoaway") == 0)
		{
			SetChanged(TRUE);
		} else if (::stricmp(szName, "edit_recentlycount") == 0)
		{
			SetChanged(TRUE);
		}
	}
	return E_NOTIMPL;
}

//
void CConfigureUIImpl::UploadPictureToSvr(HWND hWnd)
{
	if ((!m_strHeaderFileName.empty()) && CSystemUtils::FileIsExists(m_strHeaderFileName.c_str()))
	{ 
		HRESULT hr = E_FAIL;
		IContacts *pContact = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
		{
			hr = pContact->UploadHead(m_strHeaderFileName.c_str());
			pContact->Release();
		}
		if (SUCCEEDED(hr))
		{ 
			CInterfaceAnsiString strLocalFileName;
			IConfigure *pCfg = NULL;
			CInterfaceAnsiString strTmp;
			std::string strUserName;
			m_pCore->GetUserName(&strTmp);
			strUserName = strTmp.GetData();
			m_pCore->GetUserDomain(&strTmp);
			strUserName += "@";
			strUserName += strTmp.GetData();
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
			{
				if (SUCCEEDED(pCfg->GetPath(PATH_LOCAL_USER_HEAD, &strLocalFileName)))
				{
					strLocalFileName.AppendString(strUserName.c_str());
					strLocalFileName.AppendString(".bmp"); 
				}//end if (SUCCEEDED(pCfg->
				pCfg->Release();
			} //end if (SUCCEEDED(m_pCore->
			CSystemUtils::CopyFilePlus(m_strHeaderFileName.c_str(), strLocalFileName.GetData(), TRUE);
			//广播通知
			m_pCore->BroadcastMessage("ConfigureUI", hWnd, "uploadheader", "succ", NULL);
			::SkinMessageBox(hWnd, L"上传头像图片成功", L"提示", MB_OK);
		} else
			::SkinMessageBox(hWnd, L"上传头像图片失败", L"提示", MB_OK);
	} else
	{
		::SkinMessageBox(hWnd, L"请先选择要上传的头像图片文件", L"提示", MB_OK);
	}
	//
}

//
void CConfigureUIImpl::AddContactTip()
{
	IContactPanel *pPanel = NULL;
	IContacts *pContact = NULL;
	IConfigure *pCfg = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContactPanel), (void **)&pPanel))
		&& SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact))
		&& SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		CInterfaceUserList ulList;
		if (SUCCEEDED(pPanel->ShowPanel(m_hWnd, L"选择要关注的联系人", NULL, TRUE, NULL)))
		{ 
			pPanel->GetSelContact(&ulList);
			LPORG_TREE_NODE_DATA pData;
			TCHAR szwText[256];
			CInterfaceAnsiString strName;
			while (SUCCEEDED(ulList.PopBackUserInfo(&pData)))
			{
				if (SUCCEEDED(pCfg->AddContactOnlineTip(pData->szUserName)))
				{
					memset(szwText, 0, sizeof(TCHAR) * 256);
					if (pData->szDisplayName)
						CStringConversion::UTF8ToWideChar(pData->szDisplayName, szwText, 255);
					else
					{
						if (SUCCEEDED(pContact->GetRealNameById(pData->szUserName, NULL, &strName)))
						{
							CStringConversion::UTF8ToWideChar(strName.GetData(), szwText, 255);
						} else
							CStringConversion::StringToWideChar(pData->szUserName, szwText, 255);
					}
					::SkinAddTreeChildNode(m_hWnd, L"contacttiptree", pData->id, NULL, szwText, TREENODE_TYPE_LEAF, pData, NULL, NULL, NULL);
					::SkinExpandTree(m_hWnd, L"contacttiptree", NULL, TRUE, TRUE);
					::SkinUpdateControlUI(m_hWnd, L"contacttiptree"); 
				} //end if (SUCCEEDED(pCfg->AddContactOnlineTip(...
			} //end while(...
		} //end if (SUCCEEDED(pPanel
		pCfg->Release();
		pContact->Release();
		pPanel->Release();
	} //end if (SUCCEEDED(m_pCore..
}

//
void CConfigureUIImpl::DelContactTip()
{
	void *pSelNode = NULL;
	LPORG_TREE_NODE_DATA pData = NULL;
	TCHAR szName[MAX_PATH] = {0};
	int nSize = MAX_PATH - 1;
	CTreeNodeType tnType;
	if (::SkinGetSelectTreeNode(m_hWnd, L"contacttiptree", szName, &nSize, &pSelNode, &tnType, (void **)&pData))
	{
		if ((tnType == TREENODE_TYPE_LEAF) && pData)
		{
			IConfigure *pCfg = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
			{
				if (SUCCEEDED(pCfg->DelContactOnlineTip(pData->szUserName)))
				{
					LPORG_TREE_NODE_DATA pBackData = new ORG_TREE_NODE_DATA();
					memset(pBackData, 0, sizeof(ORG_TREE_NODE_DATA));
					::strncpy(pBackData->szUserName, pData->szUserName, 63);
					::SkinAdjustTreeNode(m_hWnd, L"contacttiptree", NULL, NULL, TREENODE_TYPE_LEAF, pBackData, FALSE, FALSE);
					delete pBackData;
				} //end if (SUCCEEDED(pCfg->DelContactOnlineTip(...
				pCfg->Release();
			} //end if (SUCCEEDED(m_pCore->			 
		} //end if ((tnType == 
	} //end if (::SkinGetSelectTreeNode(...
}

//广播消息
STDMETHODIMP CConfigureUIImpl::DoBroadcastMessage(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData)
{
	return E_NOTIMPL;
}

STDMETHODIMP CConfigureUIImpl::SetCoreFrameWork(ICoreFrameWork *pCore)
{
	if (m_pCore)
		m_pCore->Release();
	m_pCore = pCore;
	if (m_pCore)
	{
		m_pCore->AddRef();
		//order
		m_pCore->AddOrderEvent((ICoreEvent *) this, "MainWindow", "SystemMenu", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "MainWindow", "mainmenu", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "maintraymenu", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "treeleaf", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "mainwindow", "afterinit");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "recentlymenu", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "freleafmenu", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "chatwindow", "peerlogo", "click");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "configwindow", NULL, NULL);
		m_pCore->AddOrderEvent((ICoreEvent *) this, "personconfig", NULL, NULL);
		//		
		//order protocol
		m_pCore->AddOrderProtocol((IProtocolParser *) this, "sys", "getvcard");
	}
	return S_OK;
}

STDMETHODIMP CConfigureUIImpl::GetSkinXmlString(IAnsiString *szXmlString)
{
	HRESULT hr = E_FAIL;
	IConfigure *pCfg = NULL; 
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{ 
		hr = pCfg->GetSkinXml("ConfigUI.xml",szXmlString); 
		pCfg->Release();
	}
	return hr;  
}

//
STDMETHODIMP CConfigureUIImpl::CoreFrameWorkError(int nErrorNo, const char *szErrorMsg)
{
	switch(nErrorNo)
	{
		case CORE_ERROR_LOGOUT:
			{
				if ((m_hWnd != NULL) && (::IsWindow(m_hWnd)))
					::SkinCloseWindow(m_hWnd);
				if ((m_hWndInfo != NULL) && (::IsWindow(m_hWndInfo)))
					::SkinCloseWindow(m_hWndInfo);
				break;
			}
	}
	return S_OK;
}

void CConfigureUIImpl::CheckStatusMapUIByCfg(IConfigure *pCfg, BOOL bCommon, const char *szSection, 
		      const char *szParam, const TCHAR *szCtrlName)
{
	CInterfaceAnsiString strTmp;
	if (SUCCEEDED(pCfg->GetParamValue(bCommon, szSection, szParam, (IAnsiString *)&strTmp)))
	{
		if (::stricmp(strTmp.GetData(), "true") == 0)
			::SkinSetCheckBoxStatus(m_hWnd, szCtrlName, 1);
		else
			::SkinSetCheckBoxStatus(m_hWnd, szCtrlName, 0);
	}
}

void CConfigureUIImpl::CheckStatusMapUIToCfg(IConfigure *pCfg, BOOL bCommon, const char *szSection, 
		      const char *szParam, const TCHAR *szCtrlName)
{
	int nStatus = ::SkinGetCheckBoxStatus(m_hWnd, szCtrlName);
	if (nStatus == 1)
		pCfg->SetParamValue(bCommon, szSection, szParam, "true");
	else
		pCfg->SetParamValue(bCommon, szSection, szParam, "false");
}

void CConfigureUIImpl::EditMapUIByCfg(IConfigure *pCfg, BOOL bCommon, const char *szSection,
		      const char *szParam, const TCHAR *szCtrlName)
{
	CInterfaceAnsiString strValue;
	if (SUCCEEDED(pCfg->GetParamValue(bCommon, szSection, szParam, (IAnsiString *)&strValue)))
	{
		TCHAR szTmp[MAX_PATH] = {0};
		CStringConversion::StringToWideChar(strValue.GetData(), szTmp, MAX_PATH - 1);
		::SkinSetControlTextByName(m_hWnd, szCtrlName, szTmp);
	}
}

void CConfigureUIImpl::EditMapUIToCfg(IConfigure *pCfg, BOOL bCommon, const char *szSection,
		      const char *szParam, const TCHAR *szCtrlName)
{
	TCHAR szTmp[MAX_PATH] = {0};
	int nSize = MAX_PATH - 1;
	if (::SkinGetControlTextByName(m_hWnd, szCtrlName, szTmp, &nSize))
	{
		char szValue[MAX_PATH] = {0};
		CStringConversion::WideCharToString(szTmp, szValue, MAX_PATH - 1);
		pCfg->SetParamValue(bCommon, szSection, szParam, szValue);
	} else
		pCfg->SetParamValue(bCommon, szSection, szParam, ""); 
}

void CConfigureUIImpl::RadioMapUIByCfg(IConfigure *pCfg, BOOL bCommon, const char *szSection,
	       const char *szParam, const TCHAR *szCtrlName)
{
	CInterfaceAnsiString strTmp;
	if (SUCCEEDED(pCfg->GetParamValue(bCommon, szSection, szParam, (IAnsiString *)&strTmp)))
	{
		if (::stricmp(strTmp.GetData(), "true") == 0)
			::SkinSetRadioChecked(m_hWnd, szCtrlName, TRUE);
		else
			::SkinSetRadioChecked(m_hWnd, szCtrlName, FALSE);
	}
}

void CConfigureUIImpl::RadioMapUIToCfg(IConfigure *pCfg, BOOL bCommon, const char *szSection,
	       const char *szParam, const TCHAR *szCtrlName)
{
	BOOL bChecked = ::SkinGetRadioChecked(m_hWnd, szCtrlName);
	if (bChecked)
		pCfg->SetParamValue(bCommon, szSection, szParam, "true");
	else
		pCfg->SetParamValue(bCommon, szSection, szParam, "false");
}

void CConfigureUIImpl::HotKeyMapUIByCfg(IConfigure *pCfg, BOOL bCommon, const char *szSection,
	       const char *szParam, const char *szDefault, const TCHAR *szCtrlName, 
		   const TCHAR *szRadioDefault, const TCHAR *szRadioCustom)
{
	CInterfaceAnsiString strTmp;
	if (SUCCEEDED(pCfg->GetParamValue(bCommon, szSection, szParam, (IAnsiString *)&strTmp)))
	{
		if ((strTmp.GetSize() == 0) || (::stricmp(strTmp.GetData(), szDefault) == 0))
			::SkinSetRadioChecked(m_hWnd, szRadioDefault, TRUE);
		else
		{
			::SkinSetRadioChecked(m_hWnd, szRadioCustom, TRUE);
			TCHAR szTmp[64] = {0};
			CStringConversion::StringToWideChar(strTmp.GetData(), szTmp, 63);
			::SkinSetControlTextByName(m_hWnd, szCtrlName, szTmp);
		}
	} else
		::SkinSetRadioChecked(m_hWnd, szRadioDefault, TRUE);
}

void CConfigureUIImpl::HotKeyMapUIToCfg(IConfigure *pCfg, BOOL bCommon, const char *szSection,
	       const char *szParam, const char *szDefault, const TCHAR *szCtrlName, 
		   const TCHAR *szRadioDefault, const TCHAR *szRadioCustom)
{
	BOOL bChecked = ::SkinGetRadioChecked(m_hWnd, szRadioDefault);
	if (bChecked)
	{
		pCfg->SetParamValue(bCommon, szSection, szParam, "");
	} else
	{
		TCHAR szTmp[MAX_PATH] = {0};
		int nSize = MAX_PATH - 1;
		::SkinGetControlTextByName(m_hWnd, szCtrlName, szTmp, &nSize);
		char szValue[MAX_PATH] = {0};
		CStringConversion::WideCharToString(szTmp, szValue, MAX_PATH - 1);
		pCfg->SetParamValue(bCommon, szSection, szParam, szValue);
	}
}

//
void CConfigureUIImpl::SelectSoundListChanged()
{	
	static const char SOUND_ITEM_FILENAME_LIST[8][32] = {"system.wav", "friend.wav", "group.wav", "online.wav",
		                        "video.wav", "audio.wav", "shake.wav", "sms.wav"};
	int idx = ::SkinGetDropdownSelectIndex(m_hWnd, L"cb_selsound");
 	if ((idx >= 0) && (idx < 8))
	{
		IConfigure *pCfg = NULL; 
		if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			CInterfaceAnsiString strValue;
			if (SUCCEEDED(pCfg->GetParamValue(FALSE, "sound", SOUND_ITEM_NAME[idx], (IAnsiString *)&strValue)))
			{
				TCHAR szwFileName[MAX_PATH] = {0};
				CStringConversion::StringToWideChar(strValue.GetData(), szwFileName, MAX_PATH - 1);
				::SkinSetControlTextByName(m_hWnd, L"edt_filename", szwFileName);
			} else
			{
				char szAppFileName[MAX_PATH] = {0};
				char szAppPath[MAX_PATH] = {0};
				CSystemUtils::GetApplicationFileName(szAppFileName, MAX_PATH - 1);
				CSystemUtils::ExtractFilePath(szAppFileName, szAppPath, MAX_PATH - 1);
				std::string strFileName = szAppPath;
				strFileName += "sound\\";
				strFileName += SOUND_ITEM_FILENAME_LIST[idx];
				TCHAR szwFileName[MAX_PATH] = {0};
				CStringConversion::StringToWideChar(strFileName.c_str(), szwFileName, MAX_PATH - 1);
				::SkinSetControlTextByName(m_hWnd, L"edt_filename", szwFileName);				
			}
			pCfg->Release();
		} //end if (m_pCore ...
	} //end if (::SkinGetControlTextByName(..
}

//
void CConfigureUIImpl::InitSoundFile(IConfigure *pCfg)
{
	static const TCHAR SOUND_ITEM_LIST[][8] = {L"系统消息", L"好友消息", L"群消息", L"上线消息", L"视频消息", L"音频消息",
	                                           L"闪动消息", L"短信消息"};
	for (int i = 0; i < 8; i ++)
	{
		::SkinSetDropdownItemString(m_hWnd, L"cb_selsound", 9999, SOUND_ITEM_LIST[i], NULL);
	}
	::SkinSelectDropdownItem(m_hWnd, L"cb_selsound", 0);
}

void CConfigureUIImpl::InitReplys(IConfigure *pCfg)
{
	CMessageList msgList;
	if (SUCCEEDED(pCfg->GetReplyMessage(1, (IMessageList *)&msgList)))
	{
		TCHAR szTmp[MAX_PATH];
		int nId = 0;
		CInterfaceAnsiString strMsg;
		for (int i = 0; i < (int) msgList.GetCount(); i ++)
		{
			if (SUCCEEDED(msgList.GetRawMsg(i, &nId, (IAnsiString *)&strMsg)))
			{
				memset(szTmp, 0, sizeof(TCHAR) * MAX_PATH);
				CStringConversion::StringToWideChar(strMsg.GetData(), szTmp, MAX_PATH - 1);
				::SkinSetDropdownItemString(m_hWnd, L"cb_selshortreply", -1, szTmp, (void *)nId);
			}
		}
		if (SUCCEEDED(pCfg->GetParamValue(FALSE, "replysetting", "shortreply", (IAnsiString *)&strMsg)))
		{
			int idx = ::atoi(strMsg.GetData());
			::SkinSelectDropdownItem(m_hWnd, L"cb_selshortreply", idx);
		}
	}
	msgList.Clear();
	if (SUCCEEDED(pCfg->GetReplyMessage(2, (IMessageList *)&msgList)))
	{
		TCHAR szTmp[MAX_PATH];
		int nId = 0;
		CInterfaceAnsiString strMsg;
		for (int i = 0; i < (int) msgList.GetCount(); i ++)
		{
			if (SUCCEEDED(msgList.GetRawMsg(i, &nId, (IAnsiString *)&strMsg)))
			{
				memset(szTmp, 0, sizeof(TCHAR) * MAX_PATH);
				CStringConversion::StringToWideChar(strMsg.GetData(), szTmp, MAX_PATH - 1);
				::SkinSetDropdownItemString(m_hWnd, L"cb_selautoreply", -1, szTmp, (void *)nId);
			}
		}
		if (SUCCEEDED(pCfg->GetParamValue(FALSE, "replysetting", "autoreply", (IAnsiString *)&strMsg)))
		{
			int idx = ::atoi(strMsg.GetData());
			::SkinSelectDropdownItem(m_hWnd, L"cb_selautoreply", idx);
		}
	}
}

void CConfigureUIImpl::InitExceptContactTip(IConfigure *pCfg)
{
	::SkinTreeViewClear(m_hWnd, L"contacttiptree");
	CInterfaceUserList ulList; 
	IContacts *pContact = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
	{
		pCfg->GetContactOnlineTipUsers(&ulList);
		LPORG_TREE_NODE_DATA pData;
		TCHAR szwText[256];
		CInterfaceAnsiString szName;
		while (SUCCEEDED(ulList.PopBackUserInfo(&pData)))
		{
			if (SUCCEEDED(pContact->GetRealNameById(pData->szUserName, NULL, &szName)))
			{
				memset(szwText, 0, sizeof(TCHAR) * 256);
				CStringConversion::UTF8ToWideChar(szName.GetData(), szwText, 255);
				::SkinAddTreeChildNode(m_hWnd, L"contacttiptree", pData->id, NULL, szwText, TREENODE_TYPE_LEAF, pData, NULL, NULL, NULL);
			} else
			{
				delete pData;
			}
		} //end while (SUCCEEDED(ulList...
		::SkinExpandTree(m_hWnd, L"contacttiptree", NULL, TRUE, TRUE);
		pContact->Release(); 
	} //end if (SUCCEEDED(...
}

//初始化网络设置
void CConfigureUIImpl::InitNetSetting(IConfigure *pCfg)
{ 
	EditMapUIByCfg(pCfg, TRUE, "Server", "Host", L"edit_svraddr");
	EditMapUIByCfg(pCfg, TRUE, "Server", "Port", L"edit_netport");
	EditMapUIByCfg(pCfg, TRUE, "Server", "domain", L"edit_svrresource");
	EditMapUIByCfg(pCfg, TRUE, "Server", "agentaddr", L"edit_agentaddr");
	EditMapUIByCfg(pCfg, TRUE, "Server", "agentport", L"edit_agentport");
	EditMapUIByCfg(pCfg, TRUE, "Server", "agentusername", L"edit_agentusername");
	EditMapUIByCfg(pCfg, TRUE, "Server", "agentuserpwd", L"edit_agentuserpwd");
}

void CConfigureUIImpl::InitUI()
{
	IConfigure *pCfg = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		CInterfaceAnsiString strTmp;
		IContacts *pContact = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
		{
			if (SUCCEEDED(pContact->GetContactHead(NULL, &strTmp, FALSE)))
			{
				TCHAR szFileName[MAX_PATH] = {0};
				CStringConversion::StringToWideChar(strTmp.GetData(), szFileName, MAX_PATH - 1);
				::SkinSetControlAttr(m_hWnd, L"headerimage", L"filename", szFileName);
			}
			pContact->Release();
		}
		//normal
		if (IsWindowsRun())
		{
			::SkinSetCheckBoxStatus(m_hWnd, L"check_autostart", 1);
		} else
		{
			::SkinSetCheckBoxStatus(m_hWnd, L"check_autostart", 0);
		} 
		CheckStatusMapUIByCfg(pCfg, FALSE, "normal", "closewinexit", L"check_closewinexit");
		CheckStatusMapUIByCfg(pCfg, TRUE, "normal", "saveusername", L"check_saveusername");
		CheckStatusMapUIByCfg(pCfg, TRUE, "normal", "savepwd", L"check_savepwd");
		CheckStatusMapUIByCfg(pCfg, TRUE, "normal", "autologon", L"check_autologon");
		CheckStatusMapUIByCfg(pCfg, TRUE, "normal", "autologin", L"check_autologin");
		CheckStatusMapUIByCfg(pCfg, FALSE, "normal", "mini2tray", L"check_mini2tray");
		CheckStatusMapUIByCfg(pCfg, FALSE, "normal", "showmainframe", L"check_showmainframe");
		CheckStatusMapUIByCfg(pCfg, FALSE, "normal", "aimsg", L"check_aimsg");
		CheckStatusMapUIByCfg(pCfg, TRUE, "logonwindow", "showrealname", L"check_loginshowname");
		CheckStatusMapUIByCfg(pCfg, FALSE, "normal", "msgtransparent", L"check_msgTransparent");
		EditMapUIByCfg(pCfg, FALSE, "normal", "transimagefile", L"edttransparentimage");
		CheckStatusMapUIByCfg(pCfg, FALSE, "normal", "saverecently", L"check_recently");
		EditMapUIByCfg(pCfg, FALSE, "normal", "recentlycount",  L"edit_recentlycount");
		//person setting
		CheckStatusMapUIByCfg(pCfg, FALSE, "person", "notifyonline", L"check_notifyonline");
		CheckStatusMapUIByCfg(pCfg, FALSE, "person", "autosavep2pchat", L"check_autosavech");
		CheckStatusMapUIByCfg(pCfg, FALSE, "person", "autosavegroupchat", L"check_autosavegh");
		CheckStatusMapUIByCfg(pCfg, FALSE, "person", "autochangestatus", L"check_autochangestate");
		EditMapUIByCfg(pCfg, FALSE, "person", "timetoaway", L"edit_timetoaway");
		RadioMapUIByCfg(pCfg, FALSE, "person", "autopopup", L"radio_autopopup");
		RadioMapUIByCfg(pCfg, FALSE, "person", "show2dock", L"radio_show2dock");
		RadioMapUIByCfg(pCfg, FALSE, "person", "flashonly", L"radio_flashonly");

		//reply setting
		CheckStatusMapUIByCfg(pCfg, FALSE, "replysetting", "start", L"check_autoreply");
		InitReplys(pCfg);

		//sound setting
		CheckStatusMapUIByCfg(pCfg, FALSE, "sound", "play", L"check_playnotify");
		InitSoundFile(pCfg);
		RadioMapUIByCfg(pCfg, FALSE, "onlinetip", "allcontacttip", L"rd_tip_open");
		RadioMapUIByCfg(pCfg, FALSE, "onlinetip", "allcontactclose", L"rd_tip_Close");
		RadioMapUIByCfg(pCfg, FALSE, "onlinetip", "exceptcontact", L"rd_tip_except");
		InitExceptContactTip(pCfg);

		//file transfer setting
		CheckStatusMapUIByCfg(pCfg, FALSE, "filetransfer", "resumetransfer", L"check_resumetransfer");
		CheckStatusMapUIByCfg(pCfg, FALSE, "filetransfer", "autorecv1mfile", L"check_autorecv1mfile");
		EditMapUIByCfg(pCfg, FALSE, "filetransfer", "cachepath", L"edit_cachepath");
		EditMapUIByCfg(pCfg, FALSE, "filetransfer", "defaultpath", L"edit_defpath");

		//hotkey settting
		HotKeyMapUIByCfg(pCfg, FALSE, "hotkey", "pickmsg", "CTRL+ALT+I", L"edit_pickhotkey", L"radio_pickdef", L"radio_pickcustom");
		HotKeyMapUIByCfg(pCfg, FALSE, "hotkey", "cutscreen", "CTRL+ALT+C", L"edit_screenhotkey", L"radio_screendef", L"radio_screencustom");
		RadioMapUIByCfg(pCfg, FALSE, "hotkey", "entersendmsg", L"radio_entersend");
		RadioMapUIByCfg(pCfg, FALSE, "hotkey", "ctrlentersendmsg", L"radio_ctrlentersend");

		//net setting
		InitNetSetting(pCfg);

		//::SkinSetControlEnable(m_hWnd, L"setting_apply", FALSE);
		pCfg->Release();
	}
	SetChanged(FALSE);
}

void CConfigureUIImpl::SavePersonUserSetting(IConfigure *pCfg)
{
	
	//
	int nSaveUserName = ::SkinGetCheckBoxStatus(m_hWnd, L"check_saveusername");
	int nSavePwd = ::SkinGetCheckBoxStatus(m_hWnd, L"check_savepwd");
	CInterfaceAnsiString strUserName, strUserPwd, strUserDomain, strHost;
	CInterfaceAnsiString strPort, strPresence, strPresenceMemo;
	m_pCore->GetUserDomain(&strUserDomain);
	m_pCore->GetUserName(&strUserName);
	m_pCore->GetUserPassword(&strUserPwd);
	m_pCore->GetPresence(NULL, &strPresence, &strPresenceMemo);
	pCfg->GetParamValue(TRUE, "Server", "Host", &strHost);
	pCfg->GetParamValue(TRUE, "Server", "Port", &strPort);
	if (nSavePwd == 1)
	{
		pCfg->SetUserLoginInfo(strUserName.GetData(), strUserPwd.GetData(), strUserDomain.GetData(),
			TRUE, strPresence.GetData(), strHost.GetData(), ::atoi(strPort.GetData()));
	} else if (nSaveUserName == 1)
	{
		pCfg->SetUserLoginInfo(strUserName.GetData(), NULL, strUserDomain.GetData(),
			FALSE, strPresence.GetData(), strHost.GetData(), ::atoi(strPort.GetData()));
	} else
	{
		pCfg->SetUserLoginInfo(strUserName.GetData(), NULL, strUserDomain.GetData(), FALSE, NULL, NULL, 0);
	}
}

void CConfigureUIImpl::SaveUI()
{
	if (m_bChanged)
	{
		IConfigure *pCfg = NULL;
		if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			int nStatus = ::SkinGetCheckBoxStatus(m_hWnd, L"check_autostart");
			WriteWindowRun(nStatus == 1); 
			CheckStatusMapUIToCfg(pCfg, FALSE, "normal", "checkautostart", L"check_autostart");
			CheckStatusMapUIToCfg(pCfg, FALSE, "normal", "closewinexit", L"check_closewinexit");
			CheckStatusMapUIToCfg(pCfg, TRUE, "normal", "saveusername", L"check_saveusername");
			CheckStatusMapUIToCfg(pCfg, TRUE, "normal", "savepwd", L"check_savepwd");
			CheckStatusMapUIToCfg(pCfg, TRUE, "normal", "autologin", L"check_autologin");
			CheckStatusMapUIToCfg(pCfg, TRUE, "normal", "autologon", L"check_autologon");
			CheckStatusMapUIToCfg(pCfg, FALSE, "normal", "mini2tray", L"check_mini2tray");
			CheckStatusMapUIToCfg(pCfg, FALSE, "normal", "showmainframe", L"check_showmainframe");
			CheckStatusMapUIToCfg(pCfg, FALSE, "normal", "aimsg", L"check_aimsg");
			CheckStatusMapUIToCfg(pCfg, TRUE, "logonwindow", "showrealname", L"check_loginshowname");
			CheckStatusMapUIToCfg(pCfg, FALSE, "normal", "msgtransparent", L"check_msgTransparent");
			EditMapUIToCfg(pCfg, FALSE, "normal", "transimagefile", L"edttransparentimage");
			CheckStatusMapUIToCfg(pCfg, FALSE, "normal", "saverecently", L"check_recently");
			EditMapUIToCfg(pCfg, FALSE, "normal", "recentlycount",  L"edit_recentlycount");
			SavePersonUserSetting(pCfg);
			//
			//person setting
			CheckStatusMapUIToCfg(pCfg, FALSE, "person", "notifyonline", L"check_notifyonline");
			CheckStatusMapUIToCfg(pCfg, FALSE, "person", "autosavep2pchat", L"check_autosavech");
			CheckStatusMapUIToCfg(pCfg, FALSE, "person", "autosavegroupchat", L"check_autosavegh");
			CheckStatusMapUIToCfg(pCfg, FALSE, "person", "autochangestatus", L"check_autochangestate");
			EditMapUIToCfg(pCfg, FALSE, "person", "timetoaway", L"edit_timetoaway");
			RadioMapUIToCfg(pCfg, FALSE, "person", "autopopup", L"radio_autopopup");
			RadioMapUIToCfg(pCfg, FALSE, "person", "show2dock", L"radio_show2dock");
			RadioMapUIToCfg(pCfg, FALSE, "person", "flashonly", L"radio_flashonly");
			
			////reply setting
			CheckStatusMapUIToCfg(pCfg, FALSE, "replysetting", "start", L"check_autoreply");
			
			//sound setting
			CheckStatusMapUIToCfg(pCfg, FALSE, "sound", "play", L"check_playnotify");
			RadioMapUIToCfg(pCfg, FALSE, "onlinetip", "allcontacttip", L"rd_tip_open");
			RadioMapUIToCfg(pCfg, FALSE, "onlinetip", "allcontactclose", L"rd_tip_Close");
			RadioMapUIToCfg(pCfg, FALSE, "onlinetip", "exceptcontact", L"rd_tip_except"); 

			//file transfer setting
			CheckStatusMapUIToCfg(pCfg, FALSE, "filetransfer", "resumetransfer", L"check_resumetransfer");
			CheckStatusMapUIToCfg(pCfg, FALSE, "filetransfer", "autorecv1mfile", L"check_autorecv1mfile");
			EditMapUIToCfg(pCfg, FALSE, "filetransfer", "cachepath", L"edit_cachepath");
			EditMapUIToCfg(pCfg, FALSE, "filetransfer", "defaultpath", L"edit_defpath");
			//hotkey settting
			HotKeyMapUIToCfg(pCfg, FALSE, "hotkey", "pickmsg", "CTRL+ALT+I", L"edit_pickhotkey", L"radio_pickdef", L"radio_pickcustom");
			HotKeyMapUIToCfg(pCfg, FALSE, "hotkey", "cutscreen", "CTRL+ALT+C", L"edit_screenhotkey", L"radio_screendef", L"radio_screencustom");
			RadioMapUIToCfg(pCfg, FALSE, "hotkey", "entersendmsg", L"radio_entersend");
			RadioMapUIToCfg(pCfg, FALSE, "hotkey", "ctrlentersendmsg", L"radio_ctrlentersend");
	 
			//update hotkey
			IMainFrame *pFrame = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMainFrame), (void **)&pFrame)))
			{
				pFrame->UpdateHotKey(0);
				pFrame->Release();
			}
		 
			//		
			//net setting
			EditMapUIToCfg(pCfg, FALSE, "netsetting", "svraddr", L"edit_svraddr");
			EditMapUIToCfg(pCfg, FALSE, "netsetting", "svrport", L"edit_netport");
			EditMapUIToCfg(pCfg, FALSE, "netsetting", "svrresource", L"edit_svrresource");
			EditMapUIToCfg(pCfg, FALSE, "netsetting", "agentaddr", L"edit_agentaddr");
			EditMapUIToCfg(pCfg, FALSE, "netsetting", "agentport", L"edit_agentport");
			EditMapUIToCfg(pCfg, FALSE, "netsetting", "agentusername", L"edit_agentusername");
			EditMapUIToCfg(pCfg, FALSE, "netsetting", "agentuserpwd", L"edit_agentuserpwd");
			pCfg->Release();
			SetChanged(FALSE);
		}
	}
}

void CConfigureUIImpl::SetChanged(BOOL bChanged)
{
	if (bChanged != m_bChanged)
	{
		m_bChanged = bChanged;
		::SkinSetControlEnable(m_hWnd, L"setting_apply", m_bChanged);
	} //end if (bChanged != ...
}

//
STDMETHODIMP CConfigureUIImpl::DoWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes)
{
	if ((uMsg == WM_DESTROY) && (hWnd == m_hWnd))
	{
		RECT rc = {0};
		m_hWnd = NULL;
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
						pCfg->SetParamValue(FALSE, "Position", "ConfigFrame", strRect.c_str());
						pCfg->Release();
					} //end if (SUCCEEDED(...
				} //end if (m_pCore)
			} //end if (CSystemUtils::RectToString(...
		} //end if (::GetWindowRect(hWnd...
	} else if (uMsg == WM_SHOW_DETAIL_INFO)
	{
		if ((m_hWndInfo != NULL) && (::IsWindow(m_hWndInfo)))
		{
			int nSize = (int) wParam;
			char *szText = (char *)lParam;
			ShowInfoToUI(szText, nSize);
		}
	}
	return E_FAIL;
}

//
STDMETHODIMP CConfigureUIImpl::DoRecvProtocol(const BYTE *pData, const LONG lSize)
{
	TiXmlDocument xmldoc;
	if (xmldoc.Load((char *)pData, lSize))
	{
		TiXmlElement *pNode = xmldoc.FirstChildElement();
		if (pNode)
		{
			const char *pValue = pNode->Value();
			const char *pType = pNode->Attribute("type");
			if (pValue && pType)
			{
				if (::stricmp(pValue, "sys") == 0)
				{
					if (::stricmp(pType, "getvcard") == 0)
						::SendMessage(m_hWndMain, WM_SHOW_DETAIL_INFO, (WPARAM)lSize, (LPARAM)pData);
					else
						DoSysProtocol(pType, pNode);
				} else
				{
					PRINTDEBUGLOG(dtInfo, "configure ui else protocol, value:%s type:%s", pValue, pType);
				} //end else if (::stricmp(pValue, "sys") == 0)
			} //end if (pValue && pType)
		} //end if (pNode)
	} //end if (xmldoc.Load(...
	return E_FAIL;
}

 
void CConfigureUIImpl::MapToUI(const char *szText, const TCHAR *szUIName)
{
	if (szText)
	{
		TCHAR szTmp[MAX_PATH] = {0};
		CStringConversion::StringToWideChar(szText, szTmp, MAX_PATH - 1);
		::SkinSetControlTextByName(m_hWndInfo, szUIName, szTmp);
	}
}

void CConfigureUIImpl::ShowInfoToUI(const char *szText, int nSize)
{
	TiXmlDocument xmldoc;
	TiXmlElement *pNode = NULL;
	if (xmldoc.Load(szText, nSize))
	{
		pNode = xmldoc.FirstChildElement();
	}
	if (pNode)
	{
		if ((m_hWndInfo == NULL) || (!::IsWindow(m_hWndInfo)))
			return  ;
		IContacts *pContacts = NULL;
		if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContacts)))
		{
			std::string strUserName = pNode->Attribute("uid");
			//debug
			strUserName += "@kongzhong";
			CInstantUserInfo Info;
			TCHAR szTmp[MAX_PATH] = {0};
			if (SUCCEEDED(pContacts->GetContactUserInfo(strUserName.c_str(), (IInstantUserInfo *)&Info)))
			{
				CInterfaceAnsiString strSign;
				if (SUCCEEDED(Info.GetUserInfo("sign", (IAnsiString *)&strSign)))
				{
					CStringConversion::UTF8ToWideChar(strSign.GetData(), szTmp, MAX_PATH - 1);
				} //end if 
			} //		
			::SkinSetControlTextByName(m_hWndInfo, L"edtsign", szTmp); 
			//部门
			CInterfaceAnsiString strDeptNames;
			if (SUCCEEDED(pContacts->GetDeptPathNameByUserName(strUserName.c_str(), &strDeptNames)))
			{
				TCHAR szwDept[1024] = {0};
				CStringConversion::UTF8ToWideChar(strDeptNames.GetData(), szwDept, 1023);
				::SkinSetControlTextByName(m_hWndInfo, L"department", szwDept);
			}
			MapToUI(pNode->Attribute("mobile"), L"edtmobilephone");
			MapToUI(pNode->Attribute("workcell"), L"edtcell");
			MapToUI(pNode->Attribute("cphone"), L"edttel");
			MapToUI(pNode->Attribute("email"), L"edtEmail");
			 

			const char *szDetail = pNode->Attribute("userdetail");
			if (szDetail)
			{
				//<userdetail province="北京" city="北京" tel="010-86968585" postcode="100010" address="东升园" homepage="www.smartdot.com"/>
				TiXmlDocument xmldoc;
				if (xmldoc.Load(szDetail, strlen(szDetail)))
				{ 
					TiXmlElement *pFirst = xmldoc.FirstChildElement();
					if (pFirst)
					{
						MapToUI(pFirst->Attribute("province"), L"edtprovince");
						MapToUI(pFirst->Attribute("city"), L"edtcity");
						MapToUI(pFirst->Attribute("tel"), L"edtfamilytel");
						MapToUI(pFirst->Attribute("postcode"), L"edtpostcode");
						MapToUI(pFirst->Attribute("address"), L"edtaddress");
						MapToUI(pFirst->Attribute("homepage"), L"edthomepage"); 
					} //end if (pFirst)
				} //end if (xmldoc.
			} //end if (szDetail
			pContacts->Release();
		}
	}

}

BOOL CConfigureUIImpl::DoSysProtocol(const char *pType, TiXmlElement *pNode)
{
	return TRUE;
}

void CConfigureUIImpl::UIMapToXml(TiXmlElement *pNode, const char *szAttrName, const TCHAR *szUIName)
{
	TCHAR szText[MAX_PATH] = {0};
	int nSize = MAX_PATH - 1;
	char szTmp[MAX_PATH] = {0};
	if (::SkinGetControlTextByName(m_hWndInfo, szUIName, szText, &nSize))
	{
		CStringConversion::WideCharToString(szText, szTmp, MAX_PATH);
		pNode->SetAttribute(szAttrName, szTmp);
	}
}

void CConfigureUIImpl::SavePersonCfg()
{
	TCHAR szwSign[MAX_PATH] = {0};
	int nSize = MAX_PATH - 1;
	::SkinGetControlTextByName(m_hWndInfo, _T("edtsign"), szwSign, &nSize);
	char szUTF8[MAX_PATH - 1] = {0};
	CStringConversion::WideCharToUTF8(szwSign, szUTF8, MAX_PATH - 1);
	CInterfaceAnsiString strOld;
	BOOL bUpSvr = FALSE;
	IContacts *pContacts = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContacts)))
	{
		std::string strUserName;
		CInterfaceAnsiString strTmp;
		m_pCore->GetUserName(&strTmp);
		strUserName = strTmp.GetData();
		m_pCore->GetUserDomain(&strTmp);
		strUserName += "@";
		strUserName += strTmp.GetData();
		if (SUCCEEDED(pContacts->GetContactUserValue(strUserName.c_str(), "sign", (IAnsiString *)&strOld)))
		{
			if (::stricmp(szUTF8, strOld.GetData()) != 0)
			{
				bUpSvr = TRUE;
			}
		} else
		{
			bUpSvr = TRUE;
		}
		if (bUpSvr)
		{ 
			//<sys type="sign" uid="user@doamin" sign="sign"/>
			pContacts->SetContactUserInfo(strUserName.c_str(), "sign", szUTF8);
			std::string strXml;
			strXml = "<sys type=\"sign\" uid=\"";
			strXml += strUserName;
			strXml += "\" sign=\"";
			char strSign[MAX_PATH] = {0};
			CStringConversion::UTF8ToString(szUTF8, strSign, MAX_PATH - 1);
			TiXmlString strXmlSign;
			TiXmlBase::EncodeString(strSign, strXmlSign);
			strXml += strXmlSign.c_str();
			strXml += "\"/>";
			if (m_pCore)
				m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (int) strXml.size(), 0);
		}
		pContacts->Release();
	}
	// "<sys type="vcard" uid="wxz" mobile="13988888" email="a@gcom" workcell="1234"
	// faxnum="010-223909809" cphone="190009292" userdetail="<userdetail province="北京" city="北京" tel="010-86968585" postcode="100010" address="东升园" homepage="www.sma.com"/>"/>
	TiXmlDocument xmldoc;
	TiXmlDocument xmlDetail;
	static const char SYS_VCARD_XML[] ="<sys type=\"vcard\"/>";
	static const char VCARD_DETAIL_XML[] = "<userdetail/>";
	TiXmlString strDetail;
	TiXmlString strXml;
	if (xmlDetail.Load(VCARD_DETAIL_XML, strlen(VCARD_DETAIL_XML)))
	{
		TiXmlElement *pNode = xmlDetail.FirstChildElement();
		if (pNode)
		{
			UIMapToXml(pNode, "province", L"edtprovince");
			UIMapToXml(pNode, "city", L"edtcity");
			UIMapToXml(pNode, "tel", L"edtfamilytel");
			UIMapToXml(pNode, "postcode", L"edtpostcode");
			UIMapToXml(pNode, "address", L"edtaddress");
			UIMapToXml(pNode, "homepage", L"edthomepage");
		}
		xmlDetail.SaveToString(strDetail, 0);
	}
	if (xmldoc.Load(SYS_VCARD_XML, strlen(SYS_VCARD_XML)))
	{
		TiXmlElement *pNode = xmldoc.FirstChildElement();
		if (pNode)
		{
			CInterfaceAnsiString strTmp;
			m_pCore->GetUserName(&strTmp);
			pNode->SetAttribute("uid", strTmp.GetData());
			UIMapToXml(pNode, "mobile", L"edtmobilephone");
			UIMapToXml(pNode, "email", L"edtEmail");
			UIMapToXml(pNode, "workcell", L"edtcell");
			UIMapToXml(pNode, "faxnum", L"edtfax");
			UIMapToXml(pNode, "cphone", L"edttel");
			pNode->SetAttribute("userdetail", strDetail.c_str());
		}
		xmldoc.SaveToString(strXml, 0);
	}
	BOOL bSucc = FALSE;
	if (m_pCore)
		bSucc = SUCCEEDED(m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG)strXml.size(), 0));
	if (bSucc)
	{
		::SkinMessageBox(m_hWndInfo, L"更新个人资料成功", L"提示", MB_OK);
	} else
	{
		::SkinMessageBox(m_hWndInfo, L"更新个人资料失败", L"提示", MB_OK);
	}
}

BOOL CConfigureUIImpl::UpdateFromServer(const char *szUserName)
{
	if (szUserName)
	{
		std::string strXml = "<sys type=\"getvcard\" uid=\"";
		std::string strName = szUserName;
		int nPos = strName.find('@');
		if (nPos != std::string::npos)
			strXml += strName.substr(0, nPos);
		else
			strXml += szUserName;
		strXml += "\"/>";
		if (m_pCore)
			return SUCCEEDED(m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (int) strXml.size(), 0));
	}
	return FALSE;
}

STDMETHODIMP CConfigureUIImpl::DoPresenceChange(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder)
{
	return E_NOTIMPL;
}

//检测是否在windows启动时运行
BOOL CConfigureUIImpl::IsWindowsRun()
{
	char szFileName[MAX_PATH] = {0};
	if (CSystemUtils::ReadRegisterKey(HKEY_CURRENT_USER, REGISTER_START_RUN_KEY, REGISTER_START_RUN_NAME, szFileName, MAX_PATH - 1))
	{
		char szAppFileName[MAX_PATH] = {0};
		if (CSystemUtils::GetApplicationFileName(szAppFileName, MAX_PATH))
		{
			if (::stricmp(szAppFileName, szFileName) == 0)
				return TRUE;
		} // 
	}
	return FALSE;
}

//写入在windows启动时运行
void CConfigureUIImpl::WriteWindowRun(BOOL bIsRun) 
{
	if (bIsRun)
	{
		char szFileName[MAX_PATH] = {0};
		if (CSystemUtils::GetApplicationFileName(szFileName, MAX_PATH))
		{
			CSystemUtils::WriteRegisterKey(HKEY_CURRENT_USER, REGISTER_START_RUN_KEY, REGISTER_START_RUN_NAME, szFileName);
		}
	} else
	{
		CSystemUtils::DeleteRegisterKey(HKEY_CURRENT_USER, REGISTER_START_RUN_KEY, REGISTER_START_RUN_NAME);
	}
}

#pragma warning(default:4996)
