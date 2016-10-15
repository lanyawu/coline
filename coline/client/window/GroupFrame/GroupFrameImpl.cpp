#include <time.h>
#include <Commonlib/systemutils.h>
#include <Commonlib/DebugLog.h>
#include <Core/common.h>
#include "../imcommonlib/interfaceansistring.h"
#include "../IMCommonLib/InterfaceFontStyle.h"
#include "../IMCommonLib/XmlNodeTranslate.h"
#include "../IMCommonLib/MessageList.h"
#include "../P2Svr/P2Svr.h"
#include "GroupFrameImpl.h"
#include <Core/treecallbackfun.h>
#include <FileTransfer/filetransfer.h>
#include <ShellAPI.h>

#pragma warning(disable:4996)

std::map<CStdString_, int> g_GrpChatFontList;
#define MODIFY_GROUP_NAME_OK 2  //修改讨论组名称确定按钮

typedef struct CGroupFileLink
{
	std::string strTip;
	std::string strFileName;
}GROUP_FILE_LINK, *LPGROUP_FILE_LINK;

int CALLBACK CutImage(HWND hParent, BOOL bHideParent);

const char *CALLBACK GetGroupUserNodeKey(CTreeNodeType tnType, const void *pData)
{
	if (pData)
	{
		return ((LPORG_TREE_NODE_DATA)(pData))->szUserName;
	}
	return NULL;
}

BOOL CALLBACK RichEditCallBack(HWND hWnd, DWORD dwEvent, char *szFileName, DWORD *dwFileNameSize,
			  char *szFileFlag, DWORD *dwFileFlagSize, DWORD dwFlag, void *pOverlapped)
{
	if (pOverlapped)
	{
		CGroupFrameImpl *pThis = (CGroupFrameImpl *) pOverlapped;
		return pThis->RECallBack(hWnd, dwEvent, szFileName, dwFileNameSize, szFileFlag, dwFileFlagSize, dwFlag);
	}
	return FALSE;
}

CGroupFrameImpl::CGroupFrameImpl(void):
                 m_pCore(NULL),
				 m_bEnterSend(TRUE),
				 m_pGrpRoot(NULL),
				 m_hMainWnd(NULL)
{
	m_strFileTransSkinXml = "<Control xsi:type=\"FileProgressBar\"  progressimage=\"27\" bkgndimage=\"28\"  name=\"filename\" filename=\"文件2\" filesize=\"224123413\" currfilesize=\"23428342\"/>";
	memset(&m_rcLastOpen, 0, sizeof(RECT));
}


CGroupFrameImpl::~CGroupFrameImpl(void)
{
	ClearGroupItems();
	if (m_pCore)
		m_pCore->Release();
	m_pCore = NULL;
}

void CGroupFrameImpl::ClearGroupItems()
{
	std::map<CAnsiString_, CGroupItem *>::iterator it;
	for (it = m_FrameList.begin(); it != m_FrameList.end(); it ++)
	{
		if (it->second->GetHWND() && ::IsWindow(it->second->GetHWND()))
			::SkinCloseWindow(it->second->GetHWND());
		delete it->second;
	}
	m_FrameList.clear();
}

void CGroupFrameImpl::PlayTipSound(const char *szType, const char *szUserName, BOOL bLoop)
{
	IConfigure *pCfg = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		pCfg->PlayMsgSound(szType, szUserName, bLoop);
		pCfg->Release();
	}
}

const char *CGroupFrameImpl::GetImagePath()
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
		} //end if (m_pCore && SUCCEEDED(..
	} //end if (m_strImagePath...
	return m_strImagePath.c_str();
}

BOOL CGroupFrameImpl::RECallBack(HWND hWnd, DWORD dwEvent, char *szFileName, DWORD *dwFileNameSize,
			  char *szFileFlag, DWORD *dwFileFlagSize, DWORD dwFlag)
{
	switch(dwEvent)
	{
		case RICHEDIT_EVENT_SENDFILE:
			 SendFileToGroup(hWnd, szFileName);
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
					} else if (SUCCEEDED(pFrame->GetCustomEmotion(szFileFlag, &strFileName)))
					{
						strcpy(szFileName, strFileName.GetData());
						bSucc = TRUE;
					} else
					{ 
						sprintf(szFileName, "%s%s.gif", GetImagePath(), szFileFlag);
						if (!CSystemUtils::FileIsExists(szFileName))
						{
							if (SUCCEEDED(pFrame->GetDefaultEmotion("sending", &strFileName)))
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
			DoCustomLink(hWnd, dwFlag);
			break;
	}
	return FALSE;
}

//
void CGroupFrameImpl::OnWMDropFiles(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HDROP hDrop = (HDROP)wParam;
	if (hDrop == NULL)
		return ;

	TCHAR szFile[MAX_PATH]; 
	char szFileName[MAX_PATH] = {0};
	int iFileCount = ::DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	if (iFileCount <= 5)
	{
		for (int i = 0; i < iFileCount; ++i)
		{
			memset(szFile, 0, MAX_PATH * sizeof(TCHAR));
			if (::DragQueryFile(hDrop, i, szFile, MAX_PATH))
			{
				memset(szFileName, 0, MAX_PATH);
				CStringConversion::WideCharToString(szFile, szFileName, MAX_PATH - 1);
				if (CSystemUtils::FileIsExists(szFileName))
				{
					SendFileToGroup(hWnd, szFileName); 
				} //防止拖入文件
				
			} //end if (::DragQueryFile(..
		} //end for (int i
	} else
	{
		::SkinMessageBox(hWnd, L"本应用程序最多只支持同时拖曳5个文件", L"提示", MB_OK); 
	}
	::DragFinish(hDrop);
}

BOOL CGroupFrameImpl::DoCustomLink(HWND hWnd, DWORD dwFlag)
{
	int nCmd = dwFlag & 0x0000FFFF;
	int nFileId = ((dwFlag >> CUSTOM_LINK_FLAG_OFFSET) & 0x0000FFFF);
	
	//PRINTDEBUGLOG(dtInfo, "custom link, FileId:%d Flag:%d", nFileId, nCmd);
	char szLocalFileName[MAX_PATH] = {0};
	char szFileId[32] = {0};
	::itoa(nFileId, szFileId, 10);
	CTransferFileInfo FileInfo;
	std::string strXml;
    if (m_TransFileList.GetFileInfoById(nFileId, FileInfo))
	{
		if (FileInfo.m_nPeerFileId > 0)
			::itoa(FileInfo.m_nPeerFileId, szFileId, 10);
		else
			::itoa(FileInfo.m_nLocalFileId, szFileId, 10);
		switch(nCmd)
		{
		case CUSTOM_LINK_FLAG_SAVEAS:
			{
				CStringList_ FileList;
				char szTmpFile[MAX_PATH] = {0};
				strncpy(szTmpFile, FileInfo.m_strDspName.c_str(), MAX_PATH - 1);
				if (CSystemUtils::OpenFileDialog(NULL, FileInfo.hOwner, "另存文件为", "所有文件|*.*", szTmpFile,
					            FileList, FALSE, TRUE))
				{
					if (!FileList.empty())
					{ 
						strncpy(szLocalFileName, FileList.back().c_str(), MAX_PATH - 1);
						char szExTmp[MAX_PATH] = {0};
						CSystemUtils::ExtractFileExtName(szLocalFileName, szExTmp, MAX_PATH - 1);
						if (strlen(szExTmp) == 0)
						{
							CSystemUtils::ExtractFileExtName(FileInfo.m_strDspName.c_str(), szExTmp, MAX_PATH - 1);
							::strcat(szLocalFileName, ".");
							::strcat(szLocalFileName, szExTmp);
						}
					} else
					{
						return FALSE;
					} //end if (!FileList.empty())
				} else
				{
					return FALSE;
				} //else if (CSystemUtils::OpenFileDialog(NULL,
			} // 注意 中间不要有 case
		case CUSTOM_LINK_FLAG_RECV:
			 {
				BOOL bRecv = TRUE;
				 if (::strlen(szLocalFileName) == 0)
				 {
					 CInterfaceAnsiString strDefaultPath;
					 IConfigure *pCfg = NULL;
					 if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
					 {
						 pCfg->GetPath(PATH_LOCAL_RECV_PATH, &strDefaultPath);
						 pCfg->Release();
					 }
					 strcpy(szLocalFileName, strDefaultPath.GetData());
				     strcat(szLocalFileName, FileInfo.m_strDspName.c_str());
				 }
				 if (CSystemUtils::FileIsExists(szLocalFileName))
				 {
					 bRecv = (::SkinMessageBox(hWnd, L"文件已经存在，是否覆盖？", L"提示", 2) != IDOK);
				 }
				 if (bRecv)
				 {
					 std::string strUrl = FileInfo.m_OfflineSvr;
					 if (strUrl.empty())
					 {
						 CInterfaceAnsiString strTmp;
						 IConfigure *pCfg = NULL;
						 if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
						 {
							 if (SUCCEEDED(pCfg->GetServerAddr(HTTP_SVR_URL_HTTP, &strTmp)))
							 {
								 strUrl = strTmp.GetData();
							 }
							 pCfg->Release();
						 }  //end if (SUCCEEDED(m_pCore->
					 } //end if (strUrl.empty()
					 strUrl += "/offlinefile/";
					 strUrl += FileInfo.m_RemoteName;
		 
					CCustomPicItem *pItem = new CCustomPicItem();
					pItem->m_hOwner = hWnd;
					pItem->m_strFlag = FileInfo.m_strFileTag;
					pItem->m_pOverlapped = this;
					pItem->m_nFileId = nFileId;
					pItem->m_strLocalFileName = szLocalFileName;  
					pItem->m_strPeerName = FileInfo.m_strPeerName; 
					pItem->m_strUrl = strUrl;
					if (m_CustomPics.AddItem(pItem))
					{	
						::P2SvrAddDlTask(strUrl.c_str(), szLocalFileName, FILE_TYPE_NORMAL, 
							 pItem, HttpDlCallBack, FALSE);
					} else
						delete pItem;  
				 }			 
				 if (bRecv)
					 CancelCustomLink(FileInfo.hOwner, FileInfo.m_nLocalFileId, CUSTOM_LINK_FLAG_RECV | CUSTOM_LINK_FLAG_SAVEAS | CUSTOM_LINK_FLAG_REFUSE);
				 break;
			 }
		case CUSTOM_LINK_FLAG_CANCEL:

			 CancelCustomLink(FileInfo.hOwner, FileInfo.m_nLocalFileId, CUSTOM_LINK_FLAG_RECV | CUSTOM_LINK_FLAG_SAVEAS
				              | CUSTOM_LINK_FLAG_CANCEL | CUSTOM_LINK_FLAG_REFUSE | CUSTOM_LINK_FLAG_OFFLINE);
			 ::SkinRemoveChildControl(FileInfo.hOwner, L"fileprogress", FileInfo.m_strProFlag.GetData());
			 m_TransFileList.DeleteFileInfo(nFileId);
			 break;
		case CUSTOM_LINK_FLAG_REFUSE:

			 CancelCustomLink(FileInfo.hOwner, FileInfo.m_nLocalFileId, CUSTOM_LINK_FLAG_RECV | CUSTOM_LINK_FLAG_SAVEAS
				              | CUSTOM_LINK_FLAG_CANCEL | CUSTOM_LINK_FLAG_REFUSE | CUSTOM_LINK_FLAG_OFFLINE);
			 ::SkinRemoveChildControl(FileInfo.hOwner, L"fileprogress", FileInfo.m_strProFlag.GetData());
			 m_TransFileList.DeleteFileInfo(nFileId);
			 break; 
		}
	}
	if ((!strXml.empty()) && m_pCore)
		return m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (int) strXml.size(), 0);
	return FALSE;
}	//
 
//IUnknown
STDMETHODIMP CGroupFrameImpl::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IGroupFrame)))
	{
		*ppv = (IGroupFrame *) this;
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

void CGroupFrameImpl::DoCreateGroupMenu()
{
	IMainFrame *pFrame = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMainFrame), (void **)&pFrame)))
	{
		void *pSelNode = NULL;
		LPORG_TREE_NODE_DATA pSelData = NULL;
		TCHAR szName[MAX_PATH] = {0};
		int nSize = MAX_PATH - 1;
		CTreeNodeType tnType;
		if (::SkinGetSelectTreeNode(pFrame->GetSafeWnd(), L"colleaguetree", szName, 
			&nSize, &pSelNode, &tnType, (void **)&pSelData))
		{
			if (tnType == TREENODE_TYPE_GROUP)
			{
				 
				char szTmpName[MAX_PATH] = {0};
				CStringConversion::WideCharToString(szName, szTmpName, MAX_PATH - 1);
				strcat(szTmpName, "讨论");
				std::string strNewGrpName = szTmpName;
				if (InputGroupName(pFrame->GetSafeWnd(), L"请输入分组名称", strNewGrpName))
				{
					char szUserList[2048] = {0};
					int nSize = 2047;
					if (::SkinGetNodeChildUserList(pFrame->GetSafeWnd(), pSelNode, szUserList, &nSize, 
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
							}
						}else //节点未展开
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
						char szGuid[128] = {0};
						int nSize = 127;
						char szUTF8Name[MAX_PATH] = {0};
						CStringConversion::StringToUTF8(strNewGrpName.c_str(), szUTF8Name, MAX_PATH - 1);
						CSystemUtils::GetGuidString(szGuid, &nSize);
						CreateGroup(szGuid, szUTF8Name, &UserList); 
					} // end if (::SkinGetNodeChildUserList(..
				} //end if (Input...
			}//end if (tnType..
		} //end if (::SkinGetSelectTreeNode(..
		pFrame->Release();
	} //end if (m_pCore && ...
}

