#include <SmartSkin/smartskin.h> 
#include <Commonlib/systemutils.h>
#include <Commonlib/DebugLog.h>
#include <Crypto/crypto.h>
#include "FreContactsImpl.h"
#include <Core/common.h>
#include <Core/treecallbackfun.h>
#include <Commonlib/SqliteDBOP.h>
#include <Commonlib/VCardParser.h>
#include "resource.h"
#include "../IMCommonLib/InstantUserInfo.h"
#include "../IMCommonLib/InterfaceAnsiString.h"
#pragma warning(disable:4996)

#define MODIFY_REMARK_NAME_OK 2  //备注联系人确定按钮值

typedef struct
{
	std::string strUserName;
	std::string strPresence;
	std::string strMemo;
	BOOL bOrder;
}FRE_CONTACT_PRESENCE_NOTIFY_ITEM, *LPFRE_CONTACT_PRESENCE_NOTIFY_ITEM;


#define PERSON_USER_INFO_WIDTH  420
#define PERSON_USER_INFO_HEIGHT 320
CFreContactsImpl::CFreContactsImpl(void):
                  m_pCore(NULL),
				  m_hWndMain(NULL),
				  m_hExtContact(NULL),
				  m_bContactInit(FALSE),
				  m_pParentNode(NULL),
				  m_nPendId(1)
{
}


CFreContactsImpl::~CFreContactsImpl(void)
{
	if (m_hExtContact)
		::SkinCloseWindow(m_hExtContact);
	m_hExtContact = NULL;
	if (m_pCore)
	{
		m_pCore->Release();
	}
	m_pCore = NULL;
}

//IUnknown
STDMETHODIMP CFreContactsImpl::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IFreContacts)))
	{
		*ppv = (IFreContacts *) this;
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

void CFreContactsImpl::ReCreateFreMenu()
{
	::SkinDestroyMenu(m_hWndMain, L"fresubmenu"); 
	IContacts *pContact = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
	{ 
		::SkinMenuAppendItem(m_hWndMain, L"fresubmenu", 0, L"常用联系人", 0x80000001);
		pContact->LoadMenuFromExtractDept(m_hWndMain, L"fresubmenu", "1", FREQUENCY_CONTACT_TYPE_ID);
		pContact->Release();
	}
}

HRESULT CFreContactsImpl::DoMenuCommand(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szName, "freleafmenu") == 0)
	{
		switch(wParam)
		{
		case 4: //发送文件
			{
				std::string strUserName;
				BOOL bGroup;
				if (GetUserNameByTree(hWnd, L"frecontacttree", strUserName, bGroup))
				{
					IChatFrame *pFrame = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pFrame)))
					{
						pFrame->SendFileToPeer(strUserName.c_str(), NULL);
						pFrame->Release();
					} //end if (SUCCEEDED(
				} else
				{
					if (!bGroup)
						::SkinMessageBox(hWnd, L"无法给外部联系人发送文件", L"提示", MB_OK);
				}//end if (GetUserNameByTree(
			} //end 
			break;
		case 5: //详细资料
			{
				void *pSelNode = NULL;
				LPORG_TREE_NODE_DATA pSelData = NULL;
				TCHAR szName[MAX_PATH] = {0};
				int nSize = MAX_PATH - 1;
				CTreeNodeType tnType;
				if (::SkinGetSelectTreeNode(hWnd, L"frecontacttree", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
				{
					if (tnType == TREENODE_TYPE_LEAF)
					{
						if (pSelData)
						{	
							std::string strUserName = pSelData->szUserName;
							if (strUserName.find("@") == std::string::npos)
							{
								ViewExtContactCard(pSelData->szUserName); 
							} //end if (strUserName.find("@") 
						} //end if (pSelData)
					} //end if (tnType == TREENODE_TYPE_LEAF)
				} //end if (::SkinGetSelectTreeNode( 
			}
			break;
		case 6: //远程协助
			{
				std::string strUserName;
				BOOL bGroup;
				if (GetUserNameByTree(hWnd, L"frecontacttree", strUserName, bGroup))
				{
					IChatFrame *pFrame = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pFrame)))
					{
						pFrame->SendRmcRequest(strUserName.c_str());
						pFrame->Release();
					} //end if (SUCCEEDED(
				} else
				{
					if (!bGroup)
						::SkinMessageBox(hWnd, L"无法给外部联系人发送远程协助", L"提示", MB_OK);
				} //end if (GetUserNameByTree(
			} //end 
			break;
		case 7: //视频会话
			{
				std::string strUserName;
				BOOL bGroup;
				if (GetUserNameByTree(hWnd, L"frecontacttree", strUserName, bGroup))
				{
					IChatFrame *pFrame = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pFrame)))
					{
						pFrame->SendVideoRequest(strUserName.c_str());
						pFrame->Release();
					} //end if (SUCCEEDED(
				}  //end if (GetUserNameByTree(
			} //end 
			break;
		case 8: //音频会话
			{
				std::string strUserName;
				BOOL bGroup;
				if (GetUserNameByTree(hWnd, L"frecontacttree", strUserName, bGroup))
				{
					IChatFrame *pFrame = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pFrame)))
					{
						pFrame->SendAudioRequest(strUserName.c_str());
						pFrame->Release();
					} //end if (SUCCEEDED(
				}  //end if (GetUserNameByTree(
			} //end 
			break;
		case 9: //删除联系人
			{
				void *pSelNode = NULL;
				LPORG_TREE_NODE_DATA pSelData = NULL;
				TCHAR szName[MAX_PATH] = {0};
				int nSize = MAX_PATH - 1;
				CTreeNodeType tnType;
				if (::SkinGetSelectTreeNode(hWnd, L"frecontacttree", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
				{
					if (tnType == TREENODE_TYPE_LEAF)
					{
						if (pSelData)
						{	
							char szTmp[16] = {0};
							::itoa(pSelData->id, szTmp, 10);
							DeleteFreContactUser(szTmp, pSelData->szUserName);
						} //end if (pSelData)
					} //end i f(tnType
				} //end if (::SkinGetSelectTreeNode(..
			} //end 
			break;
		case 10: //备注联系人
			DoRemarkContact(hWnd);
			break;
		case 11: //添加分组
			m_pParentNode = NULL;
			DoAddDept("0");
			break;
		case 17: //查看消息记录
			{
				void *pSelNode = NULL;
				LPORG_TREE_NODE_DATA pSelData = NULL;
				TCHAR szName[MAX_PATH] = {0};
				int nSize = MAX_PATH - 1;
				CTreeNodeType tnType;
				if (::SkinGetSelectTreeNode(hWnd, L"frecontacttree", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
				{
					if (tnType == TREENODE_TYPE_LEAF)
					{
						if (pSelData)
						{	
							IMsgMgrUI *pFrame = NULL;
							if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMsgMgrUI), (void **)&pFrame)))
							{ 
								pFrame->ShowMsgMgrFrame("p2p", pSelData->szUserName, NULL); 
								pFrame->Release();
							} //end if (SUCCEEDED(
						} //end if (pSelData)
					} //end i f(tnType
				} //end if (::SkinGetSelectTreeNode(..
				break;
			}
		case 18: //发送短信
			{			
				void *pSelNode = NULL;
				LPORG_TREE_NODE_DATA pSelData = NULL;
				TCHAR szName[MAX_PATH] = {0};
				int nSize = MAX_PATH - 1;
				CTreeNodeType tnType;
				if (::SkinGetSelectTreeNode(hWnd, L"frecontacttree", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
				{
					if (tnType == TREENODE_TYPE_LEAF)
					{
						if (pSelData)
						{	
							ISMSFrame *pFrame = NULL;
							if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(ISMSFrame), (void **)&pFrame)))
							{
								CInterfaceUserList ulList;
								ulList.AddUserInfo(pSelData, TRUE, TRUE);
								pFrame->ShowSMSFrame(NULL, &ulList);
								pFrame->Release();
							} //end if (SUCCEEDED(
						} //end if (pSelData)
					} //end i f(tnType
				} //end if (::SkinGetSelectTreeNode(..
				break;
			}
		case 19: //发送传真
			{
				void *pSelNode = NULL;
				LPORG_TREE_NODE_DATA pSelData = NULL;
				TCHAR szName[MAX_PATH] = {0};
				int nSize = MAX_PATH - 1;
				CTreeNodeType tnType;
				if (::SkinGetSelectTreeNode(hWnd, L"frecontacttree", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
				{
					if (tnType == TREENODE_TYPE_LEAF)
					{
						if (pSelData)
						{	
							ISMSFrame *pFrame = NULL;
							if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(ISMSFrame), (void **)&pFrame)))
							{
								CInterfaceUserList ulList;
								ulList.AddUserInfo(pSelData, TRUE, TRUE);
								pFrame->ShowFaxFrame(NULL, &ulList);
								pFrame->Release();
							} //end if (SUCCEEDED(
						} //end if (pSelData)
					} //end i f(tnType
				} //end if (::SkinGetSelectTreeNode(..
				break;
			}
		case 20: //发送即时消息
			OpenChatFrameByTree(hWnd, L"frecontacttree");
			break;
		case 21: //发送邮件
			{
				void *pSelNode = NULL;
				LPORG_TREE_NODE_DATA pSelData = NULL;
				TCHAR szName[MAX_PATH] = {0};
				int nSize = MAX_PATH - 1;
				CTreeNodeType tnType;
				if (::SkinGetSelectTreeNode(hWnd, L"frecontacttree", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
				{
					if (tnType == TREENODE_TYPE_LEAF)
					{
						if (pSelData)
						{	
							IContacts *pContact = NULL; 
							if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
							{
								CInterfaceAnsiString strMail;
								if (SUCCEEDED(pContact->GetMailByUserName(pSelData->szUserName, &strMail)))
								{
									if (strMail.GetSize() > 0)
									{
										std::string strOpenString = "mailto:";
										strOpenString += strMail.GetData();
										TCHAR szTmp[MAX_PATH] = {0};
										CStringConversion::StringToWideChar(strOpenString.c_str(), szTmp, MAX_PATH - 1);
										::ShellExecute(NULL, L"open", szTmp, NULL, NULL, SW_SHOW); 
									} else
									{
										::SkinMessageBox(hWnd, L"对方没有预留邮箱帐号，请与对方确认邮箱帐号后再发送邮件", L"提示", MB_OK);
									} //end else if (strMail.
								} //end 
								pContact->Release();
							} //end if (SUCCEEDED(
						} //end if (pSelData)
					} //end i f(tnType
				} //end if (::SkinGetSelectTreeNode(..
				break;
			}
		}
	} else if (::stricmp(szName, "treeleaf") == 0)
	{
		if (wParam >= 0x80000000)
		{
			UINT uMenuId = wParam & 0x7FFFFFFF;
 
			void *pSelNode = NULL;
			LPORG_TREE_NODE_DATA pSelData = NULL;
			TCHAR szName[MAX_PATH] = {0};
			int nSize = MAX_PATH - 1;
			CTreeNodeType tnType;
			if (::SkinGetSelectTreeNode(hWnd, L"colleaguetree", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
			{
				if (tnType == TREENODE_TYPE_LEAF)
				{
					int nId = IsExistsFreContact(pSelData->szUserName);
					if (nId > 0)
					{
						char szTmp[16] = {0};
						::itoa(nId, szTmp, 10);
						DeleteFreContactUser(szTmp, pSelData->szUserName);
					} else
					{
						char szTmp[MAX_PATH] = {0};
						CStringConversion::WideCharToString(szName, szTmp, MAX_PATH - 1);
						char szDeptId[16] = {0};
						::itoa(uMenuId, szDeptId, 10);
						m_pParentNode = NULL;
						AddFreContactUser("0", pSelData->szUserName, szTmp, szTmp, szDeptId);
					}
				}
			} //end if (.. 
		}
	} else if (::stricmp(szName, "fregroupmenu") == 0)
	{
		switch(wParam)
		{
		case 1: 
			{
				void *pSelNode = NULL;
				LPORG_TREE_NODE_DATA pSelData = NULL;
				TCHAR szName[MAX_PATH] = {0};
				int nSize = MAX_PATH - 1;
				CTreeNodeType tnType;
				if (::SkinGetSelectTreeNode(hWnd, L"frecontacttree", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
				{
					if (tnType == TREENODE_TYPE_GROUP)
					{
						if (pSelData)
						{	
							char szTmp[16] = {0};
							::itoa(pSelData->id, szTmp, 10);
							m_pParentNode = pSelNode;
			                DoAddDept(szTmp);
						} //end if (pSelData)
					} //end i f(tnType
				} //end if (::SkinGetSelectTreeNode(.. 
			    break;
			} 
		case 100:
			{
				void *pSelNode = NULL;
				LPORG_TREE_NODE_DATA pSelData = NULL;
				TCHAR szName[MAX_PATH] = {0};
				int nSize = MAX_PATH - 1;
				CTreeNodeType tnType;
				if (::SkinGetSelectTreeNode(hWnd, L"frecontacttree", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
				{
					if (tnType == TREENODE_TYPE_GROUP)
					{ 
						DWORD dwTotalCount = 0, dwOnlineCount = 0;
						::SkinTVGetOnlineCount(hWnd, L"frecontacttree", pSelNode, &dwTotalCount, &dwOnlineCount);
						if (dwTotalCount == 0)
						{
							if (pSelData)
							{	
								char szTmp[16] = {0};
								::itoa(pSelData->id, szTmp, 10); 
								DeleteFreContactDept(szTmp, pSelData->szUserName); 
							} //end if (pSelData)
						} else
						{
							::SkinMessageBox(hWnd, L"分组下存在用户，不能删除", L"提示", MB_OK);
						}
					} //end i f(tnType
				} //end if (::SkinGetSelectTreeNode(.. 
			    break;
			} //end 
		case 101: //添加外部联系人
			{
				void *pSelNode = NULL;
				LPORG_TREE_NODE_DATA pSelData = NULL;
				TCHAR szName[MAX_PATH] = {0};
				int nSize = MAX_PATH - 1;
				CTreeNodeType tnType;
				if (::SkinGetSelectTreeNode(hWnd, L"frecontacttree", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
				{
					if (tnType == TREENODE_TYPE_GROUP)
					{  
						if (pSelData)
							m_nParentId = pSelData->id;
						m_pParentNode = pSelNode;  
					} //end i f(tnType
				} else
					m_pParentNode = NULL;//end if (::SkinGetSelectTreeNode(..  
				DoAddExtContact(hWnd);
				break;
			}
		case 102: //导入外部联系人
			{
				CStringList_ FileList;
				if (CSystemUtils::OpenFileDialog(NULL, hWnd, "选择要导入的文件", "联系人数据文件(*.db)|*.db", 
					                NULL, FileList, FALSE, FALSE))
				{ 
					if (!FileList.empty())
						ImportContactFromLocal(FileList.back().c_str());
				}
				break;
			}
		case 103: //导入电话本
			{
				CStringList_ FileList;
				if (CSystemUtils::OpenFileDialog(NULL, hWnd, "选择要导入的文件", "电话本文件(*.vcf)|*.vcf", 
					                NULL, FileList, FALSE, FALSE))
				{ 
					if (!FileList.empty())
						ImportContactFromVCF(FileList.back().c_str());
				}
				break;
			}
		case 80001: //发送广播
			{
				CInterfaceUserList ulList;
				if (GetCurrentChildNode(ulList))
				{
					IBCFrame *pFrame = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IBCFrame), (void **)&pFrame)))
					{
						pFrame->ShowBCFrame(L"发送广播消息", NULL, 1, &ulList);
						pFrame->Release();
					}
				}
				break;
			}
		case 80002: //群发文件
			{
				CInterfaceUserList ulList;
				if (GetCurrentChildNode(ulList))
				{
					IBCFrame *pFrame = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IBCFrame), (void **)&pFrame)))
					{
						pFrame->ShowBCFrame(L"群发文件", NULL, 3, &ulList);
						pFrame->Release();
					}
				}
				break;
			}
		}//switch 100
		
	} else if (::stricmp(szName, "fredefaultpopmenu") == 0) 
	{
		switch(wParam)
		{
		case 1: //添加外部联系人
			m_pParentNode = NULL;
			DoAddExtContact(hWnd);
			break;
		}
	}
	return -1;
}

