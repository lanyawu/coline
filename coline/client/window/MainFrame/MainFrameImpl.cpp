#include <time.h>
#include <Commonlib/DebugLog.h>
#include <Commonlib/systemutils.h>
#include <Commonlib/stringutils.h>
#include <SmartSkin/smartskin.h>
#include <xml/tinyxml.h>
#include <Core/common.h>
#include <Crypto/crypto.h>
#include "../P2Svr/P2Svr.h"
#include "../IMCommonLib/InterfaceAnsiString.h"
#include "../IMCommonLib/InterfaceUserList.h"
#include "../IMCommonLib/InstantUserInfo.h"

#include "MainFrameImpl.h"
#include <Core/treecallbackfun.h>
#include <ShellAPI.h>

const TCHAR UI_COMPANY_TREE_NAME[] = L"colleaguetree";
#pragma warning(disable:4996)

#define SEARCH_SEND_MSG  0
#define SEARCH_SEND_FILE 2
#define SEARCH_RMC       3
#define SEARCH_SEND_SMS  4
#define SEARCH_SEND_MAIL 5
#define SEARCH_SEDN_FAX  6

static BOOL m_bSortByOnline  = FALSE; //是否按在线方式排序
int CALLBACK CutImage(HWND hParent, BOOL bHideParent);

int CALLBACK RecentlyCompareNode(CTreeNodeType tnType1, int nStatus1, void  *pData1, 
	                                  CTreeNodeType tnType2, int nStatus2, void *pData2)
{
	if (pData1 && pData2)
	{
		LPORG_TREE_NODE_DATA pOrg1 = (LPORG_TREE_NODE_DATA)pData1;
		LPORG_TREE_NODE_DATA pOrg2 = (LPORG_TREE_NODE_DATA)pData2;
		if (pOrg2->nStamp > pOrg1->nStamp)
			return 1;
		else if (pOrg2->nStamp < pOrg1->nStamp)
			return -1;
		else
		{
			return 0;
		} 
	}
	return 0;
}

int CALLBACK CompareNode(CTreeNodeType tnType1, int nStatus1, void  *pData1, 
	                                  CTreeNodeType tnType2, int nStatus2, void *pData2)
{
	if (tnType2 > tnType1)
		return 1;
	else if (tnType2 < tnType1)
		return -1;
	else 
	{
		if (m_bSortByOnline)
		{
			if (nStatus2 > nStatus1)
				return 1;
			else if (nStatus2 < nStatus1)
				return -1;
		}
		if (pData1 && pData2)
		{
			LPORG_TREE_NODE_DATA pOrg1 = (LPORG_TREE_NODE_DATA)pData1;
			LPORG_TREE_NODE_DATA pOrg2 = (LPORG_TREE_NODE_DATA)pData2;
			if (tnType1 == TREENODE_TYPE_GROUP)
			{
				if (pOrg2->nDisplaySeq > pOrg1->nDisplaySeq)
					return -1;
				else if (pOrg2->nDisplaySeq < pOrg1->nDisplaySeq)
					return 1;
				else
					return 0;
			} else
			{
				if (pOrg2->id > pOrg1->id)
					return -1;
				else if (pOrg2->id < pOrg1->id)
					return 1;
				else
				{
					return 0;
				} //end else if (pOrg2->nDisplaySeq...
			} //end else if (tnType1 == TREENODE_TYPE_GROUP)
		} //end else if (nStatus2 <...
	} //end else if (tnType2 <...	
	return 0;
}

typedef struct 
{
	CStdString_ strId;
	std::string strFileName;
	CMainFrameImpl *pImpl;
}BANNER_HTTP_DL_ITEM, *LPBANNER_HTTP_DL_ITEM;


void CALLBACK BannerDlCallback(int nErrorCode, int nType,
		              WPARAM wParam, LPARAM lParam, void *pOverlapped)
{
	if (pOverlapped)
	{
		if (nErrorCode == ERROR_CODE_COMPLETE)
		{
			LPBANNER_HTTP_DL_ITEM pItem = (LPBANNER_HTTP_DL_ITEM)pOverlapped;
			if (pItem)
			{
				if (nType == FILE_TYPE_NORMAL)
				{ 
					if (pItem->pImpl && (lParam == 0))
					{  
						pItem->pImpl->m_strBannerLocalFile = pItem->strFileName;
						::PostMessage(pItem->pImpl->GetSafeWnd(), WM_BANNER_DL_COMPL, 0, 0);
					} //end if (pItem->
				} //
				delete pItem;
			} //
		} //end if (nErrorCode == ERROR_CODE_COMPLETE)
	} //end if (pOverlapped)
}


CMainFrameImpl::CMainFrameImpl(void):
                m_pCore(NULL),
				m_hWnd(NULL),
				m_nTryTimes(0),
				m_bAutoChanged(FALSE),
				m_ptrAutoChgTimer(NULL),
				m_ptrTimer(NULL), 
				m_dwAtomRecvHotkeyId(0),
				m_nSearchStatus(SEARCH_SEND_MSG),
				m_dwRecvHotkeyShiftState(0),
				m_dwRecvHotkeyKeyStatue(0),
				m_dwAtomCutScrHotkeyId(0),
				m_dwCutScrHotkeyShiftState(0),
				m_dwCutScrHotkeyKeyState(0) 
{
}


CMainFrameImpl::~CMainFrameImpl(void)
{
	DeleteHotKey();
	if (m_hWnd)
	{		
		if (m_ptrAutoChgTimer)
			::KillTimer(m_hWnd, m_ptrAutoChgTimer);
	    if (m_ptrTimer != NULL)
			::KillTimer(m_hWnd, m_ptrTimer);
		::SkinCloseWindow(m_hWnd);
		m_hWnd = NULL;
	}
	if (m_pCore)
	{
		m_pCore->Release();
	    m_pCore = NULL;
	} 
}

//IUnknown
STDMETHODIMP CMainFrameImpl::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IMainFrame)))
	{
		*ppv = (IMainFrame *) this;
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

HRESULT CMainFrameImpl::DoDblClickEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (m_hWnd == hWnd)
	{
		if (::stricmp(szName, "recentlytree") == 0)
		{
			DoOpenChatFrameByRecent();
		}  //end if (::stricmp(szName...
	} //end if (m_hWnd...
	return -1;
}


//广播消息
STDMETHODIMP CMainFrameImpl::DoBroadcastMessage(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData)
{
	if ((::stricmp(szFromWndName, "contacts") == 0) && (::stricmp(szType, "downorg") == 0)
		&& (::stricmp(pContent, "true") == 0))
	{
		//组织结构成功下载
		::PostMessage(m_hWnd, WM_DOWNORGCOMPLETE, 0, 0);
		//
		RefreshBanner();
	} else if ((::stricmp(szFromWndName, "LogonWindow") == 0) && (::stricmp(szType, "login") == 0)
		&& (::stricmp(pContent, "failed") == 0) && (m_nTryTimes > 0))
	{
		if (m_ptrTimer != NULL)
			::KillTimer(m_hWnd, m_ptrTimer);
		m_ptrTimer = ::SetTimer(m_hWnd, rand(), TRY_LOGON_TIMER_INTERVAL, NULL);
		m_nTryTimes ++;
		PRINTDEBUGLOG(dtInfo, "login impl connect failed");
	} else if (::stricmp(szFromWndName, "CoreFrame") == 0)
	{
		if ((::stricmp(szType, "connectsvr") == 0)
		   && (::stricmp(pContent, "failed") == 0) && (m_hWnd != NULL))
		{
			if (m_ptrTimer != NULL)
				::KillTimer(m_hWnd, m_ptrTimer);
			if (m_nTryTimes > 0)
			{
				m_ptrTimer = ::SetTimer(m_hWnd, rand(), TRY_LOGON_TIMER_INTERVAL, NULL);
				m_nTryTimes ++;
			}
			PRINTDEBUGLOG(dtInfo, "coreframe impl connect failed");
		} else if (::stricmp(szType, "getsrvar") == 0)
		{
			if (::stricmp(pContent, "bannerimage") == 0)
			{
				m_strBannerPicUrl = (char *)pData;
				GetBannerImage();
			} else if (::stricmp(pContent, "bannerurl") == 0)
			{
				m_strBannerClickUrl = (char *)pData;
			}
		}
	} else if ((::stricmp(szFromWndName, "ConfigureUI") == 0) && (::stricmp(szType, "uploadheader") == 0)
		&& (::stricmp(pContent, "succ") == 0) && (m_hWnd != NULL)) //上传头像成功
	{
		IContacts *pContact = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
		{
			CInterfaceAnsiString strFileName;
			if (SUCCEEDED(pContact->GetContactHead(m_strUserName.c_str(), &strFileName, FALSE)))
			{
				TCHAR szTmp[MAX_PATH] = {0};
				CStringConversion::StringToWideChar(strFileName.GetData(), szTmp, MAX_PATH - 1);
				::SkinSetControlAttr(m_hWnd, L"mainselfhead", L"floatimagefilename", szTmp);
				::SkinUpdateTreeNodeImageFile(m_hWnd, L"colleaguetree", m_strUserName.c_str(), strFileName.GetData(), TRUE);
			}
			pContact->Release();
		}
	} else if ((::stricmp(szFromWndName, "mainwindow") == 0) && (::stricmp(szType, "dlheader") == 0))
	{
		//用户头像下载完成
		char *pFileName = (char *)pData;
		::SkinUpdateTreeNodeImageFile(m_hWnd, L"colleaguetree", pContent, pFileName, TRUE);
		::SkinUpdateTreeNodeImageFile(m_hWnd, L"recentlytree", pContent, pFileName, FALSE);
		//
		if (::stricmp(pContent, m_strUserName.c_str()) == 0)
		{
			TCHAR szTmp[MAX_PATH] = {0};
			CStringConversion::StringToWideChar(pFileName, szTmp, MAX_PATH - 1);
			::SkinSetControlAttr(m_hWnd, L"mainselfhead", L"floatimagefilename", szTmp);
		}
	}
	return E_NOTIMPL;
}

