#include <Commonlib/stringutils.h>
#include <Commonlib/DebugLog.h>
#include <Commonlib/systemutils.h>
#include <SmartSkin/smartskin.h>
#include "ContactPanelImpl.h"
#include "../IMCommonLib/InterfaceAnsiString.h"
#include <Core/treecallbackfun.h>
#include <Core/common.h>
#define CONTACT_APP_OK  1

#pragma warning(disable:4996)

CContactPanelImpl::CContactPanelImpl(void):
                   m_pCore(NULL),
				   m_hWnd(NULL)
{
}


CContactPanelImpl::~CContactPanelImpl(void)
{
	if (m_hWnd)
		::SkinCloseWindow(m_hWnd);
	m_hWnd = NULL;
	if (m_pCore)
		m_pCore->Release();
	m_pCore = NULL;
}

//IUnknown
STDMETHODIMP CContactPanelImpl::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IContactPanel)))
	{
		*ppv = (IContactPanel *) this;
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

HRESULT CContactPanelImpl::DoClickEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{ 
	if (::stricmp(szName, "contacttree") == 0)
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
					IContacts *pContact = NULL;
					if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
					{
						pContact->ExpandTreeNodeToUI(hWnd, L"contacttree", pSelNode, pSelData->id);
						pSelData->bOpened = TRUE;
						pContact->Release();
					} 
				} // end if (pSelData ...					 
			} else
			{
				//
			} //end else if (tnType == ...
		} //end if (::GetSelectTreeNode(hWnd...
	} else if (::stricmp(szName, "btnok") == 0)
	{
		::SkinSetModalValue(hWnd, CONTACT_APP_OK);
		return 0;
	} else if (::stricmp(szName, "selAllUsers") == 0)
	{
		::SkinTreeSelectAll(hWnd, L"selcontact", NULL, TRUE);
	} else if (::stricmp(szName, "unselUsers") == 0)
	{
		::SkinTreeUnSelected(hWnd, L"selcontact", NULL, TRUE);
	} else if (::stricmp(szName, "delselusers") == 0)
	{
		DelTreeSelectedUsers(hWnd);
	} else if (::stricmp(szName, "btncancel") == 0)
	{
		::SkinCloseWindow(hWnd);
		return 0;
	}
	return -1;
}

void CContactPanelImpl::DelTreeSelectedUsers(HWND hWnd)
{
	int nSize = 0;
	::SkinTreeGetSelectedUsers(hWnd, L"selcontact", NULL, &nSize);
	if (nSize > 0)
	{
		char *szUsers = new char[nSize + 1];
		memset(szUsers, 0, nSize + 1);
		if (SkinTreeGetSelectedUsers(hWnd, L"selcontact", szUsers, &nSize))
		{
			TiXmlDocument xmldoc;
			if (xmldoc.Load(szUsers, nSize))
			{
				TiXmlElement *pNode = xmldoc.FirstChildElement();
				while (pNode)
				{
					if (pNode->Attribute("u"))
						m_SelList.DeleteUserInfo(pNode->Attribute("u"));
					pNode = pNode->NextSiblingElement();
				} // end while (
			} //end if (xmldoc..
			::SkinTreeDelSelected(hWnd, L"selcontact", TRUE);
		}
	} //end if (nSize > 0)
}

HRESULT CContactPanelImpl::DoMenuCommandEvent(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szName, "SystemMenu") == 0)
	{
		switch(wParam)
		{
		case 60006:
			 ShowPanel(NULL, NULL, NULL, FALSE, NULL); 
			 break;
		}
	}
	return -1;
}