BOOL CFreContactsImpl::ImportContactFromVCF(const char *szFileName)
{
	//
	CVCardParser vp;
	VCardList vcList;
	if (vp.LoadFromFile(szFileName, vcList))
	{
		while (!vcList.empty())
		{
			IMPORT_USER_ITEM item;
			item.strFirstName = vcList.top().m_strName;
			item.strInfoName = vcList.top().m_strName; 
			item.strOfficeMobil = vcList.top().m_strTel;
			item.strPersonMobile = vcList.top().m_strTel;
			item.strOfficeEmail = vcList.top().m_strEmail;
			item.strPersonEmail = vcList.top().m_strEmail;
			DoAddImporUser(TEL_CONTACT_DEPT_ID, &item);
			vcList.pop();
		}
	} //end if (vp.LoadFromFile(szFileName,...
	return FALSE;
}

BOOL CFreContactsImpl::GetCurrentChildNode(CInterfaceUserList &ulList)
{
	void *pSelNode = NULL;
	LPORG_TREE_NODE_DATA pSelData = NULL;
	TCHAR szName[MAX_PATH] = {0};
	int nSize = MAX_PATH - 1;
	CTreeNodeType tnType;
	if (::SkinGetSelectTreeNode(m_hWndMain, L"frecontacttree", szName, 
		&nSize, &pSelNode, &tnType, (void **)&pSelData))
	{
		if (tnType == TREENODE_TYPE_GROUP)
		{
			char szUserList[2048] = {0};
			int nSize = 2047;
			if (::SkinGetNodeChildUserList(m_hWndMain, pSelNode, szUserList, &nSize, 
		                      FALSE))
			{  
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
							ulList.AddUserInfo(pData, FALSE, TRUE);
							pNode = pNode->NextSiblingElement();
						}
						return TRUE;
					} //end if (xml.Load(...
				}
			}
		}
	}
	return FALSE;
}

//
void CFreContactsImpl::SetExtContactEnable(BOOL bEnabled)
{
	if ((m_hExtContact != NULL) && (::IsWindow(m_hExtContact)))
	{
		::SkinSetControlEnable(m_hExtContact, L"edtrealname", bEnabled);
		::SkinSetControlEnable(m_hExtContact, L"edtcorpname", bEnabled);
		::SkinSetControlEnable(m_hExtContact, L"edtcorpdeptname", bEnabled);
		::SkinSetControlEnable(m_hExtContact, L"edtmobilephone", bEnabled);
		::SkinSetControlEnable(m_hExtContact, L"edttel", bEnabled);
		::SkinSetControlEnable(m_hExtContact, L"edtcell", bEnabled);
		::SkinSetControlEnable(m_hExtContact, L"edtEmail", bEnabled);
		::SkinSetControlEnable(m_hExtContact, L"edtfax", bEnabled);
		::SkinSetControlEnable(m_hExtContact, L"edtprovince", bEnabled);
		::SkinSetControlEnable(m_hExtContact, L"edtfamilytel", bEnabled);
		::SkinSetControlEnable(m_hExtContact, L"edtcity", bEnabled);
		::SkinSetControlEnable(m_hExtContact, L"edtpostcode", bEnabled);
		::SkinSetControlEnable(m_hExtContact, L"edtaddress", bEnabled);
		::SkinSetControlEnable(m_hExtContact, L"edthomepage", bEnabled);
		::SkinSetControlVisible(m_hExtContact, L"addcontact_ok", bEnabled);
	}
}

 

//
void CFreContactsImpl::ViewExtContactCard(const char *szUserName)
{
	if ((m_hExtContact != NULL) && (!::IsWindow(m_hExtContact)))
	{
		::SkinCloseWindow(m_hExtContact);
		m_hExtContact = NULL;
	}
	if (m_hExtContact ==  NULL)
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
			pUI->CreateUIWindow(NULL, "extcontactwindow", &rc, WS_POPUP | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX
				                | WS_MINIMIZEBOX, 0, L"用户详细资料", &hTemp);
			if (hTemp && ::IsWindow(hTemp))
			{
				m_hExtContact = hTemp;
			}
		} //end if (SUCCEEDED(
	}
	if (m_hExtContact && (::IsWindow(m_hExtContact)))
	{ 
		::ShowWindow(m_hExtContact, SW_SHOW);
		CSystemUtils::BringToFront(m_hExtContact);
		SetExtContactEnable(FALSE);
	}
	UpdateFromServer(szUserName);
}