//ICoreEvent
STDMETHODIMP CMainFrameImpl::DoCoreEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
	                     LPARAM lParam, HRESULT *hResult)
{
	if (::stricmp(szType, "click") == 0)
	{
		if (::stricmp(szName, "colleaguetree") == 0)
		{
			//
			void *pSelNode = NULL;
			LPORG_TREE_NODE_DATA pSelData = NULL;
			TCHAR szName[MAX_PATH] = {0};
			int nSize = MAX_PATH - 1;
			CTreeNodeType tnType;
			if (::SkinGetSelectTreeNode(hWnd, L"colleaguetree", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
			{
				if (tnType == TREENODE_TYPE_GROUP)
				{
					if (pSelData && pSelData->bOpened == 0)
					{
						ExpandNodeByPid(pSelNode, pSelData->id);
						pSelData->bOpened = TRUE;
					} // end if (pSelData ...
					//进行状态订制
					DoStatusOrder(hWnd, pSelNode);
				} else
				{
					//Show user info
				} //end else if (tnType == ...
			} //end if (::GetSelectTreeNode(hWnd...
		} else if (::stricmp(szName, "minbutton") == 0)
		{
			if (m_hWnd == hWnd)
			{
				::ShowWindow(m_hWnd, SW_HIDE);
				*hResult = 0;
			}
		} else if (::stricmp(szName, "closebutton") == 0)
		{
			if (hWnd == m_hWnd)
			{
				IConfigure *pCfg = NULL;
				BOOL bClosed = FALSE;
				if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
				{
					CInterfaceAnsiString strValue;
					if (SUCCEEDED(pCfg->GetParamValue(FALSE, "normal", "closewinexit", &strValue)))
					{
						if (::stricmp(strValue.GetData(), "true") == 0)
							bClosed = TRUE;
					}
					pCfg->Release();
				}
				if (!bClosed)
				{
					::ShowWindow(m_hWnd, SW_HIDE);
					*hResult = 0;
				}
			}
		} else if (::stricmp(szName, "searchbutton") == 0)
		{
			TCHAR szTmp[128] = {0};
			int nSize = 127;
			if (::SkinGetControlTextByName(m_hWnd, L"searchedit", szTmp, &nSize))
			{
				char szRealName[128] = {0};
				CStringConversion::WideCharToString(szTmp, szRealName, 127); 
				IContacts *pContact = NULL;
				if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
				{
					pContact->EditHelpSearchActive(hWnd, szRealName, TRUE, FALSE);
					pContact->Release();
				}
			} 
		} else if (::stricmp(szName, "btnClearRecently") == 0)
		{
			IConfigure *pCfg = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
			{
				if (SUCCEEDED(pCfg->DelUserFromRecently(NULL)))
				{
					::SkinTreeViewClear(m_hWnd, L"recentlytree"); //清除最近联系人列表
					::SkinUpdateControlUI(m_hWnd, L"recentlytree");//
				}
				pCfg->Release();
			} //end if (SUCCEEDED(
		} else if (::stricmp(szName, "sendfax") == 0)
		{
			//发送传真
			ISMSFrame *pFrame = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(ISMSFrame), (void **)&pFrame)))
			{
				pFrame->ShowFaxFrame(NULL, NULL);
				pFrame->Release();
			}
		} else if (::stricmp(szName, "sendsms") == 0)
		{
			//发送短信
			ISMSFrame *pFrame = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(ISMSFrame), (void **)&pFrame)))
			{
				pFrame->ShowSMSFrame(NULL, NULL);
				pFrame->Release();
			}
		} else if (::stricmp(szName, "sendmail") == 0)
		{
			//发送邮件 
			BOOL bDoing = FALSE;
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
					bDoing = TRUE;
				}
				pCfg->Release();
			} //end if (SUCCEEDED(
			if (!bDoing)
				::ShellExecute(NULL, L"open", L"mailto:", NULL, NULL, SW_SHOW);
		} else if (::stricmp(szName, "broadcast") == 0)
		{
			//广播消息
		} else if (::stricmp(szName, "msgmgr") == 0)
		{
			//消息管理
			IMsgMgrUI *pMsg = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMsgMgrUI), (void **)&pMsg)))
			{
				pMsg->ShowMsgMgrFrame(NULL, NULL, NULL);
				pMsg->Release();
			}
		} else if (::stricmp(szName, "bannerview") == 0)
		{
			if ((m_hWnd == hWnd) && (!m_strBannerClickUrl.empty()))
				CSystemUtils::OpenURL(m_strBannerClickUrl.c_str());
				
		}
		//end if (::stricmp(szName, .... 
	} else if (::stricmp(szType, "treegroupclick") == 0)
	{
		if (::stricmp(szName, "colleaguetree") == 0)
		{
			void *pSelNode = NULL;
			LPORG_TREE_NODE_DATA pSelData = NULL;
			TCHAR szName[MAX_PATH] = {0};
			int nSize = MAX_PATH - 1;
			CTreeNodeType tnType;
			if (::SkinGetSelectTreeNode(hWnd, L"colleaguetree", szName, &nSize, &pSelNode, &tnType, (void **)&pSelData))
			{
				if (tnType == TREENODE_TYPE_GROUP)
				{
					if (pSelData && pSelData->bOpened == 0)
					{
						ExpandNodeByPid(pSelNode, pSelData->id);
						pSelData->bOpened = TRUE;
					} // end if (pSelData ...
					//进行状态订制
					DoStatusOrder(hWnd, pSelNode);
				} else
				{
					//Show user info
				} //end else if (tnType == ...
			} //end if (::GetSelectTreeNode(hWnd...
		}
	} else if (::stricmp(szType, "killfocus") == 0)
	{
		if (::stricmp(szName, "editinfo") == 0)
		{
			DoUpdateSign();
		} else if (::stricmp(szName, "searchedit") == 0)
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
		if (::stricmp(szName, "searchedit") == 0)
		{
			TCHAR szwText[64] = {0};
			int nSize = 63;
			if (::SkinGetControlTextByName(hWnd, L"searchedit", szwText, &nSize))
			{
				char szText[64] = {0};
				CStringConversion::WideCharToString(szwText, szText, 63);
				char szDest[64] = {0};
				CStringConversion::Trim(szText, szDest);
				RECT rc = {0};
				::SkinGetControlRect(hWnd, L"searchedit", &rc);
				POINT pt = {rc.left, rc.bottom};
				::ClientToScreen(hWnd, &pt);
				IContacts *pContact = NULL;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
				{
					pContact->ShowHelpEditWindow((ICoreEvent *) this, szDest, pt.x, pt.y, rc.right - rc.left, 100);
					pContact->Release();
				}
			} //end if (::
		} //end 
	}  else if (::stricmp(szType, "keydown") == 0)
	{
		if (::stricmp(szName, "searchedit") == 0)
		{
			IContacts *pContact = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
			{
				switch(wParam)
				{
					case VK_RETURN: 
					{
						TCHAR szTmp[128] = {0};
						int nSize = 127;				
						char szRealName[128] = {0};
						if (::SkinGetControlTextByName(m_hWnd, L"searchedit", szTmp, &nSize))
						{
							CStringConversion::WideCharToString(szTmp, szRealName, 127);
						} 
						pContact->EditHelpSearchActive(hWnd, szRealName, TRUE, FALSE);
						break;
				    } 
					case VK_ESCAPE:
					{
						pContact->EditHelpSearchActive(hWnd, NULL, FALSE, FALSE);
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
				} //end switch(..
				pContact->Release();
			} 
		} else if (stricmp(szName, "editinfo") == 0)
		{
			if (wParam == VK_RETURN)
			{
				::SkinSetControlFocus(m_hWnd, L"colleaguetree", TRUE);
			}
		} //end if (::stricmp(szName...
	} else if (::stricmp(szType, "itemactivate") == 0)
	{
		if (::stricmp(szName, "resultlist") == 0)
		{
			::SkinSetControlTextByName(m_hWnd, L"searchedit", (TCHAR *)wParam);
			IChatFrame *pChat = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pChat)))
			{
				switch(m_nSearchStatus)
				{
				case  SEARCH_SEND_FILE:
					{
						pChat->SendFileToPeer((char *)lParam, NULL);
						break;
					}
				case SEARCH_RMC:
					{
						pChat->SendRmcRequest((char *)lParam);
						break;
					}
				case SEARCH_SEND_SMS:
					{
						ISMSFrame *pFrame = NULL;
						if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(ISMSFrame), (void **)&pFrame)))
						{
							LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
							memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
							strcpy(pData->szUserName, (char *)lParam);
							CInterfaceUserList ulList;
							ulList.AddUserInfo(pData, FALSE, TRUE);
							pFrame->ShowSMSFrame(NULL, &ulList);
							pFrame->Release();
						} //end if (SUCCEEDED(m_pCore->
						break;
					}
				case SEARCH_SEND_MAIL:
					{
						IContacts *pContact = NULL;
						if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
						{
							CInterfaceAnsiString strMail;
							if (SUCCEEDED(pContact->GetMailByUserName((char *)lParam, &strMail)))
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
							} //end if (SUCCEEDED(
							pContact->Release();
						} //end if (SUCCEEDED( 
						break;
					}
				case SEARCH_SEDN_FAX:
					{
						ISMSFrame *pFrame = NULL;
						if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(ISMSFrame), (void **)&pFrame)))
						{
							LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
							memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
							strcpy(pData->szUserName, (char *)lParam);
							CInterfaceUserList ulList;
							ulList.AddUserInfo(pData, FALSE, TRUE);
							pFrame->ShowFaxFrame(NULL, &ulList);
							pFrame->Release();
						} //end if (SUCCEEDED(m_pCore->
						break;
					}
				default: 
					pChat->ShowChatFrame(NULL, (char *)lParam, NULL);
					break;
				}
				pChat->Release();
			}
		}
	} else if (::stricmp(szType, "afterinit") == 0)
	{
		if (::stricmp(szName, UI_MAIN_WINDOW_NAME) == 0)
		{
			::SkinSetGetKeyFun(hWnd, UI_COMPANY_TREE_NAME, GetTreeNodeKey);
			::SkinSetFreeNodeDataCallBack(hWnd, UI_COMPANY_TREE_NAME, FreeTreeNodeData);
			::SkinSetGetKeyFun(hWnd, L"recentlytree", GetTreeNodeKey);
			::SkinSetFreeNodeDataCallBack(hWnd, L"recentlytree", FreeTreeNodeData);
			::SkinSetControlVisible(hWnd, L"title", FALSE);
			IUIManager *pUI = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
			{
				pUI->OrderWindowMessage(UI_MAIN_WINDOW_NAME, NULL, WM_SHOWTRAYTIPINFO, (ICoreEvent *) this);
				pUI->OrderWindowMessage(UI_MAIN_WINDOW_NAME, NULL, WM_DRAWGROUPTOUI, (ICoreEvent *) this);
				pUI->OrderWindowMessage(UI_MAIN_WINDOW_NAME, NULL, WM_SHOWGROUPMSG, (ICoreEvent *) this);
				pUI->OrderWindowMessage(UI_MAIN_WINDOW_NAME, NULL, WM_OPENGROUPFRAME, (ICoreEvent *) this);
				pUI->OrderWindowMessage(UI_MAIN_WINDOW_NAME, NULL, WM_ERRORMSG, (ICoreEvent *) this);
				pUI->OrderWindowMessage(UI_MAIN_WINDOW_NAME, NULL, WM_KILLFOCUS, (ICoreEvent *) this);
				pUI->OrderWindowMessage(UI_MAIN_WINDOW_NAME, NULL, WM_SHOWGROUPTIPMSG, (ICoreEvent *) this);
				pUI->OrderWindowMessage(UI_MAIN_WINDOW_NAME, NULL, WM_DOWNORGCOMPLETE, (ICoreEvent *) this);
				pUI->OrderWindowMessage(UI_MAIN_WINDOW_NAME, NULL, WM_TIMER, (ICoreEvent *) this);
				pUI->OrderWindowMessage(UI_MAIN_WINDOW_NAME, NULL, WM_POWER, (ICoreEvent *) this);
				pUI->OrderWindowMessage(UI_MAIN_WINDOW_NAME, NULL, WM_HOTKEY, (ICoreEvent *) this);
				pUI->OrderWindowMessage(UI_MAIN_WINDOW_NAME, NULL, WM_USER_DL_HEADER, (ICoreEvent *) this);
				pUI->OrderWindowMessage(UI_MAIN_WINDOW_NAME, NULL, WM_APP_TERMINATE, (ICoreEvent *) this); 
				pUI->OrderWindowMessage(UI_MAIN_WINDOW_NAME, NULL, WM_BANNER_DL_COMPL, (ICoreEvent *) this);
				pUI->OrderWindowMessage(UI_MAIN_WINDOW_NAME, NULL, WM_PRESENCECHANGE, (ICoreEvent *) this);
				pUI->OrderWindowMessage(UI_MAIN_WINDOW_NAME, NULL, WM_PRESENCECHG_ORD, (ICoreEvent *) this);
				pUI->OrderWindowMessage(UI_MAIN_WINDOW_NAME, NULL, WM_SIGNCHANGE, (ICoreEvent *) this);
				pUI->Release();
			} //end if (SUCCEEDED(m_pCore->..
			return S_OK;
		} //end if (::stricmp(szName...
	} else if (::stricmp(szType, "menucommand") == 0)
	{
		if (::stricmp(szName, "SystemMenu") == 0)
		{
			DoSysMenuCommand(wParam);
		} else if (::stricmp(szName, "mainmenu") == 0)
		{
			DoMainMenuCommand(wParam);
		} else if (::stricmp(szName, "listmenu") == 0)
		{
			DoListMenuCommand(wParam, lParam);
		} else if (::stricmp(szName, "treegroup") == 0)
		{
			DoTreeGroupMenuCommand(wParam, lParam);
		} else if (::stricmp(szName, "recentlymenu") == 0)
		{
			DoRecentlyMenuCommand(hWnd, wParam, lParam);
		} else if (::stricmp(szName, "searchmenu") == 0)
		{
			DoSearchMenuCommand(hWnd, wParam, lParam);
		}
	} else if (::stricmp(szType, "lbdblclick") == 0)
	{
		*hResult = DoDblClickEvent(hWnd, szName, wParam, lParam);
	} //end if (::stricmp(szType...
	return E_NOTIMPL;
}
 

