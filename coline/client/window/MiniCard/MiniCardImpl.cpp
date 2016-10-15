#include <Commonlib/DebugLog.h>
#include <Commonlib/systemutils.h>
#include <Commonlib/stringutils.h>
#include <SmartSkin/smartskin.h>
#include <Core/common.h>
#include "../IMCommonLib/InterfaceAnsiString.h"
#include "../IMCommonLib/InterfaceUserList.h"
#include "MiniCardImpl.h"

#include <ShellAPI.h>

#define MINICARD_WIDTH  300
#define MINICARD_HEIGHT 200


#define MINICARD_ALIGN_LEFT   1
#define MINICARD_ALIGN_RIGHT  2
#define MINICARD_ALIGN_TOP    3
#define MINICARD_ALIGN_BOTTOM 4

#pragma warning(disable:4996)

CMiniCardImpl::CMiniCardImpl(void):
               m_pCore(NULL),
			   m_ptrTimer(0),
			   m_hWnd(NULL)
{
}


CMiniCardImpl::~CMiniCardImpl(void)
{
	if (m_hWnd)
		::SkinCloseWindow(m_hWnd);
	m_hWnd = NULL;
	if (m_pCore)
		m_pCore->Release();
	m_pCore = NULL;
}

//IUnknown
STDMETHODIMP CMiniCardImpl::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IMiniCard)))
	{
		*ppv = (IMiniCard *) this;
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

void CMiniCardImpl::ShowCurrContactCard(HWND hWnd, const TCHAR *szCtrlName)
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
			if ((rc.left - MINICARD_WIDTH) < 0)
				ShowMiniCard(pData->szUserName, rc.right + 2, pt.y, MINICARD_ALIGN_RIGHT);
			else
				ShowMiniCard(pData->szUserName, rc.left + 2, pt.y, MINICARD_ALIGN_LEFT);
		}
	}
}

HRESULT CMiniCardImpl::DoMenuCommand(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{

	return -1;
}

HRESULT CMiniCardImpl::DoTreeImageClick(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
	TCHAR szwName[128] = {0};
	CStringConversion::StringToWideChar(szName, szwName, 127);
	ShowCurrContactCard(hWnd, szwName);
	return -1;
}

HRESULT CMiniCardImpl::DoClickEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szName, "mainselfhead") == 0)
	{
		//直接切换至个人设置
		IConfigureUI *pCfg = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigureUI), (void **)&pCfg)))
		{
			pCfg->Navegate2Frame(NULL, "personpage");
			pCfg->Release();
		}
		/*CInterfaceAnsiString strUserName, strDomain;
		m_pCore->GetUserName(&strUserName);
		m_pCore->GetUserDomain(&strDomain);
		if ((strUserName.GetSize() > 0) && (strDomain.GetSize() > 0))
		{			
			RECT rc = {0};
			POINT pt = {0};
			::GetWindowRect(hWnd, &rc);
			::GetCursorPos(&pt);
			std::string strTmp = strUserName.GetData();
			strTmp += "@";
			strTmp += strDomain.GetData();
			if ((rc.left - MINICARD_WIDTH) < 0)
				ShowMiniCard(strTmp.c_str(), rc.right + 2, pt.y, MINICARD_ALIGN_RIGHT);
			else
				ShowMiniCard(strTmp.c_str(), rc.left + 2, pt.y, MINICARD_ALIGN_LEFT);
		} //end if ((strUserName.*/
	} else if (::stricmp(szName, "sendfile") == 0) //发送文件
	{
		if (m_ptrTimer)
		{
			::KillTimer(m_hWnd, m_ptrTimer);
			m_ptrTimer = NULL;
		}
		IChatFrame *pFrame = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pFrame)))
		{
			pFrame->SendFileToPeer(m_strCurrName.c_str(), NULL);
			pFrame->Release();
		}
		::SkinCloseWindow(m_hWnd);
		return 0;
	} else if (::stricmp(szName, "sendfax") == 0) //发送传真
	{
		//
	}  else if (::stricmp(szName, "sendsms") == 0) //发送短信
	{
		ISMSFrame *pFrame = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(ISMSFrame), (void **)&pFrame)))
		{   
			LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
			memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
			strcpy(pData->szUserName, m_strCurrName.c_str());
			CInterfaceUserList ulList;
			ulList.AddUserInfo(pData, FALSE, TRUE);
			pFrame->ShowSMSFrame(NULL, &ulList); 
			pFrame->Release();
		}
	} else if (::stricmp(szName, "sendmail") == 0)
	{
		//发送邮件 
		IContacts *pContact = NULL;
		CInterfaceAnsiString strTmp;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
		{
			pContact->GetMailByUserName(m_strCurrName.c_str(), &strTmp);
			pContact->Release();
		}
		if (strTmp.GetSize() > 0)
		{   
			TCHAR szwTmp[MAX_PATH] = {0};
			CStringConversion::StringToWideChar(strTmp.GetData(), szwTmp, MAX_PATH - 1);
			CStdString_ strOpen = L"mailto:";
			strOpen += szwTmp;
			::ShellExecute(NULL, L"open", strOpen.GetData(), NULL, NULL, SW_SHOW);
		}
	} else if (::stricmp(szName, "sendmsg") == 0)
	{
		//
		IChatFrame *pFrame = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pFrame)))
		{
			pFrame->ShowChatFrame(m_hWnd, m_strCurrName.c_str(), NULL);
			pFrame->Release();
		}
	} else if (::stricmp(szName, "history") == 0)
	{
		//
		IMsgMgrUI *pMsg = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMsgMgrUI), (void **)&pMsg)))
		{
			pMsg->ShowMsgMgrFrame("p2p", m_strCurrName.c_str(), NULL);
			pMsg->Release();
		}
	}
	//end if (::stricmp(szName, "
	return -1;
}