BOOL CFreContactsImpl::UpdateFromServer(const char *szUserName)
{
	if (szUserName)
	{
		std::string strXml = "<contacts type=\"getextcard\" uid=\""; 
		strXml += szUserName;
		strXml += "\"/>";
		if (m_pCore)
			return SUCCEEDED(m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (int) strXml.size(), 0));
	}
	return FALSE;
}

//添加外部联系人
BOOL CFreContactsImpl::DoAddExtContact(HWND hWnd)
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
		pUI->CreateUIWindow(NULL, "extcontactwindow", &rc, WS_POPUP | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX
			                | WS_MINIMIZEBOX, 0, L"用户详细资料", &hTemp);
		if (hTemp && ::IsWindow(hTemp))
		{
			m_hExtContact = hTemp;
			::ShowWindow(m_hExtContact, SW_SHOW);
			CSystemUtils::BringToFront(m_hExtContact); 
		}
	}
	return FALSE;
}

//
BOOL CFreContactsImpl::DoAddDept(const char *szParentId)
{
	BOOL bSucc = FALSE;
    //
	IUIManager *pUI = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
	{
		HWND hTmp = NULL;
		RECT rcScreen = {0};
		CSystemUtils::GetScreenRect(&rcScreen);

		RECT rc = {0};
		rc.left = (rcScreen.right - 260) / 2;
		rc.top = (rcScreen.bottom - 120) / 2;
		rc.right = rc.left + 260;
		rc.bottom = rc.top + 120;
		pUI->CreateUIWindow(m_hWndMain, "remarkcontactwindow",  &rc, WS_POPUP | WS_SYSMENU,
	                0, L"添加分组", &hTmp);	
		if (hTmp != NULL)
		{
			::SkinSetControlTextByName(hTmp, L"lbltip", L"分组名称");
			::SkinSetControlTextByName(hTmp, L"edtnewremark", L"分组名称"); 
			m_strRemarkName.Empty();
			if ((::SkinShowModal(hTmp) == MODIFY_REMARK_NAME_OK) && (!m_strRemarkName.IsEmpty()))
			{ 
				char szTmp[MAX_PATH] = {0};
				CStringConversion::WideCharToString(m_strRemarkName.GetData(), szTmp, MAX_PATH - 1);
				AddFreContactDept("0", szTmp, szParentId, "0");
				bSucc = TRUE;
			} //end if (::SkinShowModal(hTmp)
		} //end if (hTmp != NULL) 
	    pUI->Release();
	} //end if (m_pCore && SUCCEEDED(
	return bSucc;
}

BOOL CFreContactsImpl::DoRemarkContact(HWND hWnd)
{
	BOOL bSucc = FALSE;
	void *pSelNode = NULL;
	LPORG_TREE_NODE_DATA pSelData = NULL;
	TCHAR szName[MAX_PATH] = {0};
	int nSize = MAX_PATH - 1;
	CTreeNodeType tnType;
	if (::SkinGetSelectTreeNode(hWnd, L"frecontacttree", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
	{
		if (tnType == TREENODE_TYPE_LEAF)
		{
			if (pSelData)
			{
				//
				IUIManager *pUI = NULL;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
				{
					HWND hTmp = NULL;
					RECT rcScreen = {0};
					CSystemUtils::GetScreenRect(&rcScreen);

					RECT rc = {0};
					rc.left = (rcScreen.right - 260) / 2;
					rc.top = (rcScreen.bottom - 120) / 2;
					rc.right = rc.left + 260;
					rc.bottom = rc.top + 120;
					pUI->CreateUIWindow(hWnd, "remarkcontactwindow",  &rc, WS_POPUP | WS_SYSMENU,
				                0, L"备注联系人", &hTmp);	
					if (hTmp != NULL)
					{
						::SkinSetControlTextByName(hTmp, L"lbltip", L"联系人备注");
						::SkinSetControlTextByName(hTmp, L"edtnewremark", szName); 
						m_strRemarkName.Empty();
						if ((::SkinShowModal(hTmp) == MODIFY_REMARK_NAME_OK) && (!m_strRemarkName.IsEmpty()))
						{ 
							char szRemark[MAX_PATH] = {0};
							char szId[16] = {0}; 
							CStringConversion::WideCharToString(m_strRemarkName.GetData(), szRemark, MAX_PATH - 1);
							::itoa(pSelData->id, szId, 10); 
							UpdateFreContactRemark(szId, szRemark);
						    ::SkinAdjustTreeNode(hWnd, L"frecontacttree", NULL, m_strRemarkName.GetData(), 
								TREENODE_TYPE_LEAF,  pSelData, TRUE, TRUE);
							::SkinUpdateControlUI(hWnd, L"frecontacttree");
							bSucc = TRUE;
						} //end if (::SkinShowModal(hTmp)
					} //end if (hTmp != NULL) 
				    pUI->Release();
				} //end if (m_pCore && SUCCEEDED(
			} //end if (pSelData..
		} //end if (tnType == 
	} //end if (::SkinGetSelectTreeNode(...
	return bSucc;
}

//ICoreEvent
STDMETHODIMP CFreContactsImpl::DoCoreEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
                     LPARAM lParam, HRESULT *hResult)
{
	if (::stricmp(szType, "click") == 0)
	{
		 if (::stricmp(szName, "ok") == 0)
		 {
			TCHAR szNewName[256] = {0};
			int nSize = 255;
			if (::SkinGetControlTextByName(hWnd, L"edtnewremark", szNewName, &nSize))
			{ 
				m_strRemarkName = szNewName;
				if (!m_strRemarkName.IsEmpty())
				{
					::SkinSetModalValue(hWnd, MODIFY_REMARK_NAME_OK);
					::SkinCloseWindow(hWnd);
				} else
				{
					::SkinMessageBox(hWnd, L"联系人备注不能为空", L"提示", MB_OK);
				}
			} else
			{
				::SkinMessageBox(hWnd, L"联系人备注不能为空", L"提示", MB_OK);
			}
		 } else if (::stricmp(szName, "cancel") == 0)
		 {
			 ::SkinCloseWindow(hWnd);
		 } else if (::stricmp(szName, "addcontact_ok") == 0)
		 {
			 DoAddContactToSvr(hWnd);
		 } else if (::stricmp(szName, "edit_contacts") == 0)
		 {
			 SetExtContactEnable(TRUE);
			 ::SkinSetControlTextByName(hWnd, L"addcontact_ok", L"修改");
			 ::SkinSetControlVisible(hWnd, L"edit_contacts", FALSE);
		 }
	} else if (::stricmp(szType, "afterinit") == 0)
	{
		if (::stricmp(szName, "mainwindow") == 0)
		{
			m_hWndMain = hWnd;
			IUIManager *pUI = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
			{
				pUI->OrderWindowMessage("mainwindow", NULL, WM_CONTACTS_DL_COMP, (ICoreEvent *) this);
				pUI->OrderWindowMessage("mainwindow", NULL, WM_CONTACTS_SVR_ACK, (ICoreEvent *) this);
				pUI->OrderWindowMessage("mainwindow", NULL, WM_FREPRESENCECHG, (ICoreEvent *) this); 
				pUI->OrderWindowMessage("mainwindow", NULL, WM_FRECNT_DETAIL, (ICoreEvent *) this);
			 
				pUI->Release();
			} //end if (SUCCEEDED(..
			::SkinSetGetKeyFun(hWnd, L"frecontacttree", GetTreeNodeKey);
			::SkinSetFreeNodeDataCallBack(hWnd, L"frecontacttree", FreeTreeNodeData);
		} //end if (::stricmp(szName...
	} else if (::stricmp(szType, "lbdblclick") == 0)
	{
		if (::stricmp(szName, "frecontacttree") == 0)
		{
			OpenChatFrameByTree(hWnd, L"frecontacttree");
		}
	} else if (::stricmp(szType, "menucommand") == 0)
	{
		*hResult = DoMenuCommand(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "initmenupopup") == 0)
	{
		*hResult = DoInitMenuPopup(hWnd, szName, wParam, lParam);
	}
	return E_NOTIMPL;
}

void CFreContactsImpl::UIMapToXml(TiXmlElement *pNode, const char *szAttrName, const TCHAR *szUIName)
{
	TCHAR szText[MAX_PATH] = {0};
	int nSize = MAX_PATH - 1;
	char szTmp[MAX_PATH] = {0};
	if (::SkinGetControlTextByName(m_hExtContact, szUIName, szText, &nSize))
	{
		CStringConversion::WideCharToString(szText, szTmp, MAX_PATH);
		pNode->SetAttribute(szAttrName, szTmp);
	}
}

//
void CFreContactsImpl::ClearInfo(HWND hWnd)
{
	::SkinSetControlTextByName(hWnd, L"edtrealname", L"");
	::SkinSetControlTextByName(hWnd, L"edtmobilephone", L"");
	::SkinSetControlTextByName(hWnd, L"edtEmail", L"");
    ::SkinSetControlTextByName(hWnd, L"edtcell", L"");
	::SkinSetControlTextByName(hWnd, L"edtfax", L"");
	::SkinSetControlTextByName(hWnd, L"edttel", L"");
    ::SkinSetControlTextByName(hWnd, L"edtprovince", L"");
	::SkinSetControlTextByName(hWnd, L"edtcity", L"");
    ::SkinSetControlTextByName(hWnd, L"edtcorpname", L"");
	::SkinSetControlTextByName(hWnd, L"edtcorpdeptname", L"");
	::SkinSetControlTextByName(hWnd, L"edtfamilytel", L"");
	::SkinSetControlTextByName(hWnd, L"edtpostcode", L"");
	::SkinSetControlTextByName(hWnd, L"edtaddress", L"");
	::SkinSetControlTextByName(hWnd, L"edthomepage", L""); 
}

//添加外部联系人至服务器
BOOL CFreContactsImpl::DoAddContactToSvr(HWND hWnd)
{  
	// "<contacts type="addextcontact" uid="wxz" name="张三" mobile="13988888" email="a@gcom" workcell="1234"
	// faxnum="010-223909809" cphone="190009292" userdetail="<userdetail province="北京" city="北京" corpname="慧点科技" corpdept="创新研究院" tel="010-86968585" postcode="100010" address="东升园" homepage="www.sma.com"/>"/>
	TiXmlDocument xmldoc;
	TiXmlDocument xmlDetail;
	static const char SYS_VCARD_XML[] ="<contacts type=\"addextcontact\"/>";
	static const char VCARD_DETAIL_XML[] = "<userdetail/>";
	TiXmlString strDetail;
	TiXmlString strXml;
	std::string strId = "0";
	std::string strUserName;
	std::string strName;
	std::string strMobile;
	std::string strTel;
	std::string strEmail;
	std::string strFax;
	if (xmlDetail.Load(VCARD_DETAIL_XML, strlen(VCARD_DETAIL_XML)))
	{
		TiXmlElement *pNode = xmlDetail.FirstChildElement();
		if (pNode)
		{ 
			UIMapToXml(pNode, "name", L"edtrealname");
			UIMapToXml(pNode, "mobile", L"edtmobilephone");
			UIMapToXml(pNode, "email", L"edtEmail");
			UIMapToXml(pNode, "workcell", L"edtcell");
			UIMapToXml(pNode, "faxnum", L"edtfax"); 
			UIMapToXml(pNode, "cphone", L"edttel"); 
			UIMapToXml(pNode, "province", L"edtprovince");
			UIMapToXml(pNode, "city", L"edtcity");
			UIMapToXml(pNode, "corpname", L"edtcorpname");
			UIMapToXml(pNode, "corpdept", L"edtcorpdeptname");
			UIMapToXml(pNode, "tel", L"edtfamilytel");
			UIMapToXml(pNode, "postcode", L"edtpostcode");
			UIMapToXml(pNode, "address", L"edtaddress");
			UIMapToXml(pNode, "homepage", L"edthomepage");
			if (pNode->Attribute("corpname"))
				strUserName += pNode->Attribute("corpname");
			if (pNode->Attribute("corpdept"))
				strUserName += pNode->Attribute("corpdept"); 
			if (pNode->Attribute("mobile"))
				strMobile = pNode->Attribute("mobile");
			if (pNode->Attribute("email"))
				strEmail = pNode->Attribute("email");
			if (pNode->Attribute("cphone"))
				strTel = pNode->Attribute("cphone");
			if (pNode->Attribute("faxnum"))
				strFax = pNode->Attribute("faxnum");
		}
		xmlDetail.SaveToString(strDetail, 0);
	}
	//get uid
	TCHAR szText[MAX_PATH] = {0};
	int nSize = MAX_PATH - 1;
	char szTmp[MAX_PATH] = {0};
	if (::SkinGetControlTextByName(m_hExtContact, L"edtrealname", szText, &nSize))
	{
		CStringConversion::WideCharToString(szText, szTmp, MAX_PATH - 1);
		if (::strlen(szTmp) > 0)
		{
			strName = szTmp; 
			strUserName += strName;
		}
	} 
	
	CInterfaceAnsiString strTmp;
	m_pCore->GetUserName(&strTmp); 
	strUserName += strTmp.GetData();
	BOOL bExistsUID = FALSE;
	memset(szText, 0, sizeof(TCHAR) * MAX_PATH);
	nSize = MAX_PATH - 1;
	if (::SkinGetControlTextByName(m_hExtContact, L"lblUserName", szText, &nSize))
	{
		CStringConversion::WideCharToString(szText, szTmp, MAX_PATH - 1);
		if (::strlen(szTmp) > 0)
		{
			strUserName = szTmp;
			bExistsUID = TRUE;
		}
	}
	if (!bExistsUID)
	{
		char szTmp[64] = {0};
		md5_encode(strUserName.c_str(), strUserName.size(), szTmp);
		strUserName = szTmp;
	}
	char szDeptId[16] = {0};
	if (m_nParentId > 0)
		::itoa(m_nParentId, szDeptId, 10);
	else
		::strcpy(szDeptId, EXT_CONTACT_DEPT_ID);
	int nTag = AddPendList(CFreContactItem::FRE_CONTACT_TYPE_ADDUSER, m_pParentNode, strId.c_str(), strUserName.c_str(), 
		strName.c_str(), strName.c_str(), strMobile.c_str(), strTel.c_str(), strEmail.c_str(), szDeptId, TRUE);
	if (xmldoc.Load(SYS_VCARD_XML, strlen(SYS_VCARD_XML)))
	{
		TiXmlElement *pNode = xmldoc.FirstChildElement();
		if (pNode)
		{ 
			char szId[16] = {0};
			::itoa(nTag, szId, 10);
			pNode->SetAttribute("tag", szId);
			if (!strMobile.empty())
				pNode->SetAttribute("mobile", strMobile.c_str());
			if (!strTel.empty())
				pNode->SetAttribute("tel", strTel.c_str());
			if (!strEmail.empty())
				pNode->SetAttribute("email", strEmail.c_str());
			if (!strFax.empty())
				pNode->SetAttribute("fax", strFax.c_str());
			pNode->SetAttribute("deptid", szDeptId);
			pNode->SetAttribute("name", strName.c_str());
			pNode->SetAttribute("uid", strUserName.c_str());
			pNode->SetAttribute("userdetail", strDetail.c_str());
		}
		xmldoc.SaveToString(strXml, 0);
	}
	if (m_pCore)
	{
		if (SUCCEEDED(m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG)strXml.size(), 0)))
		{
			//::SkinMessageBox(hWnd, L"增加外部联系人成功", L"提示", MB_OK);
			ClearInfo(hWnd);
			return TRUE;
		} //end if (SUCCEEDED(
	} //end if (m_pCore)*/
	return FALSE;
}