void CMainFrameImpl::DoSearchMenuCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{
	case 4: //发送文件
		m_nSearchStatus = SEARCH_SEND_FILE;
		break;
	case 6: //远程协助
		m_nSearchStatus = SEARCH_RMC;
		break;
	case 18: //发送短信
		m_nSearchStatus = SEARCH_SEND_SMS;
		break;
	case 20: //发送消息
		m_nSearchStatus = SEARCH_SEND_MSG;
		break;
	case 21: //发送邮件
		m_nSearchStatus = SEARCH_SEND_MAIL;
		break;
	case 22: //发送传真
		m_nSearchStatus = SEARCH_SEDN_FAX;
		break;
	}
}


BOOL CMainFrameImpl::DoSysProtocol(TiXmlElement *pNode, const char *pType)
{
	/*if (::stricmp(pType, "presence") == 0)
	{
		//<sys type="presence" uid="user@doamin"  presence="online" memo="在线"/> 
		if (::stricmp(m_strUserName.c_str(), pNode->Attribute("uid")) == 0)
		{
			TCHAR szTmp[128] = {0};
			if (pNode->Attribute("memo"))
				CStringConversion::StringToWideChar(pNode->Attribute("memo"), szTmp, 127);
			UpdateStatusToUI(pNode->Attribute("presence"), szTmp);
			return TRUE;
		}
	}*/
	return FALSE;
}

STDMETHODIMP CMainFrameImpl::DoPresenceChange(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder)
{
	if (::stricmp(m_strUserName.c_str(),szUserName) == 0)
	{
		TCHAR szTmp[128] = {0};
		if (szMemo)
			CStringConversion::StringToWideChar(szMemo, szTmp, 127);
		UpdateStatusToUI(szNewPresence, szTmp); 
	}
	return UpdateUserPresence(szUserName, szNewPresence, m_bSortByOnline);
}

STDMETHODIMP CMainFrameImpl::DoRecvProtocol(const BYTE *pData, const LONG lSize)
{
	TiXmlDocument xmldoc;
	BOOL bDid = FALSE;
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
					bDid = DoSysProtocol(pNode, pType);
				} //end if (::stricmp(pValue..
			} //end if (pValue && pType)
		} //end if (pNode)
	} //end if (xmldoc.Load(...
	if (bDid)
		return S_OK;
	return S_FALSE;
}

STDMETHODIMP CMainFrameImpl::SetCoreFrameWork(ICoreFrameWork *pCore)
{
	if (m_pCore)
	{
		m_pCore->Release();
		m_pCore = NULL;
	}
	m_pCore = pCore;
	if (m_pCore)
	{
		m_pCore->AddRef();
		//order event
		m_pCore->AddOrderEvent((ICoreEvent *) this, "MainWindow", "minbutton", "click");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "MainWindow", "closebutton", "click");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "MainWindow", "colleaguetree", "click");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "MainWindow", "bannerview", "click");
		
		m_pCore->AddOrderEvent((ICoreEvent *) this, UI_MAIN_WINDOW_NAME, "sendfax", "click");
		m_pCore->AddOrderEvent((ICoreEvent *) this, UI_MAIN_WINDOW_NAME, "sendsms", "click");
		m_pCore->AddOrderEvent((ICoreEvent *) this, UI_MAIN_WINDOW_NAME, "sendmail", "click");
		m_pCore->AddOrderEvent((ICoreEvent *) this, UI_MAIN_WINDOW_NAME, "broadcast", "click");		
		m_pCore->AddOrderEvent((ICoreEvent *) this, UI_MAIN_WINDOW_NAME, "msgmgr", "click");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "MainWindow", "editinfo", "killfocus");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "editinfo", "keydown");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "MainWindow", "recentlytree", "lbdblclick");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "searchedit", "editchanged");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "searchedit", "killfocus");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "searchedit", "keydown"); 		
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "searchbutton", "click");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "colleaguetree", "treegroupclick");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "mainwindow", "btnClearRecently", "click");
		//order
		m_pCore->AddOrderEvent((ICoreEvent *) this, UI_MAIN_WINDOW_NAME, UI_MAIN_WINDOW_NAME, "afterinit");
		m_pCore->AddOrderEvent((ICoreEvent *) this, UI_MAIN_WINDOW_NAME, "SystemMenu", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, UI_MAIN_WINDOW_NAME, "mainmenu", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, UI_MAIN_WINDOW_NAME, "listmenu", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, UI_MAIN_WINDOW_NAME, "treegroup", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, UI_MAIN_WINDOW_NAME, "showheadermenu", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, UI_MAIN_WINDOW_NAME, "recentlymenu", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, UI_MAIN_WINDOW_NAME, "searchmenu", "menucommand");
		//
		m_pCore->AddOrderProtocol((IProtocolParser *) this, "sys", "presence");
	}
	return S_OK;
}

void CMainFrameImpl::DoRecentlyMenuCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{
	case 20: //发送即时消息
		 DoOpenChatFrameByRecent();
		 break;
	case 4: //发送文件
		{
			std::string strUserName;
			BOOL bGroup = TRUE;
			if (GetUserNameByRecent(strUserName, bGroup))
			{
				if (bGroup)
				{
					IGroupFrame *pFrame = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IGroupFrame), (void **)&pFrame)))
					{
						pFrame->SendFileToGroup(strUserName.c_str(), NULL);
						pFrame->Release();
					}
				} else
				{
					IChatFrame *pFrame = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pFrame)))
					{
						pFrame->SendFileToPeer(strUserName.c_str(), NULL);
						pFrame->Release();
					} //end if (SUCCEEDED(
				}
			} //end if (GetUserNameByRecent(
		}  //
		break;
	case 5: //详细资料
		{
			std::string strUserName;
			BOOL bGroup;
			if (GetUserNameByRecent(strUserName, bGroup))
			{
				if (!bGroup)
				{
					IConfigureUI *pFrame = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigureUI), (void **)&pFrame)))
					{
						pFrame->ViewUserInfo(strUserName.c_str());
						pFrame->Release();
					} //end if (SUCCEEDED(
				} // end if (!bGroup)
			} //end if (GetUserNameByRecent(
		}  //
		break;
	case 6: ////远程协助
		{
			std::string strUserName;
			BOOL bGroup;
			if (GetUserNameByRecent(strUserName, bGroup))
			{
				if (!bGroup)
				{
					IChatFrame *pFrame = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pFrame)))
					{
						pFrame->SendRmcRequest(strUserName.c_str());
						pFrame->Release();
					} //end if (SUCCEEDED(
				} //end if (!bGroup)
			} //end if (GetUserNameByRecent(
		}  //
		break;
	case 7: //视频会话
		{
			std::string strUserName;
			BOOL bGroup;
			if (GetUserNameByRecent(strUserName, bGroup))
			{
				if (!bGroup)
				{
					IChatFrame *pFrame = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pFrame)))
					{
						pFrame->SendVideoRequest(strUserName.c_str());
						pFrame->Release();
					} //end if (SUCCEEDED(
				} //end if (!bGroup)
			} //end if (GetUserNameByRecent(
		}  //
		break;
	case 8: //音频会话
		{
			std::string strUserName;
			BOOL bGroup;
			if (GetUserNameByRecent(strUserName, bGroup))
			{
				if (!bGroup)
				{
					IChatFrame *pFrame = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pFrame)))
					{
						pFrame->SendAudioRequest(strUserName.c_str());
						pFrame->Release();
					} //end if (SUCCEEDED(
				} //end if (!bGroup)
			} //end if (GetUserNameByRecent(
		}  //
		break;
	case 17: //查看消息记录
		{
			std::string strUserName;
			BOOL bGroup;
			if (GetUserNameByRecent(strUserName, bGroup))
			{
				IMsgMgrUI *pUI = NULL;
				if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IMsgMgrUI), (void **)&pUI)))
				{
					if (bGroup)
						pUI->ShowMsgMgrFrame("grp", strUserName.c_str(), NULL);
					else
						pUI->ShowMsgMgrFrame("p2p", strUserName.c_str(), NULL);
					pUI->Release();
				} //end if (SUCCEEDED(
			} //end if (GetUserNameByRecent
		} 
		break;
	case 18: //发送短信
		{
			std::string strUserName;
			BOOL bGroup;
			if (GetUserNameByRecent(strUserName, bGroup))
			{
				if (bGroup)
				{
					IGroupFrame *pFrame = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IGroupFrame), (void **)&pFrame)))
					{
						pFrame->SendSMSToGroup(strUserName.c_str(), NULL);
						pFrame->Release();
					}
				} else
				{
					ISMSFrame *pFrame = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(ISMSFrame), (void **)&pFrame)))
					{
						LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
						memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
						strcpy(pData->szUserName, strUserName.c_str());
						CInterfaceUserList ulList;
						ulList.AddUserInfo(pData, FALSE, TRUE);
						pFrame->ShowSMSFrame(NULL, &ulList);
						pFrame->Release();
					} //end if (SUCCEEDED(m_pCore->
				}
			} //end if (GetUserNameByRecent(
			break;
		} //end case 18
	case 19: //从最近联系人列表移除
		{
			IConfigure *pCfg = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
			{
				std::string strUserName;
				BOOL bGroup;
				if (GetUserNameByRecent(strUserName, bGroup))
				{
					if (SUCCEEDED(pCfg->DelUserFromRecently(strUserName.c_str())))
					{
						LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
						memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
						TCHAR szName[64] = {0};
						strncpy(pData->szUserName, strUserName.c_str(), MAX_USER_NAME_SIZE - 1);
						::SkinAdjustTreeNode(m_hWnd, L"recentlytree", NULL, szName, TREENODE_TYPE_LEAF, pData, FALSE, FALSE);
						::SkinUpdateControlUI(m_hWnd, L"recentlytree");
						delete pData;
					}
				}
				pCfg->Release();
			} //end if (SUCCEEDED(m_pCore->QueryInterface(__..
			break;
		}
	case 21: //发送邮件
		{
			std::string strUserName;
			BOOL bGroup;
			if (GetUserNameByRecent(strUserName, bGroup))
			{
				if (bGroup)
				{
					IGroupFrame *pFrame = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IGroupFrame), (void **)&pFrame)))
					{
						pFrame->SendMailToGroup(strUserName.c_str());
						pFrame->Release();
					} //end if (SUCCEEDED(m_pCore->
				} else //end if (bGroup)
				{
					IContacts *pContact = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
					{
						CInterfaceAnsiString strMail;
						if (SUCCEEDED(pContact->GetMailByUserName(strUserName.c_str(), &strMail)))
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
						} //end if (SUCCEEDED(
						pContact->Release();
					} //end if (SUCCEEDED(
				} //end else if (bGroup)
			} //end if (GetUser
			break;
		} //end case 21
	case 22: //发送传真
		{
			std::string strUserName;
			BOOL bGroup;
			if (GetUserNameByRecent(strUserName, bGroup))
			{
				if (bGroup)
				{
					IGroupFrame *pFrame = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IGroupFrame), (void **)&pFrame)))
					{
						//pFrame->SendSMSToGroup(strUserName.c_str(), NULL);
						pFrame->Release();
					}
				} else
				{
					ISMSFrame *pFrame = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(ISMSFrame), (void **)&pFrame)))
					{
						LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
						memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
						strcpy(pData->szUserName, strUserName.c_str());
						CInterfaceUserList ulList;
						ulList.AddUserInfo(pData, FALSE, TRUE);
						pFrame->ShowFaxFrame(NULL, &ulList);
						pFrame->Release();
					} //end if (SUCCEEDED(m_pCore->
				}
			} //end if (GetUserNameByRecent(
			break;
		}
	} //end switch(...
}

