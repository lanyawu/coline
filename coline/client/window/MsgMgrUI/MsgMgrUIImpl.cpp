#include <Commonlib/systemutils.h>
#include <SmartSkin/smartskin.h>
#include <Core/common.h>
#include "../IMCommonlib/InterfaceAnsiString.h"
#include "../IMCommonLib/InterfaceUserList.h"
#include "../IMCommonLib/MessageList.h"
#include "../IMCommonLib/InterfaceFontStyle.h"
#include "../IMCommonLib/XmlNodeTranslate.h"
#include "MsgMgrUIImpl.h"

 


#define GROUP_LEAF_SIGN_ID   0xFFF00000  //分组的标识ID号
#define SMS_LEAF_SIGN_ID     0xFFF00001  //短信标识ID号
#define GROUP_NODE_SIGN_ID   0xFF000001  //讨论组节点ID号
#define SMS_NODE_SIGN_ID     0xFF000002  //短信节点ID号
#define OA_NODE_SIGN_ID      0xFF000003  //业务中心ID号
#define SYSTEM_NODE_SIGN_ID  0xFF000004  //系统消息节点ID号
#define FAX_NODE_SIGN_ID     0xFF000005  //传真

#pragma warning(disable:4996)

const char SKIN_HISTORY_BUTTON_XML[] = "<Control xsi:type=\"ImageButton\" name=\"historybutton\"  image=\"115\" text=\"历史记录\"/>";
const TCHAR UI_CONTACTS_TREE_NAME[] = L"contacttree";

#define MESSAGE_PAGE_COUNT  20

BOOL CALLBACK RichEditCallBack(HWND hWnd, DWORD dwEvent, char *szFileName, DWORD *dwFileNameSize,
			  char *szFileFlag, DWORD *dwFileFlagSize, DWORD dwFlag, void *pOverlapped)
{
	if (pOverlapped)
	{
		CMsgMgrUIImpl *pThis = (CMsgMgrUIImpl *) pOverlapped;
		return pThis->RECallBack(hWnd, dwEvent, szFileName, dwFileNameSize, szFileFlag, dwFileFlagSize, dwFlag);
	}
	return FALSE;
}

CMsgMgrUIImpl::CMsgMgrUIImpl(void):
               m_pCore(NULL),
			   m_hWnd(NULL)
{
	//
}


CMsgMgrUIImpl::~CMsgMgrUIImpl(void)
{
	if (m_hWnd)
		::SkinCloseWindow(m_hWnd);
	m_hWnd = NULL;
	if (m_pCore)
		m_pCore->Release();
	m_pCore = NULL;
}

//IUnknown
STDMETHODIMP CMsgMgrUIImpl::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if (::IsEqualGUID(riid, IID_IUnknown) || ::IsEqualGUID(riid, __uuidof(IMsgMgrUI)))
	{
		*ppv = (IMsgMgrUI *) this;
		_AddRef();
		return S_OK;
	} else if (::IsEqualGUID(riid, __uuidof(ICoreEvent)))
	{
		*ppv = (ICoreEvent *) this;
		_AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

void CMsgMgrUIImpl::ShowMgrFrameByHWND(HWND hWnd)
{
	CInterfaceAnsiString strUserName;
	IChatFrame *pFrame = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pFrame)))
	{
		pFrame->GetUserNameByHWND(hWnd, (IAnsiString *)&strUserName);
		pFrame->Release();
	}
	if (strUserName.GetSize() > 0)
		ShowMsgMgrFrame("p2p", strUserName.GetData(), NULL);
	else
	{
		IGroupFrame *pFrame = NULL;
		if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IGroupFrame), (void **)&pFrame)))
		{
			pFrame->GetGroupIdByHWND(hWnd, &strUserName);
			pFrame->Release();
		}
		if (strUserName.GetSize() > 0)
			ShowMsgMgrFrame("grp", strUserName.GetData(), NULL);
	}
}

	//
void CMsgMgrUIImpl::ExpandGroupTree(void *pParentNode)
{
    //讨论组列表
	CInterfaceUserList Groups;
	IMsgMgr *pMgr = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMsgMgr), (void **)&pMgr)))
	{
		pMgr->GetGroups(&Groups);
		LPORG_TREE_NODE_DATA pData;
		TCHAR szText[MAX_PATH] = {0};
		while (SUCCEEDED(Groups.PopFrontUserInfo(&pData)))
		{
			memset(szText, 0, sizeof(TCHAR) * MAX_PATH);
			pData->id = GROUP_LEAF_SIGN_ID;
			if (pData->szDisplayName)
				CStringConversion::UTF8ToWideChar(pData->szDisplayName, szText, MAX_PATH - 1);
			else
				CStringConversion::StringToWideChar(pData->szUserName, szText, MAX_PATH - 1);
			::SkinAddTreeChildNode(m_hWnd, UI_CONTACTS_TREE_NAME, pData->id, pParentNode, szText,
				TREENODE_TYPE_LEAF, pData, NULL, NULL, NULL);
		} //end while (
		pMgr->Release();
	} //end if (m_pCore->
}

//
void CMsgMgrUIImpl::ExpandSmsTree(void *pParentNode)
{
    //讨论组列表
	CInterfaceUserList smsList;
	IMsgMgr *pMgr = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMsgMgr), (void **)&pMgr)))
	{
		//pMgr->GetMsg("sms", 
		pMgr->Release();
	} //end if (m_pCore->
}

