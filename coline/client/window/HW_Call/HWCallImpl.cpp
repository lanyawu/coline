#include <Core/common.h>
#include <SmartSkin/smartskin.h>
#include "HWCallImpl.h"
#include "../IMCommonLib/InterfaceAnsiString.h"
#import "eSpace.tlb" rename_namespace("Huawei_eSpace") named_guids 

CHWCallImpl::CHWCallImpl(void):
             m_pCore(NULL)
{
}


CHWCallImpl::~CHWCallImpl(void)
{
	if (m_pCore)
		m_pCore->Release();
	m_pCore = NULL;
}

//IUnknown
STDMETHODIMP CHWCallImpl::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IHWCallImpl)))
	{
		*ppv = (IHWCallImpl *) this;
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

	//ICoreEvent
STDMETHODIMP CHWCallImpl::DoCoreEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
		                     LPARAM lParam, HRESULT *hResult)
{
	if (::stricmp(szType, "menucommand") == 0)
	{
		if (::stricmp(szName, "treeleaf") == 0)
		{
			switch(wParam)
			{
			case 50:
				 DoCallPhone(hWnd);
				 break;
			}
		}
	}
	return E_NOINTERFACE;
}
BOOL CHWCallImpl::GetCurrentUserTel(HWND hWnd, std::string &strTel)
{
	BOOL bSucc = FALSE;
	void *pSelNode = NULL;
	LPORG_TREE_NODE_DATA pSelData = NULL;
	TCHAR szName[MAX_PATH] = {0};
	int nSize = MAX_PATH - 1;
	CTreeNodeType tnType;
	if (::SkinGetSelectTreeNode(hWnd, L"colleaguetree", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
	{
		if (tnType == TREENODE_TYPE_LEAF)
		{
			IContacts *pContact = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
			{
				CInterfaceAnsiString strValue;
				if (SUCCEEDED(pContact->GetUserValueByParam(pSelData->szUserName, "workphone", &strValue)))
				{
					strTel = strValue.GetData();
					bSucc = TRUE;
				}
				pContact->Release();
			}
			 
		}
	} //end if (.
	return bSucc;
}

void CHWCallImpl::DoCallPhone(HWND hWnd)
{
	try 
	{  
		std::string strTel;
		if (GetCurrentUserTel(hWnd, strTel))
		{
			HANDLE tH = NULL;
			BOOL bPriv = FALSE; 
			TOKEN_PRIVILEGES tPriv = {0};
			if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &tH))
			{ 
				LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &(tPriv.Privileges[0].Luid));				
				tPriv.PrivilegeCount = 1;
				tPriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
				bPriv = AdjustTokenPrivileges(tH, FALSE, &tPriv, 0, NULL, NULL);
			}
			Huawei_eSpace::IECSFrameworkPtr ptr;
			HRESULT hr = ptr.CreateInstance(__uuidof(Huawei_eSpace::ECSFramework)); 
			if (SUCCEEDED(hr))
			{ 
				TCHAR szwNumber[MAX_PATH] = {0};
				CStringConversion::StringToWideChar(strTel.c_str(), szwNumber, MAX_PATH - 1);
				//成功
				//param:1-电话类型(语音呼叫为1)，2-暂不使用，3-eSpaceAccount(可以为空)，4-CalleeNum，5-Name(可以为空)
				ptr->Exec(1, 0, L"", szwNumber, L"");
			} else
			{
				ITrayMsg *pTray = NULL;
				if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(ITrayMsg), (void **)&pTray)))
				{
					LPVOID lpMsgBuf = NULL;
					::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, hr, 
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
					char szTmp[MAX_PATH] = {0};
					CStringConversion::WideCharToString((TCHAR *)lpMsgBuf, szTmp, MAX_PATH - 1);
					std::string strErrorMsg = "错误原因:";
					strErrorMsg += szTmp;
					::LocalFree(lpMsgBuf);
					pTray->ShowTipPanel("启动拔号程序失败", strErrorMsg.c_str());
					pTray->Release();
				} //
			} //end if (SUCCEEDED(hr)
			
			//
			if (bPriv)
			{
				tPriv.Privileges[0].Attributes = 0;
                AdjustTokenPrivileges(tH, FALSE, &tPriv, 0,(PTOKEN_PRIVILEGES) NULL, NULL);
			}
		} 
	}
	catch(_com_error& err) 
	{
		err.ErrorMessage(); 
	} 

}

STDMETHODIMP CHWCallImpl::SetCoreFrameWork(ICoreFrameWork *pCore)
{
	if (m_pCore)
		m_pCore->Release();
	m_pCore = pCore;
	if (m_pCore)
	{
		m_pCore->AddRef();
		//订制事件
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "treeleaf", "menucommand");
	}
	return S_OK;
}

STDMETHODIMP CHWCallImpl::GetSkinXmlString(IAnsiString *szXmlString)
{
	HRESULT hr = E_FAIL;
	IConfigure *pCfg = NULL; 
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{ 
		hr = pCfg->GetSkinXml("hwcall.xml",szXmlString); 
		pCfg->Release();
	}
	return hr;  
}

	//
STDMETHODIMP CHWCallImpl::CoreFrameWorkError(int nErrorNo, const char *szErrorMsg)
{
	return E_NOINTERFACE;
}

	//广播消息
STDMETHODIMP CHWCallImpl::DoBroadcastMessage(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData)
{
	return E_NOINTERFACE;
}

	//
STDMETHODIMP CHWCallImpl::DoWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes)
{
	return E_NOINTERFACE;
}