//
void CGroupFrameImpl::ShowMiniCard(HWND hWnd)
{
	IMiniCard *pCard = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMiniCard), (void **)&pCard)))
	{
		void *pSelNode = NULL;
		LPORG_TREE_NODE_DATA pSelData = NULL;
		TCHAR szName[MAX_PATH] = {0};
		int nSize = MAX_PATH - 1;
		CTreeNodeType tnType;
		if (::SkinGetSelectTreeNode(hWnd, L"groupuserlist", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
		{
			if (tnType == TREENODE_TYPE_LEAF)
			{
				RECT rc = {0};
				POINT pt = {0};
				::GetWindowRect(hWnd, &rc);
		        ::GetCursorPos(&pt); 
				pCard->ShowMiniCard(pSelData->szUserName, rc.right + 2, pt.y + 2, 2);
			}
		}
		pCard->Release();
	}
}

//
HRESULT CGroupFrameImpl::DoMenuCommand(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szName, "treegroup") == 0)
	{
		switch(wParam)
		{
		case 11001: //创建讨论组 
			{
				DoCreateGroupMenu();
				break;
			}
			break;
		}
	} else if (::stricmp(szName, "groupusermenu") == 0)
	{
		switch(wParam)
		{
		case 11002: //对话
			 OpenChatFrame(hWnd);
			 break;
		case 11003: //查看资料
			 ShowMiniCard(hWnd);
			 break;
		case 11004: //移除此人
			{
				void *pSelNode = NULL;
				LPORG_TREE_NODE_DATA pSelData = NULL;
				TCHAR szName[MAX_PATH] = {0};
				int nSize = MAX_PATH - 1;
				CTreeNodeType tnType;
				if (::SkinGetSelectTreeNode(hWnd, L"groupuserlist", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
				{
					if (tnType == TREENODE_TYPE_LEAF)
					{
						RemoveUserByHWND(hWnd, pSelData->szUserName);
					}
				}
			}
			break;

		}
	} else if (::stricmp(szName, "groupmenu") == 0)
	{
		switch(wParam)
		{
		case 11010: //打开分组对话
			 OpenGroupFrameAction(hWnd);
			 break;
		case 11011: //退出分组
			 ExitGroupAction(hWnd);
			 break;
		case 11012: //解散分组
			 DeleteGroupAction(hWnd);
			 break;
		case 11013: //修改分组名称
			 ModifyGroupName(hWnd);
			 break;
		case 11014: //发送文件
			{
				void *pSelNode = NULL;
				LPORG_TREE_NODE_DATA pSelData = NULL;
				TCHAR szName[MAX_PATH] = {0};
				int nSize = MAX_PATH - 1;
				CTreeNodeType tnType;
				if (::SkinGetSelectTreeNode(hWnd, L"grouptree", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
				{
					if (tnType == TREENODE_TYPE_GROUP)
					{
						SendFileToGroup(pSelData->szUserName, NULL);
					}
				}
				break;
			}
		case 11015: //发送短信
			{
				void *pSelNode = NULL;
				LPORG_TREE_NODE_DATA pSelData = NULL;
				TCHAR szName[MAX_PATH] = {0};
				int nSize = MAX_PATH - 1;
				CTreeNodeType tnType;
				if (::SkinGetSelectTreeNode(hWnd, L"grouptree", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
				{
					if (tnType == TREENODE_TYPE_GROUP)
					{
						SendSMSToGroup(pSelData->szUserName, NULL);
					}
				}
				break;
			}
		case 11016: //发送邮件
			{
				void *pSelNode = NULL;
				LPORG_TREE_NODE_DATA pSelData = NULL;
				TCHAR szName[MAX_PATH] = {0};
				int nSize = MAX_PATH - 1;
				CTreeNodeType tnType;
				if (::SkinGetSelectTreeNode(hWnd, L"grouptree", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
				{
					if (tnType == TREENODE_TYPE_GROUP)
					{
						SendMailToGroup(pSelData->szUserName);
					}
				}
				break;
			}
		}
	} else if (::stricmp(szName, "sendmenu") == 0)
	{
		switch(wParam)
		{
		case 30001:
			 m_bEnterSend = TRUE;
			 break;
		case 30002:
			 m_bEnterSend = FALSE;
			 break;
		}
		IConfigure *pCfg = NULL;
		if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			if (m_bEnterSend)
			{
				pCfg->SetParamValue(FALSE, "hotkey", "entersendmsg", "true");
				pCfg->SetParamValue(FALSE, "hotkey", "ctrlentersendmsg", "false");
			} else
			{
				pCfg->SetParamValue(FALSE, "hotkey", "ctrlentersendmsg", "true");
				pCfg->SetParamValue(FALSE, "hotkey", "entersendmsg", "false");
			}
			pCfg->Release();
		}
	} else if (::stricmp(szName, "RichEditReadOnlyMenu") == 0)
	{
		switch(wParam)
		{
		case 20: //复制
			::SkinRichEditCommand(hWnd, L"messagedisplay", "copy", NULL);
			break;
		case 21: //全选
			::SkinRichEditCommand(hWnd, L"messagedisplay", "selectall", NULL);
			break;
	    case 26:
			::SkinRichEditCommand(hWnd, L"messagedisplay", "clear", NULL);
			break;
		}
	} else if (::stricmp(szName, "RichEditPopMenu") == 0) //输入框的右键菜单
	{
		switch(wParam)
		{
		case 20: //复制
			::SkinRichEditCommand(hWnd, L"messageedit", "copy", NULL);
			break;
		case 22: //剪切
			::SkinRichEditCommand(hWnd, L"messageedit", "cut", NULL);
			break;
		case 23: //粘贴
			::SkinRichEditCommand(hWnd, L"messageedit", "paste", NULL);
			break;
		case 25: //全选
			::SkinRichEditCommand(hWnd, L"messageedit", "selectall", NULL);
			break;
		}
	} else if (::stricmp(szName, "phrasemenu") == 0) //短语
	{
		TCHAR szwCaption[MAX_PATH] = {0};
		int nSize = MAX_PATH - 1;
		if (::SkinMenuGetItemCaption(hWnd, L"phrasemenu", wParam, szwCaption, &nSize))
		{
			::SkinSetControlAttr(hWnd, L"messageedit", L"text", szwCaption);
			SendMessageToPeer(hWnd);
		}
	} else if (::stricmp(szName, "chatshortcutmenu") == 0) //快捷菜单
	{
		switch(wParam)
		{
			case 100:
			{
				COLORREF cr;
				if (CSystemUtils::OpenColorDialog(NULL, hWnd, cr))
				{
					if (m_pCore)
					{
						IUIManager *pUI = NULL;
						if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
						{
							if (SUCCEEDED(pUI->BlendSkinStyle(cr)))
							{
								IConfigure *pCfg = NULL;
								if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
								{
									char szTmp[16] = {0};
									::itoa(cr, szTmp, 10);
									pCfg->SetParamValue(FALSE, "Skin", "background", szTmp);
									pCfg->Release();
								} //end if (SUCCEEDED(m_pCore->...
							} //end if (SUCCEEDED(pUI->...
							pUI->Release();
						} //end if (SUCCEEDED(m_pCore->
					} //end if (m_pCore)
				} //end if (CSystemUtils::OpenColorDialog(NULL... 
				break;
			} 
			case 101: //智能合并
			{
				IConfigure *pCfg = NULL;
				if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
				{
					BOOL bChecked = (BOOL) lParam;
					if (bChecked)
					{
						::SkinSetControlAttr(hWnd, L"messagedisplay", L"mergemsg", L"true");
						pCfg->SetParamValue(FALSE, "normal", "aimsg", "true");
					} else
					{
						::SkinSetControlAttr(hWnd, L"messagedisplay", L"mergemsg", L"false");
						pCfg->SetParamValue(FALSE, "normal", "aimsg", "false");
					}
					pCfg->Release();
					::SkinUpdateControlUI(hWnd, L"messagedisplay");
				}
				break;
			} 
			case 102: //透明背景
			{
				IConfigure *pCfg = NULL;
				if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
				{
					BOOL bChecked = (BOOL) lParam;
					if (bChecked)
					{
						::SkinSetControlAttr(hWnd, L"messagedisplay", L"transparent", L"true");
						CInterfaceAnsiString strValue;
						if (SUCCEEDED(pCfg->GetParamValue(FALSE, "normal", "transimagefile", &strValue)))
						{
							TCHAR szTmp[MAX_PATH] = {0};
							CStringConversion::StringToWideChar(strValue.GetData(), szTmp, MAX_PATH - 1);
							::SkinSetControlAttr(hWnd, L"chatdisplaycanvs", L"imagefile", szTmp);
						}
						pCfg->SetParamValue(FALSE, "normal", "msgtransparent", "true");
					} else
					{
					    ::SkinSetControlAttr(hWnd, L"chatdisplaycanvs", L"imagefile", NULL);
						::SkinSetControlAttr(hWnd, L"messagedisplay", L"transparent", L"false");
						pCfg->SetParamValue(FALSE, "normal", "msgtransparent", "false");
					}
					pCfg->Release();
					::SkinUpdateControlUI(hWnd, L"chatdisplaycanvs");
				}
				break;
			}
		} //
	}
	return -1;
}

//退出分组
void CGroupFrameImpl::ExitGroupAction(HWND hWnd)
{

	void *pSelNode = NULL;
	LPORG_TREE_NODE_DATA pSelData = NULL;
	TCHAR szName[MAX_PATH] = {0};
	int nSize = MAX_PATH - 1;
	CTreeNodeType tnType;
	if (::SkinGetSelectTreeNode(hWnd, L"grouptree", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
	{
		char szGrpId[MAX_PATH] = {0};
		strncpy(szGrpId, pSelData->szUserName, MAX_PATH - 1);
		CStdString_ strTip = L"您确定要退出此分组";
		BOOL bManager = IsGroupManager(szGrpId);
		if (bManager)
		{
			strTip = L"您是此讨论组的管理员，退出后，将会解散此分组，是否确定退出？";
		}
		if (::SkinMessageBox(hWnd, strTip, L"提示", 2 /*MB_YESNO*/) == IDOK)
		{
			if (bManager)
				DeleteGroupById(szGrpId);
			else
				ExitGroupById(szGrpId);
			::SkinAdjustTreeNode(hWnd, L"grouptree", NULL, szName, TREENODE_TYPE_GROUP,
				pSelData, FALSE, FALSE);
		} //end if (::SkinGetSelectTreeNode(hWnd,..
	} //end if (::SkinGetSelectTreeNode(
}

//
void CGroupFrameImpl::ShowFileLink(HWND hWnd, const char *szTip, const char *szFileName)
{
	IUIManager *pUI = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
	{
		LPGROUP_FILE_LINK pData = new GROUP_FILE_LINK();
		pData->strTip = szTip;
		pData->strFileName = szFileName;
		LRESULT hr = S_OK;
		pUI->SendMessageToWindow("mainwindow", WM_GRP_FILE_LINK, (WPARAM) hWnd, (LPARAM) pData, &hr);
		pUI->Release();
	}
}

//
BOOL CGroupFrameImpl::InputGroupName(HWND hWnd, const TCHAR *szTitle, std::string &strNewGrpName)
{
	//
	BOOL bSucc = FALSE;
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
		pUI->CreateUIWindow(hWnd, "modigrpnamewindow",  &rc, WS_POPUP | WS_SYSMENU,
	                0, szTitle, &hTmp);	
		if (hTmp != NULL)
		{
			TCHAR szGrpName[MAX_PATH] = {0};
			CStringConversion::StringToWideChar(strNewGrpName.c_str(), szGrpName, MAX_PATH - 1);
			::SkinSetControlTextByName(hTmp, L"edtnewgrpname", szGrpName); 
			if (::SkinShowModal(hTmp) == MODIFY_GROUP_NAME_OK)
			{
				strNewGrpName = m_strNewGrpName; 
				bSucc = TRUE;
			} //end if (::SkinShowModal(hTmp)
		} //end if (hTmp != NULL)
		pUI->Release();
	} //end if (m_pCore && SUCCEEDED(
	return bSucc;
}

//
void CGroupFrameImpl::ModifyGroupName(HWND hWnd)
{
	InitSelfUserInfo();
	void *pSelNode = NULL;
	LPORG_TREE_NODE_DATA pSelData = NULL;
	TCHAR szName[MAX_PATH] = {0};
	int nSize = MAX_PATH - 1;
	CTreeNodeType tnType;
	if (::SkinGetSelectTreeNode(hWnd, L"grouptree", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
	{
		if (tnType == TREENODE_TYPE_GROUP)
		{
			char szGrpId[MAX_PATH] = {0};
			strncpy(szGrpId, pSelData->szUserName, MAX_PATH - 1);
			CGroupItem *pItem = GetGroupItemById(szGrpId);
			if (pItem && ::stricmp(pItem->GetCreator(), m_strUserName.c_str()) == 0)
			{
				char szTmpName[MAX_PATH] = {0};
				CStringConversion::WideCharToString(szName, szTmpName, MAX_PATH - 1);
				std::string strNewGrpName = szTmpName; 
				if (InputGroupName(hWnd, L"输入新的分组名称", strNewGrpName))
				{ 
					UpdateGroupName(szGrpId, strNewGrpName.c_str());
					memset(szName, 0, sizeof(TCHAR) * MAX_PATH);
					CStringConversion::StringToWideChar(strNewGrpName.c_str(), szName, MAX_PATH - 1);
					::SkinAdjustTreeNode(hWnd, L"grouptree", NULL, szName, TREENODE_TYPE_GROUP,  pSelData, TRUE, TRUE);
					::SkinUpdateControlUI(hWnd, L"grouptree");
				} //end if (InputGroupName(hWnd, L"",
			} else
			{
				::SkinMessageBox(hWnd, L"您没有修改讨论组名称的权限", L"提示", MB_OK);
			}//end if (pItem && ::stricmp(pItem->GetCreator()
		} //end if (tnType == TREENODE_TYPE_GROUP)
	} //end if (::SkinGetSelectTreeNode(hWnd..
}

//解散分组
void CGroupFrameImpl::DeleteGroupAction(HWND hWnd)
{
	
	void *pSelNode = NULL;
	LPORG_TREE_NODE_DATA pSelData = NULL;
	TCHAR szName[MAX_PATH] = {0};
	int nSize = MAX_PATH - 1;
	CTreeNodeType tnType;
	if (::SkinGetSelectTreeNode(hWnd, L"grouptree", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
	{
		if (IsGroupManager(pSelData->szUserName))
		{
			CStdString_ strTip = L"您确定要解散此分组<";
			strTip += szName;
			strTip += L">";
			if (::SkinMessageBox(hWnd, strTip, L"提示", 2/*MB_YESNO*/) == IDOK)
		    {
				char szGrpId[MAX_PATH] = {0};
				strncpy(szGrpId, pSelData->szUserName, MAX_PATH - 1);
				DeleteGroupById(szGrpId);
			} //end if (::SkinGetSelectTreeNode(hWnd,..
		} else
		{
			::SkinMessageBox(hWnd, L"只有管理员才能解散分组", L"提示", MB_OK);
		}
	}
}

//
void CGroupFrameImpl::Invitation(HWND hWnd, IUserList *ulSrcUsers)
{
	IContactPanel *pPanel = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContactPanel), (void **)&pPanel)))
	{
		if (SUCCEEDED(pPanel->ShowPanel(hWnd, L"选择讨论组成员", NULL, TRUE, ulSrcUsers)))
		{
			CInterfaceUserList ulNewList;
			pPanel->GetSelContact(&ulNewList); 
			TCHAR szGrpName[MAX_PATH] = {0};
			CStringConversion::UTF8ToWideChar(m_strRealName.c_str(), szGrpName, MAX_PATH - 1);
			lstrcat(szGrpName, L"   发起的讨论");
			char szTmpName[MAX_PATH] = {0};
			CStringConversion::WideCharToString(szGrpName, szTmpName, MAX_PATH - 1);
			std::string strNewGrpName = szTmpName;
			if (InputGroupName(hWnd, L"请输入分组名称", strNewGrpName))
			{
				char szUTF8Name[MAX_PATH] = {0};
				char szGuid[128] = {0};
				int nSize = 127;
				CStringConversion::StringToUTF8(strNewGrpName.c_str(), szUTF8Name, MAX_PATH - 1);
				CSystemUtils::GetGuidString(szGuid, &nSize);
				CreateGroup(szGuid, szUTF8Name, &ulNewList);  
			}
		}
		pPanel->Release();
	} //end if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(...
}

//
void CGroupFrameImpl::DoCreateGroup(HWND hWnd)
{
	if (m_pCore)
	{
		if (m_pCore->CanAllowAction(USER_ROLE_GROUP) != S_OK)
		{
			::SkinMessageBox(hWnd, L"没有创建讨论组权限", L"提示", MB_OK);
			return;
		}
	}
	InitSelfUserInfo();
	CInterfaceUserList ulList;
	LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
	memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
	strcpy(pData->szUserName, m_strUserName.c_str());
	if (FAILED(ulList.AddUserInfo(pData, FALSE, TRUE)))
	{
		delete pData;
	}
	Invitation(hWnd, &ulList);
}

//
void CGroupFrameImpl::RemoveUserByHWND(HWND hWnd, const char *szUserName)
{ 
	CGroupItem *pItem = GetGroupItemByHWND(hWnd);
	if (pItem)
	{
		if (::stricmp(szUserName, m_strUserName.c_str()) == 0)
		{
			BOOL bManager = IsGroupManager(pItem->GetGroupId());
			CStdString_ strTip = L"您确定要退出此分组";
			if (bManager)
				strTip = L"您是此讨论组的管理员，退出后，将会解散此分组，是否确定退出？";
			if (::SkinMessageBox(hWnd, strTip, L"提示", 2 /*MB_YESNO*/) == IDOK)
			{ 
				char szGrpId[MAX_PATH] = {0};
				strncpy(szGrpId, pItem->GetGroupId(), MAX_PATH - 1);
				if (bManager)
					DeleteGroupById(szGrpId);
				else
					ExitGroupById(szGrpId);
			} //end if (::SkinGetSelectTreeNode(hWnd,..
			return ;
		}
 
		if (stricmp(pItem->GetCreator(), m_strUserName.c_str()) == 0)
		{
			pItem->DeleteGroupUser(szUserName);
			std::string strXml = "<grp type=\"revisegroup\" guid=\"";
			strXml += pItem->GetGroupId();
			strXml += "\" name=\"";
			char szTmp[MAX_PATH] = {0};
			CStringConversion::UTF8ToString(pItem->GetDispName(), szTmp, MAX_PATH - 1);
			strXml += szTmp;
			strXml += "\" creator=\"";
			strXml += pItem->GetCreator();
			strXml += "\">";
			 
			strXml += "<u type=\"del\" uid=\"";
			strXml += szUserName;
			strXml += "\"/>";
			strXml += "</grp>";
			LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
			memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
			strncpy(pData->szUserName, szUserName, MAX_USER_NAME_SIZE - 1);
			::SkinAdjustTreeNode(hWnd, L"groupuserlist", NULL, NULL, TREENODE_TYPE_LEAF, pData, FALSE, FALSE);
			delete pData;	 
			m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG)strXml.size(), 0);
		} else
		{
			::SkinMessageBox(hWnd, L"没有移除讨论组成员权限", L"警告", MB_OK);
		}
	}
}

void CGroupFrameImpl::DoGroupUserSet(HWND hWnd)
{
	CGroupItem *pItem = GetGroupItemByHWND(hWnd);
	if (pItem)  
	{
		if (stricmp(pItem->GetCreator(), m_strUserName.c_str()) == 0)
		{
			IContactPanel *pPanel = NULL;
			if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContactPanel), (void **)&pPanel)))
			{
				CInterfaceUserList ulOldList;
				pItem->GetUserList(&ulOldList); 
				if (SUCCEEDED(pPanel->ShowPanel(hWnd, L"选择讨论组成员", NULL, TRUE, &ulOldList)))
				{
					CInterfaceUserList ulNewList;
					pPanel->GetSelContact(&ulNewList);
					pItem->GetUserList(&ulOldList);
					CInterfaceUserList ulAdd, ulSub;
					if (SUCCEEDED(ulNewList.CompareSub(&ulOldList, &ulAdd, &ulSub)))
					{
						pItem->SetNewUserList(&ulNewList);
						//
						if ((ulAdd.GetUserCount() > 0) || (ulSub.GetUserCount() > 0))
						{
							std::string strXml = "<grp type=\"revisegroup\" guid=\"";
							strXml += pItem->GetGroupId();
							strXml += "\" name=\"";
							char szTmp[MAX_PATH] = {0};
							CStringConversion::UTF8ToString(pItem->GetDispName(), szTmp, MAX_PATH - 1);
							strXml += szTmp;
							strXml += "\" creator=\"";
							strXml += pItem->GetCreator();
							strXml += "\">";
							LPORG_TREE_NODE_DATA pData;
							while (SUCCEEDED(ulSub.PopBackUserInfo(&pData)))
							{
								strXml += "<u type=\"del\" uid=\"";
								strXml += pData->szUserName;
								strXml += "\"/>";
								::SkinAdjustTreeNode(hWnd, L"groupuserlist", NULL, NULL, TREENODE_TYPE_LEAF, pData, FALSE, FALSE);
								if (pData->szDisplayName)
									delete []pData->szDisplayName;
								delete pData;
							}
							TCHAR szText[MAX_PATH];
							IContacts *pContact = NULL;
							CInterfaceAnsiString szName;
							if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
							{
								while (SUCCEEDED(ulAdd.PopBackUserInfo(&pData)))
								{
									strXml += "<u type=\"add\" uid=\"";
									strXml += pData->szUserName;
									strXml += "\"/>";
									memset(szText, 0, sizeof(TCHAR) * MAX_PATH);
									if (SUCCEEDED(pContact->GetRealNameById(pData->szUserName, NULL, &szName)))
									{
										CStringConversion::UTF8ToWideChar(szName.GetData(), szText, MAX_PATH - 1);
									} else
									{
										CStringConversion::StringToWideChar(pData->szUserName, szText, MAX_PATH - 1);
									}
									if (::SkinAdjustTreeNode(hWnd, L"groupuserlist", NULL, szText, TREENODE_TYPE_LEAF, pData,TRUE, TRUE)) 
									{
										::SkinUpdateControlUI(hWnd, L"groupuserlist");
									} else
									{
										if (pData->szDisplayName)
											delete []pData->szDisplayName;
										delete pData;
									}
								}
								pContact->Release();
							} //end if (m_pCore && ...
							strXml += "</grp>";
							m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG)strXml.size(), 0);
						} //end if ((ulAdd.GetUserCount()
					} //end if (SUCCEEDED(ulNewList.CompareSub..
				}
				pPanel->Release();
			} //end if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(...
		} else
		{
			::SkinMessageBox(hWnd, L"没有权限设置讨论组成员", L"警告", MB_OK);
		}
	} //end if (pItem)
}

	//广播消息
STDMETHODIMP CGroupFrameImpl::DoBroadcastMessage(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData)
{
	return E_NOTIMPL;
}

BOOL CGroupFrameImpl::DoEmotionPanelClick(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	char *szFileName = (char *)wParam;
	char *szTag = (char *)lParam;
	return ::SkinInsertImageToRichEdit(hWnd, L"messageedit", szFileName, szTag, 0); 
}

