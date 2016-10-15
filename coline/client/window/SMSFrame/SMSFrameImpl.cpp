#include <time.h>
#include <Commonlib/DebugLog.h>
#include <Commonlib/systemutils.h>
#include <Commonlib/stringutils.h>
#include <SmartSkin/smartskin.h>
 
#include <Core/common.h>
#include "../IMCommonLib/InterfaceAnsiString.h"
#include "../IMCommonLib/MessageList.h"
#include "../P2Svr/P2Svr.h"
#include "SMSFrameImpl.h"

#define MESSAGE_PAGE_COUNT 15
#pragma warning(disable:4996)

typedef struct CReceiverSmsItem
{
	std::string strSender;
	std::string strContent;
	std::string strReceiver;
}RECEIVE_SMS_ITEM, *LPRECEIVE_SMS_ITEM;

CSMSFrameImpl::CSMSFrameImpl(void):
               m_pCore(NULL),
			   m_hWnd(NULL),
			   m_hMainWnd(NULL),
			   m_hFaxWnd(NULL),
			   m_hShowWnd(NULL),
			   m_bTerminated(FALSE),
			   m_hViewWnd(NULL)
{
	m_SignThread = ::CreateEvent(NULL, FALSE, TRUE, NULL);
	m_hThread = ::CreateThread(NULL, 0, CheckSmsStatusThread, this, 0, NULL);
}


CSMSFrameImpl::~CSMSFrameImpl(void)
{
	m_bTerminated = TRUE;
	::SetEvent(m_SignThread);
	::WaitForSingleObject(m_hThread, 5000);
	::CloseHandle(m_hThread);
	if (m_hWnd)
		::SkinCloseWindow(m_hWnd);
	m_hWnd = NULL;
	if (m_hViewWnd)
		::SkinCloseWindow(m_hViewWnd);
	m_hViewWnd = NULL;
	if (m_hFaxWnd)
		::SkinCloseWindow(m_hFaxWnd);
	m_hFaxWnd = NULL;
	if (m_hShowWnd)
		::SkinCloseWindow(m_hShowWnd);
	m_hShowWnd = NULL;
	if (m_pCore)
		m_pCore->Release();
	m_pCore = NULL;
}

//IUnknown
STDMETHODIMP CSMSFrameImpl::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(ISMSFrame)))
	{
		*ppv = (ISMSFrame *) this;
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

HRESULT CSMSFrameImpl::DoMenuCommand(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szName, "treeleaf") == 0)
	{
		switch(wParam)
		{
		case 70001:
			{
				void *pSelNode = NULL;
				LPORG_TREE_NODE_DATA  pData = NULL;
				TCHAR szName[MAX_PATH] = {0};
				int nSize = MAX_PATH - 1;
				CTreeNodeType tnType;
				if (::SkinGetSelectTreeNode(hWnd, L"colleaguetree", szName, &nSize, &pSelNode, &tnType, (void **)&pData))
	            {
					if (tnType == TREENODE_TYPE_LEAF)
					{
						if (pData)
						{
							CInterfaceUserList ulList;
							ulList.AddUserInfo(pData, TRUE, TRUE); 
				            ShowSMSFrame(NULL, &ulList);
						}
					} //end if (tnType == 
				} //end if (::SkinGetSelectTreeNode(hWnd...
				break;
			} //end case
		case 31: //发送传真
			{
				void *pSelNode = NULL;
				LPORG_TREE_NODE_DATA  pData = NULL;
				TCHAR szName[MAX_PATH] = {0};
				int nSize = MAX_PATH - 1;
				CTreeNodeType tnType;
				if (::SkinGetSelectTreeNode(hWnd, L"colleaguetree", szName, &nSize, &pSelNode, &tnType, (void **)&pData))
	            {
					if (tnType == TREENODE_TYPE_LEAF)
					{
						if (pData)
						{
							CInterfaceUserList ulList;
							ulList.AddUserInfo(pData, TRUE, TRUE); 
							ShowFaxFrame(NULL, &ulList);
						}
					} //end if (tnType == 
				} //end if (::SkinGetSelectTreeNode(hWnd...
				break;
			}
		} //end switch(..
	} else if (::stricmp(szName, "treegroup") == 0)
	{
		switch(wParam)
		{
		case 70003: //发送传真
			{
				DoSendFaxFrame(hWnd);
			}
		}
	}//end if (::stricmp(...
	return -1;
}

void CSMSFrameImpl::DoSendFaxFrame(HWND hWnd)
{ 
	void *pSelNode = NULL;
	LPORG_TREE_NODE_DATA pSelData = NULL;
	TCHAR szName[MAX_PATH] = {0};
	int nSize = MAX_PATH - 1;
	CTreeNodeType tnType;
	if (::SkinGetSelectTreeNode(hWnd, L"colleaguetree", szName, 
		&nSize, &pSelNode, &tnType, (void **)&pSelData))
	{
		if (tnType == TREENODE_TYPE_GROUP)
		{
			char szUserList[2048] = {0};
			int nSize = 2047;
			if (::SkinGetNodeChildUserList(hWnd, pSelNode, szUserList, &nSize, 
		                      FALSE))
			{
				CInterfaceUserList UserList; 
				if (nSize > 0)
				{
					TiXmlDocument xml;
					if (xml.Load(szUserList, nSize))
					{
						TiXmlElement *pNode = xml.FirstChildElement(); 
						while (pNode)
						{
							LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
							memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
							strcpy(pData->szUserName, pNode->Attribute("u"));
							UserList.AddUserInfo(pData, FALSE, TRUE);
							pNode = pNode->NextSiblingElement();
						}
					} //end if (xml.Load(...
				} else //节点未展开
				{
					IContacts *pContact = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
					{
						char szDeptId[16] = {0};
						::itoa(pSelData->id, szDeptId, 10);
						pContact->GetChildListByDeptId(szDeptId, &UserList, 0);
						pContact->Release();
					}
				} 
				ShowFaxFrame(NULL, &UserList); 
			} // end if (::SkinGetNodeChildUserList(..
		} //end if (tnType..
	}  
}

void CSMSFrameImpl::DoSelReceiver(HWND hWnd)
{
	IContactPanel *pPanel = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContactPanel), (void **)&pPanel)))
	{
		std::string strNumbersTmp;
		GetInputNumbers(hWnd, strNumbersTmp); 
		CInterfaceUserList ulList; 
		std::string strSelList;
		int nIdx = 0;
		IContacts *pContact = NULL;
		if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
		{
			std::string strUserName, strRealName, strPhone;
			const char *p1 = strNumbersTmp.c_str();
			char *szTmp = new char[strNumbersTmp.size() + 1];
			while (p1)
			{
				memset(szTmp, 0, strNumbersTmp.size() + 1);
				if (!CSystemUtils::GetStringBySep(p1, szTmp, ';', nIdx))
					break;
				//去除空格及回车
				int nSize = ::strlen(szTmp);
				while (true)
				{
					if ((szTmp[nSize - 1] == '\n')
						|| (szTmp[nSize - 1] == ' ')
						|| (szTmp[nSize - 1] == 0xd))
					{
						szTmp[nSize - 1] = '\0';
						nSize --;
					} else
						break;
				}
				if (::strlen(szTmp) > 0)
				{					
					//pTmp->sz
					if (GetUserNameByPhonInfo(pContact, szTmp, strPhone, strUserName, strRealName))
					{
						LPORG_TREE_NODE_DATA pTmp = new ORG_TREE_NODE_DATA();
					    memset(pTmp, 0, sizeof(ORG_TREE_NODE_DATA));
						strncpy(pTmp->szUserName, strUserName.c_str(), 63);
						if (!strRealName.empty())
						{
							pTmp->szDisplayName = new char[strRealName.size() + 1];
							memset(pTmp->szDisplayName, 0, strRealName.size() + 1);
							strcpy(pTmp->szDisplayName, strRealName.c_str()); 
						}
						if (!ulList.AddUserInfo(pTmp, FALSE, TRUE))
						{
							if (pTmp->szDisplayName)
								delete []pTmp->szDisplayName;
							delete pTmp;
						}
					} else if (CSystemUtils::IsMobileNumber(szTmp))
					{
						strSelList += szTmp;
						strSelList += ";";
					}
				}
				nIdx ++;
			}
			delete []szTmp;
			pContact->Release();
		}
		CStdString_ strCaption = L"选择短信接收者";
		if (hWnd == m_hFaxWnd)
			strCaption = L"选择传真接收者"; 
		if (SUCCEEDED(pPanel->ShowPanel(hWnd, strCaption.GetData(), NULL, TRUE, &ulList)))
		{ 
			ulList.Clear();
			pPanel->GetSelContact(&ulList); 
			ShowUserListToInputFrame(hWnd, &ulList, strSelList.c_str()); 
		}
		pPanel->Release();
	}
}