//ICoreEvent
STDMETHODIMP CMsgMgrUIImpl::DoCoreEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
                     LPARAM lParam, HRESULT *hResult)
{
	if (::stricmp(szType, "afterinit") == 0)
	{
		if ((::stricmp(szName, "chatWindow") == 0) || (::stricmp(szName, "groupWindow") == 0))
		{ 
			::SkinAddChildControl(hWnd, L"midtoolbar", SKIN_HISTORY_BUTTON_XML, NULL, NULL, -1); 
			::SkinSetControlAttr(hWnd, L"historybutton", L"tooltip", L"查看交流的历史记录");
			return S_OK;
		} //end if (::stricmp(szName...
	} else if (::stricmp(szType, "click") == 0)
	{
		if (::stricmp(szName, "historybutton") == 0)
		{
			ShowMgrFrameByHWND(hWnd);
		} else if (::stricmp(szName, "contacttree") == 0)
		{
			RefreshByTreeNode(hWnd);
		}  else if (::stricmp(szName, "prevPage") == 0)
		{
			int nPage = m_nCurrPage - 1;
			DisplayHistoryMsg(nPage);
		} else if (::stricmp(szName, "nextpage") == 0)
		{
			int nPage = m_nCurrPage + 1;
			DisplayHistoryMsg(nPage);
		} else if (::stricmp(szName, "searchbutton") == 0)
		{
			TCHAR szwTmp[MAX_PATH] = {0};
			int nSize = MAX_PATH - 1;
			::SkinGetControlTextByName(m_hWnd, L"searchedit", szwTmp, &nSize);
			if (nSize > 0)
			{
				char szTmp[MAX_PATH] = {0};
				CStringConversion::WideCharToString(szwTmp, szTmp, MAX_PATH - 1);
				SearchHistoryMsg(NULL, NULL, szTmp);
			} else
				::SkinMessageBox(m_hWnd, L"请输入搜索关键字", L"提示", MB_OK);
		} else if (::stricmp(szName, "import") == 0)
		{
			//导入
			CStringList_ FileList; 
			if (CSystemUtils::OpenFileDialog(NULL, m_hWnd, "打开聊天记录文件", "XML文件|*.xml", NULL,
				            FileList, FALSE))
			{
				if (!FileList.empty())
				{  
					if (ImportMsgFile(FileList.back().c_str()))
						::SkinMessageBox(hWnd, L"导入消息记录成功", L"提示", MB_OK);
					else
						::SkinMessageBox(hWnd, L"导入消息记录失败", L"提示", MB_OK);
				} //end if (!FileList..
			} //end if (CSystemUtils::
		} else if (::stricmp(szName, "export") == 0)
		{
			ExportCurrentNodeToFile(0);
		}
	} else if (::stricmp(szType, "menucommand") == 0)
	{
		DoMenuCommand(hWnd, szName, wParam, lParam);
	}  else if (::stricmp(szType, "keydown") == 0)
	{
		if (::stricmp(szName, "searchedit") == 0)
		{ 
			switch(wParam)
			{
			case VK_RETURN: 
					{
						TCHAR szwTmp[MAX_PATH] = {0};
						int nSize = MAX_PATH - 1;
						::SkinGetControlTextByName(m_hWnd, L"searchedit", szwTmp, &nSize);
						if (nSize > 0)
						{
							char szTmp[MAX_PATH] = {0};
							CStringConversion::WideCharToString(szwTmp, szTmp, MAX_PATH - 1);
							SearchHistoryMsg(NULL, NULL, szTmp);
						} else
							::SkinMessageBox(hWnd, L"请输入搜索关键字", L"提示", MB_OK);
						*hResult = 0;
						break;
					}
			} //end switch(...
		} //end if (::stricmp(..
	}
	return E_FAIL;
}

//
void CMsgMgrUIImpl::RefreshByTreeNode(HWND hWnd)
{
	//
	void *pSelNode = NULL;
	LPORG_TREE_NODE_DATA pSelData = NULL;
	TCHAR szName[MAX_PATH] = {0};
	int nSize = MAX_PATH - 1;
	CTreeNodeType tnType;
	if (::SkinGetSelectTreeNode(hWnd, L"contacttree", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
	{
		if (tnType == TREENODE_TYPE_GROUP)
		{
			if (pSelData && pSelData->bOpened == 0)
			{
				switch(pSelData->id)
				{
				case GROUP_NODE_SIGN_ID:
					ExpandGroupTree(pSelNode);
					break; 
				case SMS_NODE_SIGN_ID:
					ExpandSmsTree(pSelNode);
					break;
				case  OA_NODE_SIGN_ID:
					break;
				case SYSTEM_NODE_SIGN_ID:
					break;
				default:
					ExpandNodeByPid(pSelNode, pSelData->id);
					break;
				}
				pSelData->bOpened = TRUE;
			} // end if (pSelData ...					 
		} else
		{
			m_strTipName = szName;
			if (pSelData->id == GROUP_LEAF_SIGN_ID)
				ShowHistoryMsg("grp", pSelData->szUserName);
			else
				ShowHistoryMsg("p2p", pSelData->szUserName);
		} //end else if (tnType == ...
	} //end if (::GetSelectTreeNode(hWnd...
}

BOOL CMsgMgrUIImpl::DoMenuCommand(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szName, "SystemMenu") == 0)
	{
		switch(wParam)
		{
		case 50005:
			 ShowMsgMgrFrame(NULL, NULL, NULL);
			 break;
		}
	} else if (::stricmp(szName, "chatshortcutmenu") == 0)
	{
		if (wParam == 50005)
			ShowMgrFrameByHWND(hWnd);
	} else if (::stricmp(szName, "groupshortmenu") == 0)
	{
		if (wParam == 50005)
			ShowMgrFrameByHWND(hWnd);
	} else if (::stricmp(szName, "treeleaf") == 0)
	{
		if (wParam == 50005)
			ShowMgrFrameByTree(hWnd);
	} else if (::stricmp(szName, "importmenu") == 0)
	{
		if (wParam == 50002) //导出为XML文件
		{
			ExportCurrentNodeToFile(0);
		} else if (wParam == 50003) //导入xml文件
		{
			CStringList_ FileList; 
			if (CSystemUtils::OpenFileDialog(NULL, m_hWnd, "打开聊天记录文件", "XML文件|*.xml", NULL,
				            FileList, FALSE))
			{
				if (!FileList.empty())
				{  
					ImportMsgFile(FileList.back().c_str());
				} //end if (!FileList..
			} //end if (CSystemUtils::
		} //end els if (wParam
	} else if (::stricmp(szName, "msgpopupmenu") == 0)
	{
		switch(wParam)
		{
		case 1: //复制
			::SkinRichEditCommand(hWnd, L"messagedisplay", "copy", NULL);
			break;
		case 2: //全选
			::SkinRichEditCommand(hWnd, L"messagedisplay", "selectall", NULL);
			break;
		case 3: //删除
			DeleteCurrentMsg(hWnd);
			break;
		}
	} else if (::stricmp(szName, "msgtreemenu") == 0)
	{
		switch(wParam)
		{
		case 1: //导出消息记录 
			ExportCurrentNodeToFile(0);
			break;
		case 2: //删除消息记录
			DeleteMsgByTreeNode(hWnd);
			break;
		case 3: //刷新
			RefreshByTreeNode(hWnd);
			break;
		}
	}
	return FALSE;
}