HRESULT CGroupFrameImpl::DoClickEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szName, "grouptree") == 0)
	{
		//
		void *pSelNode = NULL;
		LPORG_TREE_NODE_DATA pSelData = NULL;
		TCHAR szName[MAX_PATH] = {0};
		int nSize = MAX_PATH - 1;
		CTreeNodeType tnType;
		if (::SkinGetSelectTreeNode(hWnd, L"grouptree", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
		{
			if (tnType == TREENODE_TYPE_GROUP)
			{
				if (pSelData && pSelData->bOpened == 0)
				{ 

					pSelData->bOpened = TRUE;
				} // end if (pSelData ... 
			} else
			{
				//Show user info
			} //end else if (tnType == ...
		} //end if (::GetSelectTreeNode(hWnd...
	} else if (::stricmp(szName, "sendchatmsg") == 0)
	{
		SendMessageToPeer(hWnd);
	} else if (::stricmp(szName, "grpinvitation") == 0)
	{
		DoGroupUserSet(hWnd);
	} else if (::stricmp(szName, "btnCreateGroup") == 0)
	{
		DoCreateGroup(hWnd);
	} else if (::stricmp(szName, "ok") == 0)
	{
		TCHAR szNewName[256] = {0};
		int nSize = 255;
		if (::SkinGetControlTextByName(hWnd, L"edtnewgrpname", szNewName, &nSize))
		{
			char szTmp[256] = {0};
			CStringConversion::WideCharToString(szNewName, szTmp, 255);
			m_strNewGrpName = szTmp;
			if (!m_strNewGrpName.empty())
			{
				::SkinSetModalValue(hWnd, MODIFY_GROUP_NAME_OK);
				::SkinCloseWindow(hWnd);
			} else
			{
				::SkinMessageBox(hWnd, L"讨论组名称不能为空", L"提示", MB_OK);
			}
		} else
		{
			::SkinMessageBox(hWnd, L"讨论组名称不能为空", L"提示", MB_OK);
		}
	} else if (::stricmp(szName, "cancel") == 0)
	{ 
		::SkinSetModalValue(hWnd, 0);
		::SkinCloseWindow(hWnd);
	} else if (::stricmp(szName, "closebutton") == 0)
	{
		::SkinSetModalValue(hWnd, 0);
	} if (::stricmp(szName, "invitation") == 0)
	{
		IChatFrame *pFrame = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pFrame)))
		{
			CInterfaceAnsiString strUserName;
			if (SUCCEEDED(pFrame->GetUserNameByHWND(hWnd, &strUserName)))
			{
				InitSelfUserInfo();
				CInterfaceUserList ulUsers;
				LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
				memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
				strcpy(pData->szUserName, strUserName.GetData());
				if (FAILED(ulUsers.AddUserInfo(pData, FALSE, TRUE)))
				{
					delete pData;
				}
				pData = new ORG_TREE_NODE_DATA();
				memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
				strcpy(pData->szUserName, m_strUserName.c_str());
				if (FAILED(ulUsers.AddUserInfo(pData, FALSE, TRUE)))
				{
					delete pData;
				}
				Invitation(hWnd, &ulUsers);
			}
			pFrame->Release();
		}
	} else if (::stricmp(szName, "CutScreen") == 0)
	{
		DoCutScreen(hWnd, FALSE);
	} else if (::stricmp(szName, "picture") == 0)
	{
		DoSendPicture(hWnd);
	} else if (::stricmp(szName, "transfile") == 0)
	{ 
		CStringList_ FileList;
		if (CSystemUtils::OpenFileDialog(NULL, hWnd, "选择要发送的文件", "所有文件(*.*)|*.*", 
			                NULL, FileList, FALSE, FALSE))
		{
			std::string strFileName;
			if (!FileList.empty())
				strFileName = FileList.back();
			if ((!strFileName.empty()) && CSystemUtils::FileIsExists(strFileName.c_str()))
			{
				SendFileToGroup(hWnd,  strFileName.c_str()); 			
			} //end if ((!strFileName.empty()) ...
		}
	} else if (::stricmp(szName, "emotion") == 0)
	{
		IEmotionFrame *pFrame = NULL;
		if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IEmotionFrame), (void **)&pFrame)))
		{
			POINT pt = {0};
			::GetCursorPos(&pt);
			pFrame->ShowEmotionFrame((ICoreEvent *) this, hWnd, pt.x, pt.y);
			pFrame->Release();
		}
	} else if (::stricmp(szName, "emotionpanel") == 0)
	{
		DoEmotionPanelClick(hWnd, wParam, lParam);
		::SkinSetControlFocus(hWnd,  L"messageedit", TRUE);
	}  else if (::stricmp(szName, "deletegroup") == 0)
	{
		CGroupItem *pItem = GetGroupItemByHWND(hWnd);
		if (pItem)
		{
			if (::stricmp(pItem->GetCreator(), m_strUserName.c_str()) == 0)
			{ 
				if (::SkinMessageBox(hWnd, L"您确定要解散此分组", L"提示", 2 /*MB_YESNO*/) == IDOK)
		        {
					char szTmp[128] = {0};
					strcpy(szTmp, pItem->GetGroupId());
					DeleteGroupById(szTmp);
				}
			}else
				::SkinMessageBox(hWnd, L"只有创建者才能解散讨论组", L"警告", MB_OK);
		}
	} else if (::stricmp(szName, "exitgroup") == 0)
	{
		CGroupItem *pItem = GetGroupItemByHWND(hWnd);
		if (pItem)
		{
			CStdString_ strTip = L"您确定要退出此分组";
			BOOL bManager =  IsGroupManager(pItem->GetGroupId());
			if (bManager)
			{
				strTip = L"您是此讨论组的管理员，退出后，将会解散此分组，是否确定退出？";
			}
			if (::SkinMessageBox(hWnd, strTip, L"提示", 2 /*MB_YESNO*/) == IDOK)
		    {
				char szGrpId[MAX_PATH] = {0};
				strncpy(szGrpId, pItem->GetGroupId(), MAX_PATH - 1);
				if (bManager)
					DeleteGroupById(szGrpId);
				else
					ExitGroupById(szGrpId);
			} //end if (::SkinGetSelectTreeNode(hWnd,..
		}
	} else  if (::stricmp(szName, "fontset") == 0)
	{
		BOOL bVisible = !::SkinGetControlVisible(hWnd, L"FontSetting");
		::SkinSetControlVisible(hWnd, L"FontSetting", bVisible);
		if (bVisible)
			::SkinSetControlAttr(hWnd, L"fontset", L"down", L"true");
		else
			::SkinSetControlAttr(hWnd, L"fontset", L"down", L"false");
	} else if (::stricmp(szName, "fontbold") == 0)
	{
		IConfigure *pCfg = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			TCHAR szValue[16] = {0};
			int nSize = 15;
			if (::SkinGetControlAttr(hWnd, L"fontbold", L"down", szValue, &nSize))
			{
				if (_tcsicmp(szValue, L"true") == 0)
				{
					pCfg->SetParamValue(FALSE, "ChatFont", "fontbold", "false");
					::SkinSetControlAttr(hWnd, L"fontbold", L"down", L"false");
				} else
				{
					pCfg->SetParamValue(FALSE, "ChatFont", "fontbold", "true");
					::SkinSetControlAttr(hWnd, L"fontbold", L"down", L"true");
				}
				RefreshInputChatFont(hWnd, pCfg);
			}
			pCfg->Release();
		}		
	} else if (::stricmp(szName, "fontitalic") == 0)
	{
		IConfigure *pCfg = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			TCHAR szValue[16] = {0};
			int nSize = 15;
			if (::SkinGetControlAttr(hWnd, L"fontitalic", L"down", szValue, &nSize))
			{
				if (_tcsicmp(szValue, L"true") == 0)
				{
					pCfg->SetParamValue(FALSE, "ChatFont", "fontitalic", "false");
					::SkinSetControlAttr(hWnd, L"fontitalic", L"down", L"false");
				} else
				{
					pCfg->SetParamValue(FALSE, "ChatFont", "fontitalic", "true");
					::SkinSetControlAttr(hWnd, L"fontitalic", L"down", L"true");
				}
				RefreshInputChatFont(hWnd, pCfg);
			}
			pCfg->Release();
		}	
	} else if (::stricmp(szName, "fontunderline") == 0)
	{
		IConfigure *pCfg = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			TCHAR szValue[16] = {0};
			int nSize = 15;
			if (::SkinGetControlAttr(hWnd, L"fontunderline", L"down", szValue, &nSize))
			{
				if (_tcsicmp(szValue, L"true") == 0)
				{
					pCfg->SetParamValue(FALSE, "ChatFont", "fontunderline", "false");
					::SkinSetControlAttr(hWnd, L"fontunderline", L"down", L"false");
				} else
				{
					pCfg->SetParamValue(FALSE, "ChatFont", "fontunderline", "true");
					::SkinSetControlAttr(hWnd, L"fontunderline", L"down", L"true");
				}
				RefreshInputChatFont(hWnd, pCfg);
			}
			pCfg->Release();
		}	
	} else if (::stricmp(szName, "fontcolor") == 0)
	{
		COLORREF cr;
		if (CSystemUtils::OpenColorDialog(NULL, hWnd, cr))
		{
			IConfigure *pCfg = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
			{
				char szValue[16] = {0};
				::itoa(cr, szValue, 10);
				pCfg->SetParamValue(FALSE, "ChatFont", "FontColor",  szValue);
				RefreshInputChatFont(hWnd, pCfg);
				pCfg->Release();
			} //end if (SUCCEEDED(m_pCore->
		} //end if (CSystemUtils::
	}
	return -1;
}

BOOL CGroupFrameImpl::CanClosed(HWND hWnd)
{

	BOOL bCan = TRUE;
	CGroupItem *pItem = GetGroupItemByHWND(hWnd); 
	if (pItem)
	{		 
		int nCount = m_TransFileList.HasOwnerWindow(hWnd);
		if (nCount > 0)
		{
			TCHAR szTmp[MAX_PATH] = {0};
			::wsprintf(szTmp, L"有 %d 个文件尚未发送完毕，是否要关闭", nCount);
			int n = ::SkinMessageBox(hWnd,szTmp, L"提示", 2/*MBI_OKCANCEL*/);
			if (n == IDOK)
			{
				std::vector<int> List;
				m_TransFileList.GetOwnerWindowList(hWnd, List);
				while (!List.empty())
				{
					CTransferFileInfo Info;
					if (m_TransFileList.GetFileInfoById(List.back(), Info))
					{
						char szFileId[16] = {0};
						::itoa(Info.m_nLocalFileId, szFileId, 10);
						std::string strXml;
						if (Info.m_bSender)
						{
							 //<trs type="filecancel" from="wuxiaozhong@gocom" to="admin@gocom" senderfileid="3"/>
							 strXml = "<grp type=\"filecancel\" from=\"";
							 strXml += m_strUserName;
							 strXml += "\" to=\"";
							 strXml += Info.m_strPeerName;
							 strXml += "\" senderfileid=\"";
							 strXml += szFileId;
							 strXml += "\"/>"; 
						} else
						{
										 //<trs type="filedecline" from="admin@gocom" to="wuxiaozhong@gocom" senderfileid="1"/>
							 strXml = "<grp type=\"filedecline\" from=\"";
							 strXml += m_strUserName;
							 strXml += "\" to=\"";
							 strXml += Info.m_strPeerName;
							 strXml += "\" senderfileid=\"";
							 strXml += szFileId;
							 strXml += "\"/>";
						}
						if (m_pCore)
							m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (int) strXml.size(), 0);
						::CancelFileTrans(List.back());	
						CancelCustomLink(Info.hOwner, Info.m_nLocalFileId, CUSTOM_LINK_FLAG_RECV | CUSTOM_LINK_FLAG_SAVEAS
								              | CUSTOM_LINK_FLAG_CANCEL | CUSTOM_LINK_FLAG_REFUSE | CUSTOM_LINK_FLAG_OFFLINE);
					    ::SkinRemoveChildControl(Info.hOwner, L"fileprogress", Info.m_strProFlag.GetData());
						m_TransFileList.DeleteFileInfo(List.back());
					}
					List.pop_back();
				} 
			} else
				bCan = FALSE;
		} //end if (nCount > 0)
	} //end if (it != m_ChatFrameList.end()
	return bCan;
}

//
BOOL CGroupFrameImpl::DoSendPicture(HWND hWnd)
{
	CStringList_ FileList;
	if (CSystemUtils::OpenFileDialog(NULL, hWnd, "选择要发送的文件", "所有图片文件(*.*)|*.bmp;*.gif;*.jpg;*.png", 
		                NULL, FileList, FALSE, FALSE))
	{
		std::string strFileName;
		if (!FileList.empty())
			strFileName = FileList.back();
		if ((!strFileName.empty()) && CSystemUtils::FileIsExists(strFileName.c_str()))
		{
			return ::SkinREInsertOlePicture(hWnd, L"messageedit", strFileName.c_str()); 						
		} //end if ((!strFileName.empty()) ...
	} //end if (CSystemUtils::OpenFileDialog(...
	return FALSE;
}

void CGroupFrameImpl::OpenChatFrame(HWND hWnd)
{
	void *pSelNode = NULL;
	LPORG_TREE_NODE_DATA pSelData = NULL;
	TCHAR szName[MAX_PATH] = {0};
	int nSize = MAX_PATH - 1;
	CTreeNodeType tnType;
	if (::SkinGetSelectTreeNode(hWnd, L"groupuserlist", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
	{
		if (tnType == TREENODE_TYPE_LEAF)
		{
			IChatFrame *pChat = NULL;
			if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pChat)))
			{
				pChat->ShowChatFrame(hWnd, pSelData->szUserName, NULL);
				pChat->Release();
			} //end if (m_pCore && 
		} //end if (tnType == TREENODE_TYPE_LEAF)
	} //end if (::GetSelectTreeNode(hWnd...
}

void CGroupFrameImpl::DoCutScreen(HWND hWnd, BOOL bHide)
{
	if (CutImage(hWnd, bHide) == IDOK)
	{
		::SkinRichEditCommand(hWnd, L"messageedit", "paste", NULL);
	} //
}

//
void CGroupFrameImpl::OpenGroupFrameAction(HWND hWnd)
{
	void *pSelNode = NULL;
	LPORG_TREE_NODE_DATA pSelData = NULL;
	TCHAR szName[MAX_PATH] = {0};
	int nSize = MAX_PATH - 1;
	CTreeNodeType tnType;
	if (::SkinGetSelectTreeNode(hWnd, L"grouptree", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
	{
		std::map<CAnsiString_, CGroupItem *>::iterator it = m_FrameList.find(pSelData->szUserName);
		if (it != m_FrameList.end())
		{
			ShowGroupFrame(it->second->GetGroupId(), it->second->GetDispName());
		} //end if (it != m_FrameList.end())
	} //end if (::SkinGetSelectTreeNode(hWnd,..
}

//
HRESULT CGroupFrameImpl::DoDblClickEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szName, "grouptree") == 0)
	{
		OpenGroupFrameAction(hWnd);
	} else if (::stricmp(szName, "groupuserlist") == 0)
	{
		OpenChatFrame(hWnd);
	}
	return -1;
}

BOOL CGroupFrameImpl::DoSysProtocol(const char *pType, TiXmlElement *pNode)
{ 
	return FALSE;
}

//ICoreEvent
STDMETHODIMP CGroupFrameImpl::DoCoreEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
	                     LPARAM lParam, HRESULT *hResult)
{
	if (::stricmp(szType, "menucommand") == 0)
	{
		*hResult = DoMenuCommand(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "click") == 0)
	{
		*hResult = DoClickEvent(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "lbdblclick") == 0)
	{
		*hResult = DoDblClickEvent(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "itemselect") == 0)
	{
		*hResult = DoItemSelectEvent(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "enterkeydown") == 0)
	{
		SHORT sCtrl = ::GetKeyState(VK_CONTROL) & 0xF000;
		if ( (sCtrl != 0) && (!m_bEnterSend))
		{
			SendMessageToPeer(hWnd);
			*hResult = 0;
		} else if ((sCtrl == 0) && m_bEnterSend)
		{
			SendMessageToPeer(hWnd);
		    *hResult = 0;
		} 
	} else if (::stricmp(szType, "afterinit") == 0)
	{
		if (::stricmp("mainwindow", szName) == 0)
		{
			::SkinSetGetKeyFun(hWnd, L"grouptree", GetTreeNodeKey);
			::SkinSetFreeNodeDataCallBack(hWnd, L"grouptree", FreeTreeNodeData);
			IUIManager *pUI = NULL;
		    if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
		    {
			    pUI->OrderWindowMessage("mainwindow", NULL, WM_GRP_UPDL_PROGR, (ICoreEvent *) this);
				pUI->OrderWindowMessage("mainwindow", NULL, WM_GRP_RM_FILE_PRO, (ICoreEvent *) this);
				pUI->OrderWindowMessage("mainwindow", NULL, WM_GRP_EXIT_GROUP, (ICoreEvent *) this);
				pUI->OrderWindowMessage("mainwindow", NULL, WM_GRP_APPEND_PRO, (ICoreEvent *) this);
				pUI->OrderWindowMessage("mainwindow", NULL, WM_GRP_FILE_LINK, (ICoreEvent *) this);
				pUI->OrderWindowMessage("mainwindow", NULL, WM_GRP_ADD_USER, (ICoreEvent *) this);
				pUI->OrderWindowMessage("mainwindow", NULL, WM_GRP_DELETE, (ICoreEvent *) this);
			    pUI->Release();
		     }
		} else if (::stricmp("groupWindow", szName) == 0)
		{
			::SkinSetGetKeyFun(hWnd, L"groupuserlist", GetTreeNodeKey);
			::SkinSetFreeNodeDataCallBack(hWnd, L"groupuserlist", FreeTreeNodeData);
			InitGroupFrame(hWnd);
			IUIManager *pUI = NULL;
			if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
			{  				
				pUI->OrderWindowMessage("groupWindow", hWnd, WM_KEYDOWN, (ICoreEvent *) this); 
				pUI->Release();
			}
		}
	} else if (::stricmp(szType, "initmenupopup") == 0)
	{
		*hResult = DoInitMenuPopup(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "link") == 0)
	{
		*hResult = DoLinkEvent(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "onclosequery") == 0)
	{
		if (::stricmp(szName, "groupWindow") == 0)
		{
			if (!CanClosed(hWnd))
				*hResult = 0;
		} //end if (::stricmp(szName, "")
	} //end else if (::stricmp(...
	return S_OK;
}

BOOL CGroupFrameImpl::FileRecvEvent(HWND hOwner, const char *szFileFlag)
{ 
	CTransferFileInfo FileInfo;
	if (m_TransFileList.GetFileInfoByFlag(szFileFlag, FileInfo))
	{
		char szLocalFileName[MAX_PATH] = {0};
		CInterfaceAnsiString strDefaultPath;
		IConfigure *pCfg = NULL;
		if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			pCfg->GetPath(PATH_LOCAL_RECV_PATH, &strDefaultPath);
			pCfg->Release();
		}
		strcpy(szLocalFileName, strDefaultPath.GetData());
	    strcat(szLocalFileName, FileInfo.m_strDspName.c_str());
		BOOL bRecv = TRUE;
		if (CSystemUtils::FileIsExists(szLocalFileName))
		{
			bRecv = (::SkinMessageBox(hOwner, L"文件已经存在，是否覆盖？", L"提示", 2) == IDOK);
		}
		if (bRecv)
		{
			return FileRecvToLocal(hOwner, FileInfo, szLocalFileName);
		} else
		{
			return FileSaveAsEvent(hOwner, szFileFlag);
		}
	}
	return FALSE;
}
	//
BOOL CGroupFrameImpl::FileRecvToLocal(HWND hOwner, CTransferFileInfo &FileInfo, const char *szLocalFileName)
{
	char szInternetIp[32] = {0};
	char szIntranetIp[32] = {0};
	WORD wInternetPort = 0;
	WORD wIntranetPort = 0;
    
	//
	CStdString_ strName = L"fr_";
	strName += FileInfo.m_strProFlag;
	::SkinSetControlVisible(hOwner, strName, FALSE);
	
	strName = L"fs_";
	strName += FileInfo.m_strProFlag;
	::SkinSetControlVisible(hOwner, strName, FALSE);
 
	std::string strUrl = FileInfo.m_OfflineSvr;
	if (strUrl.empty())
	{
		CInterfaceAnsiString strTmp;
		IConfigure *pCfg = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			if (SUCCEEDED(pCfg->GetServerAddr(HTTP_SVR_URL_HTTP, &strTmp)))
			{
				strUrl = strTmp.GetData();
			}
		    pCfg->Release();
		}  //end if (SUCCEEDED(m_pCore->
	} //end if (strUrl.empty()
	strUrl += "/offlinefile/";
	strUrl += FileInfo.m_RemoteName;
		 
	CCustomPicItem *pItem = new CCustomPicItem();
	pItem->m_hOwner =  hOwner;
	pItem->m_strFlag = FileInfo.m_strFileTag;
	pItem->m_pOverlapped = this;
	pItem->m_nFileId = FileInfo.m_nLocalFileId;
	pItem->m_strLocalFileName = szLocalFileName;  
	pItem->m_strPeerName = FileInfo.m_strPeerName; 
	pItem->m_strUrl = strUrl;
	if (m_CustomPics.AddItem(pItem))
	{	
		::P2SvrAddDlTask(strUrl.c_str(), szLocalFileName, FILE_TYPE_NORMAL, 
							pItem, HttpDlCallBack, FALSE);
		return TRUE;
	} else
		delete pItem;  
	 
	return FALSE;
}
 