HRESULT CSMSFrameImpl::DoClickEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (hWnd == m_hWnd)
	{
		if (::stricmp(szName, "selreceiver") == 0)
		{
			DoSelReceiver(hWnd);
		} else if (::stricmp(szName, "btnsend") == 0)
		{
			SendSms();
		} else if (::stricmp(szName, "btncancel") == 0)
		{
			::SkinCloseWindow(hWnd);
		} else if (::stricmp(szName, "btnView") == 0)
		{
			if (hWnd == m_hFaxWnd)
				ShowSmsView(NULL, "fax");
			else
				ShowSmsView(NULL, "sms");
		}
	} else if (hWnd == m_hViewWnd)
	{
		if (::stricmp(szName, "prevPage") == 0)
		{
			int nPage = m_nCurrPage;
			nPage --;
			DisplayHistoryMsg(nPage);
		} else if (::stricmp(szName, "nextpage") == 0)
		{
			int nPage = m_nCurrPage;
			nPage ++;
			DisplayHistoryMsg(nPage);
		} else if (::stricmp(szName, "viewsms") == 0)
		{
			//查看记录
			ViewCurrSmsMsg();
		} else if (::stricmp(szName, "deletesms") == 0)
		{
			//删除记录
			DeleteCurrSmsMsg();
		} else if (::stricmp(szName, "refreshsms") == 0)
		{
			//刷新记录
			std::string strType = m_strShowType;
			InitSmsView(NULL, strType.c_str());
		}
	} else if (hWnd == m_hFaxWnd)
	{
		if (::stricmp(szName, "faxbrowser") == 0)
		{
			CStringList_ FileList;
		 	if (CSystemUtils::OpenFileDialog(NULL, hWnd, "选择要发送的文件", "支持的传真文件(*.doc;*.txt;*.xls;*.ppt;*.tif;*.tiff)|*.doc;*.txt;*.xls;*.ppt;*.tif;*.tiff", 
				                NULL, FileList, FALSE, FALSE))
			{
				std::string strFileName;
				if (!FileList.empty())
					strFileName = FileList.back();
				if ((!strFileName.empty()) && CSystemUtils::FileIsExists(strFileName.c_str()))
				{
					TCHAR szwFileName[MAX_PATH] = {0};
					CStringConversion::StringToWideChar(strFileName.c_str(), szwFileName, MAX_PATH - 1);
					return ::SkinSetControlTextByName(hWnd, L"faxfilename", szwFileName);  						
				} //end if ((!strFileName.empty()) ...
			} //end if (CSystemUtils::OpenFileDialog(...
		} else if (::stricmp(szName, "selreceiver") == 0)
		{
			DoSelReceiver(hWnd);
		} else if (::stricmp(szName, "btnsend") == 0)
		{
			SendFax(hWnd);
		} else if (::stricmp(szName, "btncancel") == 0)
		{
			::SkinCloseWindow(hWnd);
		} else if (::stricmp(szName, "btnView") == 0)
		{
			if (hWnd == m_hFaxWnd)
				ShowSmsView(NULL, "fax");
			else
				ShowSmsView(NULL, "sms");
		} 
	} else
	{
		if (::stricmp(szName, "acksms") == 0)
		{
			this->ShowSMSFrame(NULL, NULL);
			TCHAR szTemp[MAX_PATH] = {0};
			int nSize = MAX_PATH - 1;
			::SkinGetControlTextByName(hWnd, L"smssender", szTemp, &nSize);
			::SkinSetControlTextByName(m_hWnd, L"numbers", szTemp);
			::SkinCloseWindow(hWnd);
		} else if (::stricmp(szName, "cancel") == 0)
		{
			::SkinCloseWindow(hWnd);
		}
	}//end if (hWnd == m_hWnd)
	return -1;
}

//
void CSMSFrameImpl::SendFaxFileToSvr(const char *szReceiverName, const char *szFileName, const char *szTitle)
{
	IConfigure *pCfg = NULL; 
	CInterfaceAnsiString strSvr; 
	std::string strRealName;
	std::string strSenderName;
	std::string strReceiverRealName; //接收者姓名
	CInterfaceAnsiString strTmp; 
	std::string strFaxNum;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		pCfg->GetServerAddr(HTTP_SVR_URL_FAX, &strSvr);
		pCfg->Release();
	}
	m_pCore->GetUserName(&strTmp);
	strSenderName = strTmp.GetData();
	strSenderName += "@";
	m_pCore->GetUserDomain(&strTmp);
	strSenderName += strTmp.GetData();

	IContacts *pContact = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
	{
		if (SUCCEEDED(pContact->GetRealNameById(strSenderName.c_str(), NULL, &strTmp)))
		{
			char szTmp[128] = {0};
			CStringConversion::UTF8ToString(strTmp.GetData(), szTmp, 127);
			strRealName = szTmp;
		}
		if (SUCCEEDED(pContact->GetFaxByName(strSenderName.c_str(), &strTmp)))
		{
			strFaxNum = strTmp.GetData();
		}
		if (SUCCEEDED(pContact->GetRealNameById(szReceiverName, NULL, &strTmp)))
		{
			char szTmp[128] = {0};
			CStringConversion::UTF8ToString(strTmp.GetData(), szTmp, 127);
			strReceiverRealName = szTmp;
		}
		pContact->Release();
	}
	if (strSvr.GetSize() > 0)
	{
		//
		std::string strParams;
		strParams = "SenderName="; //发送者真实姓名（会显示在传真文件的台头）
		strParams += strRealName;
		strParams += ";LogonName="; //发送者用户名
		strParams += strSenderName;
		strParams += ";FaxNum="; //传真号码
		strParams += strFaxNum;
		strParams += ";title="; //标题 
		strParams += szTitle;
		strParams += ";RcverName="; //接收人（会显示在传真文件的台头）
		strParams += strReceiverRealName; 
		::P2SvrPostFile(strSvr.GetData(), szFileName, strParams.c_str(), FILE_TYPE_NORMAL, this, NULL, TRUE); 
	}
}