//ICoreEvent
STDMETHODIMP CMiniCardImpl::DoCoreEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
	                     LPARAM lParam, HRESULT *hResult)
{
	if (::stricmp(szType, "imageclick") == 0)
	{
		*hResult = DoTreeImageClick(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "mouseleave") == 0)
	{
		if ((m_hWnd != NULL) && ::IsWindow(m_hWnd))
		{
			if (m_ptrTimer == 0)
			{
				m_ptrTimer = ::GetTickCount();
				::SetTimer(m_hWnd, m_ptrTimer, 1000, NULL);
			}
		}
	} else if (::stricmp(szType, "menucommand") == 0)
	{
		*hResult = DoMenuCommand(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "click") == 0)
	{
		*hResult = DoClickEvent(hWnd, szName, wParam, lParam);
	}
	return E_NOTIMPL;
}

//广播消息
STDMETHODIMP CMiniCardImpl::DoBroadcastMessage(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData)
{
	return E_NOTIMPL;
}

STDMETHODIMP CMiniCardImpl::SetCoreFrameWork(ICoreFrameWork *pCore)
{
	if (m_pCore)
		m_pCore->Release();
	m_pCore = pCore;
	if (m_pCore)
	{
		m_pCore->AddRef();
		//order event
		m_pCore->AddOrderEvent((ICoreEvent *) this, "MainWindow", "colleaguetree", "imageclick");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "MainWindow", "colleaguetree", "mouseleave");			
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "mainselfhead", "click");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "MiniCardWindow", NULL, NULL);

		//order protocol
		m_pCore->AddOrderProtocol((IProtocolParser *) this, "sys", "getvcard");
	}
	return S_OK;
}

STDMETHODIMP CMiniCardImpl::GetSkinXmlString(IAnsiString *szXmlString)
{
	HRESULT hr = E_FAIL;
	IConfigure *pCfg = NULL; 
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{ 
		hr = pCfg->GetSkinXml("MiniCard.xml",szXmlString); 
		pCfg->Release();
	}
	return hr;  
}

//
STDMETHODIMP CMiniCardImpl::CoreFrameWorkError(int nErrorNo, const char *szErrorMsg)
{
	switch(nErrorNo)
	{
		case CORE_ERROR_LOGOUT:
			{
				if ((m_hWnd != NULL) && (::IsWindow(m_hWnd)))
					::SkinCloseWindow(m_hWnd);
				break;
			}
	}
	return S_OK;
}