BOOL CMainFrameImpl::GetUserNameByRecent(std::string &strUserName, BOOL &bGroup)
{
	void *pSelNode = NULL;
	LPORG_TREE_NODE_DATA pData = NULL;
	TCHAR szName[MAX_PATH] = {0};
	int nSize = MAX_PATH - 1;
	CTreeNodeType tnType;
	bGroup = TRUE;
	if (::SkinGetSelectTreeNode(m_hWnd, L"recentlytree", szName, &nSize, &pSelNode, &tnType, (void **)&pData))
	{
		if (tnType == TREENODE_TYPE_LEAF)
		{
			if (pData)
			{ 
				strUserName = pData->szUserName;
				if (strUserName.find('@') != std::string::npos) //P2P聊天
				{
					bGroup = FALSE; 
				}  
				return TRUE;
			} //end if (pData)
		} //end if (tnType == 
	} 
	return FALSE;
}

void CMainFrameImpl::DoOpenChatFrameByRecent()
{		
	void *pSelNode = NULL;
	LPORG_TREE_NODE_DATA pData = NULL;
	TCHAR szName[MAX_PATH] = {0};
	int nSize = MAX_PATH - 1;
	CTreeNodeType tnType;
	if (::SkinGetSelectTreeNode(m_hWnd, L"recentlytree", szName, &nSize, &pSelNode, &tnType, (void **)&pData))
	{
		if (tnType == TREENODE_TYPE_LEAF)
		{
			if (pData)
			{ 
				std::string strId = pData->szUserName;
				if (strId.find('@') != std::string::npos) //P2P聊天
				{
					IChatFrame *pChat = NULL;
					if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pChat)))
					{
						pChat->ShowChatFrame(NULL, pData->szUserName, pData->szDisplayName);
						pChat->Release();
					}
				} else //分组
				{
					IGroupFrame *pFrame = NULL;
					if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IGroupFrame), (void **)&pFrame)))
					{
						pFrame->ShowGroupFrame(pData->szUserName, NULL);
						pFrame->Release();
					} //end if (m_pCore
				} //end if (strId.find(...
				//
			} //end if (pSelData)
		} //end if (tnType == TREENODE_TYPE_LEAF)
	} //end if (::GetSelectTreeNode(hWnd..		
}

void CMainFrameImpl::DoTreeGroupMenuCommand(WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{
		case 3: 
		{
			CTreeNodeType tnType;
			void *pSelNode = NULL;
			LPORG_TREE_NODE_DATA pData = NULL;
			TCHAR szwNodeName[MAX_PATH] = {0};	
			int nNameSize = MAX_PATH - 1;
			if (::SkinGetSelectTreeNode(m_hWnd, L"colleaguetree", szwNodeName, &nNameSize, &pSelNode, &tnType, (void **)&pData))
			{
				if (tnType == TREENODE_TYPE_GROUP)
				{
					if (pData && pData->bOpened == 0)
					{
						ExpandNodeByPid(pSelNode, pData->id);
						pData->bOpened = TRUE;
					} // end if (pSelData ...
					::SkinExpandTree(m_hWnd, L"colleaguetree", pSelNode, TRUE, FALSE);
					//进行状态订制
					DoStatusOrder(m_hWnd, pSelNode);
				} //end if (tnType == 
			} //end if (::SkinGetSelectTreeNode(
			break;
		} 
		case 30://
		{
			IContacts *pContact = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
			{
				POINT pt = {0};
				::GetCursorPos(&pt);
				RECT rc = {0};
				::GetWindowRect(m_hWnd, &rc);
				pContact->ShowSearchFrame(pt.x, pt.y, rc.right - rc.left, 42);
				pContact->Release();
			}
			break;
		} 
		case 68:
			{
				::SkinSetTreeIconType(m_hWnd, UI_COMPANY_TREE_NAME, 1);
				::SkinSetTreeIconType(m_hWnd, L"recentlytree", 1);
				IConfigure *pCfg = NULL;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
				{
					pCfg->SetParamValue(FALSE, "normal", "treeicon", "1");
					pCfg->Release();
				}
				::SkinUpdateControlUI(m_hWnd, L"colleaguetree");
				::SkinUpdateControlUI(m_hWnd, L"recentlytree");
				break;
			}
		case 69:
			{
				::SkinSetTreeIconType(m_hWnd, UI_COMPANY_TREE_NAME, 2);
				::SkinSetTreeIconType(m_hWnd, L"recentlytree", 2);
				IConfigure *pCfg = NULL;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
				{
					pCfg->SetParamValue(FALSE, "normal", "treeicon", "2");
					pCfg->Release();
				}
				::SkinUpdateControlUI(m_hWnd, L"colleaguetree");
				::SkinUpdateControlUI(m_hWnd, L"recentlytree");
				break;
			}
		case 11:
			{
				IConfigure *pCfg = NULL;
				if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
				{
					TCHAR szValue[16] = {0};
					if (BOOL(lParam) == TRUE)
					{ 
					    pCfg->SetParamValue(FALSE, "normal", "showcustompicture", "true");
						::lstrcpy(szValue, L"true");
					} else
					{ 
					    pCfg->SetParamValue(FALSE, "normal", "showcustompicture", "false");
						::lstrcpy(szValue, L"false");
					}
					::SkinSetControlAttr(m_hWnd, L"colleaguetree", L"showcustompic", szValue);
					::SkinUpdateControlUI(m_hWnd, L"colleaguetree");
					::SkinSetControlAttr(m_hWnd, L"recentlytree", L"showcustompic", szValue);
					::SkinUpdateControlUI(m_hWnd, L"recentlytree");
				
					pCfg->Release();
				}
				break;
			}
		case 100: //是否显示个性签名
			{
				TCHAR szValue[16] = {0};
				if (BOOL(lParam) == TRUE)
				{  
					::lstrcpy(szValue, L"true");
				} else
				{  
					::lstrcpy(szValue, L"false");
				}
				::SkinSetControlAttr(m_hWnd, L"colleaguetree", L"showpersonlabel", szValue);
				::SkinUpdateControlUI(m_hWnd, L"colleaguetree");
				break;
			}
		case 101: //显示分机号
		{
			//
			TCHAR szValue[16] = {0};
			if (BOOL(lParam) == TRUE)
				::lstrcpy(szValue, L"true");
			else
				::lstrcpy(szValue, L"false");
			::SkinSetControlAttr(m_hWnd, L"colleaguetree", L"showextradata", szValue);
			::SkinUpdateControlUI(m_hWnd, L"colleaguetree");
			break;
		}
	}
}

void CMainFrameImpl::DoListMenuCommand(WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{
	case 10:
		{
			//
			TCHAR szValue[16] = {0};
			if (BOOL(lParam) == TRUE)
				::lstrcpy(szValue, L"true");
			else
				::lstrcpy(szValue, L"false");
			::SkinSetControlAttr(m_hWnd, L"colleaguetree", L"showextradata", szValue);
			::SkinUpdateControlUI(m_hWnd, L"colleaguetree");
			break;
		}
	case 11:
		{
			IConfigure *pCfg = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
			{
				TCHAR szValue[16] = {0};
				if (BOOL(lParam) == TRUE)
				{ 
				    pCfg->SetParamValue(FALSE, "normal", "showcustompicture", "true");
					::lstrcpy(szValue, L"true");
				} else
				{ 
				    pCfg->SetParamValue(FALSE, "normal", "showcustompicture", "false");
					::lstrcpy(szValue, L"false");
				}
				::SkinSetControlAttr(m_hWnd, L"colleaguetree", L"showcustompic", szValue);
				::SkinUpdateControlUI(m_hWnd, L"colleaguetree");
				::SkinSetControlAttr(m_hWnd, L"recentlytree", L"showcustompic", szValue);
				::SkinUpdateControlUI(m_hWnd, L"recentlytree");
			
				pCfg->Release();
			}
			break;
		} //end case 11
	case 12:
		{
			IConfigure *pCfg = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
			{
				TCHAR szValue[16] = {0};
				if (BOOL(lParam) == TRUE)
				{ 
				    pCfg->SetParamValue(FALSE, "normal", "sortbyonline", "true"); 
					m_bSortByOnline = TRUE;
				} else
				{ 
				    pCfg->SetParamValue(FALSE, "normal", "sortbyonline", "false");  
					m_bSortByOnline = FALSE;
				}  
				::SkinSortTreeNode(m_hWnd, UI_COMPANY_TREE_NAME, NULL, CompareNode, TRUE, TRUE);
				::SkinUpdateControlUI(m_hWnd, UI_COMPANY_TREE_NAME); 
				pCfg->Release();
			}
			break;
		}
	} //end switch(..
}

//
BOOL CMainFrameImpl::ExpandNodeByPid(void *pParentNode, const int nPid)
{
	IContacts *pContacts = NULL;
	BOOL bSucc = FALSE;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContacts)))
	{
		pContacts->ExpandTreeNodeToUI(m_hWnd, UI_COMPANY_TREE_NAME, pParentNode, nPid); 
		pContacts->Release();
	}
	if (m_bSortByOnline)
		::SkinSortTreeNode(m_hWnd, UI_COMPANY_TREE_NAME, pParentNode, CompareNode, FALSE, TRUE);
	return bSucc;
}

STDMETHODIMP CMainFrameImpl::UpdateUserLabel(const char *szUserName,  const char *szUTF8Label)
{
	if (!m_strUserName.empty() && (stricmp(m_strUserName.c_str(), szUserName) == 0))
	{
		//
		TCHAR szwSign[MAX_PATH] = {0};
		CStringConversion::UTF8ToWideChar(szUTF8Label, szwSign, MAX_PATH - 1);
		::SkinSetControlTextByName(m_hWnd, _T("editinfo"), szwSign);
	}
	::SkinUpdateUserLabelToNode(m_hWnd, L"recentlytree", szUserName, szUTF8Label, FALSE);
	if (::SkinUpdateUserLabelToNode(m_hWnd, UI_COMPANY_TREE_NAME, szUserName, szUTF8Label, 
						         TRUE))
		return S_OK;
	return E_FAIL;
}

BOOL CMainFrameImpl::DoStatusOrder(HWND hWnd, void *pNode)
{
	BOOL bSucc = FALSE;
	BOOL bExpanded = FALSE;
	if (::SkinGetNodeIsExpanded(hWnd, pNode, &bExpanded))
	{
		//
		char szUserList[2048] = {0};
		int nSize = 2047;
		if (::SkinGetNodeChildUserList(hWnd, pNode, szUserList, &nSize, 
			                      FALSE))
		{
			IContacts *pContacts = NULL;
			if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContacts)))
			{
				TiXmlDocument xml;
				if (xml.Load(szUserList, nSize))
				{
					TiXmlElement *pNode = xml.FirstChildElement();
					if (bExpanded)
					{
						//先获取状态
						TiXmlElement *pChild = pNode;
						CInstantUserInfo UserInfo;
						char szStatus[64] = {0};
						int nStatusSize = 0; 
						while (pChild)
						{
							const char *szUserName = pChild->Attribute("u");
							if (szUserName)
							{ 
								UserInfo.Clear();
								if (SUCCEEDED(pContacts->GetContactUserInfo(szUserName, (IInstantUserInfo *)&UserInfo)))
								{
									CInterfaceAnsiString strStatus;
									if (SUCCEEDED(UserInfo.GetUserStatus((IAnsiString *)&strStatus)))
									{
										UpdateUserPresence(szUserName, strStatus.GetData(), FALSE);			 
									} //end if (SUCCEEDED(...
									if (SUCCEEDED(UserInfo.GetUserInfo("sign", &strStatus)))
									{ 
										UpdateUserLabel(szUserName, strStatus.GetData());
									}
								} //end if (it != m_UserInfos....
							} //end if (szUserName)
							pChild = pChild->NextSiblingElement();
						} //end while (pChild)
					
						bSucc = pContacts->AddOrderUserList(szUserList);
					} else
						bSucc = pContacts->DeleteOrderUserList(szUserList);
				} //end if (xml.load...
				pContacts->Release();
			} // end if (m_pCore...
			//
		} //end if (::GetNodeChildUserList(hWnd...
	} //end if (::GetNodeIsExpanded(...
	return bSucc;
}