//
void  CMsgMgrUIImpl::DeleteMsgByTreeNode(HWND hWnd)
{
	//
	void *pSelNode = NULL;
	LPORG_TREE_NODE_DATA pSelData = NULL;
	TCHAR szName[MAX_PATH] = {0};
	int nSize = MAX_PATH - 1;
	CTreeNodeType tnType;
	if (::SkinGetSelectTreeNode(hWnd, L"contacttree", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
	{
		if (tnType == TREENODE_TYPE_LEAF)
		{  
			CStdString_ strTip = L"是否删除与<";
			strTip += szName;
			strTip += L">所有聊天记录";
			if (::SkinMessageBox(hWnd, strTip, L"提示", 2) == IDOK)
			{
				IMsgMgr *pMgr = NULL;
				if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMsgMgr), (void **)&pMgr)))
				{
					if (pSelData->id == GROUP_LEAF_SIGN_ID)
					{
						pMgr->ClearMsg(0, "grp", pSelData->szUserName); 
					} else
					{
						pMgr->ClearMsg(0, "p2p", pSelData->szUserName);
					}
					::SkinRichEditCommand(hWnd, L"messagedisplay", "clear", NULL);
					pMgr->Release();
				} //end if (SUCCEEDED(
			} //end if (::SkinMessageBox(hWnd..
		} //end else if (tnType == ...
	} //end if (::GetSelectTreeNode(hWnd...
}

void CMsgMgrUIImpl::DeleteCurrentMsg(HWND hWnd)
{
	BOOL bSucc = FALSE;
	char szId[32] = {0};
	if (::SkinGetREChatId(hWnd, L"messagedisplay", szId))
	{
		CStdString_ strTip = L"是否删除当前选择消息记录";
		if (::SkinMessageBox(hWnd, strTip, L"提示", 2) == IDOK)
		{
			IMsgMgr *pMgr = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMsgMgr), (void **)&pMgr)))
			{
				bSucc = SUCCEEDED(pMgr->ClearMsg(::atoi(szId), m_strCurrType.c_str(), NULL));
				pMgr->Release();
			} 
			if (bSucc)
			{
				::SkinREClearChatMsg(hWnd, L"messagedisplay", szId);
				::SkinMessageBox(hWnd, L"删除记录成功", L"提示", MB_OK);
			} else
				::SkinMessageBox(hWnd, L"删除记录失败", L"提示", MB_OK);
		} //end if (::SkinMessageBox(hWnd,
	} //end if (::SkinGetREChatId(
}

void CMsgMgrUIImpl::ExportCurrentNodeToFile(int nFileType)
{
	CStringList_ FileList;
	BOOL bSucc = FALSE;
	char szFileName[MAX_PATH] = {0};
	while (TRUE)
	{
		char szTmpFile[MAX_PATH] = {0}; 
		if (CSystemUtils::OpenFileDialog(NULL, m_hWnd, "另存文件为", "XML文件|*.xml", szTmpFile,
				            FileList, FALSE, TRUE))
		{
			if (!FileList.empty())
			{ 
				memset(szFileName, 0, MAX_PATH);
				strncpy(szFileName, FileList.back().c_str(), MAX_PATH - 1);
				char szExTmp[MAX_PATH] = {0};
				CSystemUtils::ExtractFileExtName(szFileName, szExTmp, MAX_PATH - 1);
				if (strlen(szExTmp) == 0)
				{ 
					::strcat(szFileName, ".xml");
				}
				if (CSystemUtils::FileIsExists(szFileName))
				{
					if (::SkinMessageBox(m_hWnd, L"文件已经存在，是否覆盖？", L"提示", 2) == IDOK)
					{
						bSucc = TRUE;
						break;
					} else
					{
						continue;
					}
				} else
				{
					bSucc = TRUE;
					break;
				}
			} else
			{
				bSucc = FALSE;
				break;
			} //end if (!FileList.empty())
		} else
		{
			bSucc = FALSE;
			break;
		} //else if (CSystemUtils::OpenFileDialog(NULL, 
	} //end while(..
	if (bSucc)
	{
		void *pSelNode = NULL;
		LPORG_TREE_NODE_DATA pSelData = NULL;
		TCHAR szName[MAX_PATH] = {0};
		int nSize = MAX_PATH - 1;
		CTreeNodeType tnType;
		if (::SkinGetSelectTreeNode(m_hWnd, L"contacttree", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
		{
			if (tnType == TREENODE_TYPE_LEAF)
			{
				if (ExportP2PMsgToFile(pSelData->szUserName, nFileType, szFileName))
					::SkinMessageBox(m_hWnd, L"导出成功", L"提示", MB_OK);
			} else
				::SkinMessageBox(m_hWnd, L"只支持单个用户及单个讨论组的聊天记录导出", L"提示", MB_OK);
		} else 
		{
			::SkinMessageBox(m_hWnd, L"请选择要导出记录的用户", L"提示", MB_OK);
		}//end if (::SkinGetSelectTreeNode(  
	} //end if (bSucc)
}

void CMsgMgrUIImpl::ShowMgrFrameByTree(HWND hWnd)
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
			ShowMsgMgrFrame("p2p", pSelData->szUserName, NULL); 
		} //end else if (tnType == ...
	} //end if (::GetSelectTreeNode(hWnd...
}

//广播消息
STDMETHODIMP CMsgMgrUIImpl::DoBroadcastMessage(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData)
{
	return E_NOTIMPL;
}

void CMsgMgrUIImpl::SearchHistoryMsg(const char *szType, const char *szUserName, const char *szKey)
{
	IMsgMgr *pMgr = NULL;
	int nPageCount = 0;
	::SkinRichEditCommand(m_hWnd, L"messagedisplay", "clear", NULL);
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMsgMgr), (void **)&pMgr)))
	{
		int nCount = pMgr->GetSearchMsgCount(szType, szKey, szUserName);
		if (szType)
			m_strCurrType = szType;
		else
			m_strCurrType.clear();
		if (szUserName)
			m_strUserName = szUserName;
		else
			m_strUserName.clear();
		m_strKey = szKey;
		m_nTotalPage = nCount / MESSAGE_PAGE_COUNT;		
		if ((nCount % MESSAGE_PAGE_COUNT) != 0) 
			m_nTotalPage ++;
		nPageCount = m_nTotalPage;
		if (nPageCount < 0)
			nPageCount = 0;
		pMgr->Release();
	} 
	RefreshWhoTip();
	if (m_nTotalPage > 0)
		DisplayHistoryMsg(nPageCount);
	else
		::SkinMessageBox(m_hWnd, L"没有搜索到任何结果", L"提示", MB_OK);
}

void CMsgMgrUIImpl::ShowHistoryMsg(const char *szType, const char *szUserName)
{
	IMsgMgr *pMgr = NULL;
	int nPageCount = 0;
	::SkinRichEditCommand(m_hWnd, L"messagedisplay", "clear", NULL);
	
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMsgMgr), (void **)&pMgr)))
	{
		int nCount = pMgr->GetMsgCount(szType, szUserName);
		m_strCurrType = szType;
		m_strUserName = szUserName;
		m_strKey.clear();
		m_nTotalPage = nCount / MESSAGE_PAGE_COUNT;		
		if ((nCount % MESSAGE_PAGE_COUNT) != 0)	 
			m_nTotalPage ++;
		nPageCount = m_nTotalPage;
		if (nPageCount < 0)
			nPageCount = 0;
		pMgr->Release();
	}
	RefreshWhoTip();
	if (m_nTotalPage > 0)
		DisplayHistoryMsg(nPageCount);
}