//
void CSMSFrameImpl::SendFax(HWND hWnd)
{
	if (m_pCore)
	{
		std::string strTmp;
		GetInputNumbers(m_hWnd, strTmp); 
		TCHAR szwTmp[1024] = {0};
		int nSize = 1023; 
		if (strTmp.empty())
		{
			::SkinMessageBox(m_hWnd, L"传真接收人员不能为空", L"警告", MB_OK);
			return ;
		}
		if (::SkinGetControlTextByName(m_hWnd, L"faxfilename", szwTmp, &nSize))
		{ 
			char szFaxFileName[MAX_PATH] = {0};
			CStringConversion::WideCharToString(szwTmp, szFaxFileName, 1023); 
			TCHAR szwTitle[MAX_PATH] = {0};
			int nSignSize = MAX_PATH - 1;
			char szTitle[MAX_PATH] = {0};
			if (::SkinGetControlTextByName(m_hWnd, L"faxtitle", szwTitle, &nSignSize))
				CStringConversion::WideCharToString(szwTitle, szTitle, MAX_PATH - 1);
			std::string strNumbers;
			std::string strFailed;
			if (CheckNumbersValid(strTmp.c_str(), strNumbers, strFailed))
			{
				SendFaxFileToSvr(strNumbers.c_str(), szFaxFileName, szTitle); 
			} else
			{
				TCHAR *szText = new TCHAR[strFailed.size() + 1];
				memset(szText, 0, sizeof(TCHAR) * (strFailed.size() + 1));
				CStringConversion::StringToWideChar(strFailed.c_str(), szText, strFailed.size());
				::SkinMessageBox(m_hWnd, szText, L"警告", MB_OK);
			}  
		} else
			::SkinMessageBox(m_hWnd, L"传真内容不能为空", L"警告", MB_OK);
	}
}

BOOL CSMSFrameImpl::CheckNumbersValid(const char *szInput, std::string &strNumbers, std::string &strFailed)
{
	int nIdx = 0;
	int nSize = strlen(szInput)  + 1;
	char *szPerNumber = new char[nSize];
	
	BOOL bSucc = TRUE;
	IContacts *pContact = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
	{
		std::string strPhone, strUserName, strRealName;
		while (TRUE)
		{		
			memset(szPerNumber, 0, nSize);
			if (!CSystemUtils::GetStringBySep(szInput, szPerNumber, ';', nIdx))
				break;
			//去除空格及回车
			int nSize = ::strlen(szPerNumber);
			while (true)
			{
				if ((szPerNumber[nSize - 1] == '\n')
					|| (szPerNumber[nSize - 1] == ' ')
					|| (szPerNumber[nSize - 1] == 0xd))
				{
					szPerNumber[nSize - 1] = '\0';
					nSize --;
				} else
					break;
			}
			if (::strlen(szPerNumber) == 0)
				break;
			if (!CSystemUtils::IsMobileNumber(szPerNumber)) 
			{ 
				if (GetUserNameByPhonInfo(pContact, szPerNumber, strPhone, strUserName, strRealName)) 
				{
					if (CSystemUtils::IsMobileNumber(strPhone.c_str()))
					{
						strNumbers += szPerNumber;
				        strNumbers += ";";
					} else
					{
						bSucc = FALSE;
						strFailed += "此用户手机号码有误,用户名：";
						strFailed += szPerNumber;
						strFailed += ",号码:";
						strFailed += strPhone;
						strFailed += "\n";
					}
				} else
				{
					bSucc = FALSE;
					strFailed += "没有找到此用户手机号码,用户名：";
					strFailed += szPerNumber;
					strFailed += "\n";
				}
			} else
			{
				strNumbers += szPerNumber;
				strNumbers += ";";
			}
			nIdx ++;
		}
		pContact->Release();
	}
	delete []szPerNumber;
	return bSucc;
}

//
void CSMSFrameImpl::StatInputCount(HWND hWnd)
{
	TCHAR szwTmp[1024] = {0};
	int nSize = 1023;
	int nCount = 0;
	if (::SkinGetControlTextByName(m_hWnd, L"smscontent", szwTmp, &nSize)) 
	{
		nCount = ::lstrlen(szwTmp);
	}
	
	TCHAR szTmp[MAX_PATH] = {0};
	wsprintf(szTmp, L"%d", nCount);
	::SkinSetControlTextByName(hWnd, L"contentcount", szTmp);
}

//统计接收者人数
void CSMSFrameImpl::StatReceiverCount(HWND hWnd)
{
	int nCount = 0; 
	std::string strRecievers; 
	if (GetInputNumbers(hWnd, strRecievers))
	{
		char *szTmp = new char[strRecievers.size() + 1]; 
		int idx = 0;
		while (TRUE)
		{		
			memset(szTmp, 0, strRecievers.size() + 1);
			if (!CSystemUtils::GetStringBySep(strRecievers.c_str(), szTmp, ';', idx))
				break;
			idx ++;
			if (::strlen(szTmp) > 0)
				nCount ++;
		}
		delete []szTmp;
	}
	TCHAR szwTmp[64] = {0};
	wsprintf(szwTmp, L"%d", nCount);
	::SkinSetControlTextByName(hWnd, L"receivercount", szwTmp);
}

void CSMSFrameImpl::SendSms()
{
	if (m_pCore)
	{
		std::string strTmp;
		GetInputNumbers(m_hWnd, strTmp); 
		TCHAR szwTmp[4096] = {0};
		int nSize = 4096; 
		if (strTmp.empty())
		{
			::SkinMessageBox(m_hWnd, L"短信接收人员不能为空", L"警告", MB_OK);
			return ;
		}
		if (::SkinGetControlTextByName(m_hWnd, L"smscontent", szwTmp, &nSize))
		{ 
			char p[4096] = {0};
			CStringConversion::WideCharToString(szwTmp, p, 4095);
			std::string strContent = p;
			TCHAR szwSign[MAX_PATH] = {0};
			int nSignSize = MAX_PATH - 1;
			char szSign[MAX_PATH] = {0};
			if (::SkinGetControlTextByName(m_hWnd, L"smssign", szwSign, &nSignSize))
				CStringConversion::WideCharToString(szwSign, szSign, MAX_PATH - 1);
			std::string strNumbers;
			std::string strFailed;
			if (CheckNumbersValid(strTmp.c_str(), strNumbers, strFailed))
			{
				SendSMS(strNumbers.c_str(), strContent.c_str(), szSign);
				::SkinSetControlTextByName(m_hWnd, L"smscontent", L"");
				StatInputCount(m_hWnd);
				/*ITrayMsg *pTray = NULL;
				if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(ITrayMsg), (void **)&pTray)))
				{
					pTray->ShowTipPanel("短信发送提示", "已发送至短信网关，稍后将返回发送结果"); 
					pTray->Release();
				} */
			} else
			{
				TCHAR *szText = new TCHAR[strFailed.size() + 1];
				memset(szText, 0, sizeof(TCHAR) * (strFailed.size() + 1));
				CStringConversion::StringToWideChar(strFailed.c_str(), szText, strFailed.size());
				::SkinMessageBox(m_hWnd, szText, L"警告", MB_OK);
			}  
		} else
			::SkinMessageBox(m_hWnd, L"短信内容不能为空", L"警告", MB_OK);
	}
}

//解析短信协议
STDMETHODIMP CSMSFrameImpl::ParserSMSProtocol(const char *szContent, IAnsiString *strSender, IAnsiString *strReceiver,
		IAnsiString *strTime, IAnsiString *strGuid, IAnsiString *strSign, IAnsiString *strText)
{
	TiXmlDocument xmlDoc;
	if (xmlDoc.Load(szContent, ::strlen(szContent)))
	{
		TiXmlElement *pRoot = xmlDoc.FirstChildElement();
		if (pRoot)
		{
			//
		}
	}
	return E_FAIL;
}