//
STDMETHODIMP CMainFrameImpl::UpdateUserPresence(const char *szUserName, const char *szPresence, BOOL bSort)
{
	char *szTmpUserName = new char[strlen(szUserName) + 1];
	strcpy(szTmpUserName, szUserName);
	szTmpUserName[strlen(szUserName)] = '\0';
	char *szTmpPresence = new char[strlen(szPresence) + 1];
	strcpy(szTmpPresence, szPresence);
	szTmpPresence[strlen(szPresence)] = '\0';
	if (bSort)
		::PostMessage(m_hWnd, WM_PRESENCECHG_ORD, (WPARAM) szTmpUserName, (LPARAM) szTmpPresence);
	else
		::PostMessage(m_hWnd, WM_PRESENCECHANGE, (WPARAM)szTmpUserName, (LPARAM)szTmpPresence);

	return E_FAIL;
}

void CMainFrameImpl::DoUpdateSign()
{
	TCHAR szwSign[MAX_PATH] = {0};
	int nSize = MAX_PATH - 1;
	::SkinGetControlTextByName(m_hWnd, _T("editinfo"), szwSign, &nSize);
	char szUTF8[MAX_PATH - 1] = {0};
	CStringConversion::WideCharToUTF8(szwSign, szUTF8, MAX_PATH - 1);
	CInterfaceAnsiString strOld;
	BOOL bUpSvr = FALSE;
	IContacts *pContacts = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContacts)))
	{
		if (SUCCEEDED(pContacts->GetContactUserValue(m_strUserName.c_str(), "sign", (IAnsiString *)&strOld)))
		{
			if (::stricmp(szUTF8, strOld.GetData()) != 0)
			{
				bUpSvr = TRUE;
			}
		} else
		{
			bUpSvr = TRUE;
		}
		if (bUpSvr)
		{
			::SkinUpdateUserLabelToNode(m_hWnd, UI_COMPANY_TREE_NAME, m_strUserName.c_str(), szUTF8, 
						         TRUE);
			//<sys type="sign" uid="user@doamin" sign="sign"/>
			pContacts->SetContactUserInfo(m_strUserName.c_str(), "sign", szUTF8);
			std::string strXml;
			strXml = "<sys type=\"sign\" uid=\"";
			strXml += m_strUserName;
			strXml += "\" sign=\"";
			char strSign[MAX_PATH] = {0};
			CStringConversion::UTF8ToString(szUTF8, strSign, MAX_PATH - 1);
			TiXmlString strXmlSign;
			TiXmlBase::EncodeString(strSign, strXmlSign);
			strXml += strXmlSign.c_str();
			strXml += "\"/>";
			if (m_pCore)
				m_pCore->SendRawMessage((BYTE *)strXml.c_str(), (int) strXml.size(), 0);
		}
		pContacts->Release();
	}
}

STDMETHODIMP CMainFrameImpl::GetSkinXmlString(IAnsiString *szXmlString)
{
	HRESULT hr = E_FAIL;
	IConfigure *pCfg = NULL; 
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{ 
		hr = pCfg->GetSkinXml("mainframe.xml",szXmlString); 
		pCfg->Release();
	}
	return hr;  
}

STDMETHODIMP CMainFrameImpl::CoreFrameWorkError(int nErrorNo, const char *szErrorMsg)
{
	switch(nErrorNo)
	{
	case CORE_ERROR_SOCKET_CLOSED: // 网络被断开
		{
			m_nTryTimes = 1;
			if (m_ptrTimer != NULL)
				::KillTimer(m_hWnd, m_ptrTimer);
			m_ptrTimer = ::SetTimer(m_hWnd, rand(), TRY_LOGON_TIMER_INTERVAL, NULL);
			PRINTDEBUGLOG(dtInfo, "start auto login");
		}
	case CORE_ERROR_KICKOUT: //被踢下线
		{
			char *pTmp = NULL;
			if (szErrorMsg)
			{
				pTmp = new char[strlen(szErrorMsg) + 1];
				strcpy(pTmp, szErrorMsg);
				pTmp[strlen(szErrorMsg)] = '\0';
			}
			::PostMessage(m_hWnd, WM_ERRORMSG, WPARAM(nErrorNo), LPARAM(pTmp));
			return S_OK;
		}
	case CORE_ERROR_LOGOUT: //注销
		 {
			 DoOffline();
			 ::SkinTreeViewClear(m_hWnd, L"recentlytree"); //清除最近联系人列表
			 ::SkinTreeViewClear(m_hWnd, L"colleaguetree"); //清除公司列表
		 }
		 break;
	} 
	return E_FAIL;
}

//
STDMETHODIMP CMainFrameImpl::DoWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	                                     LRESULT *lRes)
{
	switch(uMsg)
	{
		case WM_DESTROY:
		{
			RECT rc = {0};
			if (::GetWindowRect(hWnd, &rc))
			{
				std::string strRect;
				if (CSystemUtils::RectToString(rc, strRect))
				{
					if (m_pCore)
					{
						IConfigure *pCfg = NULL;
						if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
						{
							pCfg->SetParamValue(FALSE, "Position", "MainFrame", strRect.c_str());
							pCfg->Release();
						} //end if (SUCCEEDED(...
					} //end if (m_pCore)
				} //end if (CSystemUtils::RectToString(...
			} //end if (::GetWindowRect(hWnd...
			break;
		} 

		case WM_SHOWTRAYTIPINFO:
		{
			ITrayMsg *pTray = NULL;
			if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(ITrayMsg), (void **)&pTray)))
			{
				LPTRAY_ICON_TIP_INFO pInfo = (LPTRAY_ICON_TIP_INFO)wParam;
				pTray->ShowTipInfo(pInfo->szImageFile, pInfo->szTip, pInfo->szCaption, pInfo->szUrl, FALSE);
				//free
				if (pInfo->szImageFile)
					delete []pInfo->szImageFile;
				if (pInfo->szCaption)
					delete []pInfo->szCaption;
				if (pInfo->szTip)
					delete []pInfo->szTip;
				if (pInfo->szUrl)
					delete []pInfo->szUrl;
				delete pInfo;
				pTray->Release();
			}
			break;
		} 
		case WM_DRAWGROUPTOUI:
		{
			IGroupFrame *pFrame = NULL;
			if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IGroupFrame), (void **)&pFrame)))
			{
				pFrame->DrawGroupToUI(m_hWnd, (char *)wParam);
				pFrame->Release();
			}
			break;
		}
		case WM_OPENGROUPFRAME:
		{
			IGroupFrame *pFrame = NULL;
			if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IGroupFrame), (void **)&pFrame)))
			{
				pFrame->ShowGroupFrame((char *)wParam, (char *)lParam);
				pFrame->Release();
			} //end if (m_pCore ...
			break;
		}
		case WM_SHOWGROUPMSG:
		{
			IGroupFrame *pFrame = NULL;
			if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IGroupFrame), (void **)&pFrame)))
			{
				pFrame->ShowGroupMessage((char *)wParam, (char *)lParam);
				pFrame->Release();
			}
			break;
		}
		case WM_SHOWGROUPTIPMSG:
		{
			IGroupFrame *pFrame = NULL;
			if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IGroupFrame), (void **)&pFrame)))
			{
				pFrame->ShowGroupTipMsg((HWND)wParam, (TCHAR *)lParam);
				pFrame->Release();
			}
			break;
		}
		case WM_ERRORMSG: //错误消息处理
		{
			//		
			char *p = (char *)lParam;
			switch(wParam)
			{
			case CORE_ERROR_SOCKET_CLOSED:
				 DoOffline();
				 break;
			case CORE_ERROR_KICKOUT:
				 {
					 DoOffline();
					 char *pTmp = NULL;
					 if (p)
					 {
						 pTmp = new char[strlen(p) + 1]; 
						 memset(pTmp, 0, strlen(p) + 1);
						 strcpy(pTmp, p); 
					 } else
					 {
						 static char KICK_OUT_TIP_MESSAGE[] = "被迫下线";
						 pTmp = new char[::strlen(KICK_OUT_TIP_MESSAGE) + 1];
						 strcpy(pTmp, KICK_OUT_TIP_MESSAGE);
						 pTmp[::strlen(KICK_OUT_TIP_MESSAGE)] = '\0';
					 }
					 ITrayMsg *pTray = NULL;
					 if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(ITrayMsg), (void **)&pTray)))
					 {
						 pTray->ShowTipPanel("提示", pTmp);
						 pTray->Release();
					 } 
					 delete []pTmp;
				 }
				 break;
			}
			if (p)
				delete []p;
			break;
		}
		case WM_DOWNORGCOMPLETE:
		{
			ShowContacts(); 
			m_pCore->BroadcastMessage("mainwindow", m_hWnd, "showcontacts", "complete", NULL);	
			IConfigure *pCfg = NULL;
			BOOL bMiniTray = FALSE;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
			{
				CInterfaceAnsiString strValue;
				if (SUCCEEDED(pCfg->GetParamValue(FALSE, "normal", "mini2tray", &strValue)))
				{
					if (::stricmp(strValue.GetData(), "true") == 0)
						bMiniTray = TRUE;
				}
				pCfg->Release();
			}
			RegisterGoComHotkey(); 
			if (bMiniTray)
				::ShowWindow(m_hWnd, SW_HIDE);
			else
				CSystemUtils::BringToFront(m_hWnd);
			
			break;
		}
		case WM_TIMER:
		{
			if (wParam == m_ptrTimer)
			{
				::KillTimer(m_hWnd, m_ptrTimer);
				m_ptrTimer = NULL;
				//check connection live
				if (m_pCore)
				{
					m_pCore->ChangePresence(NULL, NULL); 
					PRINTDEBUGLOG(dtInfo, "Core Frame ChangePresence, times:%d", m_nTryTimes);
				}
			} else if (wParam == m_ptrAutoChgTimer)
			{
	 			IConfigure *pCfg = NULL;
				DWORD dwAutoChgInterval = 0;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
				{
					CInterfaceAnsiString strValue;
					if (SUCCEEDED(pCfg->GetParamValue(FALSE, "person", "autochangestatus", &strValue)))
					{
						if (::stricmp(strValue.GetData(), "true") == 0)
						{
							if (SUCCEEDED(pCfg->GetParamValue(FALSE, "person", "timetoaway", &strValue)))
								dwAutoChgInterval = ::atoi(strValue.GetData()) * 60 * 1000; //从分转换为毫秒
						} 
					}
					pCfg->Release();
				}
				if (dwAutoChgInterval > 0)
				{
					DWORD dwLast = CSystemUtils::GetUserLastActiveTime();
					if (GetTickCount() > (dwAutoChgInterval + dwLast))
					{
						if (!m_bAutoChanged)
						{
							CInterfaceAnsiString strValue, strMemo;
							if (SUCCEEDED(m_pCore->GetPresence(NULL, &strValue, &strMemo)))
							{
								if (::stricmp(strValue.GetData(), "online") == 0)
								{
									m_bAutoChanged = TRUE;
									m_pCore->ChangePresence("away", "离开");
								}
							}// end if (SUCCEEDED(m_pCore->GetPresence
						} //end if (!m_bAutoChanged)
					} else if (m_bAutoChanged)
					{
						m_pCore->ChangePresence("online", "在线");
						m_bAutoChanged = FALSE;
					}
				} //endif (dwAutoChgInterval
			}//end (wParam
			break;
		}
		case WM_POWERBROADCAST:
		{
			OnWMPowerBroadcast(wParam, lParam);
			break;
		}
		case WM_HOTKEY:
		{
			WORD H = lParam >> 16;
			WORD L = lParam & 0xFFFF;
			if ((L == m_dwRecvHotkeyShiftState) && (H == m_dwRecvHotkeyKeyStatue))
			{
				m_pCore->PickPendingMessage();
				//pick msg
			} else if ((L == m_dwCutScrHotkeyShiftState) && (H == m_dwCutScrHotkeyKeyState))
			{
				//cut screen
				HWND h = ::GetForegroundWindow();
				if (CutImage(m_hWnd, FALSE) == IDOK)
				{
					IChatFrame *pFrame = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IChatFrame), (void **)&pFrame)))
					{
						CInterfaceAnsiString strUserName;
						if (SUCCEEDED(pFrame->GetUserNameByHWND(h, &strUserName)))
						{
							::SkinRichEditCommand(h, L"messageedit", "paste", NULL);
						}
						pFrame->Release();
					} //end if (SUCCEEDED(
				} //end if (CutImage(...
			} //end else if 
			break;
		}
		case WM_USER_DL_HEADER: //用户头像下载完成
		{
			IContacts *pContact = NULL;
			char *pUserName = (char *)lParam;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
			{
				CInterfaceAnsiString strFileName;
				if (SUCCEEDED(pContact->GetContactHead(pUserName, &strFileName, FALSE)))
				{
					m_pCore->BroadcastMessage("mainwindow", m_hWnd, "dlHeader", pUserName, (void *)strFileName.GetData());
				}
				pContact->Release();
			}
			if (pUserName)
				delete []pUserName;
			break;
		}
		case WM_APP_TERMINATE: //中止应用程序
			{
				HWND h = m_hWnd;
				m_hWnd = NULL;
				::SkinCloseWindow(h);
				break;
			}
		case WM_PRESENCE_CHANGE:
			{
				char *szUserName = (char *)wParam;
				char *szNewPresence = (char *)lParam;
				UpdateUserPresence(szUserName, szNewPresence, m_bSortByOnline);
				break;
			}
		case WM_KILLFOCUS:
			{
				//
				break;
			} //end case WM_KILLFOCUS;
		case WM_BANNER_DL_COMPL:
			{
				if (!m_strBannerLocalFile.empty())
				{
					TCHAR szwTmp[MAX_PATH] = {0};
					CStringConversion::StringToWideChar(m_strBannerLocalFile.c_str(), szwTmp, MAX_PATH - 1);
					::SkinSetControlAttr(m_hWnd, L"bannerview", L"image", szwTmp);
				}
				break;
			};
		case WM_PRESENCECHANGE:
			{
				char *szUserName = (char *)wParam;
				char *szPresence = (char *)lParam;
				::SkinUpdateUserStatusToNode(m_hWnd, L"recentlytree", szUserName, szPresence, FALSE);
				::SkinUpdateUserStatusToNode(m_hWnd, UI_COMPANY_TREE_NAME, szUserName, 
										 szPresence, TRUE);
				delete []szUserName;
				delete []szPresence;
				break;
			}
		case WM_PRESENCECHG_ORD:
			{
				char *szUserName = (char *)wParam;
				char *szPresence = (char *)lParam;
				::SkinUpdateUserStatusToNode(m_hWnd, L"recentlytree", szUserName, szPresence, FALSE);
				void *pTmp = ::SkinUpdateUserStatusToNode(m_hWnd, UI_COMPANY_TREE_NAME, szUserName, 
										 szPresence, TRUE);
				 
     			::SkinSortTreeNode(m_hWnd, UI_COMPANY_TREE_NAME, pTmp, CompareNode, FALSE, TRUE);  
				delete []szUserName;
				delete []szPresence;
				break;
			}
		case WM_SIGNCHANGE:
			{
				char *szUserName = (char *)wParam;
				char *szSign = (char *)lParam;
				UpdateUserLabel(szUserName, szSign);
				delete []szUserName;
				delete []szSign;
				break;
			}
	}
	//end else if (uMsg == WM_DESTROY)
	return E_NOTIMPL;
}


