#include <Commonlib/systemutils.h>
#include <Commonlib/stringutils.h>
#include <SmartSkin/smartskin.h> 
#include <Core/common.h>
#include "OATipImpl.h"
#include "../IMCommonLib/InterfaceAnsiString.h"
#include <ShellAPI.h>

#define OATIP_PANEL_WIDTH  210
#define OATIP_PANEL_HEIGHT 200
#define OATIP_PANEL_BODER  10 

typedef struct COATipItem
{
	CStdString_ strTime;
	CStdString_ strCatalog;
	CStdString_ strTip;
	CStdString_ strFrom;
	CStdString_ strSender;
	CStdString_ strId;
	CStdString_ strBody;
}OATIP_ITEM, *LPOATIP_ITEM;

#pragma warning(disable:4996)

COATipImpl::COATipImpl(void):
            m_pCore(NULL),
			m_hMainWindow(NULL)
{
}


COATipImpl::~COATipImpl(void)
{
	if (m_pCore)
		m_pCore->Release();
	m_pCore = NULL;
	m_hMainWindow = NULL;
}

//IUnknown
STDMETHODIMP COATipImpl::QueryInterface(REFIID riid, LPVOID *ppv)

{
	*ppv = NULL;
	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IOATip)))
	{
		*ppv = (IOATip *) this;
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

 

//
STDMETHODIMP COATipImpl::ShowOATipPanel(const TCHAR *szSender, const TCHAR *szFrom, const TCHAR *szTime, 
		                     const TCHAR *szCatalog, const TCHAR *szId, const TCHAR *szTip, const TCHAR *szBody) 
{
	RECT rc = {0}; 
	RECT rcScreen = {0};
	CSystemUtils::GetScreenRect(&rcScreen);
	rc.left = rcScreen.right - OATIP_PANEL_WIDTH - OATIP_PANEL_BODER;
	rc.top = rcScreen.bottom - OATIP_PANEL_HEIGHT - 20;
	rc.right = rc.left + OATIP_PANEL_WIDTH;
	rc.bottom = rc.top + OATIP_PANEL_HEIGHT;
	IUIManager *pUI = NULL;
	HWND hWnd = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
	{ 	
		if (SUCCEEDED(pUI->CreateUIWindow(NULL, "OATipPanel", &rc, WS_POPUP | WS_MINIMIZEBOX, WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
				          szFrom, &hWnd)))
		{
			if (hWnd)
			{
				if (szCatalog)
					::SkinSetControlTextByName(hWnd, L"OACatalog", L"新邮件通知提醒"); 
				if (szSender)
					::SkinSetControlTextByName(hWnd, L"lblfrom",  L"通知"); 
				if (szTime)
					::SkinSetControlTextByName(hWnd, L"lblTime", szTime);
				if (szBody)
					::SkinSetControlTextByName(hWnd, L"OATipMsg", szBody);
				::ShowWindow(hWnd, SW_SHOW);
				return S_OK;
			} //end if (hWnd)
		} //end if (SUCCEEDED(pUI->		  
	} // end if (SUCCEEDED(
	return E_FAIL;
}

void COATipImpl::OpenMail()
{
	IConfigure *pCfg = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			CInterfaceAnsiString mailUrl;
			if ((SUCCEEDED(pCfg->GetServerAddr(HTTP_SVR_MAIL_URL, &mailUrl))) && (mailUrl.GetSize() > 0))
			{
				CInterfaceAnsiString userName, userPwd, userDomain;
				m_pCore->GetUserName(&userName);
				m_pCore->GetUserPassword(&userPwd);
				m_pCore->GetUserDomain(&userDomain);
				std::string strMailUrl = mailUrl.GetData();
				static const char USER_NAME_FLAGS[] = "$USERNAME";
				static const char USER_PASSWORD_FLAGS[] = "$USERPASSWORD";
				int pos = strMailUrl.find("$USERNAME");
				std::string strUserName = userName.GetData();
				strUserName += "@";
				strUserName += userDomain.GetData();
				if (pos != std::string::npos)
					strMailUrl.replace(pos, strlen(USER_NAME_FLAGS), strUserName);
				pos = strMailUrl.find(USER_PASSWORD_FLAGS);
					
				if (pos != std::string::npos)
					strMailUrl.replace(pos, strlen(USER_PASSWORD_FLAGS), userPwd.GetData());
				TCHAR szUrl[256] = {0};
				CStringConversion::StringToWideChar(strMailUrl.c_str(), szUrl, 256); 
				::ShellExecute(NULL, L"open", szUrl, NULL, NULL, SW_SHOW);
			}
			pCfg->Release();
		}
}

