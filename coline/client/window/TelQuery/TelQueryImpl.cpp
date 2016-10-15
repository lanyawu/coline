#include "TelQueryImpl.h"
#include <SmartSkin/smartskin.h>

#include "../IMCommonLib/InterfaceAnsiString.h"
#pragma warning(disable:4996)

CTelQueryImpl::CTelQueryImpl(void):
               m_pCore(NULL)
{
}


CTelQueryImpl::~CTelQueryImpl(void)
{
	if (m_pCore)
		m_pCore->Release();
	m_pCore = NULL;
}

//IUnknown
STDMETHODIMP CTelQueryImpl::QueryInterface(REFIID riid, LPVOID *ppv)

{
	*ppv = NULL;
	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(ITelQuery)))
	{
		*ppv = (ITelQuery *) this;
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

BOOL ParserTelQuery(const char *p, BOOL &bPeer, std::string &str2, std::string &strThree)
{
	std::string strQuery = p;
	int nPos = strQuery.find("电话");
	bPeer = FALSE;
	BOOL bSucc = FALSE;
	if (nPos != std::string::npos)
	{
		std::string strTmp = strQuery.substr(0, nPos);
		TCHAR *pTmp = new TCHAR[strTmp.size() + 1];
		memset(pTmp, 0, sizeof(TCHAR) * (strTmp.size() + 1));
		MultiByteToWideChar(::GetACP(), 0, strTmp.c_str(), -1, pTmp, strTmp.size());
		TCHAR szName3[4] = {0}, szName2[3] = {0}, szName1[2] = {0};
		szName1[0] = pTmp[::lstrlen(pTmp) - 1];
		if (::lstrcmpi(szName1, L"的") == 0)
			pTmp[::lstrlen(pTmp) - 1] = L'\0';
		szName1[0] = pTmp[::lstrlen(pTmp) - 1];
		if ((::lstrcmpi(szName1, L"你") == 0)
			|| (::lstrcmpi(szName1, L"您") == 0))
		{
			bPeer = TRUE; 
			bSucc = TRUE;
		} else
		{
			int nSize = ::lstrlen(pTmp);
			if (nSize >= 2)
			{
				szName2[0] = pTmp[nSize - 2];
				szName2[1] = pTmp[nSize - 1];
				char szTmp2[16] = {0};
				CStringConversion::WideCharToUTF8(szName2, szTmp2, 15);
				str2 = szTmp2;
				bSucc = TRUE;
			}
			if (nSize >= 3)
			{
				szName3[0] = pTmp[nSize - 3];
				szName3[1] = pTmp[nSize - 2];
				szName3[2] = pTmp[nSize - 1];
				char szTmp2[16] = {0};
				CStringConversion::WideCharToUTF8(szName2, szTmp2, 15);
				strThree = szTmp2;
			}			 
		}
		delete []pTmp;
	}
	return bSucc;
}

HRESULT CTelQueryImpl::DoSendP2PMsg(HWND hWnd, const char *pContent)
{
#define MAX_TELQUERY_INFO_SIZE  32  //某某的电话是多少？ 
	if (pContent && (strlen(pContent) < MAX_TELQUERY_INFO_SIZE))
	{
		std::string str2, strThree;
		BOOL bPeer = FALSE;
		if (ParserTelQuery(pContent, bPeer, str2, strThree))
		{
			IContacts *pContact = NULL;
			BOOL bSucc = FALSE;
			if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
			{
				IChatFrame *pFrame = NULL;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pFrame)))
				{
					CInterfaceAnsiString strPhone;
					CInterfaceAnsiString strRealName;
					if (bPeer)
					{						
						CInterfaceAnsiString strUserName;						  
						if (SUCCEEDED(pFrame->GetUserNameByHWND(hWnd, &strUserName)))
						{
							if (SUCCEEDED(pContact->GetPhoneByName(strUserName.GetData(), &strPhone)))
								bSucc = TRUE;
						}							
						 
					} else
					{
						if (SUCCEEDED(pContact->GetPhoneByRealName(str2.c_str(), strThree.c_str(), &strRealName, &strPhone)))
							bSucc = TRUE;
					}
					if (bSucc && strPhone.GetSize() > 0)
					{
						//show 
						TCHAR szTip[MAX_PATH] = {0};
						if (strRealName.GetSize() > 0)
							CStringConversion::UTF8ToWideChar(strRealName.GetData(), szTip, MAX_PATH - 1);
						TCHAR szTmp[128] = {0};
						CStringConversion::StringToWideChar(strPhone.GetData(), szTmp, 127);
						::lstrcat(szTip, L"电话号码:");
						::lstrcat(szTip, szTmp);
						pFrame->ShowChatTipMsg(hWnd, szTip);
					}
				    pFrame->Release();
				}
				pContact->Release();
			} //end if (pContact
		} //end if (ParserTelQuery 
	}
	return -1;
}

//ICoreEvent
STDMETHODIMP CTelQueryImpl::DoCoreEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
                             LPARAM lParam, HRESULT *hResult)
{	 
	return E_NOTIMPL;
}

//广播消息
STDMETHODIMP CTelQueryImpl::DoBroadcastMessage(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData)
{
	if (::stricmp(szType, "sendp2pmsg") == 0)
	{
		return DoSendP2PMsg(hFromWnd, pContent); 
	}
	return E_NOTIMPL;
}

STDMETHODIMP CTelQueryImpl::SetCoreFrameWork(ICoreFrameWork *pCore)
{
	if (m_pCore)
		m_pCore->Release();
	m_pCore = pCore;
	if (m_pCore)
	{
		m_pCore->AddRef();
		//
		 
	}
	return S_OK;
}

STDMETHODIMP CTelQueryImpl::GetSkinXmlString(IAnsiString *szXmlString)
{
	return E_NOTIMPL;
}

//
STDMETHODIMP CTelQueryImpl::CoreFrameWorkError(int nErrorNo, const char *szErrorMsg)
{
	return E_NOTIMPL;
}
//
STDMETHODIMP CTelQueryImpl::DoWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes)
{
	return E_NOTIMPL;
}

#pragma warning(default:4996)