HRESULT CContactPanelImpl::DoStatusChanged(HWND hWnd, const char *szName, WPARAM wParam, LPARAM lParam)
{
	if (::stricmp(szName, "contacttree") == 0)
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
				int nStatus = ::SkinGetTreeNodeStatus(pSelNode);
				BOOL bAdd = TRUE;
				if (nStatus == 0)
					bAdd = FALSE;
				LPORG_TREE_NODE_DATA pCopyData = new ORG_TREE_NODE_DATA();
			
				memset(pCopyData, 0, sizeof(ORG_TREE_NODE_DATA)); 
				pCopyData->id = pSelData->id;
				pCopyData->pid = pSelData->pid;
				strcpy(pCopyData->szUserName, pSelData->szUserName);

				if (bAdd)
				{
					LPORG_TREE_NODE_DATA pTmp = new ORG_TREE_NODE_DATA();
					memcpy(pTmp, pCopyData, sizeof(ORG_TREE_NODE_DATA));
					if (!m_SelList.AddUserInfo(pTmp, FALSE, TRUE))
						delete pTmp;
				} else
				{
					m_SelList.DeleteUserInfo(pSelData->szUserName);
				}
				if (::SkinAdjustTreeNode(hWnd, L"selcontact", NULL, szName, tnType, pCopyData, bAdd, FALSE) == NULL)
				{
					delete pCopyData;
				}
				::SkinExpandTree(hWnd, L"selcontact", NULL, TRUE, TRUE);
				::SkinUpdateControlUI(hWnd, L"selcontact");
			}else
			{
				IContacts *pContact = NULL;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
				{
					int nStatus = ::SkinGetTreeNodeStatus(pSelNode);
					BOOL bAdd = TRUE;
					if (nStatus == 0)
						bAdd = FALSE;
					CInterfaceUserList ulList;
					char szId[32] = {0};
					itoa(pSelData->id, szId, 10);
					int nType = ::atoi(pSelData->szUserName);
					if (SUCCEEDED(pContact->GetUserListByDeptId(szId, &ulList, FALSE, nType)))
					{
						LPORG_TREE_NODE_DATA pData;
						while (SUCCEEDED(ulList.PopBackUserInfo(&pData)))
						{
							memset(szName, 0, sizeof(TCHAR) * MAX_PATH);
							if (pData->szDisplayName)
								CStringConversion::UTF8ToWideChar(pData->szDisplayName, szName, MAX_PATH - 1);
							else
								CStringConversion::StringToWideChar(pData->szUserName, szName, MAX_PATH - 1);
							if (bAdd)
							{
								if (::SkinAdjustTreeNode(hWnd, L"selcontact", NULL, szName, TREENODE_TYPE_LEAF, pData, bAdd, FALSE))
								{ 
									m_SelList.AddUserInfo(pData, TRUE, TRUE); 
								} else
								{
									if (pData->szDisplayName)
										delete []pData->szDisplayName;
									delete pData;
								}
							} else
							{
								::SkinAdjustTreeNode(hWnd, L"selcontact", NULL, szName, TREENODE_TYPE_LEAF, pData, bAdd, FALSE);
								m_SelList.DeleteUserInfo(pData->szUserName);
								if (pData->szDisplayName)
									delete []pData->szDisplayName;
								delete pData;
							} //end else if (bAdd)
						} //end while（
					} //end if (SUCCEEDED(
					::SkinUpdateControlUI(hWnd, L"selcontact");
					::SkinExpandTree(hWnd, L"selcontact", NULL, TRUE, TRUE);
					pContact->Release();
				} //end else if (
			} //end else
		} //end if (::SkinGetSelect
	}
	return -1;
}