void CGroupFrameImpl::RemoveTransFileProgre(int nFileId, const char *szTip, BOOL bPost)
{
	CTransferFileInfo Info;  
	if (m_TransFileList.GetFileInfoById(nFileId, Info))
	{
		m_TransFileList.DeleteFileInfo(Info.m_nLocalFileId);
		TCHAR *szwTmp = new TCHAR[Info.m_strProFlag.GetLength() + 1];
		memset(szwTmp, 0, sizeof(TCHAR) * (Info.m_strProFlag.GetLength() + 1));
		lstrcpy(szwTmp, Info.m_strProFlag.GetData());
		::PostMessage(GetMainFrameHWND(), WM_GRP_RM_FILE_PRO, WPARAM(Info.hOwner), LPARAM(szwTmp));	
		 
		if (szTip && (!Info.m_strDspName.empty()))
		{
			char szTmp[512] = {0};
			TCHAR szwTmp[512] = {0};			
			sprintf(szTmp, szTip, Info.m_strDspName.c_str());
			CStringConversion::StringToWideChar(szTmp, szwTmp, MAX_PATH - 1); 
			ShowGroupTipMsg(Info.hOwner, szwTmp);
		} //end if (..
	}
}

BOOL CGroupFrameImpl::FileCancelEvent(HWND hOwner, const char *szFileFlag)
{
	CTransferFileInfo FileInfo;
	if (m_TransFileList.GetFileInfoByFlag(szFileFlag, FileInfo))
	{
		char szFileId[16] = {0};
		::itoa(FileInfo.m_nLocalFileId, szFileId, 10);
		std::string strXml;
		if (FileInfo.m_bSender)
		{ 
			 //<grp type="filecancel" from="wuxiaozhong@gocom" to="admin@gocom" senderfileid="3"/>
			 strXml = "<grp type=\"filecancel\" from=\"";
			 strXml += m_strUserName;
			 strXml += "\" to=\"";
			 strXml += FileInfo.m_strPeerName;
			 strXml += "\" senderfileid=\"";
			 strXml += szFileId;
			 strXml += "\"/>";
			 RemoveTransFileProgre(FileInfo.m_nLocalFileId, "发送文件 %s 被取消", FALSE); 
		} else
		{ 
			 //<grp type="filedecline" from="admin@gocom" to="wuxiaozhong@gocom" senderfileid="1"/>
			 strXml = "<grp type=\"filedecline\" from=\"";
			 strXml += m_strUserName;
			 strXml += "\" to=\"";
			 strXml += FileInfo.m_strPeerName;
			 strXml += "\" senderfileid=\"";
			 strXml += szFileId;
			 strXml += "\"/>";
			RemoveTransFileProgre(FileInfo.m_nLocalFileId, "拒绝接收文件 %s", FALSE);
		}
		if (!strXml.empty())
		{
			return SUCCEEDED(m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG)(strXml.size()), 0));
		}
	}
	return FALSE;
}

BOOL CGroupFrameImpl::FileSaveAsEvent(HWND hOwner, const char *szFileFlag)
{
	CTransferFileInfo FileInfo;
	if (m_TransFileList.GetFileInfoByFlag(szFileFlag, FileInfo))
	{
		char szLocalFileName[MAX_PATH] = {0};
		CStringList_ FileList;
		BOOL bSucc = FALSE;
		while (TRUE)
		{
			char szTmpFile[MAX_PATH] = {0};
			strncpy(szTmpFile, FileInfo.m_strDspName.c_str(), MAX_PATH - 1);
			if (CSystemUtils::OpenFileDialog(NULL, FileInfo.hOwner, "另存文件为", "所有文件|*.*", szTmpFile,
					            FileList, FALSE, TRUE))
			{
				if (!FileList.empty())
				{ 
					memset(szLocalFileName, 0, MAX_PATH);
					strncpy(szLocalFileName, FileList.back().c_str(), MAX_PATH - 1);
					char szExTmp[MAX_PATH] = {0};
					CSystemUtils::ExtractFileExtName(szLocalFileName, szExTmp, MAX_PATH - 1);
					if (strlen(szExTmp) == 0)
					{
						CSystemUtils::ExtractFileExtName(FileInfo.m_strDspName.c_str(), szExTmp, MAX_PATH - 1);
						::strcat(szLocalFileName, ".");
						::strcat(szLocalFileName, szExTmp);
					}
					if (CSystemUtils::FileIsExists(szLocalFileName))
					{
						if (::SkinMessageBox(hOwner, L"文件已经存在，是否覆盖？", L"提示", 2) == IDOK)
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
					return FALSE;
				} //end if (!FileList.empty())
			} else
			{
				return FALSE;
			} //else if (CSystemUtils::OpenFileDialog(NULL,
		} //end while(...
		if (bSucc)
			return FileRecvToLocal(hOwner, FileInfo, szLocalFileName);
	} //end if m_TransFileList...
	return FALSE;
}

//
HRESULT CGroupFrameImpl::DoLinkEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
    if (::strnicmp(szName, "fr_", 3) == 0)
	{
		//file recv
		char szFlag[128] = {0};
		strcpy(szFlag, szName + 3);
		FileRecvEvent(hWnd, szFlag);
	}  else if (::strnicmp(szName, "fs_", 3) == 0)
	{
		//file save as 
		char szFlag[128] = {0};
		strcpy(szFlag, szName + 3);
		FileSaveAsEvent(hWnd, szFlag);
	} else if (::strnicmp(szName, "fc_", 3) == 0)
	{
		//file cancel 
		char szFlag[128] = {0};
		strcpy(szFlag, szName + 3);
		FileCancelEvent(hWnd, szFlag);
	} else if (::stricmp(szName, "btnCreateGroup") == 0)
	{
		DoCreateGroup(hWnd);
	}
	return -1;
}

HRESULT CGroupFrameImpl::DoInitMenuPopup(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szName, "phrasemenu") == 0)
	{
		CGroupItem *pChat = GetGroupItemByHWND(hWnd);
		if (pChat && (!pChat->m_bInitAckMenu))
		{
			IConfigure *pCfg = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
			{
				CMessageList mlList;
				if (SUCCEEDED(pCfg->GetReplyMessage(1, &mlList)))
				{
					int nId = 0;
					CInterfaceAnsiString strMsg;
					TCHAR szwTemp[256] = {0};
					for (int i = 0; i < mlList.GetCount(); i ++)
					{
						mlList.GetRawMsg(i, &nId, &strMsg);
						memset(szwTemp, 0, sizeof(TCHAR) * 256);
						CStringConversion::StringToWideChar(strMsg.GetData(), szwTemp, 255);
						::SkinMenuAppendItem(hWnd, L"phrasemenu", 0, szwTemp, nId);
					}
				}
				pCfg->Release(); 
			}
			pChat->m_bInitAckMenu = TRUE;
		}
	}
	return -1;
}

STDMETHODIMP CGroupFrameImpl::SetCoreFrameWork(ICoreFrameWork *pCore)
{
	if (m_pCore)
		m_pCore->Release();
	m_pCore = pCore;
	if (m_pCore)
	{
		m_pCore->AddRef();

		//Order Event
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "treegroup", "menucommand"); 
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "groupmenu", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "groupusermenu", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "grouptree", "click");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "grouptree", "lbdblclick");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "mainwindow", "afterinit");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "chatwindow", "invitation", "click");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "btnCreateGroup", "click");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "btnCreateGroup", "link");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "groupWindow", NULL, NULL);
		m_pCore->AddOrderEvent((ICoreEvent *) this, "modigrpnamewindow", NULL, NULL);
		//
		m_pCore->AddOrderProtocol((IProtocolParser *) this, "grp", NULL);
		m_pCore->AddOrderProtocol((IProtocolParser *) this, "grp2", NULL);
		m_pCore->AddOrderProtocol((IProtocolParser *) this, "sys", "presence");
	}
	return S_OK;
}

STDMETHODIMP CGroupFrameImpl::GetSkinXmlString(IAnsiString *szXmlString)
{
	HRESULT hr = E_FAIL;
	IConfigure *pCfg = NULL; 
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{ 
		hr = pCfg->GetSkinXml("GroupFrame.xml",szXmlString); 
		pCfg->Release();
	}
	return hr; 
}

//
STDMETHODIMP CGroupFrameImpl::CoreFrameWorkError(int nErrorNo, const char *szErrorMsg)
{
	switch(nErrorNo)
	{
	case CORE_ERROR_LOGOUT: //注销
		{
			IMainFrame *pFrame = NULL;
			if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMainFrame), (void **)&pFrame)))
			{
				::SkinTreeViewClear(pFrame->GetSafeWnd(), L"grouptree");
				pFrame->Release();
			}
			//关闭所有窗体
			ClearGroupItems();
			//m_CustomPics.Clear();
			m_TransFileList.Clear();
			m_strImagePath.clear();
			m_strUserName.clear();
			m_strRealName.clear();
		}
	}
	return E_NOTIMPL;
}

LRESULT CGroupFrameImpl::DoItemSelectEvent(HWND hWnd, const char *szCtrlName, WPARAM wParam, LPARAM lParam)
{
	if ((::stricmp(szCtrlName, "cbFontName") == 0) && (!m_bInitFrame))
	{
		TCHAR szTmp[64] = {0};
		int nSize = 63;
		::SkinGetControlTextByName(hWnd, L"cbFontName", szTmp, &nSize);
		char szValue[31] = {0};
		CStringConversion::WideCharToString(szTmp, szValue, 31);
		IConfigure *pCfg = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			pCfg->SetParamValue(FALSE, "ChatFont", "FontName",  szValue);
			RefreshInputChatFont(hWnd, pCfg);
			pCfg->Release();
		}	
	} else if ((::stricmp(szCtrlName, "cbFontSize") == 0) && (!m_bInitFrame))
	{
		TCHAR szTmp[64] = {0};
		int nSize = 63;
		::SkinGetControlTextByName(hWnd, L"cbFontSize", szTmp, &nSize);
		char szValue[31] = {0};
		CStringConversion::WideCharToString(szTmp, szValue, 31);
		IConfigure *pCfg = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
		{
			pCfg->SetParamValue(FALSE, "ChatFont", "FontSize",  szValue);
			RefreshInputChatFont(hWnd, pCfg);
			pCfg->Release();
		}
	}
	return -1;
}

//
STDMETHODIMP CGroupFrameImpl::DoWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes)
{
    switch (uMsg) 
	{
		case WM_DESTROY:
		{
			CGroupItem *pItem = GetGroupItemByHWND(hWnd); 
			if (pItem)
			{  
				if (hWnd == pItem->GetHWND())
				{
					pItem->SetOwnerHWND(NULL);
					//save frame pos
					RECT rc = {0};
					if (::GetWindowRect(hWnd, &rc))
					{
						m_rcLastOpen.right = m_rcLastOpen.left + rc.right - rc.left;
						m_rcLastOpen.bottom = m_rcLastOpen.top + rc.bottom - rc.top;
						IConfigure *pCfg = NULL;		
						HRESULT hr = m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg);
						if (SUCCEEDED(hr))
						{ 
							std::string strRect;
							CSystemUtils::RectToString(rc, strRect);
							pCfg->SetParamValue(FALSE, "Position", "ChatFrame", strRect.c_str());
							pCfg->Release();
						}
					}
					return S_OK;
				} //end if (hWnd == it->second->			
			} //end if (it != ...
			break;
		} 
		case WM_DROPFILES:
		{
			OnWMDropFiles(hWnd, wParam, lParam);
			return S_OK;
			break;
		} 
		case WM_GRP_UPDL_PROGR:
		{
			CTransferFileInfo Info;
			if (m_TransFileList.GetFileInfoById(wParam, Info))
			{ 
				TCHAR szTmp[20] = {0};
				::_itow(lParam, szTmp, 10);
				::SkinSetControlAttr(Info.hOwner, Info.m_strProFlag.GetData(), L"currfilesize", szTmp);						 
			}
			break;
		} 
		case WM_GRP_RM_FILE_PRO:
		{
			if (lParam == 0)
			{
				CTransferFileInfo *pInfo = (CTransferFileInfo *)wParam; 
				if (GetGroupItemByHWND(pInfo->hOwner) != NULL)
				{
					::SkinRemoveChildControl(pInfo->hOwner, L"fileprogress", pInfo->m_strProFlag.GetData());
					CancelCustomLink(pInfo->hOwner, pInfo->m_nLocalFileId, CUSTOM_LINK_FLAG_RECV | CUSTOM_LINK_FLAG_SAVEAS
						              | CUSTOM_LINK_FLAG_CANCEL | CUSTOM_LINK_FLAG_REFUSE | CUSTOM_LINK_FLAG_OFFLINE);
				}
			} else
			{
				HWND h = (HWND)wParam;
				TCHAR *szFlag = (TCHAR *)lParam;
				::SkinRemoveChildControl(h, L"fileprogress", szFlag);
				delete []szFlag;
			}
			break;
		} 
		case WM_GRP_APPEND_PRO:
		{
			CTransferFileInfo *pInfo = (CTransferFileInfo *)lParam;
			TCHAR szTmp[MAX_PATH] = {0};
			int nFlagSize = MAX_PATH - 1;
			::SkinAddChildControl(pInfo->hOwner, L"fileprogress", m_strFileTransSkinXml.c_str(), szTmp, &nFlagSize, 999999);
			pInfo->m_strProFlag = szTmp;
			memset(szTmp, 0, MAX_PATH * sizeof(TCHAR));
			CStringConversion::StringToWideChar(pInfo->m_strDspName.c_str(), szTmp, MAX_PATH - 1);
			::SkinSetControlAttr(pInfo->hOwner, pInfo->m_strProFlag.GetData(), L"filename", szTmp);
			memset(szTmp, 0, MAX_PATH * sizeof(TCHAR));
			::_itow(pInfo->m_dwFileSize, szTmp, 10);
			::SkinSetControlAttr(pInfo->hOwner, pInfo->m_strProFlag.GetData(), L"filesize", szTmp);
			::SkinSetControlAttr(pInfo->hOwner, pInfo->m_strProFlag.GetData(), L"currfilesize", L"0"); 
			::SkinSetControlAttr(pInfo->hOwner, pInfo->m_strProFlag.GetData(), L"progrestyle", L"recv");
			::SkinUpdateControlUI(pInfo->hOwner, pInfo->m_strProFlag.GetData());
			break;
		} 
		case WM_GRP_EXIT_GROUP:
		{
			char *szGrpId = (char *)wParam;
			char *szUid = (char *)lParam;
			CGroupItem *pItem = GetGroupItemById(szGrpId);
			if (pItem)
			{
				pItem->DeleteGroupUser(szUid);
				if (pItem->GetHWND() && ::IsWindow(pItem->GetHWND()))
				{
					LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
					memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
					strcpy(pData->szUserName, szUid); 
					::SkinAdjustTreeNode(pItem->GetHWND(), L"groupuserlist", NULL, NULL, TREENODE_TYPE_LEAF, pData, FALSE, FALSE);
					delete pData;
					IContacts *pContact = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
					{
						CInterfaceAnsiString strName;
						if (SUCCEEDED(pContact->GetRealNameById(szUid, NULL, &strName)))
						{
							TCHAR szwName[MAX_PATH] = {0};
							CStringConversion::UTF8ToWideChar(strName.GetData(), szwName, MAX_PATH - 1);
							CStdString_ strTip = szwName;
							strTip += L"  退出了分组";
							::SkinRichEditInsertTip(pItem->GetHWND(), L"messagedisplay", NULL, 0, strTip.GetData());
						}
					} //end if (SUCCEEDED(m_pCore->
				} //end if (pItem->GetHWDN()
			} //end if (pItem)
			break;
		} 
		case WM_GRP_FILE_LINK:
		{
			HWND h = (HWND) wParam;
			LPGROUP_FILE_LINK pData = (LPGROUP_FILE_LINK) lParam;
			::SkinRichEditAddFileLink(h, L"messagedisplay", NULL, 20, pData->strTip.c_str(), pData->strFileName.c_str());
			delete pData;
			break;
		} 
		case WM_GRP_ADD_USER:
		{
			//增加一个用户
			char *szGrpId = (char *)wParam;
			char *szUserName = (char *)lParam;
			CGroupItem *pItem = GetGroupItemById(szGrpId);
			if (pItem && (pItem->GetHWND() != NULL) && (::IsWindow(pItem->GetHWND())))
			{ 
				TCHAR szText[MAX_PATH] = {0}; 
			    IContacts *pContact = NULL; 
			    LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
				strcpy(pData->szUserName, szUserName);
				if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
				{
					CInterfaceAnsiString szName;
					if (SUCCEEDED(pContact->GetRealNameById(szUserName, NULL, &szName)))
					{
						CStringConversion::UTF8ToWideChar(szName.GetData(), szText, MAX_PATH - 1); 
					}
					pContact->Release();
				}
				::SkinAdjustTreeNode(pItem->GetHWND(), L"groupuserlist", NULL, szText, TREENODE_TYPE_LEAF, pData, TRUE, FALSE);
				CInterfaceAnsiString strPresence;
				if (SUCCEEDED(pContact->GetContactUserValue(szUserName, "presence", &strPresence)))
				{
					if (strPresence.GetSize() > 0)
						::SkinUpdateUserStatusToNode(pItem->GetHWND(), L"groupuserlist", szUserName,
						                 strPresence.GetData(), FALSE);
				} 
				::SkinUpdateControlUI(pItem->GetHWND(), L"groupuserlist");
			}
			break;
		} 
		case WM_GRP_DELETE:
		{
			//解散一个讨论组
			LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
			const char *szGrpId = (char *)wParam;
			strcpy(pData->szUserName, szGrpId);
			::SkinAdjustTreeNode(GetMainFrameHWND(), L"grouptree", NULL, L"", TREENODE_TYPE_GROUP, pData, FALSE, FALSE);
			CGroupItem *pItem = GetGroupItemById(szGrpId);
			if (pItem)
			{
				//
			} 
			break;
		}
		case WM_KEYDOWN:
		{
			if (!GetKeyState(VK_CONTROL) & 0x8000) 
				::SkinSetControlFocus(hWnd, L"messageedit", TRUE); 
			break;
		}
	}
		//end else if 
	return E_FAIL;
}