HRESULT CFreContactsImpl::DoInitMenuPopup(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szName, "treeleaf") == 0)
	{
		void *pSelNode = NULL;
		LPORG_TREE_NODE_DATA pSelData = NULL;
		TCHAR szName[MAX_PATH] = {0};
		int nSize = MAX_PATH - 1;
		CTreeNodeType tnType;
		if (::SkinGetSelectTreeNode(hWnd, L"colleaguetree", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
		{
			if (tnType == TREENODE_TYPE_LEAF)
			{
				if (IsExistsFreContact(pSelData->szUserName))
				{
					::SkinSetMenuItemAttr(hWnd, L"treeleaf", 101, L"text", L"从常用联系人移除");  
				} else
				{
					::SkinSetMenuItemAttr(hWnd, L"treeleaf", 101, L"text", L"移至常用联系人"); 
				}
			} //end if (tnType..
		} //end if (::SkinGetSelect
	} //end if (::stricmp(szName...
	return -1;
}

BOOL CFreContactsImpl::GetUserNameByTree(HWND hWnd, const TCHAR *szTreeName, std::string &strUserName, BOOL &bGroup)
{
	void *pSelNode = NULL;
	LPORG_TREE_NODE_DATA pSelData = NULL;
	TCHAR szName[MAX_PATH] = {0};
	int nSize = MAX_PATH - 1;
	CTreeNodeType tnType;
	bGroup = TRUE;
	if (::SkinGetSelectTreeNode(hWnd, szTreeName, szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
	{
		if (tnType == TREENODE_TYPE_LEAF)
		{ 
			bGroup = FALSE;
			if (pSelData)
			{	
				strUserName = pSelData->szUserName;
				if (strUserName.find("@") != std::string::npos)
				{
					return TRUE;
				} else
					strUserName.clear();
			} //end if (pSelData)
		}  
		//end if (tnType == TREENODE_TYPE_LEAF)
	} //end if (::SkinGetSelectTreeNode(
	return FALSE;
}

HWND CFreContactsImpl::OpenChatFrameByTree(HWND hWnd, const TCHAR *szTreeName)
{
	std::string strUserName;
	BOOL bGroup;
	if (GetUserNameByTree(hWnd, szTreeName, strUserName, bGroup))
	{ 
		IChatFrame *pFrame = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pFrame)))
		{
			pFrame->ShowChatFrame(hWnd, strUserName.c_str(), NULL);
			pFrame->Release();
		}  
	} else
	{
		if (!bGroup)
			::SkinMessageBox(hWnd, L"无法给外部联系人发送即时消息", L"提示", MB_OK);
	}//end if (::GetSelectTreeNode(hWnd..	
	return NULL;
}

STDMETHODIMP CFreContactsImpl::SetCoreFrameWork(ICoreFrameWork *pCore)
{
	if (m_pCore)
		m_pCore->Release();
	m_pCore = pCore;
	if (m_pCore)
	{
		m_pCore->AddRef();
		//订制事件
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "mainwindow", "afterinit");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "freleafmenu", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "frecontacttree", "lbdblclick"); 
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "treeleaf", "initmenupopup");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "treeleaf", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "fregroupmenu", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "fredefaultpopmenu", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "remarkcontactwindow", NULL, NULL);
		m_pCore->AddOrderEvent((ICoreEvent *) this, "extcontactwindow", NULL, NULL);
		//订制联系人协议
		m_pCore->AddOrderProtocol((IProtocolParser *) this, "contacts", NULL);
		m_pCore->AddOrderProtocol((IProtocolParser *) this, "sys", "presence"); 
	}
	return S_OK;
}

STDMETHODIMP_(int) CFreContactsImpl::IsExistsFreContact(const char *szUserName)
{
	IContacts *pContact = NULL;
	int nId = 0;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
	{
		nId = pContact->IsExistsExtraUsers(szUserName, FREQUENCY_CONTACT_TYPE_ID);
		pContact->Release();
	}
	return nId;
}

STDMETHODIMP CFreContactsImpl::GetSkinXmlString(IAnsiString *szXmlString)
{
	HRESULT hr = E_FAIL;
	IConfigure *pCfg = NULL; 
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{ 
		hr = pCfg->GetSkinXml("frecontacts.xml",szXmlString); 
		pCfg->Release();
	}
	return hr; 
}

//
STDMETHODIMP CFreContactsImpl::CoreFrameWorkError(int nErrorNo, const char *szErrorMsg)
{
	switch(nErrorNo)
	{
	case CORE_ERROR_LOGOUT: //注销
		{
			::SkinTreeViewClear(this->m_hWndMain, L"frecontacttree");
			break;
		}
	}
	return S_OK;
}

//广播消息
STDMETHODIMP CFreContactsImpl::DoBroadcastMessage(const char *szFromWndName, HWND hFromWnd, const char *szType,
                     const char *pContent, void *pData)
{
	if ((::stricmp(szFromWndName, "mainwindow") == 0) && (::stricmp(szType, "showcontacts") == 0)
		       && (::stricmp(pContent, "complete") == 0))
	{
		std::string strXml = "<contacts type=\"get\"/>";
		return m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG)strXml.size(), 0);
	}
	return E_FAIL;
}

