#include <Commonlib/DebugLog.h>
#include <Commonlib/systemutils.h>
#include <Core/common.h>
#include "CoreFrameWorkImpl.h"
#include "../IMCommonLib/InterfaceAnsiString.h"
#include "../IMCommonLib/InstantUserInfo.h"

//是否打印客户端收发协议
//#define PRINT_SEND_AND_RECV_PROTOCOL

#pragma warning(disable:4996)

CCoreFrameWorkImpl::CCoreFrameWorkImpl(void):
                    m_pUIManager(NULL),
                    m_pCoreLogin(NULL),
					m_pContacts(NULL),
					m_pMsgMgr(NULL),
					m_pChatFrame(NULL),
					m_pGroupFrame(NULL),
					m_pConfigure(NULL),
					m_pTrayMsg(NULL),
					m_pMainFrame(NULL),
					m_pAuthSocket(NULL),
					m_hLogoIcon(NULL),
					m_bLogon(FALSE),
					m_hMutex(NULL),
					m_RoleId(-1),
					m_OfflineDid(FALSE),
					m_bLoginSucc(FALSE),
					m_bIsOnline(FALSE)
{

}


CCoreFrameWorkImpl::~CCoreFrameWorkImpl(void)
{
	Clear();
}

void CCoreFrameWorkImpl::Clear()
{
	if (m_pAuthSocket)
		delete m_pAuthSocket;
	m_pAuthSocket = NULL;
	//other plugs
	std::map<CAnsiString_, IUnknown *>::iterator it;
	for (it = m_OtherPlugs.begin(); it != m_OtherPlugs.end(); it ++)
	{
		it->second->Release();
	}
	m_OtherPlugs.clear();
	if (m_pUIManager)
	{
		m_pUIManager->ClearOrderAllMessage();
	}
	if (m_pCoreLogin)
	{
		m_pCoreLogin->Release();
		m_pCoreLogin = NULL;
	}
	if (m_pContacts)
	{
		m_pContacts->Release();
		m_pContacts = NULL;
	}
	if (m_pMsgMgr)
	{
		m_pMsgMgr->Release();
		m_pMsgMgr = NULL;
	}
	if (m_pChatFrame)
	{
		m_pChatFrame->Release();
		m_pChatFrame = NULL;
	}
	if (m_pGroupFrame)
	{
		m_pGroupFrame->Release();
		m_pGroupFrame = NULL;
	}
	if (m_pConfigure)
	{
		m_pConfigure->Release();
		m_pConfigure = NULL;
	}
	if (m_pTrayMsg)
	{
		m_pTrayMsg->Release();
		m_pTrayMsg = NULL;
	}
	if (m_pMainFrame)
	{
		m_pMainFrame->Release();
		m_pMainFrame = NULL;
	}
	if (m_pUIManager)
	{
		m_pUIManager->Release();
		m_pUIManager = NULL;
	}
	if (m_hMutex)
	{
		::CloseHandle(m_hMutex);
		m_hMutex = NULL;
	}
}

STDMETHODIMP_(BOOL) CCoreFrameWorkImpl::GetIsOnline() 
{
	return m_bIsOnline;
}

STDMETHODIMP CCoreFrameWorkImpl::ClearPlugins()
{
	InitSafeSocket();
	m_EventList.Clear();
	m_ProtoList.Clear();
	Clear();
	return S_OK;
}
//#define PRINT_SEND_AND_RECV_PROTOCOL
//IProtoSocketNotify
BOOL CCoreFrameWorkImpl::OnRecvProtocol(const char *szBuf, const int nBufSize)
{
	TiXmlDocument xml;
	if (xml.Load(szBuf, nBufSize))
	{
#ifdef PRINT_SEND_AND_RECV_PROTOCOL 
		if (nBufSize > 4096)
		{
			FILE *fp = fopen("E:\\a.txt", "a+");
			if (fp)
			{
				fseek(fp, 0, SEEK_END);
				fwrite(szBuf, nBufSize, 1, fp);
				fprintf(fp, "\n");
				fclose(fp);
			}
		} else
		{
			char *pTmp = new char[nBufSize + 1];
		    memmove(pTmp, szBuf, nBufSize);
		    pTmp[nBufSize] = '\0';
			PRINTDEBUGLOG(dtInfo, "Recv Protocol:%s", pTmp);
		    delete []pTmp;
		}
#endif
		TiXmlElement *pNode = xml.FirstChildElement();
		if (pNode)
		{
			const char *pValue = pNode->Value();
			const char *pType = pNode->Attribute("type");
			if (pValue && pType)
			{
				BOOL bDid = FALSE;
				//sys protocol
				if (::stricmp(pValue, "sys") == 0)
				{
					bDid = DoSysProtocol(pType, pNode);
				} else if (::stricmp(pValue, "msg") == 0)
				{
					bDid = DoMessageProtocol(pType, pNode); 
				} else if (::stricmp(pValue, "grp") == 0)
				{
					if (::stricmp(pType, "msg") == 0)
						bDid = DoGroupMessage(pNode);
					//
				}

				//派发消息至插件
				if (!bDid)
					m_ProtoList.DoProtocol(pValue, pType, szBuf, nBufSize);
			} //end if (pValue && pType)
#ifdef PRINT_SEND_AND_RECV_PROTOCOL
			PRINTDEBUGLOG(dtInfo, "Protocol End");
#endif
			return TRUE;
		} //end if (pNode)
	} else
	{
		char *pTmp = new char[nBufSize + 1];
		memmove(pTmp, szBuf, nBufSize);
		pTmp[nBufSize] = '\0';
		PRINTDEBUGLOG(dtInfo, "error xml protocol:%s", pTmp);
		delete []pTmp;
	}//end if (xml.Load(szBuf, nBufSize)
	return FALSE;
}