BOOL CGroupFrameImpl::ParserGroupProtocol(const char *pType, TiXmlElement *pNode)
{
	if (::stricmp(pType, "getgroup") == 0)
	{
		const char *pGrpId = pNode->Attribute("guid");
		const char *pGrpDspName = pNode->Attribute("name");
		const char *pGrpCreator = pNode->Attribute("creator");
		if (pGrpId && pGrpDspName && pGrpCreator)
		{
			CGroupItem *pItem = NULL;
			std::map<CAnsiString_, CGroupItem *>::iterator it = m_FrameList.find(pGrpId);
			if (it == m_FrameList.end())
			{
				pItem = new CGroupItem(pGrpId, NULL);
				pItem->SetCreator(pGrpCreator);
				int nSize = ::strlen(pGrpDspName) * 3;
				char *szUTF8Name = new char[nSize];
				memset(szUTF8Name, 0, nSize);
				CStringConversion::StringToUTF8(pGrpDspName, szUTF8Name, nSize - 1);
				pItem->SetDispName(szUTF8Name);
				m_FrameList.insert(std::pair<CAnsiString_, CGroupItem *>(pGrpId, pItem));
				//存储到历史记录
				IMsgMgr *pMgr = NULL;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMsgMgr), (void **)&pMgr)))
				{
					pMgr->SaveGroupInfo(pGrpId, szUTF8Name, pGrpCreator);
					pMgr->Release();
				}
				delete []szUTF8Name;
			} else
				pItem = it->second;
			if (pItem)
			{
				TiXmlElement *pChild = pNode->FirstChildElement();
				while (pChild)
				{
					const char *pId = pChild->Attribute("uid");
					if (pId)
						pItem->AddGroupUser(pId, NULL);
					pChild = pChild->NextSiblingElement();
				} //end while (pChild)
			} //end if (pItem)
		} //end if (pGrpId
	} else if (::stricmp(pType, "sendok") == 0)
	{
		IMainFrame *pFrame = NULL;
		if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMainFrame), (void **) &pFrame)))
		{
			::SendMessage(pFrame->GetSafeWnd(), WM_DRAWGROUPTOUI, 0, 0);
			pFrame->Release();
		}
	} else if (::stricmp(pType, "creategroup") == 0)
	{
		const char *pGrpId = pNode->Attribute("guid");
		const char *pGrpDspName = pNode->Attribute("name");
		const char *pGrpCreator = pNode->Attribute("creator");
		if (pGrpId && pGrpDspName && pGrpCreator)
		{
			CGroupItem *pItem = NULL;
			std::map<CAnsiString_, CGroupItem *>::iterator it = m_FrameList.find(pGrpId);
			if (it == m_FrameList.end())
			{
				pItem = new CGroupItem(pGrpId, NULL);
				pItem->SetCreator(pGrpCreator);
				int nSize = ::strlen(pGrpDspName) * 3;
				char *szUTF8Name = new char[nSize];
				memset(szUTF8Name, 0, nSize);
				CStringConversion::StringToUTF8(pGrpDspName, szUTF8Name, nSize - 1);
				pItem->SetDispName(szUTF8Name);
				m_FrameList.insert(std::pair<CAnsiString_, CGroupItem *>(pGrpId, pItem));
				//存储到历史记录
				IMsgMgr *pMgr = NULL;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMsgMgr), (void **)&pMgr)))
				{
					pMgr->SaveGroupInfo(pGrpId, szUTF8Name, pGrpCreator);
					pMgr->Release();
				}
				delete []szUTF8Name;
			} else
				pItem = it->second;
			if (pItem)
			{
				TiXmlElement *pChild = pNode->FirstChildElement();
				while (pChild)
				{
					const char *pId = pChild->Attribute("uid");
					if (pId)
						pItem->AddGroupUser(pId, NULL);
					pChild = pChild->NextSiblingElement();
				} //end while (pChild)
				//加入自身，服务器BUG
				InitSelfUserInfo();
				pItem->AddGroupUser(m_strUserName.c_str(), NULL);
			} //end if (pItem)
			IMainFrame *pFrame = NULL;
			if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMainFrame), (void **) &pFrame)))
			{
				::SendMessage(pFrame->GetSafeWnd(), WM_DRAWGROUPTOUI, WPARAM(pGrpId), 0);
				pFrame->Release();
			}
		} //
	} else if (::stricmp(pType, "msg") == 0)
	{
		//<grp type="msg" guid="%s" from="%s" creator="%s"><font name="%s" size="%s" color="%s" bold="%s" underline="%s"' +
        //   ' strikeout="%s" italic="%s"/><body>%s</body></grp>';
		return DoGroupMsg(pNode);
	} else if (::stricmp(pType, "p2pcustompic") == 0)
	{
		//// <grp type="p2pcustompic" from="wuxiaozhong@gocom"  guid="abcdefaadfadf" 
				//   filename="30f79afad4318774e447dc2db96936e0.gif" fileserver="http://imbs.smartdot.com.cn:9910" 
		DoGroupCustomPic(pNode);
	} else if (::stricmp(pType, "offlinefile") == 0)
	{
		DoRecvOfflineFile(pNode);
	} else if (::stricmp(pType, "exitgroup") == 0)
	{
		//<grp type="exitgroup" guid=""/>
		DoRecvExitGroup(pNode->Attribute("guid"), pNode->Attribute("uid"));
	} else if (::stricmp(pType, "revisegroup") == 0)
	{
		DoReviseGroup(pNode);
	} else if (::stricmp(pType, "deletegroup") == 0) //解散分组
	{
		BOOL bSucc = TRUE;
		if (pNode->Attribute("result"))
			bSucc = (stricmp(pNode->Attribute("result"), "ok") == 0);
		DoDeleteGroup(pNode->Attribute("guid"), pNode->Attribute("reason"),
			bSucc);
	}
	return FALSE;
}

//
BOOL CGroupFrameImpl::DoDeleteGroup(const char *szGrpId, const char *szResean, BOOL bSucc)
{
	//
	return ::SendMessage(GetMainFrameHWND(), WM_GRP_DELETE, (WPARAM)szGrpId, 0); 
}

//
BOOL CGroupFrameImpl::DoReviseGroup(TiXmlElement *pNode)
{
	const char *szGrpId = pNode->Attribute("guid");
	if (szGrpId)
	{
		CGroupItem *pItem = GetGroupItemById(szGrpId);
		TiXmlElement *pChild = pNode->FirstChildElement("u");
		while (pChild)
		{
			if (::stricmp(pChild->Attribute("type"), "add") == 0)
			{
				pItem->AddGroupUser(pChild->Attribute("uid"), NULL);
				//
				if ((pItem->GetHWND() != NULL) && ::IsWindow(pItem->GetHWND()))
				{
					::SendMessage(GetMainFrameHWND(), WM_GRP_ADD_USER, (WPARAM)szGrpId, (LPARAM)pChild->Attribute("uid"));
				}
			} else if (::stricmp(pChild->Attribute("type"), "del") == 0)
			{
				pItem->DeleteGroupUser(pChild->Attribute("uid"));
				if ((pItem->GetHWND() != NULL) && ::IsWindow(pItem->GetHWND()))
				{
					DoRecvExitGroup(szGrpId, pChild->Attribute("uid"));
				}
				//
			}
			pChild = pChild->NextSiblingElement("u");
		}
		return TRUE;
	}
	//
	return FALSE;
}

void CGroupFrameImpl::DoRecvExitGroup(const char *szGrpId, const char *szUid)
{
	::SendMessage(GetMainFrameHWND(), WM_GRP_EXIT_GROUP, (WPARAM)szGrpId, (LPARAM)szUid);
}

//
CGroupItem *CGroupFrameImpl::GetGroupItemById(const char *szGrpId)
{
	std::map<CAnsiString_, CGroupItem *>::iterator it = m_FrameList.find(szGrpId);
	if (it != m_FrameList.end())
		return it->second;
	return NULL;
}

void CGroupFrameImpl::DoRecvOfflineFile(TiXmlElement *pNode)
{
    //<grp type="p2pcustompic" filetype="normal" from="admin@gocom" guid="wuxiaozhong@gocom" name="apss.dll.mui" filesize="3072"
		// url="A90C62138DFB74BEA86244E2432D133EF.mui" fileserver="http://im.smartdot.com.cn:9910"/>
	const char *szFromName = pNode->Attribute("from");
	const char *szGrpId = pNode->Attribute("guid");
	if (szFromName && szGrpId)
	{
		PlayTipSound("group", szGrpId, FALSE);
		CGroupItem *pItem = GetGroupItemById(szGrpId);
		if (!pItem)
			return;
		HWND h = pItem->GetHWND();
		if ((h == NULL) || (!::IsWindow(h)))
		{
			IUIManager *pUI = NULL;
			if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
			{
				HRESULT hr;
				char szTmp[128] = {0};
				strncpy(szTmp, szGrpId, 127);
				pUI->SendMessageToWindow(UI_MAIN_WINDOW_NAME, WM_OPENGROUPFRAME, 
					      WPARAM(szTmp),0, &hr);
				h =  pItem->GetHWND();
				pUI->Release();
			} //end if (m_pCore && ...
		} else 
		{
			/*if (m_pCore)
			{
				m_pCore->StartTrayIcon("GRP", "收到离线文件", NULL);
			}*/
		}//end if (h == NULL

		if (h != NULL)
		{
			//add progress
			TCHAR szFlag[MAX_PATH] = {0};
			int nFlagSize = MAX_PATH - 1;
			CTransferFileInfo Info;
			Info.hOwner = h;
			Info.m_strDspName = pNode->Attribute("name");
			Info.m_dwFileSize = ::atoi(pNode->Attribute("filesize"));
			::SendMessage(GetMainFrameHWND(), WM_GRP_APPEND_PRO, 0, (LPARAM)&Info);  
			int nFileId = m_TransFileList.AddFileInfo(szGrpId, pNode->Attribute("name"), NULL, Info.m_strProFlag.GetData(), 
				pNode->Attribute("filetag"), "", "0", "", "0", "0", 0,
				pNode->Attribute("filesize"), h);
			if (pNode->Attribute("fileserver"))
				m_TransFileList.SetOfflineSvr(nFileId, pNode->Attribute("fileserver"));
			if (pNode->Attribute("url"))
				m_TransFileList.SetRemoteName(nFileId, pNode->Attribute("url"));
			CStdString_ strTip = _T("收到\"");
			TCHAR szTmp[MAX_PATH] = {0};
			IContacts *pContact = NULL;
			if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
			{
				CInterfaceAnsiString strRealName;
				if (SUCCEEDED(pContact->GetRealNameById(szFromName, NULL, &strRealName)))
					CStringConversion::UTF8ToWideChar(strRealName.GetData(), szTmp, MAX_PATH - 1);	
				strTip += szTmp;
				pContact->Release();
			}
			strTip += _T("\" 发送的讨论组文件 \"");
			memset(szTmp, 0, MAX_PATH * sizeof(TCHAR));
			CStringConversion::StringToWideChar(pNode->Attribute("name"), szTmp, MAX_PATH - 1); 
			strTip += szTmp;  
			AnsycShowTip(h, strTip.GetData());		
		} else
		{
			PRINTDEBUGLOG(dtInfo, "Open group Frame Failed in recv file");
		}
	}
}

BOOL CGroupFrameImpl::AnsycShowTip(HWND hWnd, const TCHAR *szTip)
{
	IUIManager *pUI = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
	{
		HRESULT hr;
		pUI->SendMessageToWindow(UI_MAIN_WINDOW_NAME, WM_SHOWGROUPTIPMSG, 
			WPARAM(hWnd), LPARAM(szTip), &hr);
		pUI->Release();
		return TRUE;
	}
	return FALSE;
}

void CGroupFrameImpl::DoGroupCustomPic(TiXmlElement *pNode)
{
	std::string strFlag;
	const char *szFileType = pNode->Attribute("filetype");
	if (szFileType && ::stricmp(szFileType, "normal") == 0)
	{
		DoRecvOfflineFile(pNode);
	} else
	{
		const char *szFromName = pNode->Attribute("guid");
		const char *szSvrAddr = pNode->Attribute("fileserver");
		const char *szSrcFile = pNode->Attribute("filename");
		if (szFromName && szSvrAddr && szSrcFile)
		{
			strFlag = szSrcFile;
			int nPos = strFlag.find(".");
			if (nPos != std::string::npos)
				strFlag = strFlag.substr(0, nPos);
			char szFileName[MAX_PATH] = {0};
			sprintf(szFileName, "%s%s.gif", GetImagePath(), strFlag.c_str());
			//
			std::string strUrl = szSvrAddr;
			strUrl += "/custompics/";
			strUrl += szSrcFile;
			if (!CSystemUtils::FileIsExists(szFileName))
			{ 
				CCustomPicItem *pItem = new CCustomPicItem();			
				pItem->m_hOwner =  NULL;
				pItem->m_strFlag = strFlag;
				pItem->m_pOverlapped = this;
				pItem->m_strLocalFileName = szFileName;  
				pItem->m_strPeerName = szFromName; 
				pItem->m_strUrl = strUrl;
				if (m_CustomPics.AddItem(pItem))
				{	
					::P2SvrAddDlTask(strUrl.c_str(), szFileName, FILE_TYPE_CUSTOMPIC, 
						 pItem, HttpDlCallBack, FALSE);
				} else
					delete pItem; 
				return ;
			} else
			{
				//
			} //end if (!CSystemUtils::FileIsExists(
		} //
	}
	return ;
}

void CGroupFrameImpl::RemoveTransFile(int nFileId, const char *szTip)
{
	CTransferFileInfo Info;
	if (m_TransFileList.GetFileInfoById(nFileId, Info))
	{
		m_TransFileList.DeleteFileInfo(nFileId);
		::SendMessage(GetMainFrameHWND(), WM_GRP_RM_FILE_PRO, WPARAM(&Info), 0);
		if (szTip && (!Info.m_strDspName.empty()))
		{
			char szTmp[512] = {0};
			TCHAR szwTmp[512] = {0};			
			sprintf(szTmp, szTip, Info.m_strDspName.c_str());
			CStringConversion::StringToWideChar(szTmp, szwTmp, MAX_PATH - 1);
			AnsycShowTip(Info.hOwner, szwTmp);
		}
	}
}

void CALLBACK HttpDlCallBack(int nErrorCode, int nType, WPARAM wParam, LPARAM lParam, void *pOverlapped)
{
	switch(nErrorCode)
	{
	case ERROR_CODE_COMPLETE:
		{
			if (nType == FILE_TYPE_CUSTOMPIC)
			{
				CCustomPicItem *pItem = (CCustomPicItem *)pOverlapped;
				CGroupFrameImpl *pThis = (CGroupFrameImpl *)pItem->m_pOverlapped;
				std::map<CAnsiString_, CGroupItem *>::iterator it = pThis->m_FrameList.find(pItem->m_strPeerName.c_str());
				if (it != pThis->m_FrameList.end())
				{			
					if (lParam == 0)
						::SkinReplaceImageInRichEdit(it->second->GetHWND(), L"messagedisplay", 
						      pItem->m_strLocalFileName.c_str(), pItem->m_strFlag.c_str());
					else
						pThis->AnsycShowTip(it->second->GetHWND(), L"下载图片失败");
				}
				pThis->m_CustomPics.DeleteItem(pItem);
			} else if (nType == FILE_TYPE_NORMAL)
			{ 
				CCustomPicItem *pItem = (CCustomPicItem *)pOverlapped;
				CGroupFrameImpl *pThis = (CGroupFrameImpl *)pItem->m_pOverlapped;
				std::string strTip = "文件 %%FILE%% 接收完毕 "; 
				pThis->ShowFileLink(pItem->m_hOwner, strTip.c_str(), pItem->m_strLocalFileName.c_str());
				CTransferFileInfo Info;
				if (pThis->m_TransFileList.GetFileInfoById(pItem->m_nFileId, Info))
				{
					char szTime[64] = {0};
					CSystemUtils::DateTimeToStr((DWORD)::time(NULL), szTime);
					std::string strMsg, strBody;
					strBody = "文件";
					strBody += pItem->m_strLocalFileName.c_str();
					strBody += " 接收成功";
					strMsg = "<tip datetime=\"";
					strMsg += szTime;
					strMsg += "\">";
					strMsg += strBody;
					strMsg += "</tip>";
					pThis->SaveGrpMsg("grp", pThis->m_strUserName.c_str(), Info.m_strPeerName.c_str(), szTime, 
						strMsg.c_str(), strBody.c_str());
				} 
				pThis->RemoveTransFile(pItem->m_nFileId, NULL) ;
				pThis->m_CustomPics.DeleteItem(pItem);
			}
			//end if (wParam == FILE_TYPE_CUSTOMPIC)
			break;
		} //end case ERROR_CODE_COMPLETE
	case ERROR_CODE_PROGRESS:
		{
			if (nType == FILE_TYPE_NORMAL)
			{
				//显示进度				
				CCustomPicItem *pItem = (CCustomPicItem *)pOverlapped;				
				CGroupFrameImpl *pThis = (CGroupFrameImpl *)pItem->m_pOverlapped;
				::PostMessage(pThis->GetMainFrameHWND(), WM_GRP_UPDL_PROGR, pItem->m_nFileId, lParam); 
			}
			break;
		}
	} //end switch(nErrorCode)
}

int CALLBACK FontDetailsEnumProc (const ENUMLOGFONTEX *lpelfe, const NEWTEXTMETRICEX *lpntme, unsigned long FontType, LPARAM lParam) 
{
	HWND h = reinterpret_cast<HWND> (lParam);
	if ((FontType == TRUETYPE_FONTTYPE)
		&& (lpelfe->elfFullName[0] != L'@'))
	{
		g_GrpChatFontList[lpelfe->elfFullName] = 0;
	}
	return 1;
}

BOOL CGroupFrameImpl::SaveGrpMsg(const char *szType, const char *szFromName, const char *szToName,
		                 const char *szTime, const char *szMsg, const char *szBody)
{
	IMsgMgr *pMsgMgr = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMsgMgr), (void **)&pMsgMgr)))
	{
		BOOL bSucc = SUCCEEDED(pMsgMgr->SaveMsg(szType, szFromName, szToName, szTime, szMsg, szBody));
		
		pMsgMgr->Release();
        return bSucc;
	}
	return FALSE;
}

HWND CGroupFrameImpl::GetMainFrameHWND()
{
	if (m_hMainWnd == NULL)
	{
		if (m_pCore)
		{
			IUIManager *pUI = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
			{
				pUI->GetWindowHWNDByName(UI_MAIN_WINDOW_NAME, &m_hMainWnd);
				pUI->Release();
			} //end if (SUCCEEDED(m_pCore->...
		} //end if (m_pCore)
	} //end if (m_hMainWnd == NULL)
	return m_hMainWnd;
}

void CGroupFrameImpl::InitGroupFrame(HWND hWnd)
{
	if (g_GrpChatFontList.empty())
	{
		//init font name
		HDC dc = ::GetDC(::GetDesktopWindow());
		LOGFONT lf;
		memset (&lf, 0, sizeof(lf));
		lf.lfCharSet	= DEFAULT_CHARSET;
		EnumFontFamiliesEx(dc, &lf, (FONTENUMPROC)FontDetailsEnumProc, reinterpret_cast<LPARAM>(hWnd), 0);
		::ReleaseDC(::GetDesktopWindow(), dc);	
	}
	std::map<CStdString_, int>::iterator it;
	for (it = g_GrpChatFontList.begin(); it != g_GrpChatFontList.end(); it ++)
	{
		::SkinSetDropdownItemString(hWnd, L"cbFontName", -1, it->first, NULL); 
	}
	//init font size
	TCHAR szTmp[8] = {0};
	for (int i = 8; i < 24; i ++)
	{
		::_itow(i, szTmp, 10);
		::SkinSetDropdownItemString(hWnd, L"cbFontSize", -1, szTmp, NULL);
	}
	IConfigure *pCfg = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		CInterfaceFontStyle FontStyle;
		pCfg->GetChatFontStyle((IFontStyle *)&FontStyle);
		CInterfaceAnsiString strTmp;
		TCHAR szValue[32] = {0};
		FontStyle.GetName((IAnsiString *)&strTmp);
		CStringConversion::StringToWideChar(strTmp.GetData(), szValue, 31);
		::SkinSetControlTextByName(hWnd, L"cbFontName", szValue);
		memset(szValue, 0, sizeof(TCHAR) * 32);
		::_ltow(FontStyle.GetSize(), szValue, 10);
		::SkinSetControlTextByName(hWnd, L"cbFontSize", szValue);
		if (FontStyle.GetBold())
			::SkinSetControlAttr(hWnd, L"fontbold", L"down", L"true");
		else
			::SkinSetControlAttr(hWnd, L"fontbold", L"down", L"false");
		if (FontStyle.GetItalic())
			::SkinSetControlAttr(hWnd, L"fontitalic", L"down", L"true");
		else
			::SkinSetControlAttr(hWnd, L"fontitalic", L"down", L"false");
		if (FontStyle.GetUnderline())
			::SkinSetControlAttr(hWnd, L"fontunderline", L"down", L"true");
		else
			::SkinSetControlAttr(hWnd, L"fontunderline", L"down", L"false");
		pCfg->Release();
	} 
}