//ICoreEvent
STDMETHODIMP CContactPanelImpl::DoCoreEvent(HWND hWnd, const char *szType, const char *szName, WPARAM wParam, 
	                     LPARAM lParam, HRESULT *hResult)
{
	if (::stricmp(szType, "click") == 0)
	{
		*hResult = DoClickEvent(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "menucommand") == 0)
	{
		*hResult = DoMenuCommandEvent(hWnd, szName, wParam, lParam);
	} else if (::stricmp(szType, "statuschange") == 0)
	{
		*hResult = DoStatusChanged(hWnd, szName, wParam, lParam);
	}  else if (::stricmp(szType, "killfocus") == 0)
	{
		if (::stricmp(szName, "searchedit") == 0)
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
				}
				pContact->Release();
			} 
		} else if (wParam == VK_ESCAPE)
		{
			::SkinCloseWindow(hWnd); //end if (::stricmp(szName...
		} else if (wParam == VK_RETURN)
		{
			::SkinSetModalValue(hWnd, CONTACT_APP_OK);
		}
	} else if (::stricmp(szType, "itemactivate") == 0)
	{
		if (::stricmp(szName, "resultlist") == 0)
		{
			::SkinSetControlTextByName(m_hWnd, L"searchedit", (TCHAR *)wParam);
			IContacts *pContact = NULL;
			if (SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
			{
				DoAdjustSelNode(pContact, m_hCurrHwnd, (char *)lParam, TRUE);
				pContact->Release();
			}
		} //
	} else if (::stricmp(szType, "afterinit") == 0)
	{
		if (::stricmp(szName, "ContactPanel") == 0)
		{
			::SkinSetGetKeyFun(hWnd, L"contacttree", GetTreeNodeKey);
			::SkinSetFreeNodeDataCallBack(hWnd, L"contacttree", FreeTreeNodeData);
			::SkinSetGetKeyFun(hWnd, L"selcontact", GetTreeNodeKey);
			::SkinSetFreeNodeDataCallBack(hWnd, L"selcontact", FreeTreeNodeData);
		}
	}
	return S_OK;
}

void CContactPanelImpl::DoAdjustSelNode(IContacts *pContact, HWND hWnd, const char *szUserName, BOOL bAdd)
{
 
	LPORG_TREE_NODE_DATA pCopyData = new ORG_TREE_NODE_DATA();

	memset(pCopyData, 0, sizeof(ORG_TREE_NODE_DATA)); 
	strcpy(pCopyData->szUserName, szUserName);

	if (bAdd)
	{
		LPORG_TREE_NODE_DATA pTmp = new ORG_TREE_NODE_DATA();
		memcpy(pTmp, pCopyData, sizeof(ORG_TREE_NODE_DATA));
		if (!m_SelList.AddUserInfo(pTmp, FALSE, TRUE))
			delete pTmp;
	} else
	{
		m_SelList.DeleteUserInfo(szUserName);
	}
	TCHAR szName[128] = {0};
	CInterfaceAnsiString strName;
	if (SUCCEEDED(pContact->GetRealNameById(szUserName, NULL, &strName)))
	{
		CStringConversion::UTF8ToWideChar(strName.GetData(), szName, 127);
	} else
		CStringConversion::StringToWideChar(szUserName, szName, 127);
	if (::SkinAdjustTreeNode(hWnd, L"selcontact", NULL, szName, TREENODE_TYPE_LEAF, pCopyData, bAdd, FALSE) == NULL)
	{
		delete pCopyData;
	} //end if (::SkinAdjustTreeNode(
	::SkinExpandTree(hWnd, L"selcontact", NULL, TRUE, FALSE);
	::SkinUpdateControlUI(hWnd, L"selcontact");
}

//广播消息
STDMETHODIMP CContactPanelImpl::DoBroadcastMessage(const char *szFromWndName, HWND hFromWnd, const char *szType,
		                     const char *pContent, void *pData)
{

	return E_NOTIMPL;
}

STDMETHODIMP CContactPanelImpl::SetCoreFrameWork(ICoreFrameWork *pCore)
{
	if (m_pCore)
		m_pCore->Release();
	m_pCore = pCore;
	if (m_pCore)
	{
		m_pCore->AddRef();
		m_pCore->AddOrderEvent((ICoreEvent *) this, "MainWindow", "SystemMenu", "menucommand");
		m_pCore->AddOrderEvent((ICoreEvent *) this, "ContactPanel", NULL, NULL);
	}
	return S_OK;
}

STDMETHODIMP CContactPanelImpl::GetSkinXmlString(IAnsiString *szXmlString)
{
	HRESULT hr = E_FAIL;
	IConfigure *pCfg = NULL; 
	if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg)))
	{ 
		hr = pCfg->GetSkinXml("ContactPanel.xml",szXmlString); 
		pCfg->Release();
	}
	return hr;  
}