void CMainFrameImpl::OnWMPowerBroadcast(WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{
	case PBT_APMSUSPEND:
		{
			
			break;
		}
	case PBT_APMRESUMESUSPEND:
		{
			if (m_pCore)
				m_pCore->ChangePresence(NULL, NULL);
			break;
		}
	}  
}

//IMainFrame
STDMETHODIMP CMainFrameImpl::BringToFront()
{
	if (m_hWnd && ::IsWindow(m_hWnd))
	{
		RefreshBanner();
		CSystemUtils::BringToFront(m_hWnd);
		return S_OK;
	}
	return E_FAIL;
}

STDMETHODIMP CMainFrameImpl::InitMainFrame()
{
	if (m_hWnd)
		::SendMessage(m_hWnd, WM_CLOSE, 0, 0);
	m_hWnd = NULL;
	HRESULT hr = E_FAIL;
	if (m_pCore)
	{
		IUIManager *pUI = NULL;
		IConfigure *pCfg = NULL;		
		hr = m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg);
		if (SUCCEEDED(hr) && pCfg)
		{
			CInterfaceAnsiString strShowMainFrame;
			BOOL bShowMain = FALSE;
			if (SUCCEEDED(pCfg->GetParamValue(FALSE, "normal", "showmainframe", (IAnsiString *)&strShowMainFrame)))
			{
				if (::stricmp(strShowMainFrame.GetData(), "true") == 0)
					bShowMain = TRUE;
			} 
			RECT rc = {0};
			CInterfaceAnsiString strPos;
			if (SUCCEEDED(pCfg->GetParamValue(FALSE, "Position", "MainFrame", (IAnsiString *)&strPos)))
			{
				CSystemUtils::StringToRect(&rc, strPos.GetData());
			}
			if (::IsRectEmpty(&rc))
			{ 
				CSystemUtils::GetScreenRect(&rc);
				rc.right -= 20;
				rc.left = rc.right - 330;
				rc.top = 100;
				rc.bottom = rc.top + 630;
			} 
			hr = m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI);
			if (SUCCEEDED(hr) && pUI)
			{
				pUI->CreateUIWindow(NULL, "MainWindow", &rc, WS_POPUP | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX,
				                WS_EX_ACCEPTFILES , L"", &m_hWnd);				
				if (::IsWindow(m_hWnd))
				{ 
					m_ptrAutoChgTimer = ::SetTimer(m_hWnd, rand(), CHECK_ACTIVE_TIME_INTERVAL, NULL);
					hr = S_OK;	
					CSystemUtils::Show2ToolBar(m_hWnd, FALSE);
				}
				pUI->OrderWindowMessage("MainWindow", NULL, WM_DESTROY, (ICoreEvent *) this);
				pUI->Release();
				pUI = NULL;
			}
			pCfg->Release();
			pCfg = NULL;
		} //end if (SUCCEEDED(hr)...
	}
	return hr;
}

void CMainFrameImpl::RefreshBanner()
{
	if (m_strBannerLocalFile.empty())
	{
		CInterfaceAnsiString strTmp;
		m_pCore->GetSvrParams("bannerimage", &strTmp, TRUE);
		m_pCore->GetSvrParams("bannerurl", &strTmp, TRUE);
		m_pCore->GetSvrParams("mailurl", &strTmp, TRUE);
	}
}

STDMETHODIMP CMainFrameImpl::ShowMainFrame()
{
	if (m_hWnd && ::IsWindow(m_hWnd))
	{
		RefreshBanner();
		if (::ShowWindow(m_hWnd, SW_SHOW))
			return S_OK;
		else
			return E_FAIL;
	} else
	{
		InitMainFrame();
		//draw user info to UI
		RefreshBanner();
		CInterfaceAnsiString strName;
		if (SUCCEEDED(m_pCore->GetUserNickName((IAnsiString *)&strName)))
		{
			TCHAR szwName[MAX_PATH] = {0};
			CStringConversion::UTF8ToWideChar(strName.GetData(), szwName, MAX_PATH - 1);
			::SkinSetControlTextByName(m_hWnd, L"userinfo", szwName);
		} 
		return S_OK;
	}
}

void CMainFrameImpl::DoOffline()
{
	//界面
	UpdateStatusToUI("offline", L"离线");
	//联系人树
	::SkinSetTreeViewStatusOffline(m_hWnd, L"colleaguetree");
	::SkinSetTreeViewStatusOffline(m_hWnd, L"recentlytree");
	::SkinUpdateControlUI(m_hWnd, L"colleaguetree");
	::SkinUpdateControlUI(m_hWnd, L"recentlytree");
}

void CMainFrameImpl::UpdateStatusToUI(const char *szPresence, const TCHAR *szMemo)
{
	CInterfaceAnsiString strDspName;
	if (m_pCore && SUCCEEDED(m_pCore->GetUserNickName((IAnsiString *)&strDspName)))
	{
		TCHAR szTmp[128] = {0};
		CStringConversion::UTF8ToWideChar(strDspName.GetData(), szTmp, 127);
		/*if (szMemo)
		{
			::lstrcat(szTmp, L"   (");
			::lstrcat(szTmp, szMemo);
			::lstrcat(szTmp, L")  ");
		}*/
		int nIdx = GetSubIdxByPresence(szPresence);
		TCHAR szIdx[32] = {0};
		::wsprintf(szIdx, L"%d", nIdx);
		::SkinSetControlAttr(m_hWnd,  L"mainstatus", L"subimage", szIdx);
		::SkinSetControlTextByName(m_hWnd, L"userinfo", szTmp);
		::SkinUpdateControlUI(m_hWnd, L"mainstatus");
		nIdx = GetMenuIdByPresence(szPresence);
		::SkinSetMenuChecked(m_hWnd, L"mainmenu", nIdx, TRUE);
		::SkinSetControlTextByName(m_hWnd, L"mainmenubtn", szMemo);
	}
}

void CMainFrameImpl::DoMainMenuCommand(WPARAM wParam)
{
	switch(wParam)
	{
	case 1:
		if (m_pCore)
			m_pCore->ChangePresence("online", "在线");
		break;
	case 2:
		if (m_pCore)
			m_pCore->ChangePresence("away", "离开");
		break;
	case 3:
		if (m_pCore)
			m_pCore->ChangePresence("busy", "忙碌");
		break;
	case 4:
		if (m_pCore)
			m_pCore->ChangePresence("appearoffline", "隐身");
		break;
	case 5: //下线
		if (m_pCore)
			m_pCore->ChangePresence("offline", "离线");
		break;
	} 
}