//
BOOL CGroupFrameImpl::ParserGroupProtocol2(const char *pType, TiXmlElement *pNode)
{
	if (::stricmp(pType, "getallgrp_result") == 0)
	{
		TiXmlElement *pChild = pNode->FirstChildElement();
		while (pChild)
		{
			const char *pGrpId = pChild->Attribute("gid");
			const char *pGrpDspName = pChild->Attribute("nm");
			const char *pGrpCreator = pChild->Attribute("cd");
			if (pGrpId && pGrpDspName && pGrpCreator)
			{
				std::map<CAnsiString_, CGroupItem *>::iterator it = m_FrameList.find(pGrpId);
				if (it == m_FrameList.end())
				{
					CGroupItem *pItem = new CGroupItem(pGrpId, NULL);
					pItem->SetCreator(pGrpCreator);
					pItem->SetDispName(pGrpDspName);
					m_FrameList.insert(std::pair<CAnsiString_, CGroupItem *>(pGrpId, pItem));
				} //end if (it == m_FrameList.end())
			} //end if (pGrpId && 
			pChild = pChild->NextSiblingElement();
		} //end while(pChild)
		return TRUE;
	} //end if (::stricmp(pType, "
	return FALSE;
}

//IProtocolParser
STDMETHODIMP CGroupFrameImpl::DoRecvProtocol(const BYTE *pData, const LONG lSize)
{
	TiXmlDocument xmldoc;
	BOOL bSucc = FALSE;
	if (xmldoc.Load((char *)pData, lSize))
	{
		TiXmlElement *pNode = xmldoc.FirstChildElement();
		const char *pName = pNode->Value();
		const char *pType = pNode->Attribute("type");
		if (pName && pType)
		{
			if (::stricmp(pName, "grp") == 0)
			{
				bSucc = ParserGroupProtocol(pType, pNode); 
			} else if (::stricmp(pName, "grp2") == 0)
			{
				bSucc = ParserGroupProtocol2(pType, pNode);
			} else if (::stricmp(pName, "sys") == 0)
			{
				bSucc = DoSysProtocol(pType, pNode);
			}
		} //end if (pName && pType)
	} //end if (xmldoc.Load(...
	if (bSucc)
		return S_OK;
	else
		return E_FAIL; 
}

int CALLBACK CompareNode(CTreeNodeType tnType1, int nStatus1, void  *pData1, 
	                                  CTreeNodeType tnType2, int nStatus2, void *pData2)
{
	if (nStatus2 > nStatus1)
		return 1;
	else if (nStatus2 < nStatus1)
		return -1;
	else 
	    return 0;
}

STDMETHODIMP CGroupFrameImpl::DoPresenceChange(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder)
{ 
	std::map<CAnsiString_, CGroupItem *>::iterator it;
	for (it = m_FrameList.begin(); it != m_FrameList.end(); it ++)
	{
		if ((it->second->GetHWND() != NULL) && ::IsWindow(it->second->GetHWND()))
		{
			::SkinUpdateUserStatusToNode(it->second->GetHWND(), L"groupuserlist", szUserName, szNewPresence, FALSE);
			::SkinSortTreeNode(it->second->GetHWND(), L"groupuserlist", NULL, CompareNode, FALSE, TRUE); 
			UpdateGroupOnlineInfo(it->second->GetHWND());
		}
	} //end for (it = m_FrameList.begin();
	return S_OK;
}

STDMETHODIMP CGroupFrameImpl::GetGroupIdByHWND(HWND hOwner, IAnsiString *strGroupId)
{
	CGroupItem *pItem = GetGroupItemByHWND(hOwner);
	if (pItem && strGroupId)
	{
		strGroupId->SetString(pItem->GetGroupId());
		return S_OK;
	}
	return E_FAIL;
}

CGroupItem *CGroupFrameImpl::GetGroupItemByHWND(HWND hWnd)
{
	std::map<CAnsiString_, CGroupItem *>::iterator it;
	for (it = m_FrameList.begin(); it != m_FrameList.end(); it ++)
	{
		if (it->second->GetHWND() == hWnd)
		{
			return it->second;
		}
	}
	return NULL;
}

void CGroupFrameImpl::InitSelfUserInfo()
{
	if (m_pCore)
	{
		if (m_strUserName.empty())
		{
			CInterfaceAnsiString strUserName;
			m_pCore->GetUserName((IAnsiString *)&strUserName);
			m_strUserName = strUserName.GetData();
			m_pCore->GetUserDomain((IAnsiString *)&strUserName);
			m_strUserName += "@";
			m_strUserName += strUserName.GetData();
		}
		if (m_strRealName.empty())
		{
			CInterfaceAnsiString strTmp; 
			m_pCore->GetUserNickName((IAnsiString *)&strTmp);
			m_strRealName = strTmp.GetData();
		}
	}
}

BOOL CGroupFrameImpl::UploadCustomPicToServer(HWND hWnd, const char *szFlag)
{
	BOOL bSucc = FALSE;
	IEmotionFrame *pFrame = NULL;
	CInterfaceAnsiString strFileName;
	BOOL bCustom = FALSE;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IEmotionFrame), (void **)&pFrame)))
	{ 
		if (SUCCEEDED(pFrame->GetSysEmotion(szFlag, &strFileName)))
			bSucc = TRUE;
		else
		{
			if (SUCCEEDED(pFrame->GetCustomEmotion(szFlag, &strFileName)))
				bCustom = TRUE;
		}

		pFrame->Release();
	}
	if (bSucc)
		return TRUE;
	 
	char szFileName[MAX_PATH] = {0};
	if (bCustom)
	{
		strcpy(szFileName, strFileName.GetData());
	} else
	{
		sprintf(szFileName, "%s%s.gif", GetImagePath(), szFlag);
	}
	//
	IConfigure *pCfg = NULL;
	if ((CSystemUtils::FileIsExists(szFileName)) && m_pCore 
		&& SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		CInterfaceAnsiString strUrl;
		if (SUCCEEDED(pCfg->GetServerAddr(HTTP_SVR_URL_CUSTOM_PICTURE, &strUrl)))
		{
			CGroupItem *pGroup = GetGroupItemByHWND(hWnd);
			if (pGroup)
			{
				CCustomPicItem *pItem = new CCustomPicItem();
				pItem->m_hOwner = hWnd;
				pItem->m_strFlag = szFlag;
				pItem->m_pOverlapped = this;
				pItem->m_strLocalFileName = szFileName; 
				CInterfaceAnsiString strUserName; 
				

				pItem->m_strPeerName = pGroup->GetGroupId(); 
				pItem->m_strUrl = strUrl.GetData();
				if (m_CustomPics.AddItem(pItem))
				{				
					::P2SvrPostFile(strUrl.GetData(), szFileName, NULL, FILE_TYPE_CUSTOMPIC, 
						pItem, HttpUpCallBack, FALSE);
				} else
					delete pItem;
			} //end if (pGroup)
		} //end if (SUCCEEDED(pCfg->GetServerAddr(....
		pCfg->Release();
		return TRUE;
	}
	return FALSE;
}

BOOL CGroupFrameImpl::SendOleResourceToPeer(HWND hWnd)
{
	//
	char *pOle = NULL;
	if (::SkinGetRichEditOleFlag(hWnd, L"messageedit", &pOle) && pOle)
	{ 
		int nIdx = 0;
		while (TRUE)
		{
			char szTmp[128] = {0};
			if (!CSystemUtils::GetStringBySep(pOle, szTmp, ';', nIdx))
				break; 
			if (::strlen(szTmp) > 0)
				UploadCustomPicToServer(hWnd, szTmp);
			nIdx ++;
		}
		free(pOle);
	}
	return TRUE;
}

void CALLBACK HttpUpCallBack(int nErrorCode, int nType, WPARAM wParam, LPARAM lParam, void *pOverlapped)
{
	switch(nErrorCode)
	{
	case ERROR_CODE_COMPLETE:
		{
			if (nType == FILE_TYPE_CUSTOMPIC)
			{
				//custom picture upload notify
				//// <grp type="p2pcustompic" from="wuxiaozhong@gocom"  guid="abcdefaadfadf" 
				//   filename="30f79afad4318774e447dc2db96936e0.gif" fileserver="http://imbs.smartdot.com.cn:9910">
				//</grp>
				CCustomPicItem *pItem = (CCustomPicItem *)pOverlapped;				
				CGroupFrameImpl *pThis = (CGroupFrameImpl *)pItem->m_pOverlapped;
				std::string strXml = "<grp type=\"p2pcustompic\" from=\"";
				strXml += pThis->m_strUserName;
				strXml += "\" guid=\"";
				strXml += pItem->m_strPeerName;
				strXml += "\" filename=\"";
				char szTmp[MAX_PATH] = {0};
				CSystemUtils::ExtractFileName(pItem->m_strLocalFileName.c_str(), szTmp, MAX_PATH - 1);
				strXml += szTmp;
				strXml += "\" fileserver=\"";
				//url = "http://imbs.smartdot.com.cn:9910/upcustompic.php"
				int nPos = pItem->m_strUrl.find_last_of(":");
				std::string strSvr;
				if (nPos != std::string::npos)
				{
					nPos = pItem->m_strUrl.find("/", nPos);
					if (nPos != std::string::npos)
					{
						strSvr = pItem->m_strUrl.substr(0, nPos);
					} //end if (nPos != ...
				} //end if (nPos != std::...
				if (!strSvr.empty())
					strXml += strSvr;
				else
					strXml += pItem->m_strUrl;
				strXml += "\"/>"; 
				if (pThis->m_pCore && lParam == 0)
					pThis->m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG)strXml.size(), 0); 
				pThis->m_CustomPics.DeleteItem(pItem);

			} else if (nType == FILE_TYPE_NORMAL)
			{
				//<grp type="p2pcustompic" filetype="normal"  from="admin@gocom" guid="13134cddefad2341" name="apss.dll.mui" filesize="3072" url="A90C62138DFB74BEA86244E2432D133EF.mui
				CCustomPicItem *pItem = (CCustomPicItem *)pOverlapped;				
				CGroupFrameImpl *pThis = (CGroupFrameImpl *)pItem->m_pOverlapped;
				std::string strXml = "<grp type=\"p2pcustompic\" filetype=\"normal\" from=\"";
				strXml += pThis->m_strUserName;
				strXml += "\" guid=\"";
				strXml += pItem->m_strPeerName;
				strXml += "\" name=\"";
				char szTmp[MAX_PATH] = {0};
				CSystemUtils::ExtractFileName(pItem->m_strLocalFileName.c_str(), szTmp, MAX_PATH - 1);
				strXml += szTmp;
				strXml += "\" filesize=\"";
				memset(szTmp, 0, MAX_PATH);
				::itoa(pItem->m_nFileSize, szTmp, 10);
				strXml += szTmp;
				strXml += "\" url=\"";
				strXml += pItem->m_strFlag;
				memset(szTmp, 0, MAX_PATH);
				CSystemUtils::ExtractFileExtName(pItem->m_strLocalFileName.c_str(), szTmp, MAX_PATH - 1);
				strXml += ".";  //szTmp 扩展名不带 .
				strXml += szTmp;
				strXml += "\" fileserver=\"";
				//url = "http://imbs.smartdot.com.cn:9910/upcustompic.php"
				int nPos = pItem->m_strUrl.find_last_of(":");
				std::string strSvr;
				if (nPos != std::string::npos)
				{
					nPos = pItem->m_strUrl.find("/", nPos);
					if (nPos != std::string::npos)
					{
						strSvr = pItem->m_strUrl.substr(0, nPos);
					} //end if (nPos != ...
				} //end if (nPos != std::...
				if (!strSvr.empty())
					strXml += strSvr;
				else
					strXml += pItem->m_strUrl;
				strXml += "\"/>";
				if (pThis->m_pCore)
					pThis->m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG)strXml.size(), 0);
				CStdString_ strTip = L"文件 ";
				TCHAR szwTmp[MAX_PATH] = {0};
				CStringConversion::StringToWideChar(pItem->m_strLocalFileName.c_str(), szwTmp, MAX_PATH - 1);
				strTip += szwTmp;
				if (lParam == 0)
					strTip += L"  发送成功";
				else
					strTip += L"  发送失败";
				char szTime[64] = {0};
				CSystemUtils::DateTimeToStr((DWORD)::time(NULL), szTime);
				std::string strMsg, strBody;
				strBody = "文件";
				strBody += pItem->m_strLocalFileName.c_str();
				strBody += " 发送成功";
				strMsg = "<tip datetime=\"";
				strMsg += szTime;
				strMsg += "\">";
				strMsg += strBody;
				strMsg += "</tip>";
				pThis->SaveGrpMsg("grp", pThis->m_strUserName.c_str(), pItem->m_strPeerName.c_str(), szTime, 
					strMsg.c_str(), strBody.c_str());
				pThis->AnsycShowTip(pItem->m_hOwner, strTip.GetData());
				pThis->RemoveTransFile(pItem->m_nFileId, NULL);
				pThis->m_CustomPics.DeleteItem(pItem);
			}//end if (wParam ==
			break;
		} //end case Error
	case ERROR_CODE_PROGRESS:
		{
			if (nType == FILE_TYPE_NORMAL)
			{
				//显示进度				
				CCustomPicItem *pItem = (CCustomPicItem *)pOverlapped;				
				CGroupFrameImpl *pThis = (CGroupFrameImpl *)pItem->m_pOverlapped;
				::PostMessage(pThis->GetMainFrameHWND(), WM_GRP_UPDL_PROGR, pItem->m_nFileId, lParam); 
			}
			break;
		}
	} //end switch(..
}

BOOL CGroupFrameImpl::CheckInputChars(const char *p)
{
	if (p)
	{
		int nSize = ::strlen(p); 
		const char *p1 = p;
		while (*p1 == ' ')
		{
			p1 ++;
			nSize --;
		}
		if (nSize > 1)
		{
			IConfigure *pCfg = NULL;
			if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
			{	
				BOOL bSucc = SUCCEEDED(pCfg->CheckKeyWord(p));
				pCfg->Release();
				return bSucc;
			} //end if (m_pCore ..
		} //end if (nSize > 1)
	} //end if (p)
	return FALSE;
}

void CGroupFrameImpl::GetUserListByItem(CGroupItem *pItem, std::string &strUserList)
{
	CInterfaceUserList userList;
	pItem->GetUserList(&userList);
	LPORG_TREE_NODE_DATA pData;
    while (SUCCEEDED(userList.PopFrontUserInfo(&pData)))
	{
		strUserList += pData->szUserName;
		strUserList += ",";
	}
}

static const char GROUP_MESSAGE_XML_FORMAT[] = "<grp type=\"msg\"><font/><body></body></grp>";

#define MAX_SEND_TEXT_LENGTH 2000  //每次最多发送的字节数
#define MAX_SEND_WCHAR_TEXT_LENGTH 1000  //每次最多发送的汉字
BOOL CGroupFrameImpl::SendMessageToPeer(HWND hWnd)
{
	if (m_pCore)
	{
		if (m_pCore->CanAllowAction(USER_ROLE_SEND_MESSAGE) != S_OK)
		{
			::SkinMessageBox(hWnd, L"没有发送消息权限", L"提示", MB_OK);
			return FALSE;
		}
	}
	//
	CCharFontStyle cf = {0};
	if (::SkinGetCurrentRichEditFont(hWnd, L"messageedit", &cf))
	{
		char *p = ::SkinGetRichEditOleText(hWnd, L"messageedit", 0);
		if (CheckInputChars(p)) // p && strlen(p) > 1)
		{
			CGroupItem *pGroup = GetGroupItemByHWND(hWnd);
			if (pGroup)
			{
				SendOleResourceToPeer(hWnd);
				//<grp type="msg" guid="%s" from="%s" creator="%s"><font name="%s" size="%s" color="%s" bold="%s" underline="%s"' +
                  //   ' strikeout="%s" italic="%s"/><body>%s</body></grp>';
				char szTime[64] = {0};
				CSystemUtils::DateTimeToStr((DWORD)::time(NULL), szTime);
				TiXmlDocument xmlDoc;
				if (xmlDoc.Load(GROUP_MESSAGE_XML_FORMAT, ::strlen(GROUP_MESSAGE_XML_FORMAT)))
				{
					TiXmlElement *pNode = xmlDoc.FirstChildElement();
					pNode->SetAttribute("guid", pGroup->GetGroupId());
					pNode->SetAttribute("from", m_strUserName.c_str());
					pNode->SetAttribute("creator", pGroup->GetCreator());
					pNode->SetAttribute("datetime", szTime);
					//Font
					TiXmlElement *pFont = pNode->FirstChildElement("font");
					if (pFont)
					{
						CXmlNodeTranslate::FontStyleToXmlNode(cf, pFont);
					}
					TiXmlElement *pBody = pNode->FirstChildElement("body");
					if (pBody)
					{
						TiXmlText pText("");
						pBody->InsertEndChild(pText);
					} 
					int nSize = ::strlen(p);
					if (nSize > MAX_SEND_TEXT_LENGTH)
					{
						TCHAR *szwSrc = new TCHAR[nSize + 1];
						memset(szwSrc, 0, sizeof(TCHAR) * (nSize + 1));
						CStringConversion::StringToWideChar(p, szwSrc, nSize);
						int nOffset = 0;
						char szTmp[MAX_SEND_TEXT_LENGTH + 1];
						TCHAR szwTmp[MAX_SEND_WCHAR_TEXT_LENGTH + 1];
						int nwSize = ::lstrlen(szwSrc);
						while (nwSize > 0)
						{ 
							TiXmlNode *pText = pBody->FirstChild();
							if (nSize > MAX_SEND_WCHAR_TEXT_LENGTH)
							{
							    memset(szTmp, 0, MAX_SEND_TEXT_LENGTH + 1); 
								memset(szwTmp, 0, (MAX_SEND_WCHAR_TEXT_LENGTH + 1) * sizeof(TCHAR));
								::lstrcpyn(szwTmp, (szwSrc + nOffset), MAX_SEND_WCHAR_TEXT_LENGTH);
								CStringConversion::WideCharToString(szwTmp, szTmp, MAX_SEND_TEXT_LENGTH);
								pText->SetValue(szTmp);
								nOffset += MAX_SEND_WCHAR_TEXT_LENGTH;
								nwSize -= MAX_SEND_WCHAR_TEXT_LENGTH;
							} else
							{
								memset(szTmp, 0, MAX_SEND_TEXT_LENGTH + 1);
								CStringConversion::WideCharToString(szwSrc + nOffset, szTmp, MAX_SEND_TEXT_LENGTH);
								pText->SetValue(szTmp);
								nSize = 0;
							}
							TiXmlString strXml;
							xmlDoc.SaveToString(strXml, 0);
							//send to peer
							m_pCore->SendRawMessage((BYTE *)strXml.c_str(), strXml.size(), 0);

							std::string strUserList;
							GetUserListByItem(pGroup, strUserList);
							pNode->SetAttribute("userlist", strUserList.c_str());
							//存消息
							strXml.clear();
							xmlDoc.SaveToString(strXml, 0);
					        SaveGrpMsg("grp", m_strUserName.c_str(), pGroup->GetGroupId(), szTime, 
						            strXml.c_str(), szTmp); 
						}
						delete []szwSrc;
					} else
					{
						TiXmlNode *pText = pBody->FirstChild(); 
						pText->SetValue(p);
						
						TiXmlString strXml;
						xmlDoc.SaveToString(strXml, 0);
						//send to peer
						m_pCore->SendRawMessage((BYTE *)strXml.c_str(), strXml.size(), 0);
						//存消息
						std::string strUserList;
						GetUserListByItem(pGroup, strUserList);
						pNode->SetAttribute("userlist", strUserList.c_str());
						//存消息
						strXml.clear();
						xmlDoc.SaveToString(strXml, 0);
					    SaveGrpMsg("grp", m_strUserName.c_str(), pGroup->GetGroupId(), szTime, 
						       strXml.c_str(), p); 
					}
					//
					//SaveMessage("grp", xmlDoc.FirstChildElement());
					//清除输入框
					::SkinRichEditCommand(hWnd, L"messageedit", "clear", NULL);
					////add to display
					::SkinAddRichChatText(hWnd, L"messagedisplay", NULL, 0, m_strRealName.c_str(), szTime, p, &cf, 
						UI_NICK_NAME_COLOR, TRUE, FALSE); 
					
				} else
				{
					PRINTDEBUGLOG(dtInfo, "Load Base P2p Xml Failed");
				} //end else if (xmlDoc.Load(...
			} //end if (it != m_ChatFrameList.end()
			free(p);
		} else 
		{
			::SkinMessageBox(hWnd, L"请输入消息内容", L"提示", MB_OK);
			::SkinSetControlFocus(hWnd, L"messageedit", TRUE);
		}//end if (p)
	} //end if (::GetCurrentRichEditFont(...
	return FALSE;
}