//
STDMETHODIMP CSMSFrameImpl::SendSMS(const char *szPeerNumbers, const char *szContent, const char *szSign)
{
	static const char SMS_PROTOCOL_XML[] = "<sms type=\"single\" guid=\"guid\" uid=\"username@GoCom.com\" number=\"number1;number2\" content=\"content\" sign=\"sign\"/>"; 
	//<sms type="single" guid="%s" uid="%s" number="%s" content="%s" sign="%s"/> 
	if (m_pCore && (szPeerNumbers != NULL) && (szContent != NULL))
	{
		std::string strUserName;
		CInterfaceAnsiString szName;
		if (SUCCEEDED(m_pCore->GetUserName(&szName)))
		{
			strUserName = szName.GetData();
		}
		if (SUCCEEDED(m_pCore->GetUserDomain(&szName)))
		{
			strUserName += "@";
			strUserName += szName.GetData();
		}
		
		//
		std::vector<std::string> vcNumbersList;
		int nIdx = 0;
		while (TRUE)
		{
			char szTmp[32] = {0};
			if (!CSystemUtils::GetStringBySep(szPeerNumbers, szTmp, ';', nIdx))
				break;
			if (::strlen(szTmp) > 0)
				vcNumbersList.push_back(szTmp);
			nIdx ++;
		}
		TiXmlDocument xmldoc;
		if ((strUserName.size() > 0) && (vcNumbersList.size() > 0) && xmldoc.Load(SMS_PROTOCOL_XML, strlen(SMS_PROTOCOL_XML)))
		{
			TiXmlElement *pNode = xmldoc.RootElement();
			//

			pNode->SetAttribute("uid", strUserName.c_str());
			pNode->SetAttribute("content", szContent);
			if (szSign)
				pNode->SetAttribute("sign", szSign);
			else
				pNode->SetAttribute("sign", "");
			

			std::vector<std::string>::iterator it;
			char szGuid[128] = {0};
			int nSize;
			IMsgMgr *pMgr = NULL;
			char szTime[64] = {0};
			CSystemUtils::DateTimeToStr((DWORD)time(NULL), szTime);
			if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMsgMgr), (void **)&pMgr)))
			{
				std::string strMobileNumber;
				CSmsBuffer *pSmsBuffer = new CSmsBuffer();
				pSmsBuffer->m_strContent = szContent;
				for (it = vcNumbersList.begin(); it != vcNumbersList.end(); it ++)
				{
					if (!CSystemUtils::IsMobileNumber((*it).c_str()))
					{
						std::string strSp = (*it).c_str();
						int nPos = strSp.find("<");
						if (nPos != std::string::npos)
						{
							strMobileNumber = strSp.substr(nPos + 1);
							strMobileNumber.pop_back();
						} else
						{
							PRINTDEBUGLOG(dtInfo, "sms frame error, mobile error:%s", (*it).c_str());
						}
					} else
					{
						strMobileNumber = (*it).c_str();
					}
					pNode->SetAttribute("number", strMobileNumber.c_str());
					memset(szGuid, 0, 128);
					nSize = 127;
					CSystemUtils::GetGuidString(szGuid, &nSize);
					LPSMS_PEND_ITEM pSmsItem = new SMS_PEND_ITEM();
					pSmsItem->dwCount = ::GetTickCount();
					pSmsItem->strContent = szContent;
					pSmsItem->strReceiver = (*it).c_str();
					m_smsPendLock.Lock(); 
					pSmsBuffer->m_SmsNumbers.insert(std::pair<std::string, std::string>(szGuid, strMobileNumber));
					m_SmsPendList.insert(std::pair<std::string, LPSMS_PEND_ITEM>(szGuid, pSmsItem)); 
					m_smsPendLock.UnLock();
					pNode->SetAttribute("guid", szGuid);
					TiXmlString strXml;
					xmldoc.SaveToString(strXml, 0);
					m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG) strXml.size(), 0);
					pMgr->SaveMsg("sms", strUserName.c_str(), (*it).c_str(), szTime, strXml.c_str(), szContent);
				} //end for (..
				m_SmsBufferLock.Lock();
				m_SmsBuffer.push_back(pSmsBuffer);
				pSmsBuffer->m_wSendTime = (DWORD) time(NULL);
				::srand((unsigned int) GetTickCount());
				pSmsBuffer->m_Timer = ::rand();
				//::SetTimer(m_hMainWnd, pSmsBuffer->m_Timer, 30, NULL);
				m_SmsBufferLock.UnLock();
				pMgr->Release();
			}
			return S_OK;
		}
	}
	return E_FAIL;
}
 

//
BOOL CSMSFrameImpl::GetInputNumbers(HWND hWnd, std::string &strNumbers)
{
	int nSize = 0;
	::SkinGetControlTextByName(hWnd, L"numbers", NULL, &nSize);
	TCHAR *szText = NULL;
	if (nSize > 0)
	{
		szText = new TCHAR[nSize + 1];
		memset(szText, 0, sizeof(TCHAR) * (nSize + 1));
	}
	if (szText)
	{
		::SkinGetControlTextByName(hWnd, L"numbers", szText, &nSize);
		char *p = new char[nSize * 2 + 1];
		memset(p, 0, nSize * 2 + 1);
		CStringConversion::WideCharToString(szText, p, nSize * 2);
		strNumbers = p;
		delete []p;
		delete []szText;
	}
	return (!strNumbers.empty());
}

BOOL CSMSFrameImpl::GetLastInputNumbers(HWND hWnd, std::string &strNumbers)
{
	std::string strTmp;
	if (GetInputNumbers(hWnd, strTmp))
	{
		int nIdx = 0;
		const char *p = strTmp.c_str();
		char *pTmp = new char[strTmp.size() + 1];
		while (p)
		{ 
			memset(pTmp, 0, strTmp.size() + 1);
			if (!CSystemUtils::GetStringBySep(p, pTmp, ';', nIdx))
				break;
			strNumbers = pTmp;
			nIdx ++;
		} 
		delete []pTmp;
	}

	return (!strNumbers.empty());
}