//ICoreEvent
STDMETHODIMP COATipImpl::DoCoreEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
                             LPARAM lParam, HRESULT *hResult)
{	 
	if (::stricmp(szType, "afterinit") == 0)
	{
		if (::stricmp(szName, "mainwindow") == 0)
		{
			m_hMainWindow = hWnd;
			IUIManager *pUI = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
			{
				pUI->OrderWindowMessage("mainwindow", NULL, WM_OATIP_SHOWPANEL, (ICoreEvent *) this);
				pUI->Release();
			} //end if (SUCCEEDED(
		} //end if (::stricmp(..
	} else if (::stricmp(szType, "link") == 0)
	{
		//
		OpenMail();
	} else if (::stricmp(szType, "click") == 0)
	{
		OpenMail();
	}
	return E_NOTIMPL;
}

//广播消息
STDMETHODIMP COATipImpl::DoBroadcastMessage(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData)
{
	 
	return E_NOTIMPL;
}

STDMETHODIMP COATipImpl::SetCoreFrameWork(ICoreFrameWork *pCore)
{
	if (m_pCore)
		m_pCore->Release();
	m_pCore = pCore;
	if (m_pCore)
	{
		m_pCore->AddRef();
		// 
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "mainwindow", "afterinit");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "OATipPanel", "OATipMsg", "link");
		m_pCore->AddOrderProtocol((IProtocolParser *) this, "int", NULL);
	}
	return S_OK;
}


STDMETHODIMP COATipImpl::GetSkinXmlString(IAnsiString *szXmlString)
{
	HRESULT hr = E_FAIL;
	IConfigure *pCfg = NULL; 
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{ 
		hr = pCfg->GetSkinXml("OATip.xml",szXmlString); 
		pCfg->Release();
	}
	return hr;   
}

//
STDMETHODIMP COATipImpl::CoreFrameWorkError(int nErrorNo, const char *szErrorMsg)
{
	return E_NOTIMPL;
}
//
STDMETHODIMP COATipImpl::DoWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes)
{
	if (uMsg == WM_OATIP_SHOWPANEL)
	{
		LPOATIP_ITEM pItem = (LPOATIP_ITEM)lParam;
		if (pItem)
		{
			return ShowOATipPanel(pItem->strSender, pItem->strFrom, pItem->strTime,
				pItem->strCatalog, pItem->strId, pItem->strTip, pItem->strBody);
		}
	}
	return E_NOTIMPL;
}

STDMETHODIMP COATipImpl::DoRecvProtocol(const BYTE *pData, const LONG lSize)
{
	TiXmlDocument xmldoc;
	if (xmldoc.Load((char *)pData, lSize))
	{
		TiXmlElement *pRoot = xmldoc.FirstChildElement();
		if (pRoot)
		{
			const char *szName = pRoot->Value();
			const char *szType = pRoot->Attribute("type");
			if (szName && szType)
			{
				if (::stricmp(szName, "int") == 0)
				{
					DoOATipProtcol(szType, pRoot); 
				} //end if (::stricmp(szName,...
			} //end if (szName && szType)
		} //end if (pRoot)
	} //end if (xmldoc.Load(..
	return S_OK;
}

STDMETHODIMP COATipImpl::DoPresenceChange(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder)
{
	return E_NOTIMPL;
}