void CMsgMgrUIImpl::DisplayMessageList(IMessageList *pMsgList)
{
	::SkinRichEditCommand(m_hWnd, L"messagedisplay", "clear", NULL);

	if (stricmp(m_strCurrType.c_str(), "grp") == 0)
	{
		IGroupFrame *pFrame = NULL;
		if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IGroupFrame), (void **)&pFrame)))
		{
			int nIdx = 0;
			int nMsgId;
			CInterfaceAnsiString strMsg;
			CInterfaceAnsiString strDspName, strDspTime, strDspText;
			CInterfaceFontStyle fs;
			CInterfaceAnsiString strMsgType;
			CInterfaceAnsiString strFontName;			
			CCharFontStyle cf;
			BOOL bSelf;
			int nClr;
			while (SUCCEEDED(pMsgList->GetRawMsg(nIdx, &nMsgId, (IAnsiString *)&strMsg)))
			{
				CInterfaceAnsiString strUserList;
				pFrame->ParserGroupProtocol(strMsg.GetData(), strMsg.GetSize(), (IAnsiString *)&strDspName, (IAnsiString *) &strUserList,
					(IAnsiString *)&strDspTime, (IAnsiString *)&strDspText, (IFontStyle *)&fs, &strMsgType, &bSelf);

				if (::stricmp(strMsgType.GetData(), "tip") == 0)
				{
					TCHAR szwTime[64] = {0};
					CStdString_ strTip;
					if (strDspTime.GetSize() > 0)
					{
						CStringConversion::StringToWideChar(strDspTime.GetData(), szwTime, 63);
						strTip += szwTime;
						strTip += L"\n";
					}
					int nLen = strDspText.GetSize();
					TCHAR *szwText = new TCHAR[nLen + 1];
					CStringConversion::StringToWideChar(strDspText.GetData(), szwText, nLen);
					strTip += szwText;
					::SkinRichEditInsertTip(m_hWnd, L"messagedisplay", NULL, 0, strTip.GetData());
					delete []szwText;
				} else
				{
					if (bSelf)
						nClr = UI_NICK_NAME_COLOR;
					else
						nClr = UI_NICK_NAME_COLOR_PEER;
					cf.cfColor = fs.GetColor();
					cf.nFontSize = fs.GetSize();
					fs.GetName((IAnsiString *)&strFontName);
					CStringConversion::StringToWideChar(strFontName.GetData(), cf.szFaceName, 31);
					cf.nFontStyle = 0;
					if (fs.GetBold())
						cf.nFontStyle |= CFE_BOLD;
					if (fs.GetItalic())
						cf.nFontStyle |= CFE_ITALIC;
					if (fs.GetStrikeout())
						cf.nFontStyle |= CFE_STRIKEOUT;
					if (fs.GetUnderline())
						cf.nFontStyle |= CFE_UNDERLINE;
					char szId[16] = {0};
					::itoa(nMsgId, szId, 10);
					TCHAR *szDspUserList = new TCHAR[strUserList.GetSize() + 1];
					memset(szDspUserList, 0, sizeof(TCHAR) * (strUserList.GetSize() + 1));
					CStringConversion::UTF8ToWideChar(strUserList.GetData(), szDspUserList, strUserList.GetSize());
					CStdString_ strTip = L"参与人(";
					strTip += szDspUserList;
					strTip += L")";
					::SkinRichEditInsertTip(m_hWnd, L"messagedisplay", NULL, 0, strTip.GetData());
					::SkinAddRichChatText(m_hWnd, L"messagedisplay", szId, 0, strDspName.GetData(), strDspTime.GetData(), 
						        strDspText.GetData(), &cf,	nClr, TRUE, FALSE);
					
				}

				nIdx ++;
			} //end while(...
			pFrame->Release(); 
		}
	} else
	{
		IChatFrame *pFrame = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pFrame)))
		{
		    int nIdx = 0;
			int nMsgId;
			CInterfaceAnsiString strMsg;
			CInterfaceAnsiString strDspName, strDspTime, strDspText;
			CInterfaceFontStyle fs;
			CInterfaceAnsiString strFontName;
			CInterfaceAnsiString strMsgType;
			CCharFontStyle cf;
			BOOL bSelf;
			int nClr;
			while (SUCCEEDED(pMsgList->GetRawMsg(nIdx, &nMsgId, (IAnsiString *)&strMsg)))
			{
				pFrame->ParserP2PProtocol(strMsg.GetData(), strMsg.GetSize(), (IAnsiString *)&strDspName,
					(IAnsiString *)&strDspTime, (IAnsiString *)&strDspText, (IFontStyle *)&fs, &strMsgType, &bSelf);

				if (::stricmp(strMsgType.GetData(), "tip") == 0)
				{
					TCHAR szwTime[64] = {0};
					CStdString_ strTip;
					if (strDspTime.GetSize() > 0)
					{
						CStringConversion::StringToWideChar(strDspTime.GetData(), szwTime, 63);
						strTip += szwTime;
						strTip += L"\n";
					}
					int nLen = strDspText.GetSize();
					TCHAR *szwText = new TCHAR[nLen + 1];
					CStringConversion::StringToWideChar(strDspText.GetData(), szwText, nLen);
					strTip += szwText;
					::SkinRichEditInsertTip(m_hWnd, L"messagedisplay", NULL, 0, strTip);
					delete []szwText;
				} else
				{
					if (bSelf)
						nClr = UI_NICK_NAME_COLOR;
					else
						nClr = UI_NICK_NAME_COLOR_PEER;
					cf.cfColor = fs.GetColor();
					cf.nFontSize = fs.GetSize();
					fs.GetName((IAnsiString *)&strFontName);
					CStringConversion::StringToWideChar(strFontName.GetData(), cf.szFaceName, 31);
					cf.nFontStyle = 0;
					if (fs.GetBold())
						cf.nFontStyle |= CFE_BOLD;
					if (fs.GetItalic())
						cf.nFontStyle |= CFE_ITALIC;
					if (fs.GetStrikeout())
						cf.nFontStyle |= CFE_STRIKEOUT;
					if (fs.GetUnderline())
						cf.nFontStyle |= CFE_UNDERLINE;
			        char szId[16] = {0};
					::itoa(nMsgId, szId, 10);
					::SkinAddRichChatText(m_hWnd, L"messagedisplay", szId, 0, strDspName.GetData(), strDspTime.GetData(), 
						        strDspText.GetData(), &cf,	nClr, TRUE, FALSE);					 
				}
				nIdx ++;
			} //end while(...
			pFrame->Release();
		}	
	}
}