//
STDMETHODIMP CMiniCardImpl::DoWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes)
{
	if (uMsg == WM_DESTROY)
	{
		if (m_hWnd == hWnd)
			m_hWnd = NULL;
	} else if (uMsg == WM_TIMER)
	{
		if (m_hWnd == hWnd)
		{
			POINT pt = {0};
			RECT rc = {0};
			::GetCursorPos(&pt);
			::GetWindowRect(m_hWnd, &rc);
			if (!::PtInRect(&rc, pt))
				::SkinCloseWindow(m_hWnd);
		} //end if (m_hWnd == hWnd)
	} //end else if (uMsg == WM_TIMER)
	return E_NOTIMPL;
}

BOOL CMiniCardImpl::DoSysProtocol(const char *pType, TiXmlElement *pNode)
{
	if (::stricmp(pType, "getvcard") == 0)
	{
		if (m_hWnd == NULL)
			return FALSE;
		IContacts *pContacts = NULL;
		if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContacts)))
		{
			std::string strUserName = pNode->Attribute("uid");

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
			::SkinSetControlTextByName(m_hWnd, L"edt_sign", szTmp);

			CInterfaceAnsiString strDeptPath;
			memset(szTmp, 0, MAX_PATH * sizeof(TCHAR));
			std::string strTmpUserName = pNode->Attribute("uid");

			if (SUCCEEDED(pContacts->GetDeptPathNameByUserName(strTmpUserName.c_str(), (IAnsiString *)&strDeptPath)))
			{
				CStringConversion::UTF8ToWideChar(strDeptPath.GetData(), szTmp, MAX_PATH - 1);
			}			
			::SkinSetControlTextByName(m_hWnd, L"edt_dept", szTmp);
			::SkinSetControlAttr(m_hWnd, L"edt_dept", L"tooltip", szTmp);

			memset(szTmp, 0, MAX_PATH * sizeof(TCHAR));
			if (pNode->Attribute("mobile"))
			{
				CStringConversion::StringToWideChar(pNode->Attribute("mobile"), szTmp, MAX_PATH - 1);
			}			
			::SkinSetControlTextByName(m_hWnd, L"edt_mobile", szTmp);

			memset(szTmp, 0, MAX_PATH * sizeof(TCHAR));
			if (pNode->Attribute("workcell"))
			{
				CStringConversion::StringToWideChar(pNode->Attribute("workcell"), szTmp, MAX_PATH - 1);				
			}
			::SkinSetControlTextByName(m_hWnd, L"edt_tel", szTmp);

			memset(szTmp, 0, MAX_PATH * sizeof(TCHAR));
			if (pNode->Attribute("email"))
			{
				CStringConversion::StringToWideChar(pNode->Attribute("email"), szTmp, MAX_PATH - 1);				
			}
			::SkinSetControlTextByName(m_hWnd, L"edt_email", szTmp);

			pContacts->Release();
		}

	}
	return FALSE;
}

//IProtocolParser
STDMETHODIMP CMiniCardImpl::DoRecvProtocol(const BYTE *pData, const LONG lSize)
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
					DoSysProtocol(pType, pNode);
				} else
				{
					PRINTDEBUGLOG(dtInfo, "miniCard else protocol, value:%s type:%s", pValue, pType);
				} //end else if (::stricmp(pValue, "sys") == 0)
			} //end if (pValue && pType)
		} //end if (pNode)
	} //end if (xmldoc.Load(...
	return E_NOTIMPL;
}

STDMETHODIMP CMiniCardImpl::DoPresenceChange(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder)
{
	return E_NOTIMPL;
}

inline void MapToSkin(HWND hWnd, const char *szTmp, const TCHAR *szSkinName)
{
	TCHAR szwTmp[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szTmp, szwTmp, MAX_PATH - 1);
	::SkinSetControlTextByName(hWnd, szSkinName, szwTmp);
}