//ICoreEvent
STDMETHODIMP CSMSFrameImpl::DoCoreEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
                     LPARAM lParam, HRESULT *hResult)
{
	if (::stricmp(szType, "menucommand") == 0)
	{
		*hResult = DoMenuCommand(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "click") == 0)
	{
		*hResult = DoClickEvent(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "killfocus") == 0)
	{
		if (::stricmp(szName, "numbers") == 0)
		{
			IContacts *pContact = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
			{
				pContact->HideHelpEditWindow();
				pContact->Release();
			}
		}
	} else if (::stricmp(szType, "editchanged") == 0)
	{
		if (::stricmp(szName, "numbers") == 0)
		{
			std::string strNumber;
			if (GetLastInputNumbers(hWnd, strNumber))
			{  
				RECT rc = {0};
				::SkinGetControlRect(hWnd, L"numbers", &rc);
				POINT pt = {rc.left, rc.bottom};
				::ClientToScreen(hWnd, &pt);
				IContacts *pContact = NULL;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
				{
					pContact->ShowHelpEditWindow((ICoreEvent *) this, strNumber.c_str(), pt.x, pt.y, rc.right - rc.left, 100);
					pContact->Release();
				} //end if (
			} //end if (::
		} else if (::stricmp(szName, "smscontent") == 0)
		{
			StatInputCount(hWnd);
		} 
	} else if (::stricmp(szType, "onchar") == 0)
	{
		if (::stricmp(szName, "numbers") == 0)
		{
			if (wParam == VK_RETURN)
				*hResult = 0;
		}
	} else if (::stricmp(szType, "keydown") == 0)
	{
		if (wParam == VK_ESCAPE)
			::SkinCloseWindow(m_hWnd); //end if (::stricmp(szName...
		else if (::stricmp(szName, "numbers") == 0)
		{
			std::string strNumber;
			if (GetInputNumbers(hWnd, strNumber))
			{
				IContacts *pContact = NULL;
				if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
				{
					switch(wParam)
					{
						case VK_RETURN:
						{ 
							pContact->EditHelpSearchActive(hWnd, strNumber.c_str(), TRUE, FALSE);
							*hResult = 0;
							break;
						}
						case  VK_ESCAPE:
						{
							pContact->EditHelpSearchActive(hWnd, NULL, FALSE, FALSE);
							*hResult = 0;
							break;
						}
						case VK_UP:
						case VK_DOWN:
						case VK_PRIOR:
						case VK_NEXT:
						case VK_HOME:
						case VK_END:
						{
							pContact->EditVirtualKeyUp(wParam);
							*hResult = 0;
							break;
						}
					}
					pContact->Release();
				}  //end if (SUCCEEDED(m_pCore->
			} //end if (GetInputNumbers
			StatReceiverCount(hWnd);
		} 
	} else if (::stricmp(szType, "itemactivate") == 0)
	{
		if (::stricmp(szName, "resultlist") == 0)
		{
			std::string strTmp;
			GetInputNumbers(m_hWnd, strTmp);
			const char *p = strTmp.c_str();
			std::vector<std::string> strNumbers;
			if (p)
			{
				int nIdx = 0; 
				char *szTmp = new char[strTmp.size() + 1];
				while (p)
				{
					memset(szTmp, 0, strTmp.size() + 1); 
					if (!CSystemUtils::GetStringBySep(p, szTmp, ';', nIdx))
						break;
					strNumbers.push_back(szTmp);
					nIdx ++;
				}
				strNumbers.pop_back(); 
				delete []szTmp;
			}
			std::string strList;

			if (strNumbers.empty())
			{ 
				GetPhoneInfoByUserName((char *)lParam, strList);
			} else
			{
				std::vector<std::string>::iterator it;
				for (it = strNumbers.begin(); it != strNumbers.end(); it ++)
				{
					strList += (*it);
					strList += ";";
				}
				std::string strPhoneInfo;
				GetPhoneInfoByUserName((char *)lParam, strPhoneInfo);
				strList += strPhoneInfo;
			}
			strList += ";";
			TCHAR *szwTmp = new TCHAR[strList.size() + 1];
			memset(szwTmp, 0, sizeof(TCHAR) * (strList.size() + 1));
			CStringConversion::StringToWideChar(strList.c_str(), szwTmp, strList.size());
			::SkinSetControlTextByName(m_hWnd, L"numbers", szwTmp); 
			StatReceiverCount(m_hWnd);
			delete []szwTmp;
		} //
	} else if (::stricmp(szType, "onclosequery") == 0)
	{
		if (::stricmp(szName, "SMSFrame") == 0)
		{
			TCHAR szwTmp[1024] = {0};
			int nSize = 1023;
			BOOL bClosed = TRUE;
			if (::SkinGetControlTextByName(hWnd, L"smscontent", szwTmp, &nSize))
			{
				if ((::lstrlen(szwTmp) > 2)
					&& (::SkinMessageBox(hWnd, L"有短信尚未发送，是否关闭", L"警告", 2) != IDOK))
					bClosed = FALSE;
			}
			if (!bClosed)
				*hResult = 0; 
		}
	} else if (::stricmp(szType, "afterinit") == 0)
	{
		if (::stricmp(szName, "mainwindow") == 0)
		{
			m_hMainWnd = hWnd;
			IUIManager *pUI = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
			{
				pUI->OrderWindowMessage("mainwindow", NULL, WM_RECEIVESMS, (ICoreEvent *)this); 
				pUI->Release();
			}
		}
	} else if (::stricmp(szType, "itemdblclk") == 0)
	{
		if (::stricmp(szName, "smslist") == 0)
			ViewCurrSmsMsg();
	}
	return E_NOTIMPL;
}

//
BOOL CSMSFrameImpl::GetUserNameByPhonInfo(IContacts *pContact, const char *szPhoneInfo, std::string &strNumber,
	                         std::string &strUserName, std::string &strRealName)
{
	if (szPhoneInfo)
	{ 
		if (!CSystemUtils::IsMobileNumber(szPhoneInfo))
		{
			std::string strSp = szPhoneInfo;
			int nPos = strSp.find("<");
			if (nPos != std::string::npos)
			{
				strNumber = strSp.substr(nPos + 1);
				strNumber.pop_back(); //pop ">"
			}
		} else
		{
			strNumber = szPhoneInfo;
		}
		if (!strNumber.empty())
		{ 
			CInterfaceAnsiString strTmpUserName, strTmpRealName;
			if (SUCCEEDED(pContact->GetUserNameByNameOrPhone(strNumber.c_str(), &strTmpUserName, &strTmpRealName)))
			{
				strUserName = strTmpUserName.GetData();
				strRealName = strTmpRealName.GetData(); 
			    return TRUE;
			} 
		} //end if (!strPhone.empty())
	} //end if (szPhoneInfo)
	return FALSE;
}

//
BOOL CSMSFrameImpl::GetPhoneInfoByUserName(const char *szUserName, std::string &strPhoneInfo, BOOL bFax)
{
	IContacts *pContact = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
	{
		BOOL bExistsRealName = FALSE;
		CInterfaceAnsiString strTmp;
		if (SUCCEEDED(pContact->GetRealNameById(szUserName, NULL, &strTmp)))
		{
			char szName[MAX_PATH] = {0};
			CStringConversion::UTF8ToString(strTmp.GetData(), szName, MAX_PATH - 1);
			strPhoneInfo = szName;
			strPhoneInfo += "<";
			bExistsRealName = TRUE;
		}
		if (bFax)
		{
			if (SUCCEEDED(pContact->GetFaxByName(szUserName, &strTmp)))
			{
				strPhoneInfo += strTmp.GetData();
			}
		} else
		{
			if (SUCCEEDED(pContact->GetPhoneByName(szUserName, &strTmp)))
			{
				strPhoneInfo += strTmp.GetData();
			}
		}
		if (bExistsRealName)
			strPhoneInfo += ">";
		pContact->Release();
		return TRUE;
	}
	return FALSE;
}

//广播消息
STDMETHODIMP CSMSFrameImpl::DoBroadcastMessage(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData)
{
	return E_NOTIMPL;
}

STDMETHODIMP CSMSFrameImpl::SetCoreFrameWork(ICoreFrameWork *pCore)
{
	if (m_pCore)
		m_pCore->Release();
	m_pCore = pCore;
	if (m_pCore)
	{
		m_pCore->AddRef();
		//order
		m_pCore->AddOrderEvent((ICoreEvent *) this, "MainWindow", "treegroup", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "MainWindow", "treeleaf", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "mainwindow", "afterinit");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "SMSFrame", NULL, NULL);
		m_pCore->AddOrderEvent((ICoreEvent *) this, "SmsView", NULL, NULL);
		m_pCore->AddOrderEvent((ICoreEvent *) this, "FaxFrame", NULL, NULL);
		m_pCore->AddOrderEvent((ICoreEvent *) this, "smsreceiver", NULL, NULL);
		//
		m_pCore->AddOrderProtocol((IProtocolParser *) this, "sms", NULL);
	}
	return S_OK;
}

STDMETHODIMP CSMSFrameImpl::GetSkinXmlString(IAnsiString *szXmlString)
{
	HRESULT hr = E_FAIL;
	IConfigure *pCfg = NULL; 
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{ 
		hr = pCfg->GetSkinXml("SMSFrame.xml",szXmlString); 
		pCfg->Release();
	}
	return hr;  
}

//
STDMETHODIMP CSMSFrameImpl::CoreFrameWorkError(int nErrorNo, const char *szErrorMsg)
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
STDMETHODIMP CSMSFrameImpl::DoWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes)
{
	if (uMsg == WM_RECEIVESMS)
	{
		//
		LPRECEIVE_SMS_ITEM pItem = (LPRECEIVE_SMS_ITEM) lParam;
		char szTime[64] = {0};
		CSystemUtils::DateTimeToStr(DWORD(::time(NULL)), szTime);
		ShowSMSContent(pItem->strSender.c_str(), szTime, pItem->strContent.c_str());
		
		
		delete pItem;
	} 
	return E_NOTIMPL;
}