//
BOOL  CMsgMgrUIImpl::InsertMsgNode(TiXmlElement *pNode, const int Id, IFontStyle *pfs, const char *szFromId, const char *szFrom, 
		const char *szToId, const char *szTo, const char *szTime, const char *szText)
{
	static char SESSIOND_ID[16] = {0};
	static char DATE____[32] = {0};
	static char TIME____[32] = {0};
	static char FROM____[256] = {0};
	static char TO______[256] = {0};
	static char FONT_NAME____[128] = {0};
	CStringConversion::UTF8ToString(szFrom, FROM____, 255);
	CStringConversion::UTF8ToString(szTo, TO______, 255);
	memset(SESSIOND_ID, 0, 16);
	memset(DATE____, 0, 32);
	memset(TIME____, 0, 32);
	::itoa(Id, SESSIOND_ID, 10);
	TiXmlElement *pChild = new TiXmlElement("Message");
	strncpy(DATE____, szTime, 10);
	strncpy(TIME____, (szTime + 11), 10);
	pChild->SetAttribute("Date", DATE____);
	pChild->SetAttribute("Time", TIME____);
	pChild->SetAttribute("DateTime", szTime);
	pChild->SetAttribute("SessionID", SESSIOND_ID);
	TiXmlElement *pFrom = new TiXmlElement("From");
	TiXmlElement *pUser = new TiXmlElement("User");
	pUser->SetAttribute("FriendlyName", szFrom);
	pUser->SetAttribute("id", szFromId);
	pFrom->LinkEndChild(pUser);
	pChild->LinkEndChild(pFrom);
	TiXmlElement *pTo = new TiXmlElement("To");
	TiXmlElement *pUser2 = new TiXmlElement("User");
	pUser2->SetAttribute("FriendlyName", szTo);
	pUser2->SetAttribute("id", szToId);
	pTo->LinkEndChild(pUser2);
	pChild->LinkEndChild(pTo);
	TiXmlElement *pText = new TiXmlElement("Text");
	//转Font
	std::string strFontStyle = "font-family:";
	CInterfaceAnsiString strName;
	if (SUCCEEDED(pfs->GetName(&strName)))
	{
		CStringConversion::StringToUTF8(strName.GetData(), FONT_NAME____, 127);
		strFontStyle += FONT_NAME____;
	}
	if (pfs->GetSize() > 0)
	{
		strFontStyle += ";font-size:";
		memset(SESSIOND_ID, 0, 16);
		::itoa(pfs->GetSize(), SESSIOND_ID, 10); 
		strFontStyle += SESSIOND_ID;
	}
	std::string strClr;
 	strFontStyle += ";font-color:";
	CXmlNodeTranslate::FontColorToString(pfs->GetColor(), strClr);	 
	strFontStyle += strClr;
	pText->SetAttribute("Style", strFontStyle.c_str());
	int nSize = ::strlen(szText);
	char *pTmp = new char[nSize * 2];  //UTF8
	memset(pTmp, 0, nSize * 2);
	CStringConversion::StringToUTF8(szText, pTmp, nSize * 2 - 1);
	TiXmlText *pBody = new TiXmlText(pTmp);
	delete []pTmp;
	pText->LinkEndChild(pBody);
	pChild->LinkEndChild(pText);
	return (pNode->LinkEndChild(pChild) != NULL);
}


BOOL CMsgMgrUIImpl::DoImportMsg(IMsgMgr *pMgr, const char *szType, TiXmlElement *pNode)
{
	TiXmlString strMsg;
	if (::stricmp(szType, "p2p") == 0)
	{
		//<msg type="p2p" from="admin@gocom" to="wuxiaozhong@gocom" Receipt="" datetime="2010-09-28 15:09:49">
		//    <font name="Arial" size="9pt" color="#000000" bold="false" underline="false" strikeout="false" italic="false"/>
		//    <body>adfadfadf</body>
		//</msg>
		//
		std::string strXml = "<msg type=\"p2p\" from=\"";
		std::string strFrom;
		std::string strTo;
		std::string strTime;
		std::string strText;
		TiXmlElement *pFrom = pNode->FirstChildElement("From");
		if (pFrom)
		{
			TiXmlElement *pUser = pFrom->FirstChildElement("User");
			if (pUser && pUser->Attribute("id"))
			{
				strFrom = pUser->Attribute("id");
				strXml += strFrom;
			} else
				return FALSE;
		}
		TiXmlElement *pTo = pNode->FirstChildElement("To");
		if (pTo)
		{
			TiXmlElement *pUser = pTo->FirstChildElement("User");
			if (pUser && pUser->Attribute("id"))
			{
				strTo = pUser->Attribute("id");
				strXml += "\" to=\"";
				strXml += strTo;
			} else
				return FALSE;
		}
		if (pNode->Attribute("DateTime"))
		{
			strTime = pNode->Attribute("DateTime");
			strXml += "\" datetime=\"";
			strXml += strTime;
		} 
		strXml += "\"/><font name=\"Arial\" size=\"9pt\" color=\"#000000\" bold=\"false\" underline=\"false\" strikeout=\"false\" italic=\"false\"/><body>";
		TiXmlElement *pText = pNode->FirstChildElement("Text");
		if (pText)
		{
			if (pText->GetText())
			{
				strText = pText->GetText();
				strXml += strText;
			}
		}
		strXml += "</body>";
		strXml+= "</msg>";
		pMgr->SaveMsg("p2p", strFrom.c_str(), strTo.c_str(), strTime.c_str(), strXml.c_str(), strText.c_str());
	} else
	{
		//
	}
	return FALSE;
}

BOOL CMsgMgrUIImpl::ImportMsgFile(const char *szFileName)
{
	TiXmlDocument xmldoc;
	BOOL bSucc = FALSE;
	if (xmldoc.LoadFile(szFileName))
	{
		TiXmlElement *pNode = xmldoc.FirstChildElement();
		if (pNode)
		{
			std::string strUserName;
			GetLogonUserName(strUserName);
			const char *szOwner = pNode->Attribute("owner");
			const char *szType = pNode->Attribute("msgtype");
			if (szOwner && szType && ::stricmp(strUserName.c_str(), szOwner) == 0)
			{
				IMsgMgr *pMgr = NULL;
				if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMsgMgr), (void **)&pMgr)))
				{
					TiXmlElement *pMsg = pNode->FirstChildElement("Message");
					while (pMsg)
					{
						DoImportMsg(pMgr, szType, pMsg);
						pMsg = pMsg->NextSiblingElement("Message");
					}
					pMgr->Release();
				} //end if (SUCCEEDED(..
			} //end if (szOwner...
		} //end if (pNode)
				
	} //end if (xmldoc..
	return FALSE;
}