STDMETHODIMP CCoreFrameWorkImpl::GetSvrParams(const char *szParamName, IAnsiString *szParamValue,
	          BOOL bRealTime)
{
	if (::stricmp("serverip", szParamName) == 0)
	{
		szParamValue->SetString(m_strAuthIp.c_str());
		return S_OK;
	} else if (::stricmp("logindomain", szParamName) == 0)
	{
		szParamValue->SetString(m_strDomain.c_str());
		return S_OK;
	} else
	{
		std::map<CAnsiString_, std::string>::iterator it = m_SvrParams.find(szParamName);
		if (it != m_SvrParams.end())
		{
			szParamValue->SetString(it->second.c_str());	
			return S_OK;
		} else
		{
			//从服务器上获取
			std::string strXml = "<sys type=\"getsrvar\" name=\"";
			strXml += szParamName;
			strXml += "\" />";
			if (m_pAuthSocket)
				m_pAuthSocket->SendRawData(strXml.c_str(), (int) strXml.size());
		}
	}
	return E_FAIL;
}

void CCoreFrameWorkImpl::OnSocketClose(CAsyncNetIoSocket *pSocket, const int nErrorNo)
{
	//
	if (pSocket == m_pAuthSocket)
	{
		m_bIsOnline = FALSE;
		//other plugs
		NotifyErrorAllPlugin(CORE_ERROR_SOCKET_CLOSED, "网络被断开");
	}
}

void CCoreFrameWorkImpl::NotifyErrorAllPlugin(int nErrorNode, const char *szMsg)
{
	std::map<CAnsiString_, IUnknown *>::iterator it;
	for (it = m_OtherPlugs.begin(); it != m_OtherPlugs.end(); it ++)
	{
		NotifyErrorEvent(it->second, nErrorNode, szMsg);
	}  
	NotifyErrorEvent(m_pCoreLogin, nErrorNode, szMsg); 
	NotifyErrorEvent(m_pUIManager, nErrorNode, szMsg);
	NotifyErrorEvent(m_pMainFrame, nErrorNode, szMsg);
	NotifyErrorEvent(m_pTrayMsg, nErrorNode, szMsg);
	NotifyErrorEvent(m_pConfigure, nErrorNode, szMsg);
	NotifyErrorEvent(m_pGroupFrame, nErrorNode, szMsg);
	NotifyErrorEvent(m_pChatFrame, nErrorNode, szMsg);
	NotifyErrorEvent(m_pMsgMgr, nErrorNode, szMsg);
	NotifyErrorEvent(m_pContacts, nErrorNode, szMsg);
}

void BroadcastCoreEvent(IUnknown *pUnknown, const char *szFromWndName, HWND hFromWnd,
	                    const char *szType, const char *szContent, void *pData)
{
	ICoreEvent *pEvent = NULL;
	if (pUnknown && SUCCEEDED(pUnknown->QueryInterface(__uuidof(ICoreEvent), (void **)&pEvent)))
	{
		pEvent->DoBroadcastMessage(szFromWndName, hFromWnd, szType, szContent, pData);
		pEvent->Release();
	}
}

	//广播消息
STDMETHODIMP CCoreFrameWorkImpl::BroadcastMessage(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData)
{
	if ((::stricmp(szFromWndName, "mainwindow") == 0) && (::stricmp(szType, "showcontacts") == 0)
		&& (::stricmp(pContent, "complete") == 0))
	{
		m_bIsOnline = TRUE; 
	}
	std::map<CAnsiString_, IUnknown *>::iterator it;
	for (it = m_OtherPlugs.begin(); it != m_OtherPlugs.end(); it ++)
	{
		BroadcastCoreEvent(it->second, szFromWndName, hFromWnd, szType, pContent, pData);
	}  
	BroadcastCoreEvent(m_pCoreLogin, szFromWndName, hFromWnd, szType, pContent, pData);
	BroadcastCoreEvent(m_pUIManager, szFromWndName, hFromWnd, szType, pContent, pData);
	BroadcastCoreEvent(m_pMainFrame, szFromWndName, hFromWnd, szType, pContent, pData);
	BroadcastCoreEvent(m_pTrayMsg, szFromWndName, hFromWnd, szType, pContent, pData);
	BroadcastCoreEvent(m_pConfigure, szFromWndName, hFromWnd, szType, pContent, pData);
	BroadcastCoreEvent(m_pGroupFrame, szFromWndName, hFromWnd, szType, pContent, pData);
	BroadcastCoreEvent(m_pChatFrame, szFromWndName, hFromWnd, szType, pContent, pData);
	BroadcastCoreEvent(m_pMsgMgr, szFromWndName, hFromWnd, szType, pContent, pData);
	BroadcastCoreEvent(m_pContacts, szFromWndName, hFromWnd, szType, pContent, pData);
	return S_OK;
}

void CCoreFrameWorkImpl::NotifyErrorEvent(IUnknown *pUnknown, int nErrorNo, const char *szMsg)
{
	ICoreEvent *pEvent = NULL;
	if (pUnknown && SUCCEEDED(pUnknown->QueryInterface(__uuidof(ICoreEvent), (void **)&pEvent)))
	{
		pEvent->CoreFrameWorkError(nErrorNo, szMsg);
		pEvent->Release();
	}
}