//发送文件至群
STDMETHODIMP CGroupFrameImpl::SendFileToGroup(const char *szGrpId, const char *szFileName)
{
	if (SUCCEEDED(ShowGroupFrame(szGrpId, NULL)))
	{
		CGroupItem *pItem = GetGroupItemById(szGrpId);
		if (pItem && (pItem->GetHWND() != NULL) && ::IsWindow(pItem->GetHWND()))
		{
			std::string strFileName;
			if ((szFileName == NULL) || ::strlen(szFileName) == 0)
			{
				CStringList_ FileList;
				if (CSystemUtils::OpenFileDialog(NULL, pItem->GetHWND(), "选择要发送的文件", "所有文件(*.*)|*.*", 
					                NULL, FileList, FALSE, FALSE))
				{ 
					if (!FileList.empty())
						strFileName = FileList.back();
				}
			} else
				strFileName = szFileName;
			
			if ((!strFileName.empty()) && CSystemUtils::FileIsExists(strFileName.c_str()))
			{
				SendFileToGroup(pItem->GetHWND(),  strFileName.c_str()); 	
				return S_OK;
			} //end if ((!strFileName.empty()) ...
			
		} //end if (pItem ..
	}
	return E_FAIL;
}

STDMETHODIMP CGroupFrameImpl::SendMailToGroup(const char *szGrpId)
{
	HRESULT hr = E_FAIL;
	CGroupItem *pItem = GetGroupItemById(szGrpId);
	if (pItem)
	{
		std::string strOpenString = "mailto:";
		CInterfaceUserList ulList;
		if (SUCCEEDED(pItem->GetUserList(&ulList)))
		{
			LPORG_TREE_NODE_DATA pData;
			CInterfaceAnsiString strTmp;
			IContacts *pContact = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
			{
				while (SUCCEEDED(ulList.PopBackUserInfo(&pData)))
				{
					if (SUCCEEDED(pContact->GetMailByUserName(pData->szUserName, &strTmp)))
					{
						if (strTmp.GetSize() > 0)
						{
							strOpenString += strTmp.GetData();
							strOpenString += ";";
						} //end if (strTmp.
					}
					delete pData;
				}
				pContact->Release();
			} //end if (SUCCEEDED(m_pCore->Q
		} //end if (SUCCEEDED(
		TCHAR szTmp[MAX_PATH] = {0};
		CStringConversion::StringToWideChar(strOpenString.c_str(), szTmp, MAX_PATH - 1);
		::ShellExecute(NULL, L"open", szTmp, NULL, NULL, SW_SHOW);
	} //end if (pItem)
	return E_FAIL;
}

//发送短信至群
STDMETHODIMP CGroupFrameImpl::SendSMSToGroup(const char *szGrpId, const char *szSMSText)
{
	HRESULT hr = E_FAIL;
	CGroupItem *pItem = GetGroupItemById(szGrpId);
	if (pItem)
	{
		CInterfaceUserList ulList;
		if (SUCCEEDED(pItem->GetUserList(&ulList)))
		{
			
			ISMSFrame *pFrame = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(ISMSFrame), (void **)&pFrame)))
			{
				hr = pFrame->ShowSMSFrame(NULL, &ulList);
				pFrame->Release();
			} 
		} //end if (SUCCEEDED(
	} //end if (pItem)
	return hr;
}

//根据讨论组ID获取名称 
STDMETHODIMP CGroupFrameImpl::GetGroupNameById(const char *szGrpId, IAnsiString *strGrpName)
{
	CGroupItem *pItem = GetGroupItemById(szGrpId);
	if (pItem)
	{
		strGrpName->SetString(pItem->GetDispName());
		return S_OK;
	}
	return E_FAIL;
}

BOOL CGroupFrameImpl::DoGroupMsg(TiXmlElement *pNode)
{
	TiXmlString strXml;
	pNode->SaveToString(strXml, 0);
	const char *szGrpId = pNode->Attribute("guid");
	IMainFrame *pFrame = NULL;
	BOOL bDid = FALSE;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMainFrame), (void **) &pFrame)))
	{
		//存消息
		 //
		const char *szTime = pNode->Attribute("datetime");
		TiXmlElement *pBody = pNode->FirstChildElement("body");
		std::string strBody;
		if (pBody)
		{
			if (pBody->GetText())
			{
				strBody = pBody->GetText();
				strBody += "\n";
			} //end if (pBody->GetText())
		} // end if (pBody)
		SaveGrpMsg("grp", pNode->Attribute("from"), szGrpId, szTime, strXml.c_str(), strBody.c_str()); 
		
		PlayTipSound("group", szGrpId, FALSE);
		CGroupItem *pItem = GetGroupItemById(szGrpId);
		if (pItem && (pItem->GetHWND() != NULL) && ::IsWindow(pItem->GetHWND()))
		{
			::SendMessage(pFrame->GetSafeWnd(), WM_SHOWGROUPMSG, WPARAM(szGrpId), LPARAM(strXml.c_str()));
			bDid = TRUE;
		}
		pFrame->Release(); 
	}
	return bDid;
}

void CGroupFrameImpl::RefreshLastOpenFrameRect()
{
	if (::IsRectEmpty(&m_rcLastOpen))
	{
		if (m_pCore)
		{
			IConfigure *pCfg = NULL;		
			HRESULT hr = m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg);
			if (SUCCEEDED(hr))
			{
				CInterfaceAnsiString strPos;
				if (SUCCEEDED(pCfg->GetParamValue(FALSE, "Position", "ChatFrame", (IAnsiString *)&strPos)))
				{
					CSystemUtils::StringToRect(&m_rcLastOpen, strPos.GetData());
				} else
				{
					memset(&m_rcLastOpen, 0, sizeof(RECT));
				} //end else if (SUCCEEDED(pCfg->GetParamValue...
				pCfg->Release();
			} //end if (SUCCEEDED(hr))
		} //end if (m_pCore)
	} else
	{
		::OffsetRect(&m_rcLastOpen, CHAT_FRAME_OFFSET_X, CHAT_FRAME_OFFSET_Y);
		RECT rcScreen;
		CSystemUtils::GetScreenRect(&rcScreen);
		if ((m_rcLastOpen.right > rcScreen.right)
			|| (m_rcLastOpen.bottom > rcScreen.bottom))
		{
			int dx =  CHAT_FRAME_OFFSET_X / 2 - m_rcLastOpen.left;
			int dy = CHAT_FRAME_OFFSET_Y / 2 - m_rcLastOpen.top;
			::OffsetRect(&m_rcLastOpen, dx, dy);
		} //end if ((m_rcLastOpen..
	} //end else if (::IsRectEmpty(...
	if (::IsRectEmpty(&m_rcLastOpen))
	{
		m_rcLastOpen.left = 100;
		m_rcLastOpen.top = 100;
		m_rcLastOpen.right = 600;
		m_rcLastOpen.bottom = 500;
	}
}

void CGroupFrameImpl::RefreshInputChatFont(HWND hWnd, IConfigure *pCfg)
{
	CInterfaceFontStyle FontStyle;
	if (SUCCEEDED(pCfg->GetChatFontStyle((IFontStyle *)&FontStyle)))
	{
		CCharFontStyle fs = {0};
		CInterfaceAnsiString strTmp;
		FontStyle.GetName((IAnsiString *)&strTmp);
		CStringConversion::StringToWideChar(strTmp.GetData(), fs.szFaceName, 31);
  
		fs.nFontSize = FontStyle.GetSize();
		fs.cfColor = FontStyle.GetColor();
 
		if (FontStyle.GetBold())
			fs.nFontStyle |= CFE_BOLD; 
		if (FontStyle.GetItalic()) 
			fs.nFontStyle |= CFE_ITALIC; 
		if (FontStyle.GetUnderline()) 
			fs.nFontStyle |= CFE_UNDERLINE;
		::SkinNotifyEvent(hWnd, L"messageedit", L"setfont", 0, LPARAM(&fs));
	}
}

void CGroupFrameImpl::LoadGroupUser(HWND hWnd, CGroupItem *pItem)
{
	CInterfaceUserList List;
	LPORG_TREE_NODE_DATA pData;
	IContacts *pContact = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
	{
		pItem->GetUserList(&List);
		std::string strXml;
		CInterfaceAnsiString strPresence;
		while (SUCCEEDED(List.PopFrontUserInfo(&pData)))
		{
			TCHAR szText[MAX_PATH] = {0};
			if (pData->szDisplayName)
			{
				CStringConversion::UTF8ToWideChar(pData->szDisplayName, szText, MAX_PATH - 1);
			} else
			{
				CInterfaceAnsiString szName;
				if (SUCCEEDED(pContact->GetRealNameById(pData->szUserName, NULL, &szName)))
				{
					CStringConversion::UTF8ToWideChar(szName.GetData(), szText, MAX_PATH - 1);
				}
			}
		 
			::SkinAddTreeChildNode(hWnd, L"groupuserlist", pData->id, NULL, szText, TREENODE_TYPE_LEAF, pData, NULL, NULL, NULL);
			if (::stricmp(pData->szUserName, pItem->GetCreator()) == 0)
			{
				::SkinUpdateTreeNodeExtraImageFile(hWnd, L"groupuserlist", pData->szUserName, 350, FALSE);
			}
			if (SUCCEEDED(pContact->GetContactUserValue(pData->szUserName, "presence", &strPresence)))
			{
				if (strPresence.GetSize() > 0)
					::SkinUpdateUserStatusToNode(hWnd, L"groupuserlist", pData->szUserName,
					                 strPresence.GetData(), FALSE);
			}
			//order xml
			strXml += "<i u=\"";
			strXml += pData->szUserName;
			strXml += "\"/>";
		}
		pContact->AddOrderUserList(strXml.c_str());
		pContact->Release();
	}
	::SkinExpandTree(hWnd, L"groupuserlist", NULL, TRUE, TRUE);
	::SkinSetTreeIconType(hWnd, L"groupuserlist", 2);
	::SkinSortTreeNode(hWnd, L"groupuserlist", NULL, CompareNode, FALSE, TRUE);
	UpdateGroupOnlineInfo(hWnd);
}

void CGroupFrameImpl::UpdateGroupOnlineInfo(HWND hWnd)
{
	DWORD dwTotalCount = 0, dwOnlineCount = 0;
	if (::SkinTVGetOnlineCount(hWnd, L"groupuserlist", NULL, &dwTotalCount, &dwOnlineCount))
	{
		TCHAR szTmp[64] = {0};
		wsprintf(szTmp, L"(%d/%d)", dwOnlineCount, dwTotalCount);
		::SkinSetControlTextByName(hWnd, L"lblgroupstat", szTmp);
	}
}

BOOL CGroupFrameImpl::OpenGroupFrame(const char *szGrpId, const char *szUTF8DspName)
{ 
	if (m_pCore)
	{ 
		InitSelfUserInfo();
		std::map<CAnsiString_, CGroupItem *>::iterator it = m_FrameList.find(szGrpId);
		if (it != m_FrameList.end())
		{
			if (it->second->GetHWND() != NULL)
			{
				if (::IsWindow(it->second->GetHWND()))
				{
					CSystemUtils::BringToFront(it->second->GetHWND());
					return TRUE;
				} else
				{
					it->second->SetOwnerHWND(NULL);
				}
			}
		 
			IUIManager *pUI = NULL; 	
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
			{
				RefreshLastOpenFrameRect();
				HWND hTemp = NULL; 
				TCHAR szCaption[MAX_PATH] = {0};
				CStringConversion::UTF8ToWideChar(it->second->GetDispName(), szCaption, MAX_PATH - 1);
				::lstrcat(szCaption, L"-----分组讨论");
				m_bInitFrame = TRUE;
				pUI->CreateUIWindow(NULL, "groupWindow", &m_rcLastOpen, WS_POPUP | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX
					                | WS_MINIMIZEBOX, WS_EX_ACCEPTFILES, szCaption, &hTemp); 
				it->second->SetOwnerHWND(hTemp);
				//Order window message
				pUI->OrderWindowMessage("groupWindow", hTemp, WM_DESTROY, (ICoreEvent *) this);
				pUI->OrderWindowMessage("groupWindow", hTemp, WM_DROPFILES, (ICoreEvent *) this);

				IMainFrame *pMain = NULL;
				if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMainFrame), (void **)&pMain)))
				{					
					pMain->ShowRecentlyUser(szGrpId, it->second->GetDispName());
					pMain->Release();
				}

				//设置回调
				::SkinSetRichEditCallBack(hTemp, L"messagedisplay", RichEditCallBack, this);
				::SkinSetRichEditCallBack(hTemp, L"messageedit", RichEditCallBack, this);
				//
				
				IConfigure *pCfg = NULL;
				if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
				{
					RefreshInputChatFont(hTemp, pCfg);
					CInterfaceAnsiString strValue;
				    m_bEnterSend = TRUE;
				    if (SUCCEEDED(pCfg->GetParamValue(FALSE, "hotkey", "ctrlentersendmsg", &strValue)))
				    {
					    if (::stricmp(strValue.GetData(), "true") == 0)
						    m_bEnterSend = FALSE;
				    }
					//设置是否智能合并
					if (SUCCEEDED(pCfg->GetParamValue(FALSE, "normal", "aimsg", &strValue)))
					{
						if (::stricmp(strValue.GetData(), "false") == 0)
							::SkinSetControlAttr(hTemp, L"messagedisplay", L"mergemsg", L"false");
					}
					//设置icon
					::SkinSetControlAttr(hTemp, L"windowicon", L"image", L"351");
					//设置是否透明
					if (SUCCEEDED(pCfg->GetParamValue(FALSE, "normal", "msgtransparent", &strValue)))
					{
						if (::stricmp(strValue.GetData(), "true") == 0)
						{
							::SkinSetControlAttr(hTemp, L"messagedisplay", L"transparent", L"true");
							if (SUCCEEDED(pCfg->GetParamValue(FALSE, "normal", "transimagefile", &strValue)))
							{
								if ((strValue.GetSize() > 0) && CSystemUtils::FileIsExists(strValue.GetData()))
								{
									TCHAR szTmp[MAX_PATH] = {0};
									CStringConversion::StringToWideChar(strValue.GetData(), szTmp, MAX_PATH - 1);
									::SkinSetControlAttr(hTemp, L"chatdisplaycanvs", L"imagefile", szTmp);
								}
							} //end if (SUCCEEDED(pCfg->
						} //end if (::stricmp(strValue..
					} //end if (SUCCEEDED(pCfg->...
					pCfg->Release();
					//
				}
				if (m_bEnterSend)
					::SkinSetMenuChecked(hTemp, L"sendmenu", 30001, TRUE);
				else
					::SkinSetMenuChecked(hTemp, L"sendmenu", 30002, TRUE);
				//order status 
				if (::IsWindow(hTemp))
				{	 
					 ::ShowWindow(hTemp, SW_SHOW);
					 CSystemUtils::BringToFront(hTemp);
					 LoadGroupUser(hTemp, it->second);
					 ::SkinSetControlFocus(hTemp, L"messageedit", TRUE);
				}
				m_bInitFrame = FALSE;

				GetPendingMsgByName(hTemp, szGrpId);
				//m_bInitFrame = FALSE;
				pUI->Release();
				pUI = NULL;
			} //end if (it != m_FrameList...
		} //end if (SUCCEEDED(hr) && pUI)
	} //end if (m_pCore)
	return TRUE;
}

BOOL CGroupFrameImpl::GetPendingMsgByName(HWND hWnd, const char *szGrpId)
{
	if (m_pCore)
	{
		CInterfaceAnsiString strProtocol;
		while (SUCCEEDED(m_pCore->GetFrontPendingMsg(szGrpId, "grp", (IAnsiString *)&strProtocol, TRUE)))
		{
			TiXmlDocument xmlDoc;
			if (xmlDoc.Load(strProtocol.GetData(), strProtocol.GetSize()))
			{
				TiXmlElement *pNode = xmlDoc.FirstChildElement();
				RecvMessageFromGroup(hWnd, pNode, NULL);
			} //end if (xmlDoc.Load(..
		} //end while (SUCCEEDED(m_pCore->...
		return TRUE;
	} // end if (m_pCore)
	return FALSE;
}

STDMETHODIMP CGroupFrameImpl::CreateGroup(const char *szGrpId, const char *szGrpName, IUserList *pUser)
{
	//<grp type="creategroup" guid="" name="" creator="">
		//   <i uid=""/>
		//</grp>
	if (m_pCore && szGrpId && szGrpName && pUser)
	{
		InitSelfUserInfo();
		std::string strXml;
		std::map<CAnsiString_, CGroupItem *>::iterator it = m_FrameList.find(szGrpId);
		if (it == m_FrameList.end())
		{
			if ((pUser->GetUserCount() > 2) && (pUser->GetUserCount() < 300))
			{
				char szDspGrpName[MAX_PATH] = {0};
				CStringConversion::UTF8ToString(szGrpName, szDspGrpName, MAX_PATH - 1);
				strXml = "<grp type=\"creategroup\" guid=\"";
				strXml += szGrpId;
				strXml += "\" name=\"";
				strXml += szDspGrpName;
				strXml += "\" creator=\"";
				strXml += m_strUserName; 
				strXml += "\">";
				CGroupItem *pGroup = new CGroupItem(szGrpId, NULL);
				LPORG_TREE_NODE_DATA pData = NULL;
				pGroup->SetDispName(szGrpName);
				pGroup->SetCreator(m_strUserName.c_str());
				//add self
				if (pGroup->AddGroupUser(m_strUserName.c_str(), m_strRealName.c_str()))
				{
					strXml += "<i uid=\"";
					strXml += m_strUserName;
					strXml += "\"/>";
				}
				while (SUCCEEDED(pUser->PopBackUserInfo(&pData)))
				{
					if (pGroup->AddGroupUser(pData->szUserName, pData->szDisplayName))
					{
						strXml += "<i uid=\"";
						strXml += pData->szUserName;
						strXml += "\"/>";
					}
				}
				strXml += "</grp>";
				m_FrameList.insert(std::pair<CAnsiString_, CGroupItem *>(szGrpId, pGroup));
				//draw to tree
				IMainFrame *pFrame = NULL;
				if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMainFrame), (void **)&pFrame)))
				{
					DrawGroupToTree(pFrame->GetSafeWnd(), pGroup);
					pFrame->Release();
				}
				//open frame
				if (OpenGroupFrame(szGrpId, pGroup->GetDispName()))
				{ 
					//
				} //end if (OpenGroupFrame(szGrpId...
			} else 
			{
				IMainFrame *pFrame = NULL;
				HWND h = NULL;
				if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMainFrame), (void **)&pFrame)))
				{
					h = pFrame->GetSafeWnd();
					pFrame->Release();
				}
				::SkinMessageBox(h, L"分组人数必须大于2并且小于300人", L"警告", MB_OK);
			}
		} else
		{
			//revisegroup
		}
		if ((!strXml.empty()) && m_pCore)
		{
			return m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG)strXml.size(), 0); 
		}
	}
	return E_FAIL;
}