BOOL CMsgMgrUIImpl::ExportP2PMsgToFile(const char *szUserName, int nFileType, const char *szFileName)
{
	CMessageList mlList;
	IMsgMgr *pMgr = NULL;
	BOOL bSucc = FALSE;
	CInterfaceAnsiString strRealName;
	CInterfaceAnsiString strUserRealName; 
	std::string strUserName;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMsgMgr), (void **)&pMgr)))
	{
		GetLogonUserName(strUserName);
		m_pCore->GetUserNickName(&strRealName); 
		IContacts *pContact = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
		{
			pContact->GetRealNameById(szUserName, NULL, &strUserRealName);
			pContact->Release();
		}
		if (SUCCEEDED(pMgr->GetRawMsg("p2p", szUserName, 0, 99999999, &mlList)))
		{
			TiXmlDocument xmldoc;
			static char MESSAGE_LOG_INIT__[] = "<Log FirstSessionID=\"1\" LastSessionID=\"9999999\"/>";
			xmldoc.Load(MESSAGE_LOG_INIT__, strlen(MESSAGE_LOG_INIT__));
			TiXmlElement *pNode = xmldoc.FirstChildElement();
			pNode->SetAttribute("owner", strUserName.c_str());
			pNode->SetAttribute("username", szUserName);
			pNode->SetAttribute("msgtype", "p2p");
			IChatFrame *pFrame = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pFrame)))
			{
			    int nIdx = 0;
				int nMsgId;
				CInterfaceAnsiString strMsg;
				CInterfaceAnsiString strDspName, strDspTime, strDspText;
				CInterfaceFontStyle fs;
				CInterfaceAnsiString strFontName;
				CInterfaceAnsiString strMsgType; 
				BOOL bSelf; 
				int nMinId = 0;
				while (SUCCEEDED(mlList.GetRawMsg(nIdx, &nMsgId, (IAnsiString *)&strMsg)))
				{
					if (nMinId == 0)
						nMinId = nMsgId;
					pFrame->ParserP2PProtocol(strMsg.GetData(), strMsg.GetSize(), (IAnsiString *)&strDspName,
						(IAnsiString *)&strDspTime, (IAnsiString *)&strDspText, (IFontStyle *)&fs, &strMsgType, &bSelf);

					if (::stricmp(strMsgType.GetData(), "tip") == 0)
					{ 
						if (bSelf)
							InsertMsgNode(pNode, nMsgId, &fs, strUserName.c_str(), strRealName.GetData(), "system", "tip", strDspTime.GetData(),
							        strDspText.GetData());
						else
							InsertMsgNode(pNode, nMsgId, &fs, "system", "tip", strUserName.c_str(), strRealName.GetData(), strDspTime.GetData(),
							      strDspText.GetData());  
					} else
					{						 
				 
						if (bSelf)
							InsertMsgNode(pNode, nMsgId, &fs, strUserName.c_str(), strRealName.GetData(), szUserName, strUserRealName.GetData(), 
							        strDspTime.GetData(), strDspText.GetData());
						else
							InsertMsgNode(pNode, nMsgId, &fs, szUserName, strUserRealName.GetData(), strUserName.c_str(), strRealName.GetData(),
							       strDspTime.GetData(), strDspText.GetData());				 
					}
					nIdx ++;
				} //end while(...
				pNode->SetAttribute("FirstSessionID", nMinId);
			    pNode->SetAttribute("LastSessionID", nMsgId);
				pFrame->Release();
			} //end if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pFrame)))
			
			FILE *fp = fopen(szFileName, "w+");
			if (fp)
			{
				static char XML_HEADER[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<?xml-stylesheet type='text/xsl' href='MessageLog.xsl'?>\n";
				fwrite(XML_HEADER, strlen(XML_HEADER), 1, fp); 
				xmldoc.SaveFile(fp);
				fclose(fp);
				bSucc = TRUE;
			} 
		} //end if (SUCCEEDED(pMgr->GetRawMsg("p2p", szUserName, 0, 99999999, &mlList)))
		pMgr->Release();
	}
	return bSucc;
}

void CMsgMgrUIImpl::RefreshWhoTip()
{
	CStdString_ strTip = L"与";
	if (m_strKey.size() == 0)
	{
		if (::stricmp(m_strCurrType.c_str(), "p2p") == 0)
		{
			IContacts *pContact = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
			{
				CInterfaceAnsiString strTmp; 
				strTip += L"用户 \"";
				if (SUCCEEDED(pContact->GetRealNameById(m_strUserName.c_str(), NULL, &strTmp)))
				{
					TCHAR szRealName[128] = {0};
					CStringConversion::UTF8ToWideChar(strTmp.GetData(), szRealName, 127);
					strTip += szRealName;
				} else
					strTip += m_strTipName;
				strTip += L"\"";
				pContact->Release();
			}
		} else if (::stricmp(m_strCurrType.c_str(), "grp") == 0)
		{
			IGroupFrame *pFrame = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IGroupFrame), (void **)&pFrame)))
			{
				CInterfaceAnsiString strTmp;
				strTip += L"讨论组 \"";
				if (SUCCEEDED(pFrame->GetGroupNameById(m_strUserName.c_str(), &strTmp)))
				{
					TCHAR szRealName[128] = {0};
					CStringConversion::UTF8ToWideChar(strTmp.GetData(), szRealName, 127);
					strTip += szRealName; 
				} else
					strTip += m_strTipName;
				strTip += L"\"";
				pFrame->Release();
			}
		}
	} else
	{
		TCHAR szTmp[MAX_PATH] = {0};
		CStringConversion::StringToWideChar(m_strKey.c_str(), szTmp, MAX_PATH - 1);
		strTip = L"搜索<";
		strTip += szTmp;
		strTip += L">";
	}
	strTip += L"的聊天记录";
	::SkinSetControlTextByName(m_hWnd, L"whomsg", strTip.GetData());
}