//
STDMETHODIMP CFreContactsImpl::DoWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes)
{
	if (uMsg == WM_CONTACTS_DL_COMP)
	{
		IContacts *pContact = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
		{
			pContact->DrawContactToUI(m_hWndMain, L"frecontacttree", NULL, NULL, TRUE, TRUE, FREQUENCY_CONTACT_TYPE_ID);
			pContact->Release();
		}
		::SkinExpandTree(m_hWndMain, L"frecontacttree", NULL, TRUE, FALSE);
		::SkinUpdateControlUI(m_hWndMain, L"frecontacttree");
		m_bContactInit = TRUE;
		ReCreateFreMenu();
		SearchAndImportContact();
	} else if (uMsg == WM_CONTACTS_SVR_ACK)
	{
		LPFRE_CONTACT_ITEM pItem = NULL;
		m_PendLock.Lock();
		std::map<int, LPFRE_CONTACT_ITEM>::iterator it = m_PendList.find((int)lParam);
		if (it != m_PendList.end())
		{
			pItem = it->second;
			m_PendList.erase(it);
		}
		m_PendLock.UnLock();
		if (pItem)
		{
			DoSvrAck(pItem);
			delete pItem;
		} else //是否为导入
		{
			LPIMPORT_DEPT_ITEM pDept = NULL;
			m_PendLock.Lock();
			std::map<int, LPIMPORT_DEPT_ITEM>::iterator it = m_ImportList.find((int)lParam);
			if (it != m_ImportList.end())
			{
				pDept = it->second;
				m_ImportList.erase(it);
			}
			m_PendLock.UnLock();
			if (pDept)
			{
				if (pDept->bSucc)
					DoImportDeptUser(pDept->nSrcId, pDept->strId.c_str(), pDept->strName.c_str());
				delete pDept;
			} //end if (pDept
		} //end else
	} else if (uMsg == WM_FREPRESENCECHG)
	{
		LPFRE_CONTACT_PRESENCE_NOTIFY_ITEM pItem = (LPFRE_CONTACT_PRESENCE_NOTIFY_ITEM) lParam;
		if (m_bContactInit)
			::SkinUpdateUserStatusToNode(m_hWndMain, L"frecontacttree", pItem->strUserName.c_str(), 
			     pItem->strPresence.c_str(), TRUE); 
		delete pItem;
	} else if (uMsg == WM_FRECNT_DETAIL)
	{
		if (m_hExtContact && ::IsWindow(m_hExtContact))
		{
			int nSize = (int) wParam;
			char *szText = (char *)lParam;
			ShowContactInfoToUI(szText, nSize);
		}
	}
	return E_NOTIMPL;
}

void CFreContactsImpl::MapToUI(const char *szText, const TCHAR *szUIName)
{
	if (szText)
	{
		TCHAR szTmp[MAX_PATH] = {0};
		CStringConversion::StringToWideChar(szText, szTmp, MAX_PATH - 1);
		::SkinSetControlTextByName(m_hExtContact, szUIName, szTmp);
	}
}

//
void CFreContactsImpl::ShowContactInfoToUI(const char *szXml, const int nXmlSize)
{
	TiXmlDocument xmldoc;
	TiXmlElement *pNode = NULL;
	if (xmldoc.Load(szXml, nXmlSize))
	{
		pNode = xmldoc.FirstChildElement();
	}
	if (pNode)
	{
		if ((m_hExtContact == NULL) || (!::IsWindow(m_hExtContact)))
			return  ;
		IContacts *pContacts = NULL;
		if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContacts)))
		{
			std::string strUserName = pNode->Attribute("uid"); 
			
			MapToUI(pNode->Attribute("uid"), L"lblUserName");
			 
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
						MapToUI(pFirst->Attribute("name"), L"edtrealname"); 
						MapToUI(pFirst->Attribute("mobile"), L"edtmobilephone");
						MapToUI(pFirst->Attribute("workcell"), L"edtcell");
						MapToUI(pFirst->Attribute("cphone"), L"edttel");
						MapToUI(pFirst->Attribute("email"), L"edtEmail");
						MapToUI(pFirst->Attribute("workcell"), L"edtcell");
						MapToUI(pFirst->Attribute("faxnum"), L"edtfax");
						MapToUI(pFirst->Attribute("corpname"), L"edtcorpname");
						MapToUI(pFirst->Attribute("corpdept"), L"edtcorpdeptname"); 
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
		} //end if (
		::SkinSetControlAttr(m_hExtContact, L"edit_contacts", L"visible", L"true");
	}
}

//
void CFreContactsImpl::DoImportDeptUser(const int nSrcId, const char *szSvrId, const char *szDeptName)
{
	LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
	pData->id = ::atoi(szSvrId);
	strncpy(pData->szUserName,  szSvrId, MAX_USER_NAME_SIZE - 1);
	TCHAR szName[MAX_PATH] = {0};
	IContacts *pContact = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
	{
		char szTmpUTF8[MAX_PATH] = {0};
		CStringConversion::StringToUTF8(szDeptName, szTmpUTF8, MAX_PATH - 1);
		pContact->AddExtractDept(szSvrId, szTmpUTF8, "0", EXT_CONTACT_DEPT_ID, FREQUENCY_CONTACT_TYPE_ID); 				 
		pContact->Release();
	}
	CStringConversion::StringToWideChar(szDeptName, szName, MAX_PATH - 1);
	void *pParentNode = NULL;
	void *pParentData = NULL;
	::SkinGetTreeNodeById(m_hWndMain, L"frecontacttree", 2, TREENODE_TYPE_GROUP, &pParentNode, &pParentData);
			 
	::SkinAddTreeChildNode(m_hWndMain, L"frecontacttree", pData->id,pParentNode, szName, TREENODE_TYPE_GROUP,
		pData, NULL, NULL, NULL);
	::SkinUpdateControlUI(m_hWndMain, L"frecontacttree");
	// 
	std::vector<LPIMPORT_USER_ITEM>::iterator it;
	for (it = m_ImportUsers.begin(); it != m_ImportUsers.end(); it ++)
	{
		//
		int nPos = (*it)->strGroups.find(szDeptName);
		if (nPos != std::string::npos)
		{
			DoAddImporUser(szSvrId, (*it));
		}
	}
}

void CFreContactsImpl::ImportUserByNullGroups()
{
	std::vector<LPIMPORT_USER_ITEM>::iterator it;
	for (it = m_ImportUsers.begin(); it != m_ImportUsers.end();)
	{
		if ((*it)->strGroups.empty())
		{		 
			DoAddImporUser(EXT_CONTACT_DEPT_ID, (*it));
			delete (*it);
			it = m_ImportUsers.erase(it);
		} else
			it ++;
	}
}

//
void CFreContactsImpl::DoAddImporUser(const char *szParentId, LPIMPORT_USER_ITEM pItem)
{
	// "<contacts type="addextcontact" uid="wxz" name="张三" mobile="13988888" email="a@gcom" workcell="1234"
	// faxnum="010-223909809" cphone="190009292" userdetail="<userdetail province="北京" city="北京" corpname="慧点科技" corpdept="创新研究院" tel="010-86968585" postcode="100010" address="东升园" homepage="www.sma.com"/>"/>
	TiXmlDocument xmldoc;
	TiXmlDocument xmlDetail;
	static const char SYS_VCARD_XML[] ="<contacts type=\"addextcontact\"/>";
	static const char VCARD_DETAIL_XML[] = "<userdetail/>";
	TiXmlString strDetail;
	TiXmlString strXml;
	std::string strId = "0";
	std::string strUserName;
    std::string strName = pItem->strFirstName;
	strName += pItem->strLastName;
	if (xmlDetail.Load(VCARD_DETAIL_XML, strlen(VCARD_DETAIL_XML)))
	{
		TiXmlElement *pNode = xmlDetail.FirstChildElement();
		if (pNode)
		{  
			if (!strName.empty()) 
				pNode->SetAttribute("name", strName.c_str()); 
			if (!pItem->strPersonMobile.empty())
				pNode->SetAttribute("mobile", pItem->strPersonMobile.c_str()); 
			if (!pItem->strOfficeEmail.empty())
				pNode->SetAttribute("email", pItem->strOfficeEmail.c_str()); 
			if (!pItem->strOfficeTel.empty())
				pNode->SetAttribute("cphone", pItem->strOfficeTel.c_str()); 
			if (!pItem->strOfficeFax.empty())
				pNode->SetAttribute("faxnum", pItem->strOfficeFax.c_str());
			if (!pItem->strCompany.empty())
				pNode->SetAttribute("corpname", pItem->strCompany.c_str());
			if (!pItem->strDepart.empty())
				pNode->SetAttribute("corpdept", pItem->strDepart.c_str());
			if (!pItem->strPersonTel.empty())
				pNode->SetAttribute("tel", pItem->strPersonTel.c_str());
			if (!pItem->strOfficeAddress.empty())
				pNode->SetAttribute("address", pItem->strOfficeAddress.c_str());
			if (!pItem->strOfficeHomePage.empty())
				pNode->SetAttribute("homepage", pItem->strOfficeHomePage.c_str());
			if (!pItem->strOfficeZipCode.empty())
				pNode->SetAttribute("postcode", pItem->strOfficeZipCode.c_str());
			if (!pItem->strFirstName.empty())
				pNode->SetAttribute("firstname", pItem->strFirstName.c_str());
			if (!pItem->strLastName.empty())
				pNode->SetAttribute("lastname", pItem->strLastName.c_str());
			if (!pItem->strDuty.empty())
				pNode->SetAttribute("duty", pItem->strDuty.c_str());
			if (!pItem->strGender.empty())
				pNode->SetAttribute("gender", pItem->strGender.c_str());
			if (!pItem->strBirthady.empty())
				pNode->SetAttribute("birthday", pItem->strBirthady.c_str());
			if (!pItem->strAnniversary.empty())
				pNode->SetAttribute("anniversary", pItem->strAnniversary.c_str());
			if (!pItem->strPersonMobile.empty())
				pNode->SetAttribute("personmobile", pItem->strPersonMobile.c_str());
			if (!pItem->strPersonEmail.empty())
				pNode->SetAttribute("personemail", pItem->strPersonEmail.c_str());
			if (!pItem->strPersonAddr.empty())
				pNode->SetAttribute("personaddr", pItem->strPersonAddr.c_str());
			if (!pItem->strPersonFax.empty())
				pNode->SetAttribute("personfax", pItem->strPersonFax.c_str());
			if (!pItem->strPersonZipCode.empty())
				pNode->SetAttribute("personpostcode", pItem->strPersonZipCode.c_str());
			if (!pItem->strChineseCode.empty())
				pNode->SetAttribute("chinesecode", pItem->strChineseCode.c_str());
			if (!pItem->strRemark.empty())
				pNode->SetAttribute("remark", pItem->strRemark.c_str());
		}
		xmlDetail.SaveToString(strDetail, 0);
	}
	//get uid
 
	TCHAR szText[MAX_PATH] = {0};
	int nSize = MAX_PATH - 1;
	BOOL bExistsUID = FALSE;
	if (::SkinGetControlTextByName(m_hExtContact, L"lblUserName", szText, &nSize))
	{
		char szTmp[128] = {0};
		CStringConversion::WideCharToString(szText, szTmp, MAX_PATH - 1);
		if (::strlen(szTmp) > 0)
		{
			strUserName = szTmp;
			bExistsUID = TRUE;
		}
	}
	if (!bExistsUID)
	{
		char szTmp[64] = {0};
		std::string struidcode = pItem->strCompany;
		struidcode += pItem->strDepart;
		struidcode += strName;
		CInterfaceAnsiString strTmp;
		m_pCore->GetUserName(&strTmp);
		struidcode += strTmp.GetData();
		md5_encode(struidcode.c_str(), struidcode.size(), szTmp);
		strUserName = szTmp;
	} 
	int nTag = AddPendList(CFreContactItem::FRE_CONTACT_TYPE_ADDUSER, m_pParentNode, strId.c_str(), strUserName.c_str(), 
		strName.c_str(), pItem->strRemark.c_str(), pItem->strPersonMobile.c_str(), 
		pItem->strPersonTel.c_str(), pItem->strPersonEmail.c_str(), szParentId, FALSE);
	if (xmldoc.Load(SYS_VCARD_XML, strlen(SYS_VCARD_XML)))
	{
		TiXmlElement *pNode = xmldoc.FirstChildElement();
		if (pNode)
		{ 
			char szId[16] = {0};
			::itoa(nTag, szId, 10);
			pNode->SetAttribute("tag", szId);
			if (!pItem->strPersonMobile.empty())
				pNode->SetAttribute("mobile", pItem->strPersonMobile.c_str());
			if (!pItem->strPersonTel.empty())
				pNode->SetAttribute("tel", pItem->strPersonTel.c_str());
			if (!pItem->strPersonEmail.empty())
				pNode->SetAttribute("email", pItem->strPersonEmail.c_str());
			if (!pItem->strPersonFax.empty())
				pNode->SetAttribute("fax", pItem->strPersonFax.c_str());
			pNode->SetAttribute("deptid", szParentId);
			
			pNode->SetAttribute("name", strName.c_str());
			pNode->SetAttribute("uid", strUserName.c_str());
			pNode->SetAttribute("userdetail", strDetail.c_str());
		}
		xmldoc.SaveToString(strXml, 0);
		m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG)strXml.size(), 0);
	} 
}