void COATipImpl::DoOATipProtcol(const char *szType, TiXmlElement *pNode)
{
	//<int type="oa" sender="admin" receiver="cbrc@gocom" from="" catalog="" title="222" datetime="2011-04-29 13:25:14"><body>222</body></int>
	TCHAR *szwTmp = NULL;
	int nSize = 0;
	LPOATIP_ITEM pItem = new OATIP_ITEM();
	const char *szTmp = pNode->Attribute("sender");
	if (szTmp && (strlen(szTmp) > 0))
	{
		nSize = strlen(szTmp);
		szwTmp = new TCHAR[nSize + 1];
		memset(szwTmp, 0, sizeof(TCHAR) * (nSize + 1));
		CStringConversion::StringToWideChar(szTmp, szwTmp, nSize);
		pItem->strSender = szwTmp;
		delete []szwTmp;
	}
	if (stricmp(szType, "email") == 0)
	{
		pItem->strCatalog = L"收到一封新的邮件";
	}
	szTmp = pNode->Attribute("from");
	if (szTmp && (strlen(szTmp) > 0))
	{
		nSize = strlen(szTmp);
		szwTmp = new TCHAR[nSize + 1];
		memset(szwTmp, 0, sizeof(TCHAR) * (nSize + 1));
		CStringConversion::StringToWideChar(szTmp, szwTmp, nSize);
		pItem->strFrom = szwTmp;
		delete []szwTmp;
	}
	
 
	szTmp = pNode->Attribute("catalog");
	if (szTmp && (strlen(szTmp) > 0))
	{
		nSize = strlen(szTmp);
		szwTmp = new TCHAR[nSize + 1];
		memset(szwTmp, 0, sizeof(TCHAR) * (nSize + 1));
		CStringConversion::StringToWideChar(szTmp, szwTmp, nSize);
		pItem->strCatalog = szwTmp;
		delete []szwTmp;
	}
 
	szTmp = pNode->Attribute("title");
	if (szTmp && (strlen(szTmp) > 0))
	{
		nSize = strlen(szTmp);
		szwTmp = new TCHAR[nSize + 1];
		memset(szwTmp, 0, sizeof(TCHAR) * (nSize + 1));
		CStringConversion::UTF8ToWideChar(szTmp, szwTmp, nSize);
		pItem->strTip = szwTmp;
		delete []szwTmp;
	}
	szTmp = pNode->Attribute("datetime");
	if (szTmp && (strlen(szTmp) > 0))
	{
		nSize = strlen(szTmp);
		szwTmp = new TCHAR[nSize + 1];
		memset(szwTmp, 0, sizeof(TCHAR) * (nSize + 1));
		CStringConversion::StringToWideChar(szTmp, szwTmp, nSize);
		pItem->strTime = szwTmp;
		delete []szwTmp;
	}
	szTmp = pNode->Attribute("id");
	if (szTmp && (strlen(szTmp) > 0))
	{
		nSize = strlen(szTmp);
		szwTmp = new TCHAR[nSize + 1];
		memset(szwTmp, 0, sizeof(TCHAR) * (nSize + 1));
		CStringConversion::StringToWideChar(szTmp, szwTmp, nSize);
		pItem->strId = szwTmp;
		delete []szwTmp;
	}
	if (stricmp(szType, "email") == 0)
	{ 
		pItem->strBody = L"您收到一封来自";
		pItem->strBody += pItem->strFrom;
		pItem->strBody += L"的新邮件\n";
		pItem->strBody += L"邮件主题：";
		pItem->strBody += pItem->strTip;
		pItem->strBody += L"";
	} else
	{
		TiXmlElement *pBody = pNode->FirstChildElement("body");
		std::string strText;
		if (pBody)
		{
			szTmp = pBody->GetText();
			if (szTmp && (strlen(szTmp) > 0))
			{
				int nSize = ::strlen(szTmp);
				char szFileName[MAX_PATH] = {0};
				char szExt[128] = {0};
				int nExtSize = 127;
				CSystemUtils::GetGuidString(szExt, &nExtSize);
				IConfigure *pCfg = NULL;
				if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
				{
					CInterfaceAnsiString strPath;
					if (SUCCEEDED(pCfg->GetPath(PATH_LOCAL_CUSTOM_PICTURE, &strPath)))
					{
						strcpy(szFileName, strPath.GetData());
					}
				}
				strcat(szFileName, szExt);
				strcat(szFileName, ".html");
				FILE *fp = fopen(szFileName, "w+b");
				if (fp)
				{
					fwrite(szTmp, 1, nSize, fp);
					fclose(fp);
					nSize = ::strlen(szFileName);
					szwTmp = new TCHAR[nSize + 1];
					memset(szwTmp, 0, sizeof(TCHAR) * (nSize + 1));
					CStringConversion::StringToWideChar(szFileName, szwTmp, nSize);
					pItem->strBody = szwTmp;
					delete []szwTmp;
				} //end if (fp)
			} //end if (szTmp)
		} //end
	}
	::PostMessage(m_hMainWindow, WM_OATIP_SHOWPANEL, 0, LPARAM(pItem));
}
#pragma warning(default:4996)