void CMsgMgrUIImpl::DisplayHistoryMsg(const int nPage)
{
	if (nPage <= 0)
		::SkinMessageBox(m_hWnd, L"已经是第一页", L"提示", MB_OK);
	else if (nPage > m_nTotalPage)
		::SkinMessageBox(m_hWnd, L"已经是最后一页", L"提示", MB_OK);
	else
	{
		m_nCurrPage = nPage;
		IMsgMgr *pMgr = NULL;
		TCHAR szwTmp[40] = {0};
		wsprintf(szwTmp, L"%d", m_nCurrPage);
		::SkinSetControlTextByName(m_hWnd, L"inputpage", szwTmp);
		memset(szwTmp, 0, sizeof(TCHAR) * 40);
		wsprintf(szwTmp, L"%d 页", m_nTotalPage);
		::SkinSetControlTextByName(m_hWnd, L"currpage", szwTmp);
		if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMsgMgr), (void **)&pMgr)))
		{			
			CMessageList msgList;
			if (m_strKey.size() == 0)
			{
				if (SUCCEEDED(pMgr->GetRawMsg(m_strCurrType.c_str(), m_strUserName.c_str(), nPage, MESSAGE_PAGE_COUNT,
					     (IMessageList*)&msgList)))
				{
					DisplayMessageList((IMessageList *)&msgList);
				}
			} else
			{
				if (SUCCEEDED(pMgr->SearchRawMsg(m_strKey.c_str(), m_strCurrType.c_str(), m_strUserName.c_str(),
					nPage, MESSAGE_PAGE_COUNT, (IMessageList *)&msgList)))
				{
					DisplayMessageList((IMessageList *)&msgList);
				} //end if (SUCCEEDED(
			} //else if (m_strKey..
			
			pMgr->Release();
		} //end if (m_pCore ...
	} //end else if (...
}

BOOL CMsgMgrUIImpl::ExpandNodeByPid(void *pParentNode, const int nPid)
{
	IContacts *pContact = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
	{
		pContact->ExpandTreeNodeToUI(m_hWnd, UI_CONTACTS_TREE_NAME, pParentNode, nPid);
		pContact->Release();
	}
	return FALSE;
}

STDMETHODIMP CMsgMgrUIImpl::SetCoreFrameWork(ICoreFrameWork *pCore)
{
	if (m_pCore)
		m_pCore->Release();
	m_pCore = pCore;
	if (m_pCore)
	{
		m_pCore->AddRef();
		//event
		m_pCore->AddOrderEvent((ICoreEvent *) this, "chatWindow", "chatWindow", "afterinit");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "groupWindow", "groupWindow", "afterinit");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "chatWindow", "historybutton", "click");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "groupwindow", "historybutton", "click");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "MainWindow", "SystemMenu", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "MainWindow", "treeleaf", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "chatwindow", "chatshortcutmenu", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "groupwindow", "groupshortmenu", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "MsgMgrWindow", NULL, NULL);
	}
	return S_OK;
}

STDMETHODIMP CMsgMgrUIImpl::GetSkinXmlString(IAnsiString *szXmlString)
{
	HRESULT hr = E_FAIL;
	IConfigure *pCfg = NULL; 
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{ 
		hr = pCfg->GetSkinXml("MsgMgrUI.xml",szXmlString); 
		pCfg->Release();
	}
	return hr;   
}

void CMsgMgrUIImpl::GetLogonUserName(std::string &strUserName)
{
	CInterfaceAnsiString strTmp;
	m_pCore->GetUserName(&strTmp);
	strUserName = strTmp.GetData();
	m_pCore->GetUserDomain(&strTmp);
	strUserName += "@";
	strUserName += strTmp.GetData();
}

//
STDMETHODIMP CMsgMgrUIImpl::CoreFrameWorkError(int nErrorNo, const char *szErrorMsg)
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
STDMETHODIMP CMsgMgrUIImpl::DoWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes)
{
	if (uMsg == WM_DESTROY)
	{
		if (hWnd == m_hWnd)
		{
			//save frame pos
			RECT rc = {0};
			m_hWnd = NULL;
			if (::GetWindowRect(hWnd, &rc))
			{
				IConfigure *pCfg = NULL;		
				HRESULT hr = m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg);
				if (SUCCEEDED(hr))
				{ 
					std::string strRect;
					CSystemUtils::RectToString(rc, strRect);
					pCfg->SetParamValue(FALSE, "Position", "MsgMgrFrame", strRect.c_str());
					pCfg->Release();
				}
			}
			return S_OK;
		}
	}
	return E_FAIL;
}