//
STDMETHODIMP CMiniCardImpl::ShowMiniCard(const char *szUserName, int x, int y, int nAlign)
{
	BOOL b = FALSE;
	if (m_hWnd && ::IsWindow(m_hWnd))
	{
		b = TRUE;
	} else
	{
		if (m_hWnd)
			::SkinCloseWindow(m_hWnd);
		m_hWnd = NULL;
		HRESULT hr = E_FAIL;
		if (m_pCore)
		{
			IUIManager *pUI = NULL;

			RECT rc = {1, 1, 300, 200};
			hr = m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI);
			if (SUCCEEDED(hr) && pUI)
			{
				pUI->CreateUIWindow(NULL, "MiniCardWindow", &rc, WS_POPUP ,
				                WS_EX_TOOLWINDOW | WS_EX_TOPMOST, L"资料", &m_hWnd);
				if (m_hWnd)
					::SkinSetCanActived(m_hWnd, FALSE);
				pUI->OrderWindowMessage("MiniCardWindow", m_hWnd, WM_TIMER, (ICoreEvent *) this);
				m_ptrTimer = 0;
				b = TRUE; 
				pUI->Release();
				pUI = NULL;
			}

		} //end if (m_pCore)
	}
	if (b)
	{
		if (::IsWindow(m_hWnd))
		{  
			m_strCurrName = szUserName;
			if (m_ptrTimer != 0)
			{
				::KillTimer(m_hWnd, m_ptrTimer);
				m_ptrTimer = 0;
			}
			switch(nAlign)
			{
			 case MINICARD_ALIGN_LEFT: //left
				 {
					 x -= MINICARD_WIDTH;
					 break;
				 }
			 case MINICARD_ALIGN_RIGHT: //right
				 {
					 break;
				 }
			 case MINICARD_ALIGN_TOP:
				 {
					 y -= MINICARD_HEIGHT;
					 break;
				 }
			 case MINICARD_ALIGN_BOTTOM:
				 {
					 break;
				 }
			}
			RECT rcScreen = {0};
			CSystemUtils::GetScreenRect(&rcScreen);
			if ((x + MINICARD_WIDTH) > rcScreen.right)
				x = rcScreen.right - MINICARD_WIDTH;
			if ((y + MINICARD_HEIGHT) > rcScreen.bottom)
				y = rcScreen.bottom - MINICARD_HEIGHT;
			if (x < 0)
				x = 0;
			if (y < 0)
				y = 0;
			::MoveWindow(m_hWnd, x, y, MINICARD_WIDTH, MINICARD_HEIGHT, TRUE);
			CSystemUtils::BringToFront(m_hWnd);
			IContacts *pContact = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
			{
				CInterfaceAnsiString strName;
				if (SUCCEEDED(pContact->GetRealNameById(szUserName, NULL, &strName)))
				{
					TCHAR szwName[MAX_PATH] = {0};
					CStringConversion::UTF8ToWideChar(strName.GetData(), szwName, MAX_PATH - 1);
					::lstrcat(szwName, L"  的资料");
					::SetWindowText(m_hWnd, szwName);
				}
				if (SUCCEEDED(pContact->GetContactHead(szUserName, &strName, FALSE)))
				{
					TCHAR szImageFile[MAX_PATH] = {0};
					CStringConversion::StringToWideChar(strName.GetData(), szImageFile, MAX_PATH - 1);
					::SkinSetControlAttr(m_hWnd, L"contactheader", L"floatimagefilename", szImageFile);
					//
				} else
				{
					::SkinSetControlAttr(m_hWnd, L"contactheader", L"floatimagefilename", NULL);
					::SkinSetControlAttr(m_hWnd, L"contactheader", L"floatimage", L"35");
				}
				if (SUCCEEDED(pContact->GetDeptPathNameByUserName(szUserName, &strName)))
				{
					TCHAR szwTmp[MAX_PATH] = {0};
					CStringConversion::UTF8ToWideChar(strName.GetData(), szwTmp, MAX_PATH - 1);
					::SkinSetControlTextByName(m_hWnd, L"edt_dept", szwTmp);
			        ::SkinSetControlAttr(m_hWnd, L"edt_dept", L"tooltip", szwTmp);
				}
				if (SUCCEEDED(pContact->GetMailByUserName(szUserName, &strName)))
				{
					MapToSkin(m_hWnd, strName.GetData(), L"edt_email");
				}
				if (SUCCEEDED(pContact->GetCellPhoneByName(szUserName, &strName)))
				{
					MapToSkin(m_hWnd, strName.GetData(), L"edt_tel");
				}
				if (SUCCEEDED(pContact->GetPhoneByName(szUserName, &strName)))
				{
					MapToSkin(m_hWnd, strName.GetData(), L"edt_mobile");
				}
				pContact->Release();
			}
			UpdateFromServer(szUserName);
			::AnimateWindow(m_hWnd, 200, AW_BLEND); 
			return S_OK;
		}
	}
	return E_FAIL;
}

BOOL CMiniCardImpl::UpdateFromServer(const char *szUserName)
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

#pragma warning(default:4996)