void CCoreFrameWorkImpl::OnSocketConnect(CAsyncNetIoSocket *pSocket, const int nErrorNo)
{
	//
	if ((pSocket == m_pAuthSocket) && (nErrorNo == 0))
	{
		//save auth servers
		char szIp[16] = {0};
		UINT uPort = 0;
		if (pSocket->GetPeerName(szIp, uPort))
		{
			m_strAuthIp = szIp;
			m_uAuthPort = (USHORT) uPort;
		} 
		BroadcastMessage("CoreFrame", NULL, "connectsvr", "succ", NULL);
	} else
	{
		BroadcastMessage("CoreFrame", NULL, "connectsvr", "failed", NULL); 
	}
}

//IUnknown
STDMETHODIMP CCoreFrameWorkImpl::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(ICoreFrameWork)))
	{
		*ppv = (ICoreFrameWork *) this;
		_AddRef();
		return S_OK;
	} else if (IsEqualIID(riid, __uuidof(IProtocolParser)))
	{
		*ppv = (IProtocolParser *) this;
		_AddRef();
		return S_OK;
	} else if (IsEqualIID(riid, __uuidof(IUIManager)))
	{
		if (m_pUIManager)
		{
			m_pUIManager->AddRef();
			*ppv = m_pUIManager;
			return S_OK;
		} else
			return E_FAIL;
	} else if (IsEqualIID(riid, __uuidof(ICoreLogin)))
	{
		if (m_pCoreLogin)
		{
			m_pCoreLogin->AddRef();
			*ppv = m_pCoreLogin;
			return S_OK;
		} else
			return E_FAIL;
	} else if (IsEqualIID(riid, __uuidof(IContacts)))
	{
		if (m_pContacts)
		{
			m_pContacts->AddRef();
			*ppv = m_pContacts;
			return S_OK;
		} else
			return E_FAIL;
	} else if (IsEqualIID(riid, __uuidof(IMsgMgr)))
	{
		if (m_pMsgMgr)
		{
			m_pMsgMgr->AddRef();
			*ppv = m_pMsgMgr;
			return S_OK;
		} else
			return E_FAIL;
	} else if (IsEqualIID(riid, __uuidof(IChatFrame)))
	{
		if (m_pChatFrame)
		{
			m_pChatFrame->AddRef();
			*ppv = m_pChatFrame;
			return S_OK;
		} else
			return E_FAIL;
	} else if (IsEqualIID(riid, __uuidof(IGroupFrame)))
	{
		if (m_pGroupFrame)
		{
			m_pGroupFrame->AddRef();
			*ppv = m_pGroupFrame;
			return S_OK;
		} else
			return E_FAIL;
	} else if (IsEqualIID(riid, __uuidof(IConfigure)))
	{
		if (m_pConfigure)
		{
			m_pConfigure->AddRef();
			*ppv = m_pConfigure;
			return S_OK;
		} else
			return E_FAIL;
	} else if (IsEqualIID(riid, __uuidof(ITrayMsg)))
	{
		if (m_pTrayMsg)
		{
			m_pTrayMsg->AddRef();
			*ppv = m_pTrayMsg;
			return S_OK;
		} else
			return E_FAIL;
	} else if (IsEqualIID(riid, __uuidof(IMainFrame)))
	{
		if (m_pMainFrame)
		{
			m_pMainFrame->AddRef();
			*ppv = m_pMainFrame;
			return S_OK;
		} else
			return E_FAIL;
	} else 
	{
		char szCLSID[MAX_PATH] = {0};
		LPOLESTR pOleStr = NULL;
		HRESULT hr = ::StringFromCLSID(riid, &pOleStr);
		if(FAILED(hr))
			return FALSE;
		CStringConversion::WideCharToString(pOleStr, szCLSID, MAX_PATH -1);
		std::map<CAnsiString_, IUnknown *>::iterator it = m_OtherPlugs.find(szCLSID);
		if (it != m_OtherPlugs.end())
		{
			return it->second->QueryInterface(riid, ppv);
		} else
			return E_NOINTERFACE; 
	}
}


STDMETHODIMP CCoreFrameWorkImpl::ShowTrayTipInfo(const TCHAR *szImageFile, const TCHAR *szTip, const TCHAR *szUrl,
		                        const TCHAR *szCaption, ICoreEvent *pEvent)
{
	if (m_pMainFrame)
	{
		LPTRAY_ICON_TIP_INFO pInfo = new TRAY_ICON_TIP_INFO();
		memset(pInfo, 0, sizeof(TRAY_ICON_TIP_INFO));
		if (szImageFile)
		{
			pInfo->szImageFile = new TCHAR[::lstrlen(szImageFile) + 1];
			lstrcpy(pInfo->szImageFile, szImageFile);
			pInfo->szImageFile[::lstrlen(szImageFile)] = L'\0';
		}
		if (szTip)
		{
			pInfo->szTip = new TCHAR[::lstrlen(szTip) + 1];
			lstrcpy(pInfo->szTip, szTip);
			pInfo->szTip[::lstrlen(szTip)] = L'\0';
		}
		if (szUrl)
		{
			pInfo->szUrl = new TCHAR[::lstrlen(szUrl) + 1];
			lstrcpy(pInfo->szUrl, szUrl);
			pInfo->szUrl[::lstrlen(szUrl)] = L'\0';
		}
		if (szCaption)
		{
			pInfo->szCaption = new TCHAR[::lstrlen(szCaption) + 1];
			lstrcpy(pInfo->szCaption, szCaption);
			pInfo->szCaption[::lstrlen(szCaption)] = L'\0';
		}

		//pInfo->
		::PostMessage(m_pMainFrame->GetSafeWnd(), WM_SHOWTRAYTIPINFO, WPARAM(pInfo), 0);

        //do message free
		return S_OK;
	}
	return E_FAIL;
}