void CGroupFrameImpl::DrawGroupToTree(HWND hWnd, CGroupItem *pItem)
{
	//
	TCHAR szName[MAX_PATH] = {0};
	int nNameSize = MAX_PATH - 1;
	void *pSelNode = NULL;
	CTreeNodeType tnType;
	void *pData = NULL;
	if (!::SkinGetNodeByKey(hWnd, L"grouptree", NULL, pItem->GetGroupId(), szName,&nNameSize,
		(void **)&pSelNode, &tnType, (void **)&pData))
	{
		LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
		memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
		strncpy(pData->szUserName, pItem->GetGroupId(), MAX_USER_NAME_SIZE - 1);
		TCHAR szText[MAX_PATH] = {0};
		CStringConversion::UTF8ToWideChar(pItem->GetDispName(), szText, MAX_PATH - 1);
		::SkinAddTreeChildNode(hWnd, L"grouptree", pData->id, NULL, szText, TREENODE_TYPE_GROUP, pData, NULL, NULL, NULL);
		::SkinUpdateControlUI(hWnd, L"grouptree");
	}
}

void CGroupFrameImpl::DeleteGrpFromMainWindow(const char *szGrpId)
{
	IMainFrame *pMain = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMainFrame), (void **)&pMain)))
	{
		LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
		memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
		strncpy(pData->szUserName, szGrpId, 63);
		if (!::SkinAdjustTreeNode(pMain->GetSafeWnd(), L"grouptree", NULL, NULL, TREENODE_TYPE_GROUP, pData, FALSE, FALSE))
		{
			PRINTDEBUGLOG(dtInfo, "delete group from main tree failed");
		}
		delete pData;
		pMain->Release();
	}
}

STDMETHODIMP CGroupFrameImpl::DrawGroupToUI(HWND hWnd, const char *szGrpId)
{
	std::map<CAnsiString_, CGroupItem *>::iterator it;
	if (szGrpId != NULL)
	{
		it = m_FrameList.find(szGrpId);
		if (it != m_FrameList.end())
		{
			DrawGroupToTree(hWnd, it->second);
		}
	} else
	{
		for (it = m_FrameList.begin(); it != m_FrameList.end(); it ++)
		{
			DrawGroupToTree(hWnd, it->second);
		}
	} 
	::SkinExpandTree(hWnd, L"grouptree", NULL, TRUE, FALSE);
	return S_OK;
}

STDMETHODIMP CGroupFrameImpl::ShowGroupFrame(const char *szGrpId, const char *szUTF8DspName)
{
	std::map<CAnsiString_, CGroupItem *>::iterator it = m_FrameList.find(szGrpId);
	BOOL bOpened = FALSE;
	if (it != m_FrameList.end())
	{
		if ((it->second->GetHWND() != NULL)
			&& (::IsWindow(it->second->GetHWND())))
		{
			CSystemUtils::BringToFront(it->second->GetHWND());
			return S_OK;
		}
	}  //end if (it != m_FrameList...
	if (OpenGroupFrame(szGrpId, szUTF8DspName))
		return S_OK;
	return E_FAIL;
}

void CGroupFrameImpl::ParserGroupMessage(TiXmlElement *pNode, std::string strGrpId, std::string &strFrom, std::string &strUserList, std::string &strTime,
		                   std::string &strDspName,  std::string &strBody, CCharFontStyle &fs)
{ 
	//<grp type="msg" guid="%s" from="%s" creator="%s"><font name="%s" size="%s" color="%s" bold="%s" underline="%s"' +
                  //   ' strikeout="%s" italic="%s"/><body>%s</body></grp>';
	if (pNode)
	{
		const char *szAttr = pNode->Attribute("from");

		if (szAttr)
			strFrom = szAttr;
		szAttr = pNode->Attribute("datetime");
		if (szAttr)
			strTime = szAttr;
		szAttr = pNode->Attribute("guid");
		if (szAttr)
			strGrpId = szAttr; 
		szAttr = pNode->Attribute("userlist");
		if (szAttr)
			strUserList = szAttr;
		TiXmlElement *pFont = pNode->FirstChildElement("font");
		if (pFont)
		{
			CXmlNodeTranslate::FontXmlNodeToStyle(pFont, fs);
		}
		TiXmlElement *pBody = pNode->FirstChildElement("body");
		std::string strText;
		if (pBody)
		{
			if (pBody->GetText())
			{
				strBody = pBody->GetText();
				strBody += "\n";
			} //end if (pBody->GetText())
		} // end if (pBody)

		//Get Display Name
		IContacts *pContacts = NULL;
		if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContacts)))
		{	 
			CInterfaceAnsiString sDspName;
			if (SUCCEEDED(pContacts->GetRealNameById(strFrom.c_str(), NULL, 
				(IAnsiString *)&sDspName)))
				strDspName = sDspName.GetData();
			pContacts->Release();
		}
	}  //end if (pNode)
}

BOOL CGroupFrameImpl::RecvMessageFromGroup(HWND hWnd, TiXmlElement *pNode, const char *szDspName)
{
	std::string strFrom, strTime, strGuid, strUserList;
	std::string strDspName, strBody;
	CCharFontStyle fs = {0};
	ParserGroupMessage(pNode, strGuid, strFrom, strUserList, strTime, strDspName, strBody, fs);
	if (!strBody.empty())
	{ 
		::SkinAddRichChatText(hWnd, L"messagedisplay", NULL, 0, strDspName.c_str(), strTime.c_str(),
			strBody.c_str(), &fs, UI_NICK_NAME_COLOR_PEER, TRUE, FALSE);
		CSystemUtils::FlashWindow(hWnd);
		return TRUE;
	} //end if (!strText.empty())
	return FALSE;
}

STDMETHODIMP CGroupFrameImpl::ShowGroupMessage(const char *szGrpId, const char *szMsg)
{
	TiXmlDocument xmldoc;
	if (xmldoc.Load(szMsg, ::strlen(szMsg)))
	{ 
		std::map<CAnsiString_, CGroupItem *>::iterator it = m_FrameList.find(szGrpId);
		if (it == m_FrameList.end())
		{
			ShowGroupFrame(szGrpId, NULL);
			it = m_FrameList.find(szGrpId);
		}
		if (it != m_FrameList.end())
		{
			if (it->second->GetHWND() != NULL)
			{
				if (::IsWindow(it->second->GetHWND()))
				{
					if (RecvMessageFromGroup(it->second->GetHWND(), xmldoc.FirstChildElement(), NULL))
						return S_OK;
				} //end if (::IsWindow(..
			} //end if (it->second->GetHWND()..
		}  //end if (it != m_FrameList... 
	} //end if (xmldoc...
	return E_FAIL;
}

STDMETHODIMP CGroupFrameImpl::ShowGroupTipMsg(HWND hOwner, TCHAR *szMsg)
{
	if (::SkinRichEditInsertTip(hOwner, L"messagedisplay", NULL, 0, szMsg))
		return S_OK;
	return E_FAIL;
}

	//
void CGroupFrameImpl::GetUserListDisplayNameById(IAnsiString *strDspNames, std::string &userList, std::string &strGuid)
{
	const char *p = userList.c_str();
	int nIdx = 0;
	std::map<CAnsiString_, CGroupItem *>::iterator it = m_FrameList.find(strGuid.c_str());
	CGroupItem *pItem = NULL;
	if (it != m_FrameList.end())
	{
		pItem = it ->second;
	}
	IContacts *pContact = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
	{
		while (TRUE)
		{
			char szTmp[32] = {0};
			if (!CSystemUtils::GetStringBySep(p, szTmp, ',', nIdx))
				break;
			 
			CInterfaceAnsiString szName;
			if (SUCCEEDED(pContact->GetRealNameById(szTmp, NULL, &szName)))
			{
				strDspNames->AppendString(szName.GetData());
				strDspNames->AppendString(",");
			}				
			nIdx ++;
		}
		pContact->Release();
    }
}

STDMETHODIMP CGroupFrameImpl::ParserGroupProtocol(const char *szContent, const int nContentSize, IAnsiString *strDispName, IAnsiString *strUserList,
                          IAnsiString *strDspTime, IAnsiString *strDspText, IFontStyle *fs,  IAnsiString *strMsgType,
						  BOOL *bSelf) 
{
	TiXmlDocument xmlDoc;
	if (xmlDoc.Load(szContent, nContentSize))
	{
		TiXmlElement *pNode = xmlDoc.FirstChildElement();
		if (pNode)
		{			
			strMsgType->SetString(pNode->Value()); 
			if (stricmp(pNode->Value(), "tip") == 0)
			{
				if (pNode->Attribute("datetime"))
					strDspTime->SetString(pNode->Attribute("datetime"));
				if (pNode->GetText())
					strDspText->SetString(pNode->GetText());
			} else
			{
				std::string sFrom, sTime, sGuid, sUserList;
			    std::string sDspName, sBody;
			    CCharFontStyle cf = {0};
			    ParserGroupMessage(pNode, sGuid, sFrom, sUserList, sTime, sDspName, sBody, cf);
				strDispName->SetString(sDspName.c_str());
				strDspTime->SetString(sTime.c_str());
				strDspText->SetString(sBody.c_str());
				if (strUserList != NULL)
				{
					GetUserListDisplayNameById(strUserList, sUserList, sGuid);
				}
				if (fs)
				{
					CXmlNodeTranslate::StyleToStringFont(cf, fs);
				}
				if (bSelf)
				{
					if (::stricmp(sFrom.c_str(), m_strUserName.c_str()) == 0)
						*bSelf = TRUE;
					else
						*bSelf = FALSE;
				}
				return S_OK;
			}
		} //end if (pNode)
	} //end if (xmlDoc.Load(...
	return E_FAIL;
}

//修改分组名称
STDMETHODIMP CGroupFrameImpl::UpdateGroupName(const char *szGrpId, const char *szNewGrpName)
{
	if (m_pCore && szGrpId && szNewGrpName)
	{
		std::map<CAnsiString_, CGroupItem *>::iterator it = m_FrameList.find(szGrpId);
		if (it != m_FrameList.end())
		{
			char szUTF8[MAX_PATH] = {0};
			CStringConversion::StringToUTF8(szNewGrpName, szUTF8, MAX_PATH - 1);
			it->second->SetDispName(szUTF8);
			////<grp type="updategroup" guid="" name=""/>
			std::string strXml = "<grp type=\"updategroup\" guid=\"";
			strXml += szGrpId;
			strXml += "\" name=\"";
			strXml += szNewGrpName;
			strXml += "\"/>"; 
			return m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG)strXml.size(), 0); 

		} //end if (it != .. 
	}
	return E_FAIL;
}


BOOL CGroupFrameImpl::IsGroupManager(const char *szGrp)
{
	InitSelfUserInfo(); 
	if (m_pCore && szGrp)
	{
		std::map<CAnsiString_, CGroupItem *>::iterator it = m_FrameList.find(szGrp);
		if (it != m_FrameList.end())
		{ 
			return (::stricmp(this->m_strUserName.c_str(), it->second->GetCreator()) == 0);
		} //end if (it != m_FrameList...
	} //end if (m_pCore && 
	return FALSE;
}

//退出分组
STDMETHODIMP CGroupFrameImpl::ExitGroupById(const char *szGrpId)
{
	InitSelfUserInfo();
	if (m_pCore && szGrpId)
	{
		std::map<CAnsiString_, CGroupItem *>::iterator it = m_FrameList.find(szGrpId);
		if (it != m_FrameList.end())
		{
			DeleteGrpFromMainWindow(szGrpId);
			if (it->second->GetHWND() && ::IsWindow(it->second->GetHWND()))
				::SkinCloseWindow(it->second->GetHWND());
			delete it->second;
			m_FrameList.erase(it);
		}
		//<grp type="exitgroup" guid=""/>  uid
		std::string strXml = "<grp type=\"exitgroup\" guid=\"";
		strXml += szGrpId;
		strXml += "\"/>";
		return m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG)strXml.size(), 0); 
	}
	return E_FAIL;
}

//删除分组
STDMETHODIMP CGroupFrameImpl::DeleteGroupById(const char *szGrpId)
{ 
	InitSelfUserInfo();
	if (m_pCore && szGrpId)
	{
		////<grp type="deletegroup" guid=""/>
		std::map<CAnsiString_, CGroupItem *>::iterator it = m_FrameList.find(szGrpId);
		if (it != m_FrameList.end())
		{
			if (stricmp(it->second->GetCreator(), m_strUserName.c_str()) == 0)
			{
				DeleteGrpFromMainWindow(szGrpId);
				if (it->second->GetHWND() && ::IsWindow(it->second->GetHWND()))
					::SkinCloseWindow(it->second->GetHWND());
				delete it->second;
				m_FrameList.erase(it);
			 
				std::string strXml = "<grp type=\"deletegroup\" guid=\"";
				strXml += szGrpId;
				strXml += "\"/>";
				return m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (LONG)strXml.size(), 0);
			}   //end else if (stricmp(it
		} //end if (it != ..
	}
	return E_FAIL;
}
 
BOOL CGroupFrameImpl::SendFileToGroup(HWND hWnd, const char *szFileName)
{
	CGroupItem *pFrame = GetGroupItemByHWND(hWnd);
	if (!pFrame)
		return FALSE;
	if (m_TransFileList.CheckIsTrans(pFrame->GetGroupId(), szFileName))
	{
		::SkinRichEditInsertTip(hWnd, L"messagedisplay", NULL, 0, L"文件正在传送中");
		return FALSE;
	}
	if (m_pCore)
	{
		if (m_pCore->CanAllowAction(USER_ROLE_SEND_FILE) != S_OK)
		{
			::SkinRichEditInsertTip(hWnd, L"messagedisplay", NULL, 0, L"没有发送文件权限");
		    return FALSE;;
		}
	}
	char szDspName[MAX_PATH] = {0};
	char szTag[MAX_PATH] = {0};
	char szFileId[32] = {0};
	int nTagSize = MAX_PATH - 1;
	::GetFileTagByName(szFileName, szTag, &nTagSize);
	char szFileSize[32] = {0};
	char szTmp[32] = {0};
	DWORD dwFileSize = (DWORD) CSystemUtils::GetFileSize(szFileName);
	::itoa(dwFileSize, szFileSize, 10);					
	CSystemUtils::ExtractFileName(szFileName, szDspName, MAX_PATH - 1);
	TCHAR szFlag[MAX_PATH] = {0};
	TCHAR szwTmp[MAX_PATH] = {0};
	int nFlagSize = MAX_PATH - 1;
	::SkinAddChildControl(hWnd, L"fileprogress", m_strFileTransSkinXml.c_str(), szFlag, &nFlagSize, 999999);
	int nFileId = m_TransFileList.AddFileInfo(pFrame->GetGroupId(), szDspName, szFileName, szFlag, 
		                            szTag, "", "0", "", "0", "0", 0, szFileSize, hWnd, TRUE);

	memset(szwTmp, 0, MAX_PATH * sizeof(TCHAR));
	CStringConversion::StringToWideChar(szDspName, szwTmp, MAX_PATH - 1);
	::SkinSetControlAttr(hWnd, szFlag, L"filename", szwTmp);
	memset(szwTmp, 0, MAX_PATH * sizeof(TCHAR));
	CStringConversion::StringToWideChar(szFileSize, szwTmp, MAX_PATH - 1);
	::SkinSetControlAttr(hWnd, szFlag, L"filesize", szwTmp);
	::SkinSetControlAttr(hWnd, szFlag, L"currfilesize", L"0");
	::SkinSetControlAttr(hWnd, szFlag, L"progrestyle", L"offline");
    ::SkinUpdateControlUI(hWnd, szFlag);
	IConfigure *pCfg = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		CInterfaceAnsiString strUrl;
		if (SUCCEEDED(pCfg->GetServerAddr(HTTP_SVR_URL_OFFLINE_FILE, &strUrl)))
		{

			CCustomPicItem *pItem = new CCustomPicItem();
			pItem->m_hOwner = hWnd;
			pItem->m_strFlag = szTag;
			pItem->m_pOverlapped = this;
			pItem->m_strLocalFileName = szFileName;  
			pItem->m_strPeerName = pFrame->GetGroupId(); 
			pItem->m_strUrl = strUrl.GetData();
			pItem->m_nFileSize = dwFileSize;
			pItem->m_nFileId = nFileId;
			std::string strParam = "filename=";
			strParam += szTag;
			strParam += ";username=";
			strParam += pFrame->GetGroupId();
			if (m_CustomPics.AddItem(pItem))
			{			
								//draw to ui
				CStdString_ strTip = _T("您给讨论组\"");
				TCHAR szTmp[MAX_PATH] = {0};
				CStringConversion::UTF8ToWideChar(pFrame->GetDispName(), szTmp, MAX_PATH - 1);	
				strTip += szTmp;
				strTip += _T("\" 发送离线文件 \"");
				memset(szTmp, 0, MAX_PATH * sizeof(TCHAR));
				CStringConversion::StringToWideChar(szDspName, szTmp, MAX_PATH - 1);
				strTip += szTmp; 						
				
				/*wsprintf(szTmp, L"\"\n \t <取消,%d>",
					            ((nFileId << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_CANCEL);
				strTip + szTmp;
				::SkinRichEditInsertTip(hWnd, L"messagedisplay", NULL, 0, strTip.GetData());*/

				::P2SvrPostFile(strUrl.GetData(), szFileName, strParam.c_str(), FILE_TYPE_NORMAL, 
					pItem, HttpUpCallBack, FALSE);
			} else
				delete pItem;
		}
		pCfg->Release();
		return TRUE;
	}
	return FALSE;
}

void CGroupFrameImpl::CancelCustomLink(HWND hWnd, DWORD dwFileId, DWORD dwFlag)
{
	DWORD dwLinkFlag = 0;
	if ((dwFlag & CUSTOM_LINK_FLAG_RECV) != 0)
	{
		dwLinkFlag = ((dwFileId  << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_RECV;
		::SkinCancelCustomLink(hWnd, L"messagedisplay", dwLinkFlag);
	} 
	if ((dwFlag & CUSTOM_LINK_FLAG_SAVEAS) != 0)
	{
		dwLinkFlag = ((dwFileId  << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_SAVEAS;
		::SkinCancelCustomLink(hWnd, L"messagedisplay", dwLinkFlag);
	} 
	if ((dwFlag & CUSTOM_LINK_FLAG_CANCEL) != 0)
	{
		dwLinkFlag = ((dwFileId  << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_CANCEL;
		::SkinCancelCustomLink(hWnd, L"messagedisplay", dwLinkFlag);
	} 
	if ((dwFlag & CUSTOM_LINK_FLAG_REFUSE) != 0)
	{
		dwLinkFlag = ((dwFileId  << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_REFUSE;
		::SkinCancelCustomLink(hWnd, L"messagedisplay", dwLinkFlag);
	} 
	if ((dwFlag & CUSTOM_LINK_FLAG_OFFLINE) != 0)
	{
		dwLinkFlag = ((dwFileId  << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_OFFLINE;
		::SkinCancelCustomLink(hWnd, L"messagedisplay", dwLinkFlag);
	}	
	if ((dwFlag & CUSTOM_LINK_FLAG_RMC_ACCEPT) != 0)
	{
		dwLinkFlag = ((dwFileId  << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_RMC_ACCEPT;
		::SkinCancelCustomLink(hWnd, L"messagedisplay", dwLinkFlag);
	}
	if ((dwFlag & CUSTOM_LINK_FLAG_RMC_REFUSE) != 0)
	{
		dwLinkFlag = ((dwFileId  << CUSTOM_LINK_FLAG_OFFSET) & 0xFFFF0000) | CUSTOM_LINK_FLAG_RMC_REFUSE;
		::SkinCancelCustomLink(hWnd, L"messagedisplay", dwLinkFlag);
	}
}
#pragma warning(default:4996)