//IMsgMgrUI
STDMETHODIMP CMsgMgrUIImpl::ShowMsgMgrFrame(const char *szType, const char *szInitUserName, LPRECT lprc)
{

	if (m_hWnd && ::IsWindow(m_hWnd))
	{
		if (::ShowWindow(m_hWnd, SW_SHOW))
		{
			CSystemUtils::BringToFront(m_hWnd);
			Navigate2(szType, szInitUserName);
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
					if (SUCCEEDED(pCfg->GetParamValue(FALSE, "Position", "MsgMgrFrame", (IAnsiString *)&strPos)))
					{
						CSystemUtils::StringToRect(&rcSave, strPos.GetData());
					}
					if (!::IsRectEmpty(&rcSave))
						rc = rcSave;
				}
				
			    IUIManager *pUI = NULL;
				if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)) && pUI)
				{
					pUI->CreateUIWindow(NULL, "MsgMgrWindow", &rc, WS_POPUP | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX,
					                0, L"消息管理器", &m_hWnd);				
					if (::IsWindow(m_hWnd))
					{
						 ::ShowWindow(m_hWnd, SW_SHOW);
						 ::SkinSetControlAttr(m_hWnd, L"logo", L"image", L"284");
					}
					pUI->OrderWindowMessage("MsgMgrWindow", m_hWnd, WM_DESTROY, (ICoreEvent *) this);
					//
					LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
					memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
					pData->bOpened = TRUE;
					pData->id = -1;
					//设置回调
				    ::SkinSetRichEditCallBack(m_hWnd, L"messagedisplay", RichEditCallBack, this);

					//设置是否智能合并
					CInterfaceAnsiString strValue;
					if (SUCCEEDED(pCfg->GetParamValue(FALSE, "normal", "aimsg", &strValue)))
					{
						if (::stricmp(strValue.GetData(), "false") == 0)
							::SkinSetControlAttr(m_hWnd, L"messagedisplay", L"mergemsg", L"false");
					}
					//设置是否透明
					if (SUCCEEDED(pCfg->GetParamValue(FALSE, "normal", "msgtransparent", &strValue)))
					{
						if (::stricmp(strValue.GetData(), "true") == 0)
						{
							::SkinSetControlAttr(m_hWnd, L"messagedisplay", L"transparent", L"true");
							if (SUCCEEDED(pCfg->GetParamValue(FALSE, "normal", "transimagefile", &strValue)))
							{
								if ((strValue.GetSize() > 0) && CSystemUtils::FileIsExists(strValue.GetData()))
								{
									TCHAR szTmp[MAX_PATH] = {0};
									CStringConversion::StringToWideChar(strValue.GetData(), szTmp, MAX_PATH - 1);
									::SkinSetControlAttr(m_hWnd, L"chatdisplaycanvs", L"imagefile", szTmp);
								}
							} //end if (SUCCEEDED(pCfg->
						}
					}
				    void *pSaveNode = ::SkinAddTreeChildNode(m_hWnd, UI_CONTACTS_TREE_NAME, pData->id, NULL,  L"联系人", 
			               TREENODE_TYPE_GROUP, pData, NULL, NULL, NULL);
					IContacts *pContact = NULL;
					if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
					{
						if (szType && szInitUserName)
						{ 
							pContact->DrawContactToUI(m_hWnd, UI_CONTACTS_TREE_NAME, szInitUserName, pSaveNode, FALSE, FALSE, 0); 
							Navigate2(szType, szInitUserName);
						} else
						{
							pContact->DrawContactToUI(m_hWnd, UI_CONTACTS_TREE_NAME, "", pSaveNode, FALSE, FALSE, 0); 
						}
						pContact->Release();
					}
 
					pData = new ORG_TREE_NODE_DATA();
					memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
					pData->bOpened = FALSE;
					pData->id = GROUP_NODE_SIGN_ID;
					if (szType && ::stricmp(szType, "grp") == 0)
					{
						pData->bOpened = TRUE;
					}
				    pSaveNode = ::SkinAddTreeChildNode(m_hWnd, UI_CONTACTS_TREE_NAME, pData->id, NULL,  L"讨论组", 
			               TREENODE_TYPE_GROUP, pData, NULL, NULL, NULL);
					

					if (szType && ::stricmp(szType, "grp") == 0)
					{
						ExpandGroupTree(pSaveNode);
						::SkinExpandTree(m_hWnd, UI_CONTACTS_TREE_NAME, pSaveNode, TRUE, TRUE);
					}

					/*pData = new ORG_TREE_NODE_DATA();
					memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
					pData->bOpened = FALSE;
					pData->id = SMS_NODE_SIGN_ID;
				    ::SkinAddTreeChildNode(m_hWnd, UI_CONTACTS_TREE_NAME, pData->id, NULL,  L"短信", 
			               TREENODE_TYPE_GROUP, pData, NULL, NULL, NULL);
					pData = new ORG_TREE_NODE_DATA();
					memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
					pData->bOpened = FALSE;
					pData->id = FAX_NODE_SIGN_ID;
				    ::SkinAddTreeChildNode(m_hWnd, UI_CONTACTS_TREE_NAME, pData->id, NULL,  L"传真", 
			               TREENODE_TYPE_GROUP, pData, NULL, NULL, NULL);
					pData = new ORG_TREE_NODE_DATA();
					memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
					pData->bOpened = FALSE;
					pData->id = SYSTEM_NODE_SIGN_ID;
				    ::SkinAddTreeChildNode(m_hWnd, UI_CONTACTS_TREE_NAME, pData->id, NULL,  L"系统消息", 
			               TREENODE_TYPE_GROUP, pData, NULL, NULL, NULL);
					::SkinSetDropdownItemString(m_hWnd, L"cbType", -1, L"联系人", NULL);
					::SkinSetDropdownItemString(m_hWnd, L"cbType", -1, L"讨论组", NULL);
					::SkinSetDropdownItemString(m_hWnd, L"cbType", -1, L"短信", NULL);
					::SkinSetDropdownItemString(m_hWnd, L"cbType", -1, L"传真", NULL);
					::SkinSetDropdownItemString(m_hWnd, L"cbType", -1, L"系统消息", NULL);
					::SkinSelectDropdownItem(m_hWnd, L"cbType", 0);*/
					::SkinUpdateControlUI(m_hWnd, UI_CONTACTS_TREE_NAME);
					pUI->Release();
					pUI = NULL;
				}
				pCfg->Release();
				pCfg = NULL;
			} //end if (SUCCEEDED(hr)... 
		} //end if (m_pCore)
		return hr;
	}
}

void CMsgMgrUIImpl::Navigate2(const char *szType, const char *szUserName)
{
	if (szType && szUserName)
		ShowHistoryMsg(szType, szUserName);
}

const char *CMsgMgrUIImpl::GetImagePath()
{
	if (m_strImagePath.empty())
	{
		IConfigure *pCfg = NULL;
		if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			CInterfaceAnsiString strPath;
			if (SUCCEEDED(pCfg->GetPath(PATH_LOCAL_CUSTOM_PICTURE, &strPath)))
			{
				m_strImagePath = strPath.GetData();
			}
			pCfg->Release();
		}
	}
	return m_strImagePath.c_str();
}

//
BOOL CMsgMgrUIImpl::RECallBack(HWND hWnd, DWORD dwEvent, char *szFileName, DWORD *dwFileNameSize,
			  char *szFileFlag, DWORD *dwFileFlagSize, DWORD dwFlag)
{
	switch(dwEvent)
	{
		case RICHEDIT_EVENT_SENDFILE:
			 break;
		case RICHEDIT_EVENT_GETFILEBYTAG:
			 sprintf(szFileName, "%s%s.gif", GetImagePath(), szFileFlag);
			 return TRUE;
		case RICHEDIT_EVENT_GETCUSTOMPIC:
			{
				IEmotionFrame *pFrame = NULL;
				BOOL bSucc = FALSE;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IEmotionFrame), (void **)&pFrame)))
				{
					CInterfaceAnsiString strFileName;
					if (SUCCEEDED(pFrame->GetSysEmotion(szFileFlag, &strFileName)))
					{
						strcpy(szFileName, strFileName.GetData());
						bSucc = TRUE;
					} else
					{ 
						sprintf(szFileName, "%s%s.gif", GetImagePath(), szFileFlag);
						if (!CSystemUtils::FileIsExists(szFileName))
						{
							if (SUCCEEDED(pFrame->GetDefaultEmotion("error", &strFileName)))
								strcpy(szFileName, strFileName.GetData());
						} //end if (!CSystemUtils::					  
					} //end else if ( 
					pFrame->Release();
				} 
			    return TRUE;
			}
		case RICHEDIT_EVENT_GETTIPPIC:
			break;
		case RICHEDIT_EVENT_CUSTOMLINKCLICK:
			/*    0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31     |
			      |---ID FLAG-----|---------------------FILE  ID---------------------------------------     |*/
			//DoCustomLink(dwFlag);
			break;
	}
	return FALSE;
}




#pragma warning(default:4996)