//ICoreFrameWork
STDMETHODIMP CCoreFrameWorkImpl::SetAgent(const char *szType, const char *szAddress, USHORT uPort, 
                 const char *szUserName, const char *szUserPwd)
{
	return E_NOTIMPL;
}

//
STDMETHODIMP CCoreFrameWorkImpl::ChangePresence(const char *szPresence, const char *szMemo) 
{
	if (!m_strPresence.empty() && (m_bIsOnline))
	{
		if (!szPresence || (::stricmp(szPresence, m_strPresence.c_str()) == 0))
			return S_FALSE;
	}
	if (szPresence)
		m_strPresence = szPresence;
	if (szMemo)
		m_strPresenceMemo = szMemo; 
	if (stricmp(m_strPresence.c_str(), "offline") == 0)
	{
		if (m_pAuthSocket)
		{
			m_bIsOnline = FALSE;
			m_pAuthSocket->Close();
			NotifyErrorAllPlugin(CORE_ERROR_SOCKET_CLOSED, NULL);
			return S_OK;
		}
	}
	//send status
	if (m_pAuthSocket && m_pAuthSocket->IsConnected())
	{
		std::string strXml = "<sys type=\"presence\" uid=\"";
		strXml += m_strUserName;
		strXml += "@";
		strXml += m_strDomain;
		strXml += "\" presence=\"";
		strXml += m_strPresence;
		strXml += "\" memo=\"";
		strXml += m_strPresenceMemo;
		strXml += "\"/>";		
		m_pAuthSocket->SendRawData(strXml.c_str(), (int) strXml.size());
		return S_OK;
	} else if (!m_bIsOnline)
	{
		if (m_bLoginSucc && m_pCoreLogin)
		{
			return m_pCoreLogin->LogonSvr(NULL, NULL, NULL, NULL, NULL);
		}
	}
	return E_FAIL;
}

//send raw protocol
STDMETHODIMP CCoreFrameWorkImpl::SendRawMessage(const BYTE *pData, const LONG lSize, const LONG lStyle)
{
	if (m_pAuthSocket)
	{
		if (m_pAuthSocket->SendRawData((char *)pData, lSize))
		{
#ifdef PRINT_SEND_AND_RECV_PROTOCOL
			PRINTDEBUGLOG(dtInfo, "Send Protocol,size:%d data:%s", lSize, (char *)pData); 
#endif
			return S_OK;
		}
	}
	return E_FAIL;
}

//user information
STDMETHODIMP CCoreFrameWorkImpl::GetUserName(IAnsiString *szUserName)
{
	if (!szUserName)
		return E_POINTER;
	szUserName->SetString(m_strUserName.c_str()); 
	return S_OK;
}

STDMETHODIMP CCoreFrameWorkImpl::GetUserDomain(IAnsiString *szDomain)
{
	if (!szDomain)
		return E_POINTER;
	if (!m_strDomain.empty())
	{
		szDomain->SetString(m_strDomain.c_str());
		return S_OK;
	} else
	{
		if (m_pConfigure)
		{
			CInterfaceAnsiString strDomain;
			if (FAILED(m_pConfigure->GetParamValue(TRUE, "Server", "domain",  szDomain)))
				if (FAILED(m_pConfigure->GetParamValue(TRUE, "login", "domain", szDomain)))
					return E_FAIL;
		}
	}
	return S_OK;
}

//权限相关 是否有权限
STDMETHODIMP CCoreFrameWorkImpl::CanAllowAction(int nAction)
{
	if (m_RoleId < 0)
	{
		m_RoleList.clear();
		if (m_pContacts)
		{
			CInterfaceAnsiString strRole;
			if (SUCCEEDED(m_pContacts->GetRoleList(m_strUserName.c_str(), (IAnsiString *)&strRole)))
			{
				m_RoleId = 0;
				if (strRole.GetSize() > 0)
				{
					m_RoleList = strRole.GetData();
				} //end if (strRole.
			} //end if (SUCCEEDED(
		} //end if (m_pContacts)
	} //end if (m_RoleId < 0)
	if (m_RoleList.size() > 0)
	{ 
		if ((nAction & USER_ROLE_SEND_MESSAGE) > 0)
		{
			if (m_RoleList.find("admin/client/send_message") != std::string::npos)
			{
				return S_OK;
			}
		} else if ((nAction & USER_ROLE_SEND_FILE) > 0)
		{
			if (m_RoleList.find("admin/client/send_file") != std::string::npos)
			{
				return S_OK;
			}
		} else if ((nAction & USER_ROLE_SEND_SMS) > 0)
		{
			if (m_RoleList.find("admin/client/sms_send") != std::string::npos)
			{
				return S_OK;
			}
		} else if ((nAction & USER_ROLE_GROUP) > 0)
		{
			if (m_RoleList.find("admin/client/create_group") != std::string::npos)
			{
				return S_OK;
			}
		}  
	} else
	{
		return S_OK;
	}
	return S_FALSE;
}