void CMainFrameImpl::DoSysMenuCommand(WPARAM wParam)
{
	switch(wParam)
	{
	case 67: //退出
		::SkinCloseWindow(m_hWnd);
		break;
	case 68:
		{
			::SkinSetTreeIconType(m_hWnd, UI_COMPANY_TREE_NAME, 1);
			::SkinSetTreeIconType(m_hWnd, L"recentlytree", 1);
			IConfigure *pCfg = NULL;
			if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
			{
				pCfg->SetParamValue(FALSE, "normal", "treeicon", "1");
				pCfg->Release();
			}
			::SkinUpdateControlUI(m_hWnd, L"colleaguetree");
			::SkinUpdateControlUI(m_hWnd, L"recentlytree");
			break;
		}
		
	case 69:
		{
			::SkinSetTreeIconType(m_hWnd, UI_COMPANY_TREE_NAME, 2);
			::SkinSetTreeIconType(m_hWnd, L"recentlytree", 2);
			IConfigure *pCfg = NULL;
			if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
			{
				pCfg->SetParamValue(FALSE, "normal", "treeicon", "2");
				pCfg->Release();
			}
			::SkinUpdateControlUI(m_hWnd, L"colleaguetree");
			::SkinUpdateControlUI(m_hWnd, L"recentlytree");
			break;
		}
	case 100: //
		 {
			COLORREF cr;
			if (CSystemUtils::OpenColorDialog(NULL, m_hWnd, cr))
			{
				ModifyShiftColor(cr);
			} //end if (CSystemUtils::OpenColorDialog(NULL...
		 } //end case 100
		 break;
	case 101:
		 {
			char szFileName[MAX_PATH] = {0};
			CStringList_ FileList;
			if (CSystemUtils::OpenFileDialog(NULL, m_hWnd, "选择背影图片文件", 
				                         "所有图片文件|*.*", NULL, FileList, FALSE))
			{
				if ((!FileList.empty()) && m_pCore)
				{
					IUIManager *pUI = NULL;
					if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI)))
					{
						pUI->AlphaBackImage(FileList.back().c_str());
						pUI->Release();
					}  //end if (SUCCEEDED(m_pCore->...					
				} //end if ((!FileList.empty()) && m_pCore)

			}  //end if (CSystemUtils::OpenFileDialog(
			break;
		  } //end case 101
	case 102:
		{
			#define UPDATER_APP_NAME "updater\\updater.exe" 
			char szAppName[MAX_PATH] = {0};
			CSystemUtils::GetApplicationFileName(szAppName, MAX_PATH - 1);
			char szAppPath[MAX_PATH] = {0};
			CSystemUtils::ExtractFilePath(szAppName, szAppPath, MAX_PATH - 1);
			std::string strUpdaterAppName = szAppPath;
			strUpdaterAppName += UPDATER_APP_NAME; 
			CSystemUtils::StartShellProcessor(strUpdaterAppName.c_str(), "checkplugin", szAppPath,  FALSE);
			break;
		}
	case 310: //配色
		{
			ModifyShiftColor(RGB(62, 123, 123));
			break;
		}
	case 311: //
		{
			ModifyShiftColor(RGB(105, 160, 200));
			break;
		}
	case 312: //配色
		{
			ModifyShiftColor(RGB(150, 113, 204));
			break;
		}
	case 313: //
		{
			ModifyShiftColor(RGB(156, 193,103));
			break;
		}
	case 314: //配色
		{
			ModifyShiftColor(RGB(193, 90, 39));
			break;
		}
	case 315: //
		{
			ModifyShiftColor(RGB(201, 38, 22));
			break;
		}
	case 316: //配色
		{
			ModifyShiftColor(RGB(216, 124,155));
			break;
		}
	case 317: //
		{
			ModifyShiftColor(RGB(137, 137,137));
			break;
		}
	case 340: //默认
		{
			//
			ModifyShiftColor(0);
			break;
		}
	}
}

//
void CMainFrameImpl::ModifyShiftColor(COLORREF cr)
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
}

STDMETHODIMP_(HWND) CMainFrameImpl::GetSafeWnd()
{
	return m_hWnd;
}

STDMETHODIMP CMainFrameImpl::ShowContacts()
{
	if (!m_pCore)
		return E_FAIL;
#ifdef PRINT_RUN_TIME_INTERVAL
	DWORD dwStart= ::GetTickCount();
#endif
	CInterfaceAnsiString strUserName;
	CInterfaceAnsiString strDomain, strRealName;
	m_pCore->GetUserName((IAnsiString *)&strUserName);
	m_pCore->GetUserDomain((IAnsiString *)&strDomain);
	m_pCore->GetUserNickName((IAnsiString *)&strRealName);

	//copy username
	m_strUserName = strUserName.GetData();
	m_strUserName += "@";
	m_strUserName += strDomain.GetData();

	IContacts *pContacts = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContacts)))
	{
		CInterfaceAnsiString strFileName;
		pContacts->GetContactHead(m_strUserName.c_str(), &strFileName, TRUE);
		pContacts->DrawContactToUI(m_hWnd, UI_COMPANY_TREE_NAME, m_strUserName.c_str(), NULL, TRUE, FALSE, 0); 
		pContacts->Release();
	}  

	int nIconType = 2; //默认显示为小图标
	//load recently list
	IConfigure *pCfg = NULL;
	BOOL bShowCustomPic = TRUE;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		CInterfaceAnsiString strType;
		if (SUCCEEDED(pCfg->GetParamValue(FALSE, "normal", "treeicon", (IAnsiString *)&strType)))
		{
			nIconType = ::atoi(strType.GetData());
		}
		if (SUCCEEDED(pCfg->GetParamValue(FALSE, "normal", "showcustompicture", &strType)))
		{
			if (stricmp(strType.GetData(), "false") == 0)
				bShowCustomPic = FALSE;
		}
		if (SUCCEEDED(pCfg->GetParamValue(FALSE, "normal", "sortbyonline", &strType)))
		{
			if (stricmp(strType.GetData(), "true") == 0)
				m_bSortByOnline = TRUE;
		}
		CInterfaceUserList recentlyList;
		if (SUCCEEDED(pCfg->GetRecentlyList((IUserList *)&recentlyList)))
		{
			void *pSaveNode = NULL;
			std::string strOrderXml;
			//add user node
			TCHAR szwDisplayText[512] = {0};
	        LPORG_TREE_NODE_DATA pData = NULL;
			CInterfaceAnsiString strStatus;
		    CInterfaceAnsiString strSign;
			IContacts *pContacts = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContacts)))
		    {			 
				TCHAR szImageFile[MAX_PATH];
				CInterfaceAnsiString strHeaderFile; 
				
				while (SUCCEEDED(recentlyList.PopFrontUserInfo(&pData)))
				{
					memset(szwDisplayText, 0, sizeof(TCHAR) * 512);
					CStringConversion::UTF8ToWideChar(pData->szDisplayName, szwDisplayText, 511);
					if (SUCCEEDED(pContacts->GetContactHead(pData->szUserName, &strHeaderFile, FALSE)))
					{
						memset(szImageFile, 0, sizeof(TCHAR) * MAX_PATH);
						CStringConversion::StringToWideChar(strHeaderFile.GetData(), szImageFile, MAX_PATH - 1);
						::SkinAddTreeChildNode(m_hWnd, L"recentlytree", pData->id, NULL, szwDisplayText,
						               TREENODE_TYPE_LEAF, pData, NULL, szImageFile, NULL);
					} else 
					{
						//是否为讨论组，则设置为讨论组图标
						std::string strId = pData->szUserName;
						if (strId.find('@') != std::string::npos) //P2P用户
						{
							::SkinAddTreeChildNode(m_hWnd, L"recentlytree", pData->id, NULL, szwDisplayText,
						               TREENODE_TYPE_LEAF, pData, NULL, NULL, NULL);
						} else //讨论组图标
						{
							//
							::SkinAddTreeChildNode(m_hWnd, L"recentlytree", pData->id, NULL, szwDisplayText,
						               TREENODE_TYPE_LEAF, pData, NULL, NULL, NULL);
						}
					}
					//更新状态及签名
	                CInstantUserInfo Info;
					if (SUCCEEDED(pContacts->GetContactUserInfo(pData->szUserName, (IInstantUserInfo *)&Info)))
					{
						if (SUCCEEDED(Info.GetUserStatus((IAnsiString *)&strStatus)) && (strStatus.GetSize() > 0))
							::SkinUpdateUserStatusToNode(m_hWnd, L"recentlytree", pData->szUserName, strStatus.GetData(), FALSE);
						if (SUCCEEDED(Info.GetUserInfo("sign", (IAnsiString *)&strSign)) && (strStatus.GetSize() > 0))
							::SkinUpdateUserLabelToNode(m_hWnd, L"recentlytree", pData->szUserName, strSign.GetData(), FALSE);
					} 
					strOrderXml += "<i u=\"";
					strOrderXml += pData->szUserName;
					strOrderXml += "\" signver=\"0\"/>"; 
					 
				} //end while (SUCCEEDED(
				pContacts->AddOrderUserList(strOrderXml.c_str());  

				//update header
				CInterfaceAnsiString strHeaderFileName;
				if (SUCCEEDED(pContacts->GetContactHead(NULL, &strHeaderFileName, FALSE)))
				{
					TCHAR szwHeaderFileName[MAX_PATH] = {0};
					CStringConversion::StringToWideChar(strHeaderFileName.GetData(), szwHeaderFileName, MAX_PATH - 1);
					::SkinSetControlAttr(m_hWnd, L"mainselfhead", L"floatimagefilename", szwHeaderFileName);
				}
				pContacts->Release();
			}  
		}
		::SkinSortTreeNode(m_hWnd, L"recentlytree", NULL, RecentlyCompareNode, TRUE, FALSE);
		::SkinSetTreeIconType(m_hWnd, L"recentlytree", nIconType);
		::SkinExpandTree(m_hWnd, L"recentlytree", NULL, TRUE, TRUE);
		pCfg->Release();
	}
	if (nIconType == 1)
	{
		::SkinSetMenuChecked(m_hWnd, L"SystemMenu", 68, TRUE);
		::SkinSetMenuChecked(m_hWnd, L"treegroup", 68, TRUE);
	} else
	{
		::SkinSetMenuChecked(m_hWnd, L"SystemMenu", 69, TRUE);
		::SkinSetMenuChecked(m_hWnd, L"treegroup", 69, TRUE);
	}
	if (bShowCustomPic)
	{ 
		::SkinSetMenuChecked(m_hWnd, L"listmenu", 11, TRUE);
		::SkinSetMenuChecked(m_hWnd, L"treegroup", 11, TRUE);
		::SkinSetControlAttr(m_hWnd, L"colleaguetree", L"showcustompic", L"true");
		::SkinSetControlAttr(m_hWnd, L"recentlytree", L"showcustompic", L"true"); 
	} else
	{
		::SkinSetMenuChecked(m_hWnd, L"listbutton", 11, FALSE);
		::SkinSetMenuChecked(m_hWnd, L"treegroup", 11, FALSE);
		::SkinSetControlAttr(m_hWnd, L"colleaguetree", L"showcustompic", L"false");
		::SkinSetControlAttr(m_hWnd, L"recentlytree", L"showcustompic", L"false"); 
	}
	//移动到自己位置
	::SkinTreeScrollToNodeByKey(m_hWnd, L"colleaguetree", m_strUserName.c_str());
	//显示个性签名
	::SkinSetMenuChecked(m_hWnd, L"treegroup", 100, TRUE);
	if (m_bSortByOnline)
		::SkinSetMenuChecked(m_hWnd, L"listmenu", 12, TRUE);
	::SkinSetTreeIconType(m_hWnd, UI_COMPANY_TREE_NAME, nIconType); 
	::SkinUpdateControlUI(m_hWnd, UI_COMPANY_TREE_NAME);
	if (m_pCore)
	{
		CInterfaceAnsiString strPresence, strPresenceMemo;
		if (SUCCEEDED(m_pCore->GetPresence(NULL, (IAnsiString *)&strPresence, (IAnsiString *)&strPresenceMemo)))
		{
			TCHAR szTmp[32] = {0};
			CStringConversion::StringToWideChar(strPresenceMemo.GetData(), szTmp, 31);
			UpdateStatusToUI(strPresence.GetData(), szTmp);
			UpdateUserPresence(m_strUserName.c_str(), strPresence.GetData(), FALSE);
			ITrayMsg *pTray = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(ITrayMsg), (void **)&pTray)))
			{
				pTray->RefreshPresence(strPresence.GetData(), strPresenceMemo.GetData());
				pTray->Release();
			} //end if (SUCCEEDED(m_pCore->QueryInterface(...
		} //end if (SUCCEEDED(m_pCore->GetPresence(...
	} //end if (m_pCore)
	//
#ifdef PRINT_RUN_TIME_INTERVAL
	DWORD  dwInterval = ::GetTickCount() - dwStart;
	PRINTDEBUGLOG(dtInfo, "MainFrame Show Contacts List Time:%d ", dwStart);
#endif 
	//读取离线消息
	m_pCore->GetOfflineMsg();
	return S_OK;
}