DWORD WINAPI CSMSFrameImpl::CheckSmsStatusThread(LPVOID lpParam)
{
	CSMSFrameImpl *pThis = (CSMSFrameImpl *)lpParam;
	while (!pThis->m_bTerminated)
	{
		::WaitForSingleObject(pThis->m_SignThread, 15000);

		if (pThis->m_bTerminated)
			break;
	    CSmsBuffer *pBuffer = NULL;
		pThis->m_SmsBufferLock.Lock();
		std::vector<CSmsBuffer *>::iterator it;
		DWORD dwNow = (DWORD) time(NULL);
		for (it = pThis->m_SmsBuffer.begin(); it != pThis->m_SmsBuffer.end(); it ++)
		{
			if (( dwNow - (*it)->m_wSendTime) > 120)  //2分钟超时
			{
				pBuffer = (*it);
				pThis->m_SmsBuffer.erase(it);
				break;
			}
		}
		pThis->m_SmsBufferLock.UnLock(); 

		if (pBuffer)
		{
			BOOL bSucc = TRUE;
			std::string strFailedNumber;
			std::map<std::string, std::string>::iterator it;
			for (it = pBuffer->m_SmsNumbers.begin(); it != pBuffer->m_SmsNumbers.end(); it ++)
			{
				if (pThis->SenderGuidIsExists(it->first.c_str(), TRUE))
				{
					strFailedNumber += it->second;
					strFailedNumber += ",";
				} //end if (
			} //end for (it ..
			std::string strResult;
			if (strFailedNumber.empty())
			{
				//
				strResult = "短信发送成功，内容：";
				strResult += pBuffer->m_strContent;
			} else
			{
				
				strResult = "以下人员发送超时：";
				strResult += strFailedNumber;
				strResult += "  短信内容:";
				strResult += pBuffer->m_strContent;
			}
			IUIManager *pUI = NULL;
			if (SUCCEEDED(pThis->m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
			{
				HWND hWnd = NULL;
				if (SUCCEEDED(pUI->GetWindowHWNDByName("mainwindow", &hWnd)))
				{
					std::string strCaption = "短信发送结果提示";
					::SendMessage(hWnd, WM_SHOWTIPPANEL, WPARAM(strCaption.c_str()), LPARAM(strResult.c_str()));
				}
				pUI->Release(); 
			}  //end if (SUCCEEDED(
			delete pBuffer;
		} //end if(pBuffer)
	};
	return 0;
}

BOOL CSMSFrameImpl::SenderGuidIsExists(const char *szGuid, BOOL bDelete)
{
	CGuardLock::COwnerLock guard(m_smsPendLock);
	std::map<std::string, LPSMS_PEND_ITEM>::iterator it = m_SmsPendList.find(szGuid);
	if (it != m_SmsPendList.end())
	{
		if (bDelete)
		{
			delete it->second;
			m_SmsPendList.erase(it);
		}
		return TRUE;
	}
	return FALSE;
}

//IProtocolParser
STDMETHODIMP CSMSFrameImpl::DoRecvProtocol(const BYTE *pData, const LONG lSize)
{
	TiXmlDocument xmlDoc;
	BOOL bDid = FALSE;
	if (xmlDoc.Load((char *)pData, lSize))
	{
		TiXmlElement *pNode = xmlDoc.FirstChildElement();
		if (pNode)
		{
			const char *szName = pNode->Value();
			const char *szType = pNode->Attribute("type"); 
			if (::stricmp(szName, "sms") == 0)
			{
				if (::stricmp(szType, "sendresult") == 0)
				{
					//<sms type="sendresult", guid="20ad9d98a98da8d9d9d9" result="ok" uid="aaa@gocom"/>
					//短信发送结果 
					if (pNode->Attribute("guid"))
					{
						BOOL bSucc = FALSE;
						if (pNode->Attribute("result"))
							bSucc = (::stricmp(pNode->Attribute("result"), "ok") == 0);
						std::string strResult;
						if (bSucc)
							strResult = "短信发送成功:";
						else
							strResult = "短信发送失败:";
						m_smsPendLock.Lock();
						std::map<std::string, LPSMS_PEND_ITEM>::iterator it = m_SmsPendList.find(pNode->Attribute("guid"));
						if (it != m_SmsPendList.end())
						{
							strResult += "\n 接收人：";
							strResult += it->second->strReceiver;
							strResult += "\n 短信内容：";
							strResult += it->second->strContent;
							if (bSucc)
							{
								delete it->second;
								m_SmsPendList.erase(it);
							} //end if (bSucc)
						} //end if (it != m_SmsPendList.end())
						m_smsPendLock.UnLock(); 
						if (!bSucc)
						{
							IUIManager *pUI = NULL;
							if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
							{
								HWND hWnd = NULL;
								if (SUCCEEDED(pUI->GetWindowHWNDByName("mainwindow", &hWnd)))
								{
									std::string strCaption = "短信发送结果提示";
									::SendMessage(hWnd, WM_SHOWTIPPANEL, WPARAM(strCaption.c_str()), LPARAM(strResult.c_str()));
								}
								pUI->Release(); 
							}  //end if (SUCCEEDED(
						} //end if (!bSucc)
					} //
				} else if (::stricmp(szType, "recv") == 0)
				{
					//接收到短信
					HWND hWnd = NULL;
					IUIManager *pUI = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
					{
						pUI->GetWindowHWNDByName("mainwindow", &hWnd);
						pUI->Release();
					}
					if (hWnd)
					{
						LPRECEIVE_SMS_ITEM pItem = new RECEIVE_SMS_ITEM();
						if (pNode->Attribute("content"))
							pItem->strContent = pNode->Attribute("content");
						if (pNode->Attribute("from"))
							pItem->strSender = pNode->Attribute("from");
						if (pNode->Attribute("uid"))
							pItem->strReceiver = pNode->Attribute("uid");
						//保存历史记录
						IMsgMgr *pMgr = NULL;
						if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMsgMgr), (void **)&pMgr)))
						{
							char szTime[64] = {0};
							CSystemUtils::DateTimeToStr((DWORD)time(NULL), szTime);
							TiXmlString strXml;
							pNode->SaveToString(strXml, 0);
							pMgr->SaveMsg("sms", pItem->strSender.c_str(), pItem->strReceiver.c_str(), 
								szTime, strXml.c_str(), pItem->strContent.c_str());
							pMgr->Release();
						}
						::PostMessage(hWnd, WM_RECEIVESMS, 0, LPARAM(pItem));
					}
				}
			} //end if ((::stricmp
		}
	}
	return E_NOTIMPL;
}

STDMETHODIMP CSMSFrameImpl::DoPresenceChange(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder)
{
	return E_NOTIMPL;
}

//
BOOL CSMSFrameImpl::ShowUserListToInputFrame(HWND hWnd, IUserList *pList, const char *szMobiles)
{
	LPORG_TREE_NODE_DATA pData;
	std::string strSelList;
	std::string strTmp;
	BOOL bSucc = FALSE;
	BOOL bFax = FALSE;
	if (hWnd == m_hFaxWnd)
		bFax = TRUE;
	while (SUCCEEDED(pList->PopBackUserInfo(&pData)))
	{ 
		if (GetPhoneInfoByUserName(pData->szUserName, strTmp, bFax))
		{
			strSelList += strTmp;
			strSelList += ";";
		}
		if (pData->szDisplayName)
			delete []pData->szDisplayName;
		delete pData;
	}
	if (szMobiles)
		strSelList += szMobiles;
	if (!strSelList.empty())
	{
		TCHAR *szTmp = new TCHAR[strSelList.size() + 1];
		memset(szTmp, 0, sizeof(TCHAR) * (strSelList.size() + 1));
		CStringConversion::StringToWideChar(strSelList.c_str(), szTmp, strSelList.size());
		::SkinSetControlTextByName(hWnd, L"numbers", szTmp);
		delete []szTmp;
		bSucc = TRUE;
	} else
	{
		::SkinSetControlTextByName(hWnd, L"numbers", L"");
		bSucc = FALSE;
	}
	StatReceiverCount(hWnd);
	return bSucc;
}