//
STDMETHODIMP CCoreFrameWorkImpl::GetUserNickName(IAnsiString *szUTF8Name)
{
	if (!szUTF8Name)
		return E_POINTER;
	if (m_strNickName.empty())
	{
		if (m_pContacts)
		{
			CInterfaceAnsiString strName;
			if (SUCCEEDED(m_pContacts->GetRealNameById(m_strUserName.c_str(), 
				                         m_strDomain.c_str(), (IAnsiString *)&strName)))
			{
				m_strNickName = strName.GetData();
			} //end if (SUCCEEDED(m_pContacts->...
		} //end if (m_pContacts)
	} //end if (m_strNickName.empty())
	if (!m_strNickName.empty())
	{
		szUTF8Name->SetString(m_strNickName.c_str());
		return S_OK;
	}
	return E_FAIL;
}

//
STDMETHODIMP CCoreFrameWorkImpl::GetUserPassword(IAnsiString *szUserPwd)
{
	if (!szUserPwd)
		return E_POINTER;
	szUserPwd->SetString(m_strUserPwd.c_str());
	return S_OK;
}

//
STDMETHODIMP CCoreFrameWorkImpl::GetUserInGroup(IAnsiString *szGroupId, IAnsiString *szUTF8GrpName)
{
	return E_NOTIMPL;
}

STDMETHODIMP CCoreFrameWorkImpl::GetOfflineMsg()
{
	//if (m_bIsOnline)
	//{  
		if (m_pGroupFrame)
		{
			std::string strXml = "<grp type=\"getgroup\" uid=\"";
			strXml += m_strUserName;
			strXml += "@";
			strXml += m_strDomain;
			strXml += "\"/>";
			SendRawMessage((BYTE *)strXml.c_str(), (LONG)strXml.size(), 0); 
		}
		static char STRING_GET_OFFLINE_MSG_XML[] = "<msg type=\"getofflinemsg\"/>";
		return SendRawMessage((BYTE *)STRING_GET_OFFLINE_MSG_XML, (int) ::strlen(STRING_GET_OFFLINE_MSG_XML), 0); 
	//}
	//return E_FAIL;
}

STDMETHODIMP CCoreFrameWorkImpl::GetPresence(const char *szUserName, IAnsiString *strPresence, IAnsiString *strPresenceMemo)
{
	if ((!szUserName) || (::stricmp(szUserName, m_strUserName.c_str()) == 0))
	{
		strPresence->SetString(m_strPresence.c_str());
		strPresenceMemo->SetString(m_strPresenceMemo.c_str());
		return S_OK;
	} else 
	{
		if (m_pContacts)
		{
			CInstantUserInfo Info;
			if (SUCCEEDED(m_pContacts->GetContactUserInfo(szUserName, &Info)))
			{
				return Info.GetUserStatus(strPresence);
			}
		} //end if (m_pContacts)
	} //end else if ((!szUserName)
	return E_FAIL;
}

//获取待处理消息简要信息
STDMETHODIMP CCoreFrameWorkImpl::GetUserPendMsgTipList(IUserPendMessageTipList *pList)
{
	if (m_PendingList.GetMessageTipList(pList))
		return S_OK;
	return E_FAIL;
}

STDMETHODIMP CCoreFrameWorkImpl::GetFrontPendingMsg(const char *szUserName, const char *szType,
	                                     IAnsiString *strProtocol, BOOL bPop)
{
	if (m_PendingList.GetFrontProtocolByName(szUserName, szType, strProtocol, bPop))
	{
		if (bPop)
		{
			CInterfaceAnsiString strFromName, strType;
			if (SUCCEEDED(GetLastPendingMsg(&strFromName, &strType)))
			{
				StartTrayIcon(strType.GetData(), strType.GetData(), NULL);
			} else
			{
				StartTrayIcon(NULL, NULL, NULL);
			} //end else if (GetLastPendingMsg(..
		} //end if (bPop)
		return S_OK;
	} else
	{
		StartTrayIcon(NULL, NULL, NULL);
	}
	return E_FAIL;
}

STDMETHODIMP CCoreFrameWorkImpl::GetLastPendingMsg(IAnsiString *strFromName, IAnsiString *strType)
{
	if (m_PendingList.GetLastProtocol(strFromName, strType))
		return S_OK;
	return E_FAIL;
}

BOOL CCoreFrameWorkImpl::GetInterfaceById(CPluginList &Plugins, int nPluginId, const IID &riid, IUnknown **ppvObject)
{
	HRESULT hr;
	std::vector<LPAI_PLUGIN_ITEM> plgs;
	if (Plugins.GetPluginListByType(nPluginId, plgs))
	{
		//
		if (plgs.size() > 0)
		{
			LPAI_PLUGIN_ITEM strGuid = plgs.back();
			if (*ppvObject)
			{
				(*ppvObject)->Release();
				(*ppvObject) = NULL;
			}
			IID iid = {0};
			TCHAR wGuid[64] = {0};
			CStringConversion::StringToWideChar(strGuid->strPluginGuid.c_str(), wGuid, 63);
			hr = ::IIDFromString(wGuid, &iid);
			if (SUCCEEDED(hr))
			{
				hr = ::CoCreateInstance(iid, NULL, CLSCTX_INPROC_SERVER, riid,	(void **)ppvObject);
				if (SUCCEEDED(hr))  
				{
					//读取配置
					ICoreEvent *pCore = NULL;
					hr = (*ppvObject)->QueryInterface(__uuidof(ICoreEvent), (void **)&pCore);
					if (SUCCEEDED(hr) && pCore)
					{
						pCore->SetCoreFrameWork((ICoreFrameWork *)this);
						//GetEventList
						GetSkinXmlInst(pCore);
						pCore->Release();
						pCore = NULL;
					}
					return TRUE;
				} else
				{
					(*ppvObject) = NULL;
					PRINTDEBUGLOG(dtInfo, "Create Interface Failed, GUID:%s", strGuid->strPluginGuid.c_str());
				}
			} else
			{
				PRINTDEBUGLOG(dtInfo, "interface IIDFromString Failed:%s", strGuid->strPluginGuid.c_str());
			} //end if (SUCCEEDED(hr))
			delete strGuid;
		} //end if (plgs.size() > 0)
	} //if (plg.GetPluginListByType(PLUGIN_TYPE_UIMANAGER, plgs))
	return FALSE;
}