BOOL CMainFrameImpl::GetDisplayTextById(const char *szUserName, TCHAR *szDisplayText)
{
	std::string strTmp = szUserName;
	CInterfaceAnsiString strDsp;
	if (strTmp.find("@") != std::string::npos)
	{
		//user
		IContacts *pContact = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
		{
			pContact->GetRealNameById(strTmp.c_str(), NULL, &strDsp);
			pContact->Release();
		}
	} else
	{
		//group
		IGroupFrame *pFrame = NULL;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IGroupFrame), (void **)&pFrame)))
		{
			pFrame->GetGroupNameById(strTmp.c_str(), &strDsp);
			pFrame->Release();
		}
	}
	if (strDsp.GetSize() > 0)
	{
		CStringConversion::UTF8ToWideChar(strDsp.GetData(), szDisplayText, MAX_PATH - 1); 
	}
	return FALSE;
}

//
STDMETHODIMP CMainFrameImpl::ShowRecentlyUser(const char *szUserName, const char *szDispName)
{
	//
	void *pSelNode = NULL;
	CTreeNodeType tnType;
	void *pData = NULL;
	if (::SkinGetNodeByKey(m_hWnd, L"recentlytree", NULL, szUserName, NULL, NULL, &pSelNode, &tnType, &pData))
	{
		LPORG_TREE_NODE_DATA p = (LPORG_TREE_NODE_DATA)pData;
		p->nStamp = (int) ::time(NULL);
	} else
	{
		LPORG_TREE_NODE_DATA p = new ORG_TREE_NODE_DATA();
		memset(p, 0, sizeof(ORG_TREE_NODE_DATA));
		strncpy(p->szUserName, szUserName, 63);
		p->nStamp = (int) ::time(NULL); 
		int nLen = ::strlen(szDispName);
		p->szDisplayName = new char[nLen + 1];
		memset(p->szDisplayName, 0, nLen + 1);
		strcpy(p->szDisplayName, szDispName);
		TCHAR szwDisplayText[512] = {0}; 
		CStringConversion::UTF8ToWideChar(szDispName, szwDisplayText, 511);
		
        std::string strOrderXml = "<i u=\"";
		strOrderXml += szUserName;
		strOrderXml += "\" signver=\"0\"/>";
		IContacts *pContacts = NULL;
		CInterfaceAnsiString strStatus;
		CInterfaceAnsiString strSign;
		if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContacts)))
		{
			CInterfaceAnsiString strHeaderFile;
			if (SUCCEEDED(pContacts->GetContactHead(szUserName, &strHeaderFile, FALSE)))
			{
			    TCHAR szImageFile[MAX_PATH] = {0};
				CStringConversion::StringToWideChar(strHeaderFile.GetData(), szImageFile, MAX_PATH - 1);
				::SkinAddTreeChildNode(m_hWnd, L"recentlytree", p->id, NULL, szwDisplayText,
				               TREENODE_TYPE_LEAF, p, NULL, szImageFile, NULL);
			} else
				::SkinAddTreeChildNode(m_hWnd, L"recentlytree", p->id, NULL, szwDisplayText,
				               TREENODE_TYPE_LEAF, p, NULL, NULL, NULL);
			CInstantUserInfo Info;
			if (SUCCEEDED(pContacts->GetContactUserInfo(szUserName, (IInstantUserInfo *)&Info)))
			{
				if (SUCCEEDED(Info.GetUserStatus((IAnsiString *)&strStatus)) && (strStatus.GetSize() > 0))
					::SkinUpdateUserStatusToNode(m_hWnd, L"recentlytree", szUserName, strStatus.GetData(), FALSE);
				if (SUCCEEDED(Info.GetUserInfo("sign", (IAnsiString *)&strSign)) && (strSign.GetSize() > 0))
					::SkinUpdateUserLabelToNode(m_hWnd, L"recentlytree", szUserName, strSign.GetData(), FALSE);
			}
			pContacts->AddOrderUserList(strOrderXml.c_str()); 
			pContacts->Release();
		} //end if (SUCCEEDED(m_pCore..
		 
	}
	IConfigure *pCfg = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		pCfg->AddRecentlyList(szUserName, szDispName);
		pCfg->Release();
	}
	::SkinSortTreeNode(m_hWnd, L"recentlytree", NULL, RecentlyCompareNode, TRUE, FALSE);
	::SkinExpandTree(m_hWnd, L"recentlytree", NULL, TRUE, TRUE);
	::SkinUpdateControlUI(m_hWnd, L"recentlytree");
	return S_OK;
}

BOOL KeyStringToWord(const char *szKey, WORD &dwShift, WORD &wKey)
{
	if (::stricmp(szKey, "无") != 0)
	{
		char szShift[32] = {0};
		char szTrim[32] = {0};
		char szTmp[128] = {0};
		strcpy(szTmp, szKey); 
		char *p = szTmp;
		while(p)
		{
			p = CStringConversion::GetStringBySep(p, szShift, '+');
			CStringConversion::Trim(szShift, szTrim);
			if (::_stricmp(szTrim, "alt") == 0)
			{
				dwShift = dwShift | MOD_ALT;
			} else if (::_stricmp(szTrim, "shift") == 0)
			{
				dwShift = dwShift | MOD_SHIFT;
			} else if (::_stricmp(szTrim, "ctrl") == 0)
			{
				dwShift = dwShift | MOD_CONTROL;
			} else if (::strlen(szTrim) == 1)
			{
				wKey = szTrim[0];
			}
			memset(szShift, 0, 32);
			memset(szTrim, 0, 32);
		}
	} else
	{
		dwShift = 0;
		wKey = 0;
	}
	if (wKey > 0)
		return TRUE;
	else
		return FALSE;
}

LPCWSTR HotkeyToString(WORD wShift, WORD wkey, TCHAR *szKey)
{
	::swprintf(szKey, 63,  _T("COLINEHOTKEY%d%d"), wShift, wkey);
	return szKey;
}

//删除注册的热键
void CMainFrameImpl::DeleteHotKey( )
{
	if (m_dwAtomRecvHotkeyId != 0)
	{
		::UnregisterHotKey(GetSafeWnd(), m_dwAtomRecvHotkeyId);
		TCHAR szKey[64] = {0};
		HotkeyToString(m_dwRecvHotkeyShiftState, m_dwRecvHotkeyKeyStatue, szKey);
		::GlobalDeleteAtom(GlobalFindAtom(szKey));
		m_dwAtomRecvHotkeyId = 0;
	}
	if (m_dwAtomCutScrHotkeyId != 0)
	{
		::UnregisterHotKey(GetSafeWnd(), m_dwAtomCutScrHotkeyId);
		TCHAR szKey[64] = {0};
		HotkeyToString(m_dwCutScrHotkeyShiftState, m_dwCutScrHotkeyKeyState, szKey);
		::GlobalDeleteAtom(GlobalFindAtom(szKey));
		m_dwAtomCutScrHotkeyId = 0;
	}	 
}
 
//
STDMETHODIMP CMainFrameImpl::UpdateHotKey(int nType)
{
	RegisterGoComHotkey();
	return S_OK;
}



//注册热键
void CMainFrameImpl::RegisterGoComHotkey()
{
	CInterfaceAnsiString strPickMsg, strCutScreen;
	strPickMsg.SetString("CTRL+ALT+I");
	strCutScreen.SetString("CTRL+ALT+C");
	IConfigure *pCfg = NULL;
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		CInterfaceAnsiString strTmp;
		if (SUCCEEDED(pCfg->GetParamValue(FALSE, "hotkey", "pickmsg", &strTmp)))
		{
			strPickMsg.SetString(strTmp.GetData());
		}
		if (SUCCEEDED(pCfg->GetParamValue(FALSE, "hotkey", "cutscreen", &strTmp)))
		{
			strCutScreen.SetString(strTmp.GetData());
		}
		pCfg->Release();
	}
	WORD wPickShift = 0, wPickKey = 0;
	WORD wCutShift = 0, wCutKey = 0;
	KeyStringToWord(strPickMsg.GetData(), wPickShift, wPickKey);
	KeyStringToWord(strCutScreen.GetData(), wCutShift, wCutKey);
	if ((m_dwRecvHotkeyShiftState != wPickShift) || (m_dwRecvHotkeyKeyStatue != wPickKey)
		|| (m_dwCutScrHotkeyShiftState != wCutShift) || (m_dwCutScrHotkeyKeyState != wCutKey))
	{
		//unregister old
		DeleteHotKey();

		//register
	 
		
		TCHAR szwKey[64] = {0};
		if ((wPickShift != 0) || (wPickKey != 0))
		{
			HotkeyToString(wPickShift, wPickKey, szwKey);
			m_dwAtomRecvHotkeyId = ::GlobalAddAtom(szwKey);
			if ((m_dwAtomRecvHotkeyId >= 0xC000) && (m_dwAtomRecvHotkeyId <= 0xFFFF))
			{
				if (::RegisterHotKey(GetSafeWnd(), m_dwAtomRecvHotkeyId, wPickShift, wPickKey))
				{
					m_dwRecvHotkeyShiftState = wPickShift;
					m_dwRecvHotkeyKeyStatue = wPickKey;
				} else
				{
					::GlobalDeleteAtom(GlobalFindAtom(szwKey));
					m_dwAtomRecvHotkeyId = 0;
					//tray tip
				} 
			} else
			{
				m_dwAtomRecvHotkeyId = 0;
			}
		} else
		{
			m_dwRecvHotkeyShiftState = 0;
			m_dwRecvHotkeyKeyStatue = 0;
		}
		//cutscreen 

		if ((wCutShift != 0) || (wCutKey != 0))
		{
			memset(szwKey, 0, sizeof(TCHAR) * 64);
			HotkeyToString(wCutShift, wCutKey, szwKey);
			m_dwAtomCutScrHotkeyId = ::GlobalAddAtom(szwKey);
			if ((m_dwAtomCutScrHotkeyId >= 0xC000) && (m_dwAtomCutScrHotkeyId <= 0xFFFF))
			{
				if (::RegisterHotKey(GetSafeWnd(), m_dwAtomCutScrHotkeyId, wCutShift, wCutKey))
				{
					m_dwCutScrHotkeyShiftState = wCutShift;
					m_dwCutScrHotkeyKeyState = wCutKey;
				} else
				{
					::GlobalDeleteAtom(GlobalFindAtom(szwKey));
					m_dwAtomCutScrHotkeyId = 0;
					//tray tip
				} //end else if (::RegisterHotKey(GetSafeWnd()...
			} else
			{
				m_dwAtomCutScrHotkeyId = 0;
			} //end else if ((m_dwAtomCutScrHotkeyId >= 0xC000)..
		} else
		{
			m_dwCutScrHotkeyShiftState = 0;
			m_dwCutScrHotkeyKeyState = 0;
		} //end else if ((wCutShift != 0)...
	} //end if ((m_dwRecvHotkeyShiftState != wPickShift)...
}

void CMainFrameImpl::GetBannerImage()
{
	if (m_strBannerPicUrl.empty())
		return;
	CInterfaceAnsiString strLocalFileName;
	IConfigure *pCfg = NULL;
	if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{
		if (SUCCEEDED(pCfg->GetPath(PATH_LOCAL_CUSTOM_PICTURE, &strLocalFileName)))
		{
			char szGuid[128] = {0};
			int nSize = 127;
			char szExt[MAX_PATH] = {0};
			CSystemUtils::ExtractFileExtName(m_strBannerPicUrl.c_str(), szExt, MAX_PATH - 1);
			md5_encode(m_strBannerPicUrl.c_str(), m_strBannerPicUrl.size(), szGuid);
			strLocalFileName.AppendString(szGuid);
			strLocalFileName.AppendString(".");
			strLocalFileName.AppendString(szExt); 
		 
			LPBANNER_HTTP_DL_ITEM pItem = new BANNER_HTTP_DL_ITEM();
			pItem->strFileName = strLocalFileName.GetData(); 
			pItem->pImpl = this;
			//
			::P2SvrAddDlTask(m_strBannerPicUrl.c_str(), strLocalFileName.GetData(), FILE_TYPE_NORMAL,
				pItem, BannerDlCallback, FALSE);
			
		}
		pCfg->Release();
	} //end if (SUCCEEDED(
}

#pragma warning(default:4996)