//ISMSFrame
STDMETHODIMP CSMSFrameImpl::ShowSMSFrame(LPRECT lprc, IUserList *pList)
{
	if (m_hWnd && ::IsWindow(m_hWnd))
	{
		if (::ShowWindow(m_hWnd, SW_SHOW))
		{
			CSystemUtils::BringToFront(m_hWnd); 
			return S_OK;
		} else
			return E_FAIL;
	} else
	{
		if (m_hWnd)
			::SkinCloseWindow(m_hWnd);
		m_hWnd = NULL;
		HRESULT hr = E_FAIL;
		if (m_pCore)
		{
			IUIManager *pUI = NULL;
			IConfigure *pCfg = NULL;		
			hr = m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg);
			if (SUCCEEDED(hr) && pCfg)
			{
				RECT rc = {100, 100, 600, 530};
				if (lprc)
					rc = *lprc;
				else
				{
					RECT rcSave = {0};
					CInterfaceAnsiString strPos;
					if (SUCCEEDED(pCfg->GetParamValue(FALSE, "Position", "SMSFrame", (IAnsiString *)&strPos)))
					{
						CSystemUtils::StringToRect(&rcSave, strPos.GetData());
					}
					if (!::IsRectEmpty(&rcSave))
						rc = rcSave;
				}
				hr = m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI);
				if (SUCCEEDED(hr) && pUI)
				{
					pUI->CreateUIWindow(NULL, "SMSFrame", &rc, WS_POPUP | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX,
					                0, L"短信发送", &m_hWnd);				
					if (::IsWindow(m_hWnd))
					{
						 ::ShowWindow(m_hWnd, SW_SHOW);
						 if (pList)
						 {
							 ShowUserListToInputFrame(m_hWnd, pList, NULL); 
						 } //end if (pList)
					}
					pUI->OrderWindowMessage("SMSFrame", NULL, WM_DESTROY, (ICoreEvent *) this);
					//					  
					pUI->Release();
					pUI = NULL;
				}
				CInterfaceAnsiString strTmp;
				if (SUCCEEDED(m_pCore->GetUserNickName(&strTmp)))
				{
					TCHAR szwTmp[MAX_PATH] = {0};
					CStringConversion::UTF8ToWideChar(strTmp.GetData(), szwTmp, MAX_PATH - 1);
					::SkinSetControlTextByName(m_hWnd, L"smssign", szwTmp);
				}
				pCfg->Release();
				pCfg = NULL;
			} //end if (SUCCEEDED(hr)... 
		} //end if (m_pCore)
		return hr;
	}
	return E_FAIL;
}
 

STDMETHODIMP CSMSFrameImpl::ShowSmsView(const char *szUserName, const char *szType)
{
	HRESULT hr = E_FAIL;
	if (m_hViewWnd && ::IsWindow(m_hViewWnd))
	{
		if (::ShowWindow(m_hViewWnd, SW_SHOW))
		{
			CSystemUtils::BringToFront(m_hViewWnd); 
			return S_OK;
		} else
			return E_FAIL;
	} else
	{
		if (m_hWnd)
			::SkinCloseWindow(m_hViewWnd);
		m_hViewWnd = NULL;
		HRESULT hr = E_FAIL;
		if (m_pCore)
		{
			IUIManager *pUI = NULL;
			IConfigure *pCfg = NULL;		
			hr = m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg);
			if (SUCCEEDED(hr) && pCfg)
			{
				RECT rc = {100, 100, 600, 530};
				 
				RECT rcSave = {0};
				CInterfaceAnsiString strPos;
				if (SUCCEEDED(pCfg->GetParamValue(FALSE, "Position", "SMSView", (IAnsiString *)&strPos)))
				{
					CSystemUtils::StringToRect(&rcSave, strPos.GetData());
				}
				if (!::IsRectEmpty(&rcSave))
					rc = rcSave;
				
				hr = m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI);
				if (SUCCEEDED(hr) && pUI)
				{
					CStdString_ strCaption = L"短信消息记录";
					if (::stricmp(szType, "fax") == 0)
						strCaption = L"传真发送记录";
					pUI->CreateUIWindow(NULL, "SMSView", &rc, WS_POPUP | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX,
					                0,  strCaption, &m_hViewWnd);				
					if (::IsWindow(m_hViewWnd))
					{
						 ::ShowWindow(m_hViewWnd, SW_SHOW);
					}
					pUI->Release();
				} //end if (SUCCEEDED(hr) && pUI)
				pCfg->Release();
			} //end if (SUCCEEDED(hr) && pCfg)
			InitSmsView(szUserName, szType);
		} //end if (m_pCore)
	}
	return hr;
}


//
BOOL CSMSFrameImpl::InitSmsView(const char *szUserName, const char *szType)
{
	int nPageCount = 0;
	IMsgMgr *pMgr = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMsgMgr), (void **)&pMgr)))
	{
		CInterfaceAnsiString strTmp;
		m_strUserName = "";
		if (SUCCEEDED(m_pCore->GetUserName(&strTmp)))
		{
			m_strUserName = strTmp.GetData();
			m_strUserName += "@";
		}
		if (SUCCEEDED(m_pCore->GetUserDomain(&strTmp)))
		{
			m_strUserName += strTmp.GetData();
		}
		m_strShowType = szType;
		int nCount = pMgr->GetMsgCount(szType, m_strUserName.c_str());  
		m_nTotalPage = nCount / MESSAGE_PAGE_COUNT;		
		if ((nCount % MESSAGE_PAGE_COUNT) != 0)	 
			m_nTotalPage ++;
		nPageCount = m_nTotalPage;
		if (nPageCount < 0)
			nPageCount = 0;
		pMgr->Release();
	}
	if (m_nTotalPage > 0)
		DisplayHistoryMsg(nPageCount);
	return FALSE;
}

void CSMSFrameImpl::DisplayHistoryMsg(int nPageCount)
{
	if (nPageCount <= 0)
		::SkinMessageBox(m_hViewWnd, L"已经是第一页", L"提示", MB_OK);
	else if (nPageCount > m_nTotalPage)
		::SkinMessageBox(m_hViewWnd, L"已经是最后一页", L"提示", MB_OK);
	else
	{
		::SkinRemoveListItem(m_hViewWnd, L"smslist", -1);
		m_nCurrPage = nPageCount;
		IMsgMgr *pMgr = NULL;
		TCHAR szwTmp[40] = {0};
		wsprintf(szwTmp, L"%d", m_nCurrPage);
		::SkinSetControlTextByName(m_hViewWnd, L"inputpage", szwTmp);
		memset(szwTmp, 0, sizeof(TCHAR) * 40);
		wsprintf(szwTmp, L"%d 页", m_nTotalPage);
		::SkinSetControlTextByName(m_hViewWnd, L"currpage", szwTmp);
		if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMsgMgr), (void **)&pMgr)))
		{			
			CMessageList msgList;
			  
			 
			if (SUCCEEDED(pMgr->GetMsg(m_strShowType.c_str(), m_strUserName.c_str(), nPageCount, MESSAGE_PAGE_COUNT,
				     (IMessageList*)&msgList)))
			{
				DisplayMessageList((IMessageList *)&msgList);
			}
			   
			pMgr->Release();
		} //end if (m_pCore ...
	} //end else if (...
 
}