BOOL CCoreFrameWorkImpl::GetSkinXmlInst(ICoreEvent *pEvent)
{
	if (m_pUIManager)
	{
		//get xml
		CInterfaceAnsiString strXml;
 
		if (pEvent->GetSkinXmlString((IAnsiString *)&strXml) == S_OK)
		{
			m_pUIManager->AddPluginSkin(strXml.GetData());
		} 
	}
	return TRUE;
}

//
STDMETHODIMP CCoreFrameWorkImpl::AddOrderProtocol(IProtocolParser *pOrder, const char *szProtoName, const char *szProtoType)
{
	//
	if (m_ProtoList.AddOrderProto(pOrder, szProtoName, szProtoType))
		return S_OK;
	else
		return E_FAIL;
}

STDMETHODIMP CCoreFrameWorkImpl::DelOrderProtocol(IProtocolParser *pOrder, const char *szProtoName, const char *szProtoType)
{
	if (m_ProtoList.DeleteOrderProto(pOrder, szProtoName, szProtoType))
		return S_OK;
	else
		return E_FAIL;
}

//
STDMETHODIMP CCoreFrameWorkImpl::AddOrderEvent(ICoreEvent *pEvent, const char *szWndName, const char *szCtrlName, 
	                      const char *szEventType)
{
	if (m_EventList.AddEvent(szWndName, szCtrlName, szEventType, pEvent))
		return S_OK;
	else
		return E_FAIL;
}

STDMETHODIMP CCoreFrameWorkImpl::DeleteOrderEvent(ICoreEvent *pEvent, const char *szWndName, const char *szCtrlName, 
		                      const char *szEventType)
{
	if (m_EventList.DeleteEvent(szWndName, szCtrlName, szEventType, pEvent))
		return S_OK;
	return E_FAIL;
}

BOOL CCoreFrameWorkImpl::GetEventListByPlgId(CPluginList &Plugins, int nPluginId)
{
	HRESULT hr;
	std::vector<LPAI_PLUGIN_ITEM> plgs;
	if (Plugins.GetPluginListByType(nPluginId, plgs))
	{
		std::vector<LPAI_PLUGIN_ITEM>::iterator plgit;
		for (plgit = plgs.begin(); plgit != plgs.end(); plgit ++)
		{
			IID iid = {0};
			TCHAR wGuid[64] = {0};
			CStringConversion::StringToWideChar((*plgit)->strPluginGuid.c_str(), wGuid, 63);
			hr = ::IIDFromString(wGuid, &iid);
			if (SUCCEEDED(hr))
			{
				IUnknown *pObject = NULL;
				hr = ::CoCreateInstance(iid, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void **)&pObject);
				if (SUCCEEDED(hr) && pObject)
				{
					ICoreEvent *pEvent = NULL;
					if (SUCCEEDED(pObject->QueryInterface(__uuidof(ICoreEvent), (void **)&pEvent)))
					{
						//set core object
						pEvent->SetCoreFrameWork((ICoreFrameWork *)this);
						//get skin
						GetSkinXmlInst(pEvent);
						pEvent->Release();
					}
					if ((*plgit)->strPluginGuid.c_str())
						m_OtherPlugs.insert(std::pair<CAnsiString_, IUnknown *>((*plgit)->strInterfaceGuid.c_str(), pObject));
					//do not release pEvent
				}  else
				{
					PRINTDEBUGLOG(dtInfo, "Get Protocol Plugin Failed, iid;%s", (*plgit)->strPluginGuid.c_str());
				} //end else if (SUCCEEDED(hr))
			} else 
			{
				PRINTDEBUGLOG(dtInfo, "IIDFromString Failed, Source IID:%s", (*plgit)->strPluginGuid.c_str());
			} //end if (SUCCEEDED(hr))
			delete (*plgit);
		} //end for (plgit = plgs.begin()...
	} //end if (plg.GetPluginListByType
	return TRUE;
}

STDMETHODIMP CCoreFrameWorkImpl::PickPendingMessage()
{
	CInterfaceAnsiString strFrom, strType;
	if (SUCCEEDED(GetLastPendingMsg(&strFrom, &strType)))
	{
		if (::stricmp(strType.GetData(), "p2p") == 0)
		{
			if (m_pChatFrame)
			{
				if (m_pChatFrame->ShowChatFrame(NULL, strFrom.GetData(), NULL) != NULL)
					return S_OK;
			} //end if (m_pChatFrame)
		} else if (::stricmp(strType.GetData(), "grp") == 0)
		{
			if (m_pGroupFrame)
			{
				if (m_pGroupFrame->ShowGroupFrame(strFrom.GetData(), NULL))
					return S_OK;
			} //end if (m_pGroupFrame->ShowGroupFrame(
		}//end if (::stricmp(strType.GetData()..
	} //end if (SUCCEEDED(
	return E_FAIL;
}