void CFreContactsImpl::DoSvrAck(LPFRE_CONTACT_ITEM pItem)
{
	switch(pItem->nType)
	{
	case CFreContactItem::FRE_CONTACT_TYPE_ADDDEPT:
		{
			LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
			pData->id = ::atoi(pItem->strId.c_str());
			strncpy(pData->szUserName,  pItem->strId.c_str(), MAX_USER_NAME_SIZE - 1);
			TCHAR szName[MAX_PATH] = {0};
			IContacts *pContact = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
			{
				char szTmpUTF8[MAX_PATH] = {0};
				CStringConversion::StringToUTF8(pItem->strUserName.c_str(), szTmpUTF8, MAX_PATH - 1);
				pContact->AddExtractDept(pItem->strId.c_str(), szTmpUTF8, "0", pItem->strParentId.c_str(), FREQUENCY_CONTACT_TYPE_ID); 				 
				pContact->Release();
			}
			CStringConversion::StringToWideChar(pItem->strUserName.c_str(), szName, MAX_PATH - 1);
			 
			::SkinAddTreeChildNode(m_hWndMain, L"frecontacttree", pData->id, pItem->pParentNode, szName, TREENODE_TYPE_GROUP,
				pData, NULL, NULL, NULL);
			::SkinUpdateControlUI(m_hWndMain, L"frecontacttree");
			ReCreateFreMenu();
			break;
		}
	case CFreContactItem::FRE_CONTACT_TYPE_ADDUSER:
		{
			IContacts *pContact = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
			{
				if (pContact->IsExistsExtraUsers(pItem->strUserName.c_str(), FREQUENCY_CONTACT_TYPE_ID) == 0)
				{
					LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
					pData->id = ::atoi(pItem->strId.c_str());
					strncpy(pData->szUserName,  pItem->strUserName.c_str(), MAX_USER_NAME_SIZE - 1); 
					TCHAR szName[MAX_PATH] = {0};
					CStringConversion::StringToWideChar(pItem->strRealName.c_str(), szName, MAX_PATH - 1);
					void *pParentNode = NULL;
					void *pParentData = NULL;
					int nParentId = ::atoi(pItem->strParentId.c_str()); 
					if (pItem->pParentNode == NULL)
						::SkinGetTreeNodeById(m_hWndMain, L"frecontacttree", nParentId, TREENODE_TYPE_GROUP, &pParentNode, &pParentData);
					else
						pParentNode = pItem->pParentNode;
					::SkinAddTreeChildNode(m_hWndMain, L"frecontacttree", pData->id, pParentNode, szName, TREENODE_TYPE_LEAF,
						pData, NULL, NULL, NULL);
					
					char szTmpUTF8[MAX_PATH] = {0};
					CStringConversion::StringToUTF8(pItem->strRealName.c_str(), szTmpUTF8, MAX_PATH - 1);
					pContact->AddExtractUser(pItem->strId.c_str(), pItem->strUserName.c_str(), szTmpUTF8, 
						pItem->strParentId.c_str(), pItem->strMobile.c_str(), pItem->strTel.c_str(),
						pItem->strEmail.c_str(), pItem->strFax.c_str(), FREQUENCY_CONTACT_TYPE_ID);
	 				CInstantUserInfo Info;
					if (SUCCEEDED(pContact->GetContactUserInfo(pItem->strUserName.c_str(), &Info)))
					{
						CInterfaceAnsiString strTmp;
						if (SUCCEEDED(Info.GetUserStatus(&strTmp)))
						{
							::SkinUpdateUserStatusToNode(m_hWndMain, L"frecontacttree", pItem->strUserName.c_str(), strTmp.GetData(), TRUE);
						}
						if (SUCCEEDED(Info.GetUserInfo("sign", &strTmp)))
						{ 
							::SkinUpdateUserLabelToNode(m_hWndMain, L"frecontacttree", pItem->strUserName.c_str(), strTmp.GetData(), TRUE);
						}
					}
					
					::SkinExpandTree(m_hWndMain, L"frecontacttree", NULL, TRUE, FALSE);
					::SkinUpdateControlUI(m_hWndMain, L"frecontacttree");
					
					if (pItem->bShowTip)
					{
						ITrayMsg *pTray = NULL;
						if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(ITrayMsg), (void **)&pTray)))
						{
							std::string strTip; 
							if (pItem->strParentId == EXT_CONTACT_DEPT_ID)
							{
								strTip = "成功添加外部联系人（用户名：";
								strTip += pItem->strRealName;
								strTip += ")";
								pTray->ShowTipPanel("添加外部联系人", strTip.c_str());
							} else if (pItem->strParentId == FRE_CONTACT_DEPT_ID)
							{
								strTip = "成功添加常用联系人（用户名：";
								strTip += pItem->strRealName;
								strTip += ")";
								pTray->ShowTipPanel("添加常用联系人", strTip.c_str());

							} else
							{
								strTip = "成功添加电话本联系人（用户名：";
								strTip += pItem->strRealName;
								strTip += ")";
								pTray->ShowTipPanel("添加电话本联系人", strTip.c_str());
							}
							pTray->Release();
						}
					} else
					{
						//导入功能
					}
				} else
				{
					char szTmpUTF8[MAX_PATH] = {0};
					CStringConversion::StringToUTF8(pItem->strRealName.c_str(), szTmpUTF8, MAX_PATH - 1);
					pContact->AddExtractUser(pItem->strId.c_str(), pItem->strUserName.c_str(), szTmpUTF8, 
						pItem->strParentId.c_str(), pItem->strMobile.c_str(), pItem->strTel.c_str(),
						pItem->strEmail.c_str(), pItem->strFax.c_str(), FREQUENCY_CONTACT_TYPE_ID);
					if (pItem->bShowTip)
					{
						//记录重复
						ITrayMsg *pTray = NULL;
						if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(ITrayMsg), (void **)&pTray)))
						{
							std::string strTip = "用户（用户名:"; 
							strTip += pItem->strRealName;
							strTip += ")资料已修改成功";
							pTray->ShowTipPanel("导入用户提示", strTip.c_str());
							pTray->Release();
						}
					} else
					{
						//导入功能提示
					}
				}
				pContact->Release();
			}
			break;
		}
	case CFreContactItem::FRE_CONTACT_TYPE_DELDEPT:
		{ 
			::SkinTVDelNodeByID(m_hWndMain, L"frecontacttree", ::atoi( pItem->strId.c_str()), TREENODE_TYPE_GROUP);
			 
			::SkinUpdateControlUI(m_hWndMain, L"frecontacttree");
			IContacts *pContact = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
			{ 
				pContact->DeleteExtractDept(pItem->strId.c_str(), FREQUENCY_CONTACT_TYPE_ID);
				pContact->Release();
			}
		}
		break;
	case CFreContactItem::FRE_CONTACT_TYPE_DELUSER:
		{
			LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
			strncpy(pData->szUserName, pItem->strUserName.c_str(), MAX_USER_NAME_SIZE - 1);
			::SkinAdjustTreeNode(m_hWndMain, L"frecontacttree", NULL, NULL, TREENODE_TYPE_LEAF, pData, FALSE, TRUE);
			delete pData;
			::SkinUpdateControlUI(m_hWndMain, L"frecontacttree");
			IContacts *pContact = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
			{ 
				pContact->DeleteExtractUser(pItem->strId.c_str(), FREQUENCY_CONTACT_TYPE_ID);
				pContact->Release();
			}
		}
		break;
	case CFreContactItem::FRE_CONTACT_TYPE_MODIFYDEPT:
		break;
	case CFreContactItem::FRE_CONTACT_TYPE_REMARKUSER:
		if (pItem->bSucc)
		{ 
			::SkinMessageBox(m_hWndMain, L"修改联系人备注成功", L"提示", MB_OK);
		} else
			::SkinMessageBox(m_hWndMain, L"修改联系人备注失败", L"提示", MB_OK);
		break;
	}
}