//
STDMETHODIMP CContactPanelImpl::CoreFrameWorkError(int nErrorNo, const char *szErrorMsg)
{
	switch(nErrorNo)
	{
	case CORE_ERROR_LOGOUT: //注销
		{
			if (m_hWnd)
				::SkinCloseWindow(m_hWnd);
			if (m_hCurrHwnd)
				::SkinCloseWindow(m_hCurrHwnd);
			m_SelList.Clear(); 
			m_hWnd = NULL;
			m_hCurrHwnd = NULL;
		}
	}
	return S_OK;
}

//
STDMETHODIMP CContactPanelImpl::DoWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *lRes)
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
					pCfg->SetParamValue(FALSE, "Position", "ContactPanelFrame", strRect.c_str());
					pCfg->Release();
				} //end if (SUCCEEDED(hr)
			} //end if (::GetWindowRect(hWnd, &rc))
			return S_OK;
		} //end if (hWnd == m_hWnd)
	} //end if (uMsg == WM_DESTROY)
	return E_FAIL;
}

//IProtocolParser
STDMETHODIMP CContactPanelImpl::DoRecvProtocol(const BYTE *pData, const LONG lSize)
{
	return E_NOTIMPL;
}


STDMETHODIMP CContactPanelImpl::DoPresenceChange(const char *szUserName, const char *szNewPresence, const char *szMemo, BOOL bOrder)
{
	return E_NOTIMPL;
}

#define CONTACT_PANEL_WIDTH   600
#define CONTACT_PANEL_HEIGHT  500
BOOL CContactPanelImpl::InitContactPanel(HWND hParent, LPRECT lprc, const TCHAR *szCaption, HWND *hWnd, IUserList *pUsers)
{
	if (m_pCore)
	{
		IUIManager *pUI = NULL;
		IConfigure *pCfg = NULL;		
		HRESULT hr = m_pCore->QueryInterface(__uuidof(IConfigure), (void **)&pCfg);
		if (SUCCEEDED(hr) && pCfg)
		{
			RECT rc = {100, 100, CONTACT_PANEL_WIDTH + 100, CONTACT_PANEL_HEIGHT + 100};
			if (lprc)
				rc = *lprc;
			else if (hParent)
			{
				RECT rcParent = {0};
				::GetWindowRect(hParent, &rcParent);
				rc.left = rcParent.left +  (rcParent.right - rcParent.left - CONTACT_PANEL_WIDTH) / 2;
				rc.top = rcParent.top + (rcParent.bottom - rcParent.top - CONTACT_PANEL_HEIGHT) / 2;
				if (rc.left < 0)
					rc.left = 0;
				if (rc.top < 0)
					rc.top = 0;
				rc.right = rc.left + CONTACT_PANEL_WIDTH;
				rc.bottom = rc.top + CONTACT_PANEL_HEIGHT;
				RECT rcScreen = {0};
				CSystemUtils::GetScreenRect(&rcScreen); 
				if (rc.right > rcScreen.right)
				{
					rc.right = rcScreen.right;
					rc.left = rc.right - CONTACT_PANEL_WIDTH;
				}
				if (rc.bottom > rcScreen.bottom)
				{
					rc.bottom = rcScreen.bottom;
					rc.top = rc.bottom - CONTACT_PANEL_HEIGHT;
				}
			} else
			{
				RECT rcSave = {0};
				CInterfaceAnsiString strPos;
				if (SUCCEEDED(pCfg->GetParamValue(FALSE, "Position", "ContactPanelFrame", (IAnsiString *)&strPos)))
				{
					CSystemUtils::StringToRect(&rcSave, strPos.GetData());
				}
				if (!::IsRectEmpty(&rcSave))
					rc = rcSave;
			}
			hr = m_pCore->QueryInterface(__uuidof(IUIManager), (void **)&pUI);
			if (SUCCEEDED(hr) && pUI)
			{
				pUI->CreateUIWindow(hParent, "ContactPanel", &rc, WS_POPUP | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX,
				                0, szCaption, hWnd);				
				
				//pUI->OrderWindowMessage("ContactPanel", NULL, WM_DESTROY, (ICoreEvent *) this);
				//
				LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
				memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
				pData->bOpened = TRUE;
				pData->id = -1;
			    void *pSaveNode = ::SkinAddTreeChildNode(*hWnd, L"contacttree", pData->id, NULL,  L"联系人", 
		               TREENODE_TYPE_GROUP, pData, NULL, NULL, NULL);
				IContacts *pContact = NULL;
				if (m_pCore && SUCCEEDED(m_pCore->QueryInterface(__uuidof(IContacts), (void **)&pContact)))
				{
					pContact->DrawContactToUI(*hWnd, L"contacttree", NULL, pSaveNode, FALSE, FALSE, 0);
					pData = new ORG_TREE_NODE_DATA();
					memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
					pData->bOpened = TRUE;
					pData->id = -2;
		            pSaveNode = ::SkinAddTreeChildNode(*hWnd, L"contacttree", pData->id, NULL,  L"外部联系人", 
		               TREENODE_TYPE_GROUP, pData, NULL, NULL, NULL);
			        pContact->DrawContactToUI(*hWnd, L"contacttree", NULL, pSaveNode, TRUE, FALSE, FREQUENCY_CONTACT_TYPE_ID);
		 
					::SkinSetControlAttr(*hWnd, L"contacttree", L"showcheckstatus", L"true");
					//
					//
					TCHAR szText[MAX_PATH];
					if (pUsers)
					{
						while (SUCCEEDED(pUsers->PopFrontUserInfo(&pData)))
						{
							memset(szText, 0, sizeof(TCHAR) * MAX_PATH - 1);
							if (pData->szDisplayName)
								CStringConversion::UTF8ToWideChar(pData->szDisplayName, szText, MAX_PATH - 1);
							else
							{
								CInterfaceAnsiString szName;
								if (SUCCEEDED(pContact->GetRealNameById(pData->szUserName, NULL, &szName)))
								{
									CStringConversion::UTF8ToWideChar(szName.GetData(), szText, MAX_PATH - 1);
								} else
								{
									CStringConversion::StringToWideChar(pData->szUserName, szText, 63);
								}
							}
							m_SelList.AddUserInfo(pData, TRUE, TRUE);
							::SkinAddTreeChildNode(*hWnd, L"selcontact", pData->id, NULL, szText, TREENODE_TYPE_LEAF, pData, NULL, NULL, NULL);
						}
					}
					::SkinSetControlAttr(*hWnd, L"selcontact", L"showcheckstatus", L"true");
					::SkinExpandTree(*hWnd, L"selcontact", NULL, TRUE, TRUE);
					pContact->Release();
				}
				
				pUI->Release();
				pUI = NULL;
			}
			pCfg->Release();
			pCfg = NULL;
		} //end if (SUCCEEDED(hr)... 
		return TRUE;
	} //end if (m_pCore)
	return FALSE;
}

