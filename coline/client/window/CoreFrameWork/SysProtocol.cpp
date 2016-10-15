#include <Commonlib/DebugLog.h>
#include <Commonlib/systemutils.h>
#include <Core/common.h>
#include "../IMCommonLib/InterfaceAnsiString.h"
#include "CoreFrameWorkImpl.h"

#pragma warning(disable:4996)

#define PERSON_CONFIGURE_FILENAME "\\person.cfg"
#define PERSON_MESSAGE_FILENAME "\\msg.db";

BOOL CCoreFrameWorkImpl::RegisterMutex()
{
	if (m_hMutex)
		::CloseHandle(m_hMutex);
	m_hMutex = NULL;
	TCHAR szTmp[MAX_PATH] = {0};
	CStdString_ strMutexName = L"CoLine-"; 
	CStringConversion::StringToWideChar(m_strUserName.c_str(), szTmp, MAX_PATH);
	strMutexName += szTmp;
	 
	m_hMutex = ::CreateMutex(NULL, FALSE, strMutexName);
	if (m_hMutex != NULL)
	{
		HWND h = NULL;
		if (m_pMainFrame)
			h = m_pMainFrame->GetSafeWnd();
		if (::GetLastError() == ERROR_ALREADY_EXISTS)
		{
			HWND hExists =  FindWindow(L"MainWindow", strMutexName);
			if (hExists)
			{
				CSystemUtils::BringToFront(hExists);
			}
			::PostMessage(h, WM_APP_TERMINATE, 0, 0);
			return FALSE;
		} else
			::SetWindowText(m_pMainFrame->GetSafeWnd(), strMutexName);
	}
	return TRUE;
}

//protocol
BOOL CCoreFrameWorkImpl::DoSysProtocol(const char *szType, TiXmlElement *pNode)
{
	BOOL bDid = FALSE;
	if (::stricmp(szType, "login") == 0)
	{
		const char *szResult = pNode->Attribute("result");
		if (::stricmp(szResult, "ok") == 0)
		{	
			//init person configure
			if (m_pConfigure)
			{
				CInterfaceAnsiString strPersonSrcPath;
				if (FAILED(m_pConfigure->GetParamValue(TRUE, "Path", "PersonPath", (IAnsiString *)&strPersonSrcPath)))
				{
					char szTmp[MAX_PATH] = {0};
					char szPrivatePath[MAX_PATH] = {0};
					CSystemUtils::GetLocalAppPath(szTmp, MAX_PATH - 1);
					CSystemUtils::IncludePathDelimiter(szTmp, szPrivatePath, MAX_PATH - 1);
		            strcat(szPrivatePath, APPLICATION_PATH_NAME);
					strPersonSrcPath.SetString(szPrivatePath); 
				}
				if (strPersonSrcPath.GetSize() > 0)
				{
					std::string strPersonPath = strPersonSrcPath.GetData();
					strPersonPath += m_strUserName;
					CSystemUtils::ForceDirectories(strPersonPath.c_str());
					
					std::string strPersonName = strPersonPath + PERSON_CONFIGURE_FILENAME;
					m_pConfigure->InitCfgFileName(strPersonName.c_str(), m_strUserName.c_str(), FALSE);
					std::string strValue = strPersonPath;
					if (strValue.back() != '\\')
						strValue.push_back('\\');
					m_pConfigure->SetParamValue(FALSE, "Path", "PersonPath", strValue.c_str());
					//init message mgr
					if (m_pMsgMgr)
					{
						strPersonName = strPersonPath + PERSON_MESSAGE_FILENAME;
						m_pMsgMgr->InitMsgMgr(strPersonName.c_str(), m_strUserName.c_str());
					}

					if (m_pUIManager)
					{
						CInterfaceAnsiString strTmp;
						COLORREF cr = 0;//RGB(62,123,123);
						if (SUCCEEDED(m_pConfigure->GetParamValue(FALSE, "skin", "background", &strTmp)))
						{
							cr = ::atoi(strTmp.GetData());
						}
						m_pUIManager->BlendSkinStyle(cr);
					}
				} //end if (SUCCEEDED(...
			} //end if (m_pConfigure)

			if (m_pConfigure)
				m_pConfigure->SetSvrParam("serverip", m_strAuthIp.c_str()); 
			TiXmlAttribute *pAttr = pNode->FirstAttribute();
		    while (pAttr)
			{
				m_SvrParams[pAttr->Name()] = pAttr->Value();				
				if (m_pConfigure)
					m_pConfigure->SetSvrParam(pAttr->Name(), pAttr->Value());
				pAttr = pAttr->Next();
			} //end while (pAttr) 
			std::string strInitPresence = m_strPresence;
			std::string strInitMemo = m_strPresenceMemo;
			m_strPresence.clear();
			m_strPresenceMemo.clear();
			ChangePresence(strInitPresence.c_str(), strInitMemo.c_str());		
			
		} //else 
	} else if (::stricmp(szType, "getsrvar") == 0)
	{
		const char *szParamName = pNode->Attribute("name");
		const char *szParamValue = pNode->Attribute("value");
		if (szParamName && szParamValue)
		{
			m_SvrParams[szParamName] = szParamValue;
			if (m_pConfigure)
				m_pConfigure->SetSvrParam(szParamName, szParamValue);
			BroadcastMessage("coreframe", NULL, "getsrvar", szParamName, (void *)szParamValue);
		} else
		{
			PRINTDEBUGLOG(dtInfo, "Get Var Param Failed, ParamName:%s ParamValue:%s", szParamName, szParamValue);
		}
	} else if (::stricmp(szType, "organize") == 0)
	{
		//contact plugin doing 
	} else if (::stricmp(szType, "presence") == 0)
	{
		//get org
		if (m_bLogon && !m_bLoginSucc)
		{
			std::string strXml = "<sys type=\"organize\" domain=\"";
			strXml += m_strDomain;
			strXml +="\" version=\"";
	        
			CInterfaceAnsiString strVersion;
		 
			if (m_pConfigure && SUCCEEDED(m_pConfigure->GetParamValue(FALSE, "contacts", "version", (IAnsiString *)&strVersion)))
			{
				CInterfaceAnsiString strFileName;
				if (m_pContacts && SUCCEEDED(m_pContacts->GetCacheContactsFileName((IAnsiString *)&strFileName)))
			   {
				   if (!CSystemUtils::FileIsExists(strFileName.GetData()))
					   strXml += "0";
				   else
					   strXml += strVersion.GetData();
				} else
					strXml += strVersion.GetData();
			} else
			{
				strXml += "0";
			}
			strXml += "\"/>";
			if (m_pAuthSocket)
			{
				m_pAuthSocket->SendRawData(strXml.c_str(), (int) strXml.size());
			    m_bLogon = FALSE;
			}
		} else if (m_bLogon)
		{
			m_bIsOnline = TRUE;
			if (m_pContacts)
				m_pContacts->OrderAllStatusFromSvr();
			GetOfflineMsg();
		}
		m_bLoginSucc = TRUE;
	} else if (::stricmp(szType, "kickout") == 0)
	{
		//<sys type="kickout"/>
		m_bIsOnline = FALSE;
		if (m_pAuthSocket)
			m_pAuthSocket->Close();
		NotifyErrorAllPlugin(CORE_ERROR_KICKOUT, "此用户名在另一处登陆，您被迫下线");
	} else
	{
		PRINTDEBUGLOG(dtInfo, "sys protocol, type:%s", szType);
	}
	return bDid;
}

#pragma warning(default:4996)