//
STDMETHODIMP CFreContactsImpl::DoRecvProtocol(const BYTE *pData, const LONG lSize)
{
	TiXmlDocument xmldoc;
	if (xmldoc.Load((char *)pData, lSize))
	{
		TiXmlElement *pNode = xmldoc.FirstChildElement();
		if (pNode)
		{
			const char *szValue = pNode->Value();
			const char *szType = pNode->Attribute("type");
			if (szValue && szType)
			{
				if (::stricmp(szValue, "contacts") == 0)
					DoContactProtocol(szType, pNode);
			} 
		} //end if (pNode)
	} //end if (xmldoc.Load(..
	return E_NOTIMPL;
}

//
STDMETHODIMP CFreContactsImpl::DoPresenceChange(const char *szUserName, const char *szNewPresence, 
	                                      const char *szMemo, BOOL bOrder)
{
	if ((m_hWndMain != NULL) && (szUserName != NULL) && (szNewPresence != NULL))
	{
		if (m_bContactInit)
		{
			LPFRE_CONTACT_PRESENCE_NOTIFY_ITEM pItem = new FRE_CONTACT_PRESENCE_NOTIFY_ITEM();
			if (szMemo)
				pItem->strMemo = szMemo;
			pItem->strPresence = szNewPresence;
			pItem->strUserName = szUserName;
			pItem->bOrder = bOrder;
			::PostMessage(m_hWndMain, WM_FREPRESENCECHG, 0, (LPARAM) pItem);
		}
		//return ::SendMessage(m_hWndMain, WM_FREPRESENCECHG, (WPARAM) szUserName, (LPARAM) szNewPresence);
		
		//   ::SkinUpdateUserStatusToNode(m_hWndMain, L"frecontacttree", szUserName, 
		//                     szNewPresence, TRUE); 
	}
 
	return E_FAIL;
}

BOOL CFreContactsImpl::DoContactProtocol(const char *szType, TiXmlElement *pNode)
{
	if (::stricmp(szType, "getack") == 0)
	{
		IContacts *pContact = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
		{
			char UTF8Tmp[MAX_PATH] = {0};
			TiXmlElement *pDept = pNode->FirstChildElement("depts");
			if (pDept)
			{
				TiXmlElement *pTmp = pDept->FirstChildElement("dept");
				while (pTmp)
				{
					memset(UTF8Tmp, 0, MAX_PATH);
					CStringConversion::StringToUTF8(pTmp->Attribute("name"), UTF8Tmp, MAX_PATH - 1);
					pContact->AddExtractDept(pTmp->Attribute("id"), UTF8Tmp, pTmp->Attribute("dispseq"),
						pTmp->Attribute("parentid"), FREQUENCY_CONTACT_TYPE_ID);
					pTmp = pTmp->NextSiblingElement("dept");
				}
			}
			TiXmlElement *pUsers = pNode->FirstChildElement("users");
			if (pUsers)
			{
				TiXmlElement *pTmp = pUsers->FirstChildElement("user");
				while (pTmp)
				{
					memset(UTF8Tmp, 0, MAX_PATH);
					CStringConversion::StringToUTF8(pTmp->Attribute("realname"), UTF8Tmp, MAX_PATH - 1);
					pContact->AddExtractUser(pTmp->Attribute("id"), pTmp->Attribute("username"), UTF8Tmp,
						pTmp->Attribute("deptid"), pTmp->Attribute("mobile"), pTmp->Attribute("tel"),
						pTmp->Attribute("email"), pTmp->Attribute("fax"), FREQUENCY_CONTACT_TYPE_ID);
					pTmp = pTmp->NextSiblingElement("user");
				} //end while (pTmp)
			} //end if (pUsers)
			pContact->Release();
		} //end
		//通知绘制到界面
		if (m_hWndMain)
			::SendMessage(m_hWndMain, WM_CONTACTS_DL_COMP, 0, 0);
		 
	}  else if (::stricmp(szType, "deldept") == 0)
	{
		DoContactSvrAckProto(pNode, NULL);
		//
	} else if (::stricmp(szType, "deluser") == 0)
	{
		DoContactSvrAckProto(pNode, NULL);
	} else if (::stricmp(szType, "adddept") == 0)
	{
		DoContactSvrAckProto(pNode, NULL);
	} else if (::stricmp(szType, "adduser") == 0)
	{
		DoContactSvrAckProto(pNode, NULL);
	} else if (::stricmp(szType, "remarkuser") == 0)
	{ 
		DoContactSvrAckProto(pNode, NULL);
	} else  if (::stricmp(szType, "getextcard") == 0)
	{
		TiXmlString strXml;
		pNode->SaveToString(strXml, 0);
		::SendMessage(m_hWndMain, WM_FRECNT_DETAIL, (WPARAM)strXml.size(), (LPARAM)strXml.c_str()); 
	} //
	//end if (::stricmp(
	return S_OK;
}

//
BOOL CFreContactsImpl::DoContactSvrAckProto(TiXmlElement *pNode, TiXmlElement *pChild)
{
	const char *szAck = pNode->Attribute("result"); 
	const char *szTag = pNode->Attribute("tag");
	if (pChild)
		szTag = pChild->Attribute("tag");
	int nTag = 0;
	if (szTag)
		nTag = ::atoi(szTag);
	if (nTag > 0)
	{
		CGuardLock::COwnerLock guard(m_PendLock);
		std::map<int, LPFRE_CONTACT_ITEM>::iterator it =  m_PendList.find(nTag);
		if (it != m_PendList.end())
		{
			if (szAck && (::stricmp(szAck, "ok") == 0))
			{
				it->second->bSucc = TRUE;
				if (pNode->Attribute("id"))
					it->second->strId = pNode->Attribute("id");
				if (pChild && pChild->Attribute("id"))
					it->second->strId = pChild->Attribute("id");
			} else
			{
				it->second->bSucc = FALSE;
			}
			if (m_hWndMain)
				::PostMessage(m_hWndMain, WM_CONTACTS_SVR_ACK, 0, (LPARAM) nTag);
		} else 
		{
			//导入
			std::map<int, LPIMPORT_DEPT_ITEM>::iterator it = m_ImportList.find(nTag);
			if (it != m_ImportList.end())
			{
				if (szAck && (::stricmp(szAck, "ok") == 0))
				{
					it->second->bSucc = TRUE;
					if (pNode->Attribute("id"))
						it->second->strId = pNode->Attribute("id");
					if (pChild && pChild->Attribute("id"))
						it->second->strId = pChild->Attribute("id");
				} else
				{
					it->second->bSucc = FALSE;
				}
				if (m_hWndMain)
					::PostMessage(m_hWndMain, WM_CONTACTS_SVR_ACK, 0, (LPARAM) nTag);
			}
		}
		return TRUE;
	}
	return FALSE;
}

//
STDMETHODIMP CFreContactsImpl::AddFreContactDept(const char *szId, const char *szDeptName, const char *szParentId, 
		const char *szDispSeq)
{
	if (m_pCore)
	{
		//<contacts type="adddept">
		//   <dept id="0" name="一级部门" parentid="0" dispseq="0"/>
		//</contacts>
		int nTag = AddPendList(CFreContactItem::FRE_CONTACT_TYPE_ADDDEPT, m_pParentNode, szId, szDeptName, NULL, NULL,
			        NULL, NULL, NULL, szParentId, TRUE);
		std::string strXml = "<contacts type=\"adddept\"><dept id=\""; 
		if (szId)
			strXml += szId;
		else
			strXml += "0";
		strXml += "\" name=\"";
		if (szDeptName)
			strXml += szDeptName;
		strXml += "\" parentid=\"";
		if (szParentId)
			strXml += szParentId;
		strXml += "\" dispseq=\"";
		if (szDispSeq)
			strXml += szDispSeq;
		else
			strXml += "0";
		char szTmp[16] = {0};
		::itoa(nTag, szTmp, 10);
		strXml += "\" tag=\"";
		strXml += szTmp;
		strXml += "\"/></contacts>";
		return m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG) strXml.size(), 0); 
	}
	return E_FAIL;
}

STDMETHODIMP CFreContactsImpl::AddFreContactUser(const char *szId, const char *szUserName, const char *szRealName,
		const char *szRemark, const char *szDeptId)
{
	if (m_pCore)
	{
		//<contacts type="adduser">
		//   <user id="0" username="aaa@gocom" realname="用户名" remark="备注" deptid="0"/>
		//</contacts> 
		char szTmp[MAX_PATH] = {0};
		char szRemarkTmp[MAX_PATH] = {0};
		if (szRealName)
			strcpy(szTmp, szRealName);
		if (szRemark)
			strcpy(szRemarkTmp, szRemark);
		if (szRealName == NULL)
		{
			IContacts *pContact = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
			{
				CInterfaceAnsiString strTmp;
				if (SUCCEEDED(pContact->GetRealNameById(szUserName, NULL, &strTmp)))
				{
					memset(szTmp, 0, MAX_PATH);
					CStringConversion::UTF8ToString(strTmp.GetData(), szTmp, MAX_PATH - 1);
					if (szRemark == NULL)
						CStringConversion::UTF8ToString(strTmp.GetData(), szRemarkTmp, MAX_PATH - 1);
				}
				pContact->Release();
			} //end if (SUCCEEDED(..
		} //end if (szRealName == ..
		int nTag = AddPendList(CFreContactItem::FRE_CONTACT_TYPE_ADDUSER, m_pParentNode, szId, szUserName, szTmp, szRemarkTmp, 
			                   NULL, NULL, NULL, szDeptId, TRUE);
		std::string strXml = "<contacts type=\"adduser\"><user id=\"";
		if (szId)
		{
			strXml += szId;
		} else
			strXml += "0";
		strXml += "\" username=\"";
		if (szUserName)
			strXml += szUserName;
		strXml += "\" realname=\""; 
		strXml += szTmp;
		strXml += "\" remark=\""; 
		strXml += szRemarkTmp; 
		strXml += "\" deptid=\"";
		if (szDeptId) 
			strXml += szDeptId;
		else
			strXml += "0";
		char szTagId[16] = {0};
		::itoa(nTag, szTagId, 10);
		strXml += "\" tag=\"";
		strXml += szTagId;
		strXml += "\"/></contacts>";
		return m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG)strXml.size(), 0);
	}
	return E_FAIL;
}