//
STDMETHODIMP CCoreFrameWorkImpl::InitPlugins(HINSTANCE hInstace)
{
	CPluginList plg;
	if (plg.LoadPluginsFromReg(PLUGIN_REGISTER_DIR))
	{
		//m_pUIManager
		GetInterfaceById(plg, PLUGIN_TYPE_CONFIGURE, __uuidof(IConfigure), (IUnknown **)&m_pConfigure);
		if (m_pConfigure && (!m_CfgFileName.empty()))
			m_pConfigure->InitCfgFileName(m_CfgFileName.c_str(), "", TRUE);
		GetInterfaceById(plg, PLUGIN_TYPE_UIMANAGER, __uuidof(IUIManager), (IUnknown **)&m_pUIManager);
		if (m_pUIManager)
		{
			CInterfaceAnsiString strSkinPath, strXml;
			char szFileName[MAX_PATH] = {0};
			if (SUCCEEDED(m_pConfigure->GetPath(PATH_LOCAL_SKIN, &strSkinPath))
				&& SUCCEEDED(m_pConfigure->GetSkinXml("default.xml", &strXml)))
			{
				if (FAILED(m_pUIManager->CreateSkinByXmlStream(strXml.GetData(), 
					strXml.GetSize(), strSkinPath.GetData())))
					return E_FAIL;
			} else
			{
			    PRINTDEBUGLOG(dtInfo, "Skin File Error:%s", szFileName);
				return E_FAIL;
			}
		}
		GetInterfaceById(plg, PLUGIN_TYPE_LOGINFRAME, __uuidof(ICoreLogin), (IUnknown **)&m_pCoreLogin);
		GetInterfaceById(plg, PLUGIN_TYPE_MAINFRAME, __uuidof(IMainFrame), (IUnknown **)&m_pMainFrame);
		GetInterfaceById(plg, PLUGIN_TYPE_CONTACTS, __uuidof(IContacts), (IUnknown **)&m_pContacts); 
		GetInterfaceById(plg, PLUGIN_TYPE_MSGMGR, __uuidof(IMsgMgr), (IUnknown **)&m_pMsgMgr);
		GetInterfaceById(plg, PLUGIN_TYPE_CHATFRAME, __uuidof(IChatFrame), (IUnknown **)&m_pChatFrame);
		GetInterfaceById(plg, PLUGIN_TYPE_GROUPFRAME, __uuidof(IGroupFrame), (IUnknown **)&m_pGroupFrame); 		
		GetInterfaceById(plg, PLUGIN_TYPE_TRAYMSG, __uuidof(ITrayMsg), (IUnknown **)&m_pTrayMsg);
		if (m_pTrayMsg)
		{ 
			m_pTrayMsg->InitTrayMsg(hInstace, NULL, "CoLine 未登陆");
			m_pTrayMsg->ShowTrayIcon();
		}
		m_hLogoIcon = ::LoadIcon(hInstace, L"LOGO");
		//Get Protocol Plugins
		GetEventListByPlgId(plg, PLUGIN_TYPE_PROTOCOL);
		GetEventListByPlgId(plg, PLUGIN_TYPE_EXTERNAL);
		COLORREF cr = 0;//RGB(62,123,123);
		if (m_pConfigure)
		{			
			CInterfaceAnsiString strClr;
			if (SUCCEEDED(m_pConfigure->GetParamValue(TRUE, "Skin", "background", (IAnsiString *)&strClr)))
			{
				cr = ::atoi(strClr.GetData());
			} //end if (m_pConfigure->GetParamValue(..
		} //end if (m_pConfigure)
		if (m_pUIManager)
		{
			m_pUIManager->BlendSkinStyle(cr);
		} //end if (m_pUIManager)
		//初始化主窗口
		if (m_pMainFrame)
			return m_pMainFrame->InitMainFrame(); 
	}
	return E_FAIL;
}

STDMETHODIMP CCoreFrameWorkImpl::Logout()
{
	Offline();
	m_bLoginSucc = FALSE;
	m_bLogon = FALSE; 
	std::string strTmp = m_strUserName;
	m_strUserName.clear();
	m_strUserPwd.clear();
	m_strPresence.clear();
	m_strPresenceMemo.clear();
	m_strNickName.clear();
	m_strDomain.clear();
	m_SvrParams.clear();
	m_RoleList = -1;
	//m_PendingList.Clear();
	NotifyErrorAllPlugin(CORE_ERROR_LOGOUT, NULL);
	if (m_pMainFrame)
		::ShowWindow(m_pMainFrame->GetSafeWnd(), SW_HIDE);
	if (m_pCoreLogin)
		m_pCoreLogin->ShowLogonWindow(strTmp.c_str(), NULL, FALSE);
	return S_OK;
}

STDMETHODIMP CCoreFrameWorkImpl::Offline()
{
	if (m_pAuthSocket)
		m_pAuthSocket->Close();
	m_bIsOnline = FALSE;
	return S_OK;
}