//获取当前选择的ID号
int  CSMSFrameImpl::GetCurrSmsMsgId(int &nUISel)
{
	nUISel = ::SkinGetListSelItem(m_hViewWnd, L"smslist");
	int nRet = -1;
	if (nUISel >= 0)
	{
		TCHAR szName[MAX_PATH] = {0};
		void *pData = NULL;
		if (::SkinGetListItemInfo(m_hViewWnd, L"smslist", szName, &pData, nUISel))
		{
			if (pData)
				nRet = (int)pData;
		}
	} //
	return nRet;
}

//删除当前选择的记录
void CSMSFrameImpl::DeleteCurrSmsMsg()
{
	int nUISel = 0;
	int nSelId = GetCurrSmsMsgId(nUISel);
	if (nSelId > 0)
	{
		IMsgMgr *pMgr = NULL;
		if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMsgMgr), (void **)&pMgr)))
		{ 
			if (SUCCEEDED(pMgr->ClearMsg(nSelId, m_strShowType.c_str(), m_strUserName.c_str())))
			{
				ITrayMsg *pTray = NULL;
				if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(ITrayMsg), (void **)&pTray)))
				{
					pTray->ShowTipPanel("删除短信", "删除当前短信记录成功");
					pTray->Release();
				} 
			}
			::SkinDeleteListItem(m_hViewWnd, L"smslist", nUISel);
			pMgr->Release();
		}	
	} else
	{
		::SkinMessageBox(m_hViewWnd, L"请选择要删除的记录", L"提示", MB_OK);
	}
}

//查看当前选择的记录
void CSMSFrameImpl::ViewCurrSmsMsg()
{
	int nUISel = 0;
	int nSelId = GetCurrSmsMsgId(nUISel);
	if (nSelId > 0)
	{
		IMsgMgr *pMgr = NULL;
		if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMsgMgr), (void **)&pMgr)))
		{			
			CMessageList msgList;
			  
			 
			if (SUCCEEDED(pMgr->GetMsgById(m_strShowType.c_str(), nSelId,  
				     (IMessageList*)&msgList)))
			{
				int nIdx = 0;
				int nMsgId;
				CInterfaceAnsiString strMsg, strType, strFromName, strToName, strTime;
			    std::string strDspText;
				if (SUCCEEDED(msgList.GetMsg(nIdx, &nMsgId, &strType, &strFromName, &strToName,
					&strTime, (IAnsiString *)&strMsg)))
				{
					ShowSMSContent(strFromName.GetData(), strTime.GetData(), strMsg.GetData());
				}
			} //end if (SUCEEDED(..
			pMgr->Release();
		} //end if (m_pCore->
	} else
	{
		::SkinMessageBox(m_hViewWnd, L"请选择要查看的记录", L"提示", MB_OK);
	}
}

void CSMSFrameImpl::DisplayMessageList(IMessageList *pList)
{  
	int nIdx = 0;
	int nMsgId;
	CInterfaceAnsiString strMsg, strType, strFromName, strToName, strTime;
    std::string strDspText;
	while (SUCCEEDED(pList->GetMsg(nIdx, &nMsgId, &strType, &strFromName, &strToName,
		&strTime, (IAnsiString *)&strMsg)))
	{ 
		int nItemIdx = ::SkinAppendListItem(m_hViewWnd, L"smslist", strToName.GetData(), (void *)nMsgId);
		if (nItemIdx >= 0)
		{
			::SkinAppendListSubItem(m_hViewWnd, L"smslist", nItemIdx, 1, strMsg.GetData());
			::SkinAppendListSubItem(m_hViewWnd, L"smslist", nItemIdx, 2, strTime.GetData()); 
		}
		nIdx ++;
	}
		 
}

//
STDMETHODIMP CSMSFrameImpl::ShowFaxFrame(LPRECT lprc, IUserList *pList)
{
	if (m_hFaxWnd && ::IsWindow(m_hFaxWnd))
	{
		if (::ShowWindow(m_hFaxWnd, SW_SHOW))
		{
			CSystemUtils::BringToFront(m_hFaxWnd); 
			return S_OK;
		} else
			return E_FAIL;
	} else
	{
		if (m_hFaxWnd)
			::SkinCloseWindow(m_hFaxWnd);
		m_hFaxWnd = NULL;
		HRESULT hr = E_FAIL;
		if (m_pCore)
		{
			IUIManager *pUI = NULL;
			IConfigure *pCfg = NULL;		
			hr = m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg);
			if (SUCCEEDED(hr) && pCfg)
			{
				RECT rc = {100, 100, 600, 530};
				if (lprc)
					rc = *lprc;
				else
				{
					RECT rcSave = {0};
					CInterfaceAnsiString strPos;
					if (SUCCEEDED(pCfg->GetParamValue(FALSE, "Position", "FaxFrame", (IAnsiString *)&strPos)))
					{
						CSystemUtils::StringToRect(&rcSave, strPos.GetData());
					}
					if (!::IsRectEmpty(&rcSave))
						rc = rcSave;
				}
				hr = m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI);
				if (SUCCEEDED(hr) && pUI)
				{
					pUI->CreateUIWindow(NULL, "FaxFrame", &rc, WS_POPUP | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX,
					                0, L"", &m_hFaxWnd);				
					if (::IsWindow(m_hFaxWnd))
					{
						 ::ShowWindow(m_hFaxWnd, SW_SHOW);
						 if (pList)
						 {
							 ShowUserListToInputFrame(m_hFaxWnd, pList, NULL); 
						 } //end if (pList)
					}
					pUI->OrderWindowMessage("FaxFrame", m_hFaxWnd, WM_DESTROY, (ICoreEvent *) this);
					//					  
					pUI->Release();
					pUI = NULL;
				}
				CInterfaceAnsiString strTmp;
				if (SUCCEEDED(m_pCore->GetUserNickName(&strTmp)))
				{
					TCHAR szwTmp[MAX_PATH] = {0};
					CStringConversion::UTF8ToWideChar(strTmp.GetData(), szwTmp, MAX_PATH - 1);
					::SkinSetControlTextByName(m_hFaxWnd, L"smssign", szwTmp);
				}
				pCfg->Release();
				pCfg = NULL;
			} //end if (SUCCEEDED(hr)... 
		} //end if (m_pCore)
		return hr;
	}
	return E_FAIL;
}

//
void CSMSFrameImpl::ShowSMSContent(const char *szSender, const char *szTime, const char *szContent)
{
	if (m_hShowWnd)
	{
		if (!::IsWindow(m_hShowWnd))
		{
			::SkinCloseWindow(m_hShowWnd);
			m_hShowWnd = NULL;
		}
	}
	if (m_hShowWnd == NULL)
	{
		IUIManager *pUI = NULL;
		HRESULT hr = m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI);
		if (SUCCEEDED(hr) && pUI)
		{
			RECT rc = {0};
			CSystemUtils::GetScreenCenterRect(300, 200, rc);
			pUI->CreateUIWindow(NULL, "smsreceiver", &rc, WS_POPUP | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX,
			                0, L"短信消息", &m_hShowWnd);				
			 
			pUI->Release();
		}
	}
	if (m_hShowWnd)
	{
		//
		TCHAR szwSender[MAX_PATH] = {0};
		CStringConversion::StringToWideChar(szSender, szwSender, MAX_PATH - 1);
		int n = ::strlen(szContent);
		TCHAR *szwContent = new TCHAR[n + 1];
		memset(szwContent, 0, sizeof(TCHAR) * (n + 1));
		CStringConversion::StringToWideChar(szContent, szwContent, n);
		::SkinSetControlTextByName(m_hShowWnd, L"smssender", szwSender);
		::SkinSetControlAttr(m_hShowWnd, L"smscontent", L"text", szwContent);
		CSystemUtils::BringToFront(m_hShowWnd);
	}
}

#pragma warning(default:4996)