int CFreContactsImpl::AddPendList(CFreContactItem::emFreContactType nType, void *pParentNode, const char *szId, const char *szUserName, 
	                 const char *szRealName, const char *szRemark,  const char *szMobile, 
					 const char *szTel, const char *szEmail, const char *szDeptId, BOOL bShowTip)
{
	LPFRE_CONTACT_ITEM pItem = new FRE_CONTACT_ITEM();
	pItem->nType = nType;
	pItem->bSucc = FALSE;
	pItem->bShowTip = bShowTip;
	pItem->pParentNode = pParentNode;
	if (szId)
		pItem->strId = szId;
	if (szUserName)
		pItem->strUserName = szUserName;
	if (szRealName)
		pItem->strRealName = szRealName;
	if (szRemark)
		pItem->strRemarkName = szRemark;
	if (szDeptId)
		pItem->strParentId = szDeptId;
	if (szMobile)
		pItem->strMobile = szMobile;
	if (szTel)
		pItem->strTel = szTel;
	if (szEmail)
		pItem->strEmail = szEmail;
	m_PendLock.Lock();
    m_nPendId ++;
	int nTag = m_nPendId;
	m_PendList.insert(std::pair<int, LPFRE_CONTACT_ITEM>(nTag, pItem));
	m_PendLock.UnLock();
	return nTag;
}

//
STDMETHODIMP CFreContactsImpl::UpdateFreContactRemark(const char *szId, const char *szRemark)
{
	if (m_pCore)
	{
		//<contacts type="remarkuser">
		//   <user id="23" remark=""/>
		//</contacts>
		int nTag = AddPendList(CFreContactItem::FRE_CONTACT_TYPE_REMARKUSER, NULL, szId, NULL, NULL, szRemark, 
			                   NULL, NULL, NULL, NULL, TRUE);
		std::string strXml = "<contacts type=\"remarkuser\"><user id=\"";
		strXml += szId;
		strXml += "\" remark=\"";
		strXml += szRemark;
		char szTmp[16] = {0};
		::itoa(nTag, szTmp, 10);
		strXml += "\" tag=\"";
		strXml += szTmp;
		strXml += "\"/>";
		strXml += "</contacts>";
		return m_pCore->SendRawMessage((BYTE *) strXml.c_str(), (LONG) strXml.size(), 0);
	}
	return E_NOTIMPL;
}

//
STDMETHODIMP CFreContactsImpl::DeleteFreContactDept(const char *szId, const char *szName)
{
	if (m_pCore)
	{
		//<contacts type="deldept" id="1"/>
		int nTag = AddPendList(CFreContactItem::FRE_CONTACT_TYPE_DELDEPT, NULL, szId, szName, NULL, NULL, 
			NULL, NULL, NULL, NULL, TRUE);
		std::string strXml = "<contacts type=\"deldept\" id=\"";
		strXml += szId;
		char szTmp[16] = {0};
		::itoa(nTag, szTmp, 10);
		strXml += "\" tag=\"";
		strXml += szTmp;
		strXml += "\"/>";
		return m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG) strXml.size(), 0);
	}
	return E_FAIL;
}

//导入本地用户记录
STDMETHODIMP CFreContactsImpl::ImportContactFromLocal(const char *szFileName)
{ 
	CSqliteDBOP db(szFileName, NULL);
	if (db.TableIsExists("cgroups") && db.TableIsExists("contacts"))
	{
		int nRow, nCol;
		char **szResult = NULL; 
		std::string strSql = "select firstname,lastname,infoname,gender,company,department,duty,officetel,\
							 officeemail,officefax,officemobile,officehomepage,officeaddress,officezipcode,birthday,\
							 anniversary,personaltel,personalmobile,personalemail,personalfax,personaladdress,\
							 personalzipcode,remark,chinesecode,groups from contacts"; 
		//缓存
		if (db.Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			for (int i = 1; i <= nRow; i ++)
			{
				LPIMPORT_USER_ITEM pItem = new IMPORT_USER_ITEM();
				if (szResult[i * nCol])
					pItem->strFirstName = szResult[i * nCol];
				if (szResult[i * nCol + 1])
					pItem->strLastName = szResult[i * nCol + 1];
				if (szResult[i * nCol + 2])
					pItem->strInfoName = szResult[i * nCol + 2];
				if (szResult[i * nCol + 3])
					pItem->strGender = szResult[i * nCol + 3];
				if (szResult[i * nCol + 4])
					pItem->strCompany = szResult[i * nCol + 4];
				if (szResult[i * nCol + 5])
					pItem->strDepart = szResult[i * nCol + 5];
				if (szResult[i * nCol + 6])
					pItem->strDuty = szResult[i * nCol + 6];
				if (szResult[i * nCol + 7])
					pItem->strOfficeTel = szResult[i * nCol + 7];
				if (szResult[i * nCol + 8])
					pItem->strOfficeEmail = szResult[i * nCol + 8];
				if (szResult[i * nCol + 9])
					pItem->strOfficeFax = szResult[i * nCol + 9];
				if (szResult[i * nCol + 10])
					pItem->strOfficeMobil = szResult[i * nCol + 10];
				if (szResult[i * nCol + 11])
					pItem->strOfficeHomePage = szResult[i * nCol + 11];
				if (szResult[i * nCol + 12])
					pItem->strOfficeAddress = szResult[i * nCol + 12];
				if (szResult[i * nCol + 13])
					pItem->strOfficeZipCode = szResult[i * nCol + 13];
				if (szResult[i * nCol + 14])
					pItem->strBirthady = szResult[i * nCol + 14];
				if (szResult[i * nCol + 15])
					pItem->strAnniversary = szResult[i * nCol + 15];
				if (szResult[i * nCol + 16])
					pItem->strPersonTel = szResult[i * nCol + 16];
				if (szResult[i * nCol + 17])
					pItem->strPersonMobile = szResult[i * nCol + 17];
				if (szResult[i * nCol + 18])
					pItem->strPersonEmail = szResult[i * nCol + 18];
				if (szResult[i * nCol + 19])
					pItem->strPersonFax = szResult[i * nCol + 19];
				if (szResult[i * nCol + 20])
					pItem->strPersonAddr = szResult[i * nCol + 20];
				if (szResult[i * nCol + 21])
					pItem->strPersonZipCode = szResult[i * nCol + 21];
				if (szResult[i * nCol + 22])
					pItem->strRemark = szResult[i * nCol + 22];
				if (szResult[i * nCol + 23])
					pItem->strChineseCode = szResult[i * nCol + 23];
				if (szResult[i * nCol + 24])
					pItem->strGroups = szResult[i * nCol + 24]; 
				m_ImportUsers.push_back(pItem);
			}
		}
		CSqliteDBOP::Free_Result(szResult);
		//查询分组
		if (db.Open("select id,name from cgroups", &szResult, nRow, nCol))
		{ 
			for (int i = 1; i <= nRow; i ++)
			{
				if (szResult[i * nCol] && szResult[i * nCol + 1])
				{
					LPIMPORT_DEPT_ITEM pItem = new IMPORT_DEPT_ITEM();
					pItem->nSrcId = ::atoi(szResult[i * nCol]); 
					pItem->bSucc = FALSE;
					pItem->strName = szResult[i * nCol + 1];
					m_PendLock.Lock();
				    m_nPendId ++;
					int nTag = m_nPendId;
					m_ImportList.insert(std::pair<int, LPIMPORT_DEPT_ITEM>(nTag, pItem));
					m_PendLock.UnLock();
					std::string strXml = "<contacts type=\"adddept\"><dept id=\"0\" dispseq=\"0\" parentid=\"2\" name=\""; 
					
					strXml += pItem->strName;
					 
					char szTmp[16] = {0};
					::itoa(nTag, szTmp, 10);
					strXml += "\" tag=\"";
					strXml += szTmp;
					strXml += "\"/></contacts>";
					m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG) strXml.size(), 0); 
				} //end if (szResult[i * nCol]
			} //end for (i
			CSqliteDBOP::Free_Result(szResult);
		} //end if (db
		//导入无分组外部联系人
		ImportUserByNullGroups();
	} //end db
	return E_NOTIMPL;
}

//
STDMETHODIMP CFreContactsImpl::DeleteFreContactUser(const char *szId, const char *szUserName)
{
	if (m_pCore)
	{
		//<contacts type="deluser" id="2"/>
		int nTag = AddPendList(CFreContactItem::FRE_CONTACT_TYPE_DELUSER, NULL, szId, szUserName, NULL, NULL, NULL, NULL, NULL, NULL, TRUE);
		std::string strXml = "<contacts type=\"deluser\" id=\"";
		strXml += szId;
		char szTmp[16] = {0};
		::itoa(nTag, szTmp, 10);
		strXml += "\" tag=\"";
		strXml += szTmp;
		strXml += "\"/>";
		return m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG) strXml.size(), 0);
	}
	return E_FAIL;
}

//查找并导入外部联系人
BOOL CFreContactsImpl::SearchAndImportContact()
{
	IConfigure *pCfg = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{ 
		BOOL bImport = FALSE;
		CInterfaceAnsiString strTmp;
		if (SUCCEEDED(pCfg->GetParamValue(FALSE, "normal", "isimportcontact", &strTmp)))
		{
			if (::stricmp(strTmp.GetData(), "true") == 0)
				bImport = TRUE;
		}
		if (!bImport)
		{ 
			std::string strFileName;
			char szAppPath[MAX_PATH] = {0};
			//
			CSystemUtils::GetSystemAppPath(szAppPath, MAX_PATH - 1);
			strFileName = szAppPath;
			m_pCore->GetUserName(&strTmp);
			strFileName += "\\CoLine\\";
			strFileName += strTmp.GetData();
			strFileName += "@";
			m_pCore->GetUserDomain(&strTmp);
			strFileName += strTmp.GetData();
			strFileName += "\\contacts.db";
			if (CSystemUtils::FileIsExists(strFileName.c_str()))
			{
				ImportContactFromLocal(strFileName.c_str());
				PRINTDEBUGLOG(dtInfo, "contact import succ:%s", strFileName.c_str());
			} else
			{
				PRINTDEBUGLOG(dtInfo, "Contact file not found:%s", strFileName.c_str());
			}
			pCfg->SetParamValue(FALSE, "normal", "isimportcontact", "true");
		}
		//
		pCfg->Release();
	}
	return FALSE;
}

#pragma warning(default:4996)