STDMETHODIMP CCoreFrameWorkImpl::InitSafeSocket()
{
	if (m_pAuthSocket)
		delete m_pAuthSocket;
	m_pAuthSocket = NULL;
	return S_OK;
}

//建立安全连接
STDMETHODIMP CCoreFrameWorkImpl::EstablishSafeSocket(const char *szSvrHost, USHORT uPort)
{
	if (m_pAuthSocket)
		delete m_pAuthSocket;
	m_pAuthSocket = NULL;
	m_pAuthSocket = new CProtoInterfaceSocket(this);
	if (m_pAuthSocket->Connect(szSvrHost, uPort))
		return S_OK;
	else
	{
		BroadcastMessage("CoreFrame", NULL, "connectsvr", "failed", NULL);
	}
	return E_FAIL;
}

BOOL CCoreFrameWorkImpl::SendAuthData()
{
	//<sys type="login" username="liwenfang" password="123" token="" domain="GoCom"/>
	if (m_pAuthSocket && m_pAuthSocket->IsConnected())
	{		
		m_strDomain.clear();
		if (m_pConfigure)
		{
			CInterfaceAnsiString strDomain;
			if (SUCCEEDED(m_pConfigure->GetParamValue(TRUE, "Server", "domain", (IAnsiString *)&strDomain)))
				m_strDomain = strDomain.GetData();
			else if (SUCCEEDED(m_pConfigure->GetParamValue(TRUE, "login", "domain", (IAnsiString *)&strDomain)))
				m_strDomain = strDomain.GetData();
		}
		if (m_strUserName.empty() || m_strUserPwd.empty() || m_strDomain.empty())
		{
			return FALSE;
		} else
		{
			if (RegisterMutex())
			{
				m_bLogon = TRUE;
				std::string strXml = "<sys type=\"login\" username=\"";
				strXml += m_strUserName;
				strXml += "\" password=\"";
				strXml += m_strUserPwd;
				strXml += "\" token=\"\" domain=\"";
				strXml += m_strDomain;
				strXml += "\"/>";
				return m_pAuthSocket->SendRawData(strXml.c_str(), (int) strXml.size());
			}
		} //end else if (m_strUserName.empty()..		
	}// end if (m_pAuthSocket && m_pAuthSocket->..
	return TRUE;
}

//认证用户
STDMETHODIMP CCoreFrameWorkImpl::AuthUser(const char *szUserName, const char *szUserPwd, 
	                     const char *szPresence, const char *szPresenceMemo)
{
	m_bIsOnline = FALSE;
	if (szUserName)
		m_strUserName = szUserName;
	if (szUserPwd)
		m_strUserPwd = szUserPwd;
	if (szPresence)
		m_strPresence = szPresence;
	if (szPresenceMemo)
		m_strPresenceMemo = szPresenceMemo;
 	SendAuthData();  
	return S_OK;
}

//
STDMETHODIMP CCoreFrameWorkImpl::StartTrayIcon(const char *szMsgType, const char *szTip, HICON hIcon)
{
	if (m_pTrayMsg)
	{
		HICON h = hIcon;
		if (h == NULL)
		{
			std::map<CAnsiString_, HICON >::iterator it = m_hAniIconList.find(szMsgType);	

			if (it != m_hAniIconList.end())
			{
				h = it->second;
			} else
				h = m_hLogoIcon;
		}
		m_pTrayMsg->StopAnimate();
		if (h && szMsgType)
		{			
			m_pTrayMsg->AddAnimateIcon(NULL);
			m_pTrayMsg->AddAnimateIcon(h);
			m_pTrayMsg->StartAnimate(szTip);
		} else
			m_pTrayMsg->ShowTrayIcon();
		return S_OK; 
	} //end if (m_pTrayMsg)
	return E_FAIL;
}

//
STDMETHODIMP CCoreFrameWorkImpl::AddTrayMsgTypeIcon(const char *szMsgType, HICON hIcon)
{
	std::map<CAnsiString_, HICON >::iterator it = m_hAniIconList.find(szMsgType);
	if (it == m_hAniIconList.end())
	{
		m_hAniIconList.insert(std::pair<CAnsiString_, HICON >(szMsgType, hIcon));
		return S_OK;
	}
	return E_FAIL;
}

//
STDMETHODIMP CCoreFrameWorkImpl::DoCoreEvent(HWND hWnd, const char *szWndName, const char *szType, const char *szName,
	             WPARAM wParam, LPARAM lParam, HRESULT *hResult)
{
	return m_EventList.DoEvent(hWnd, szWndName, szName, szType, wParam, lParam, hResult);
}

//
STDMETHODIMP CCoreFrameWorkImpl::DoPresenceChanged(const char *szUserName, const char *szPresence, const char *szMemo, BOOL bOrder)
{
	//转到窗体消息
	return m_ProtoList.DoPresenceChanged(szUserName, szPresence, szMemo, bOrder);
}

//
STDMETHODIMP CCoreFrameWorkImpl::InitConfigure(const char *szCfgFileName)
{
	if (szCfgFileName)
	{
		m_CfgFileName = szCfgFileName;
		return S_OK;
	} else
		return E_FAIL;
}

//IProtocolParser
STDMETHODIMP CCoreFrameWorkImpl::DoRecvProtocol(const BYTE *pData, const LONG lSize)
{
	return E_NOTIMPL;
}

STDMETHODIMP CCoreFrameWorkImpl::DoPresenceChange(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder)
{
	return E_NOTIMPL;
}

#pragma warning(default:4996)