//IContactPanel
STDMETHODIMP CContactPanelImpl::ShowPanel(HWND hParent, const TCHAR *szCaption, LPRECT lprc, BOOL bModal, IUserList *pUsers)
{
	if (bModal)
	{
		HWND hWnd = NULL;
		if (InitContactPanel(hParent, lprc, szCaption, &hWnd, pUsers))
		{
			m_hCurrHwnd = hWnd;
			if (::SkinShowModal(hWnd) == CONTACT_APP_OK)
			{ 
				return S_OK;
			} 
		}
	} else
	{
		if (m_hWnd && ::IsWindow(m_hWnd))
		{
			m_hCurrHwnd = m_hWnd;
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
			if (InitContactPanel(NULL, lprc, szCaption, &m_hWnd, pUsers))
			{
				if (::IsWindow(m_hWnd))
				{
					 ::ShowWindow(m_hWnd, SW_SHOW);
				}
			}
			return S_OK;
		}
	}
	return E_FAIL;
}

//
 
STDMETHODIMP CContactPanelImpl::GetSelContact(IUserList *pList)
{
	LPORG_TREE_NODE_DATA pData;
	while (SUCCEEDED(m_SelList.PopBackUserInfo(&pData)))
	{
		pList->AddUserInfo(pData, FALSE);
	} 
	return S_OK;
}

#pragma warning(default:4996)
