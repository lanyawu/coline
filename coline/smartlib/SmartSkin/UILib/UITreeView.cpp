#include "common.h"
#include <UILIb/UITreeView.h>
#include <CommonLib/StringUtils.h> 
#include <UILib/UIScroll.h>
#include <fstream>
#include <Commonlib/debuglog.h>
#pragma warning(disable:4996)

#define BORDER_CX 2
#define BORDER_CY 2
#define CHECK_STATUS_BORDER 10

//状态顺序
#define STATUS_IMAGE_SEQ_LEAVE  1 
#define STATUS_IMAGE_SEQ_BUSY   2
#define STATUS_IMAGE_SEQ_MOBILE 4

BOOL XYInRect(int x, int y, RECT &rc)
{
	if ((x >= rc.left) && (x <= rc.right) && (y >= rc.top) && (y <= rc.bottom))
		return TRUE;
	return FALSE;
}

int GetStatusByName(const char *szStatus)
{
	if (szStatus && ::stricmp(szStatus, " ") != 0)
	{
		if (::stricmp(szStatus, "offline") == 0)
			return ONLINE_STATUS_TYPE_OFFLINE;
		else if (::stricmp(szStatus, "online") == 0)
			return ONLINE_STATUS_TYPE_ONLINE;
		else if (::stricmp(szStatus, "appearoffline") == 0)
			return ONLINE_STATUS_TYPE_HIDE;
		else if (::stricmp(szStatus, "busy") == 0)
			return ONLINE_STATUS_TYPE_BUSY;
		else if (::stricmp(szStatus, "away") == 0)
			return ONLINE_STATUS_TYPE_LEAVE;
		else if (::stricmp(szStatus, "iphone") == 0)
			return ONLINE_STATUS_TYPE_MOBILE;
		else if (::stricmp(szStatus, "ipad") == 0)
			return ONLINE_STATUS_TYPE_MOBILE;
		else if (::stricmp(szStatus, "ipod") == 0)
			return ONLINE_STATUS_TYPE_MOBILE;
		else if (::stricmp(szStatus, "itouch") == 0)
			return ONLINE_STATUS_TYPE_MOBILE;
		else if (::stricmp(szStatus, "android") == 0)
			return ONLINE_STATUS_TYPE_MOBILE;
		else
			return ONLINE_STATUS_TYPE_BUSY;
	}
	return ONLINE_STATUS_TYPE_OFFLINE;
}

//CTreeNodeItem
CTreeNodeItem::CTreeNodeItem(CTreeNodeItem *pParent, CTreeNodeType NodeType, IUITreeViewApp *pTreeView):
               m_bSelected(FALSE),
			   m_pData(NULL),
			   m_bIsFocus(FALSE),
			   m_pParent(pParent),
			   m_dwCurrSeq(0),
			   m_nCheckStatus(0),
			   m_dwNodeCount(0),
			   m_bExpanded(FALSE),
			   m_Nodes(NULL),
			   m_byteStatus(0),
			   m_szLabel(NULL),
			   m_dwNodeId(0),
			   m_nExtraImageId(0),
			   m_dwLeafCount(0),
			   m_dwOnlineCount(0),
			   m_NodeType(NodeType),
			   m_pTreeView(pTreeView)
{
	memset(m_szName, 0, sizeof(TCHAR) * MAX_NODE_NAME_SIZE);
	memset(&m_rcItem, 0, sizeof(RECT));
	memset(&m_rcImage, 0, sizeof(RECT));
	memset(&m_rcCheck, 0, sizeof(RECT));
	memset(&m_rcLinks, 0, sizeof(RECT) * MAX_RECT_LINKS_COUNT);
	m_nLinksCount = MAX_RECT_LINKS_COUNT;
}

CTreeNodeItem::~CTreeNodeItem()
{
	Clear(); //清除
	if (m_pTreeView)
		m_pTreeView->FreeNodeExtData(m_NodeType, &m_pData);
	if (m_szLabel)
		delete []m_szLabel;
}

void *CTreeNodeItem::GetData()
{
	return m_pData;
}

//计算到某个节点的高度
BOOL  CTreeNodeItem::CalcHeightToKey(const char *szKey, DWORD &dwTop)
{
	if (m_NodeType == TREENODE_TYPE_GROUP)
	{
		dwTop = dwTop + GROUP_NODE_TEXT_HEIGHT + TREE_NODE_VERT_DISTANCE;
		if (m_bExpanded)
		{
			for (DWORD i = 0; i < m_dwNodeCount; i ++)
			{
				if (m_Nodes[i]->CalcHeightToKey(szKey, dwTop))
					return TRUE;
			} //end for (..
		} //end if (m_bExpanded)
	} else if (m_NodeType == TREENODE_TYPE_LEAF)
	{
		if (stricmp(m_pTreeView->GetNodeKey(m_NodeType, m_pData), szKey) == 0)
			return TRUE;
		else
			dwTop = dwTop + m_pTreeView->GetLeafNodeHeight() + TREE_NODE_VERT_DISTANCE;
	} //end else if (m
	return FALSE;
}

DWORD CTreeNodeItem::CalcHeight(DWORD &dwTop)
{
	if (m_NodeType == TREENODE_TYPE_GROUP)
	{
		dwTop = dwTop + GROUP_NODE_TEXT_HEIGHT + TREE_NODE_VERT_DISTANCE;
		if (m_bExpanded)
		{
			for (DWORD i = 0; i < m_dwNodeCount; i ++)
			{
				m_Nodes[i]->CalcHeight(dwTop);
			}
		}
	} else if (m_NodeType == TREENODE_TYPE_LEAF)
	{
		dwTop = dwTop + m_pTreeView->GetLeafNodeHeight() + TREE_NODE_VERT_DISTANCE;
	}
	return dwTop;
}

//调整滚动条位置
BOOL CTreeNodeItem::AdjustScrollPos(int &dwTop, int &dwScrollPos)
{
	if (dwTop >= (int) (dwScrollPos + m_pTreeView->GetTreeViewTop()))
	{
		if ((dwTop - dwScrollPos) < (int) m_pTreeView->GetTreeViewTop())
		{
			dwScrollPos = dwTop - m_pTreeView->GetTreeViewTop();
		}
		return TRUE;
	}
	if (m_NodeType == TREENODE_TYPE_GROUP)
	{
		dwTop = dwTop + GROUP_NODE_TEXT_HEIGHT + TREE_NODE_VERT_DISTANCE;
		if (m_bExpanded)
		{
			for (DWORD i = 0; i < m_dwNodeCount; i ++)
			{
				if (m_Nodes[i]->AdjustScrollPos(dwTop, dwScrollPos))
					return TRUE;
			}
		}
	} else if (m_NodeType == TREENODE_TYPE_LEAF)
	{
		dwTop = dwTop + m_pTreeView->GetLeafNodeHeight() + TREE_NODE_VERT_DISTANCE;
	}
	return FALSE;
}

//导出到文件
void CTreeNodeItem::SaveToStream(std::ofstream &ofs, char *szPreChars, BYTE byteFileType)
{
	char szTemp[512] = {0};
	char szPre[128] = {0};
	sprintf(szPre, "%s\t", szPreChars);
	for (int i = 0; i < (int) GetChildCount(); i ++)
	{
		char szName[128] = {0};
		CStringConversion::WideCharToString(m_Nodes[i]->GetName(), szName, 127);
		sprintf(szTemp, "%s%s\n", szPreChars, szName);
		ofs.write(szTemp, strlen(szTemp));
		if (m_Nodes[i]->GetNodeType() == TREENODE_TYPE_GROUP)
			m_Nodes[i]->SaveToStream(ofs, szPre, byteFileType);
	}
} 
 
//获取叶子节点数
DWORD CTreeNodeItem::GetLeafCount()
{
	return m_dwLeafCount;
}

//子节点发生变化
void CTreeNodeItem::OnNodeChange()
{
	m_dwLeafCount = 0;
	m_dwOnlineCount = 0;
	StatChildNode(m_dwLeafCount, m_dwOnlineCount);
	if (m_pParent)
		m_pParent->OnNodeChange();
}

RECT CTreeNodeItem::GetCheckRect()
{
	return m_rcCheck;
}

void CTreeNodeItem::SelectAll(BOOL bRecursived) //全选
{
	for (int i = 0; i < (int) GetChildCount(); i ++)
	{
		if (m_Nodes[i]->GetNodeType() == TREENODE_TYPE_LEAF)
		{
			m_Nodes[i]->SetCheckStatus(CHECK_STATUS_CHECKED);			 
		} else if (bRecursived)
		{
			m_Nodes[i]->SelectAll(bRecursived);
		} //end else if (bRecursived)
	} //end for (...
}

void CTreeNodeItem::UnSelected(BOOL bRecursived) //反选
{
	for (int i = 0; i < (int) GetChildCount(); i ++)
	{
		if (m_Nodes[i]->GetNodeType() == TREENODE_TYPE_LEAF)
		{
			if (m_Nodes[i]->GetCheckStatus() == CHECK_STATUS_CHECKED)
				m_Nodes[i]->SetCheckStatus(CHECK_STATUS_NORMAL);	
			else
				m_Nodes[i]->SetCheckStatus(CHECK_STATUS_CHECKED);
		} else if (bRecursived)
		{
			m_Nodes[i]->UnSelected(bRecursived);
		} //end else if (
	}//end for (int i ... 
}

void CTreeNodeItem::DeleteSelected(BOOL bRecursived) //删除选择的节点
{
	std::vector<int> vcDelList;
	for (int i = 0; i < (int) GetChildCount(); i ++)
	{
		if (m_Nodes[i]->GetNodeType() == TREENODE_TYPE_LEAF)
		{
			if (m_Nodes[i]->GetCheckStatus() == CHECK_STATUS_CHECKED)
				vcDelList.push_back(i); 
		} else if (bRecursived)
		{
			m_Nodes[i]->UnSelected(bRecursived);
		}
	} 
	int idx = 0;
	while (!vcDelList.empty())
	{
		idx = vcDelList.back();
		DeleteNode(idx);
		vcDelList.pop_back();
	}
}

//get selected user lsit
BOOL CTreeNodeItem::GetSelectUserList(std::string &strUsers, BOOL bRecursive)
{
	for (int i = 0; i < (int) GetChildCount(); i ++)
	{
		if (m_Nodes[i]->GetNodeType() == TREENODE_TYPE_LEAF)
		{

			if ((m_Nodes[i]->GetData() != NULL) && (m_Nodes[i]->GetCheckStatus() == CHECK_STATUS_CHECKED))
			{
				strUsers += "<i u=\"";
				strUsers += m_pTreeView->GetNodeKey(TREENODE_TYPE_LEAF, m_Nodes[i]->GetData());
				strUsers += "\"/>";
			}
		} else if (bRecursive)
		{
			m_Nodes[i]->GetSelectUserList(strUsers, bRecursive);
		}
	}
	return TRUE;
}

//getuser list
BOOL CTreeNodeItem::GetUserList(std::string &strUsers, BOOL bRecursive)
{
	for (int i = 0; i < (int) GetChildCount(); i ++)
	{
		if (m_Nodes[i]->GetNodeType() == TREENODE_TYPE_LEAF)
		{

			if (m_Nodes[i]->GetData() != NULL)
			{
				strUsers += "<i u=\"";
				strUsers += m_pTreeView->GetNodeKey(TREENODE_TYPE_LEAF, m_Nodes[i]->GetData());
				strUsers += "\"/>";
			}
		} else if (bRecursive)
		{
			m_Nodes[i]->GetUserList(strUsers, bRecursive);
		}
	}
	return TRUE;
}

LPVOID CTreeNodeItem::UpdateUserStatusToNode_(const char *szUserName, const char *szStatus, 
	                                           BOOL bMulti)
{
	LPVOID pReturn = NULL;
	for (int i = 0; i < (int) GetChildCount(); i ++)
	{
		if (m_Nodes[i]->GetNodeType() == TREENODE_TYPE_LEAF)
		{
			if (m_Nodes[i]->GetData() != NULL)
			{ 
				const char *p = m_pTreeView->GetNodeKey(m_Nodes[i]->GetNodeType(), m_Nodes[i]->GetData());
				if ((p != NULL) && (::stricmp(p, szUserName) == 0))
				{
					m_Nodes[i]->SetStatus(GetStatusByName(szStatus));
					pReturn = m_Nodes[i];
					if (!bMulti)
						return pReturn;
				}
			}
		} else  
		{
			LPVOID pTmp = m_Nodes[i]->UpdateUserStatusToNode_(szUserName, szStatus,  bMulti);
			if (pTmp != NULL)
			{
				pReturn = pTmp;
				if (!bMulti)
					return pTmp;
			} //end if (pTmp != NULL)
		} //end else if (m_Nodes[i]->GetNodeType() == TREENODE_TYPE_LEAF)
	} //end for (...
	return pReturn;
}

//
LPVOID CTreeNodeItem::UpdateImageFile(const char *szKey, const char *szImageFile, BOOL bMulti)
{
	for (int i = 0; i < (int) GetChildCount(); i ++)
	{
		if (m_Nodes[i]->GetNodeType() == TREENODE_TYPE_LEAF)
		{
			if (m_Nodes[i]->GetData() != NULL)
			{ 
				const char *p = m_pTreeView->GetNodeKey(m_Nodes[i]->GetNodeType(), m_Nodes[i]->GetData());
				if ((p != NULL) && (::stricmp(p, szKey) == 0))
				{
					TCHAR szwLabel[MAX_PATH] ={0}; 
					m_Nodes[i]->SetGraph(szImageFile);
					return m_Nodes[i];
				} //end if (::stricmp(p + nOffset...
			} //end if (m_Nodes[i]->GetData()...
		} else  
		{
			LPVOID pTmp = m_Nodes[i]->UpdateImageFile(szKey, szImageFile,  bMulti);
			if ((pTmp != NULL) && (!bMulti))
				return pTmp;
		}
	}
	return NULL;
}

void CTreeNodeItem::SetExtraImageId(const int nImageId)
{
	m_nExtraImageId = nImageId;
}

//
LPVOID CTreeNodeItem::UpdateExtraData(const char *szKey, const TCHAR *szExtra, BOOL bMulti)
{
	for (int i = 0; i < (int) GetChildCount(); i ++)
	{
		if (m_Nodes[i]->GetNodeType() == TREENODE_TYPE_LEAF)
		{
			if (m_Nodes[i]->GetData() != NULL)
			{ 
				const char *p = m_pTreeView->GetNodeKey(m_Nodes[i]->GetNodeType(), m_Nodes[i]->GetData());
				if ((p != NULL) && (::stricmp(p, szKey) == 0))
				{ 
					m_Nodes[i]->SetExtraData(szExtra);
					return m_Nodes[i];
				} //end if (::stricmp(p + nOffset...
			} //end if (m_Nodes[i]->GetData()...
		} else  
		{
			LPVOID pTmp = m_Nodes[i]->UpdateExtraData(szKey, szExtra,  bMulti);
			if ((pTmp != NULL) && (!bMulti))
				return pTmp;
		}
	}
	return NULL;
}

//
LPVOID CTreeNodeItem::UpdateExtraImageFile(const char *szKey, const int nExtraImageId, BOOL bMulti)
{
	for (int i = 0; i < (int) GetChildCount(); i ++)
	{
		if (m_Nodes[i]->GetNodeType() == TREENODE_TYPE_LEAF)
		{
			if (m_Nodes[i]->GetData() != NULL)
			{ 
				const char *p = m_pTreeView->GetNodeKey(m_Nodes[i]->GetNodeType(), m_Nodes[i]->GetData());
				if ((p != NULL) && (::stricmp(p, szKey) == 0))
				{ 
					m_Nodes[i]->SetExtraImageId(nExtraImageId);
					return m_Nodes[i];
				} //end if (::stricmp(p + nOffset...
			} //end if (m_Nodes[i]->GetData()...
		} else  
		{
			LPVOID pTmp = m_Nodes[i]->UpdateExtraImageFile(szKey, nExtraImageId,  bMulti);
			if ((pTmp != NULL) && (!bMulti))
				return pTmp;
		}
	}
	return NULL;
}

//
LPVOID CTreeNodeItem::UpdateUserLabelToNode_(const char *szUserName,  const char *szUTF8Label, 
	                                          BOOL bMulti)
{
	for (int i = 0; i < (int) GetChildCount(); i ++)
	{
		if (m_Nodes[i]->GetNodeType() == TREENODE_TYPE_LEAF)
		{
			if (m_Nodes[i]->GetData() != NULL)
			{ 
				const char *p = m_pTreeView->GetNodeKey(m_Nodes[i]->GetNodeType(), m_Nodes[i]->GetData());
				if ((p != NULL) && (::stricmp(p, szUserName) == 0))
				{ 
					TCHAR szwLabel[MAX_PATH] ={0};
					CStringConversion::UTF8ToWideChar(szUTF8Label, szwLabel, MAX_PATH - 1);
					m_Nodes[i]->SetLabel(szwLabel);
					return m_Nodes[i];
				} //end if (::stricmp(p + nOffset...
			} //end if (m_Nodes[i]->GetData()...
		} else  
		{
			LPVOID pTmp = m_Nodes[i]->UpdateUserLabelToNode_(szUserName, szUTF8Label,  bMulti);
			if ((pTmp != NULL) && (!bMulti))
				return pTmp;
		}
	}
	return NULL;
}

//统计所有节点下的子节点数和在线节点数
void CTreeNodeItem::StatChildNode(DWORD &dwChildCount, DWORD &dwOnlineCount)
{
	for (int i = 0; i < (int) GetChildCount(); i ++)
	{
		if (m_Nodes[i]->GetNodeType() == TREENODE_TYPE_LEAF)
		{
			dwChildCount ++;
			if (m_Nodes[i]->GetStatus() > ONLINE_STATUS_TYPE_HIDE)
				dwOnlineCount ++;
		} else
		{
			m_Nodes[i]->StatChildNode(dwChildCount, dwOnlineCount);
		}
	}
}

void CTreeNodeItem::SetExtraData(const TCHAR *szExtra)
{
	if (szExtra)
		m_strExtra = szExtra;
	else
		m_strExtra.Empty();
}

//
BOOL CTreeNodeItem::GetNodeById(const DWORD dwId, CTreeNodeType tnType, void **pNode, void **pData)
{
	for (int i = 0; i < (int) GetChildCount(); i ++)
	{
		if (m_Nodes[i]->GetNodeType() == tnType)
		{
			if (m_Nodes[i]->GetNodeId() == dwId)
			{
				*pData = m_Nodes[i]->GetData();
				*pNode = m_Nodes[i];
				return TRUE;
			}
		}
		if (m_Nodes[i]->GetNodeType() == TREENODE_TYPE_GROUP)
		{
			if (m_Nodes[i]->GetNodeById(dwId, tnType, pNode, pData))
				return TRUE;
		}  
	}
	return FALSE;
}

BOOL CTreeNodeItem::GetNodeByKey(const char *szKey, TCHAR *szName, int *nNameLen, void **pSelNode,
	                         CTreeNodeType *tnType, void **pData)
{
	for (int i = 0; i < (int) GetChildCount(); i ++)
	{
 
		if (m_Nodes[i]->GetData() != NULL)
		{ 
			const char *p = m_pTreeView->GetNodeKey(m_Nodes[i]->GetNodeType(), m_Nodes[i]->GetData());
			if ((p != NULL) && (::stricmp(p, szKey) == 0))
			{
				*tnType = m_Nodes[i]->GetNodeType();
				*pData = m_Nodes[i]->GetData();
				*pSelNode = m_Nodes[i];
				int nLen = ::lstrlen(m_Nodes[i]->GetName());
				if (nNameLen && szName)
				{
					if (*nNameLen >= nLen)
						::lstrcpy(szName, m_Nodes[i]->GetName());
					*nNameLen = nLen;
				}
				return TRUE;
			} //end if (::stricmp(p + nOffset...
		} //end if (m_Nodes[i]->GetData()...
		if (m_Nodes[i]->GetNodeType() == TREENODE_TYPE_GROUP)
		{
			if ( m_Nodes[i]->GetNodeByKey(szKey, szName, nNameLen, pSelNode, tnType, pData))
				return TRUE; 
		}
	}
	return FALSE;
}

void CTreeNodeItem::SetCheckStatus(const int nStatus)
{
	m_nCheckStatus = nStatus;
	for (int i = 0; i < (int) GetChildCount(); i ++)
	{ 
		m_Nodes[i]->SetCheckStatus(nStatus);			 
	}
	Invalidate();	
}

int CTreeNodeItem::GetCheckStatus()
{
	if (GetNodeType() == TREENODE_TYPE_LEAF)
		return m_nCheckStatus;
	int nStatus = m_nCheckStatus;
	int nCurrStatus;
	for (int i = 0; i < (int) GetChildCount(); i ++)
	{ 
		if (m_Nodes[i]->GetNodeType() == TREENODE_TYPE_LEAF)
		{
			if (nStatus != m_Nodes[i]->m_nCheckStatus)
			{
				nStatus = CHECK_STATUS_GRAY;
				break;				
			}
		} else
		{
			nCurrStatus = m_Nodes[i]->GetCheckStatus();			
			if (nStatus != nCurrStatus)
			{
				nStatus = CHECK_STATUS_GRAY;
				break;
			}			
		}
	}
	return nStatus;
}

//绘制此节点
void CTreeNodeItem::DrawItem(HDC hDc, const RECT &rc, const BOOL &bIsShowCount, const BOOL bInvalidate)
{
	if (((rc.right - rc.left) <= 0) || ((rc.bottom - rc.top) <= 0))
		return ;
	
	UITYPE_COLOR iTextColor = UICOLOR_CONTROL_TEXT_NORMAL;
	int nLinks = m_nLinksCount;
	if (m_NodeType == TREENODE_TYPE_GROUP)
	{
		//node image which indicates whether the current node is expaned or not.
		if ((bInvalidate) && ((rc.bottom - rc.top) < GROUP_NODE_TEXT_HEIGHT))
		{
			Invalidate();
			return;
		}

		//计算区域
		m_rcItem = rc;
		m_rcItem.bottom = m_rcItem.top + GROUP_NODE_TEXT_HEIGHT;
		if (m_rcItem.bottom > rc.bottom)
			m_rcItem.bottom = rc.bottom;

		CPaintManagerUI* pManager = m_pTreeView->GetPaintManager();
		if (m_pTreeView->ShowCheckStatus())
		{			
			m_rcCheck.left = rc.left - CHECK_STATUS_IMAGE_WIDTH - 2;
			m_rcCheck.top = rc.top + (GROUP_NODE_TEXT_HEIGHT - CHECK_STATUS_IMAGE_HEIGHT) / 2;
			m_rcCheck.right = rc.left - 2;
			m_rcCheck.bottom = m_rcCheck.top + CHECK_STATUS_IMAGE_HEIGHT;
			CBlueRenderEngineUI::DoPaintCheckStatus(hDc, pManager, m_rcCheck, m_pTreeView->GetCheckStatusImageId(), 
					GetCheckStatus());
		} 
		//节点图片
		if (m_pTreeView)
		{ 
			m_rcImage.left = rc.left;
			m_rcImage.top = rc.top + (GROUP_NODE_TEXT_HEIGHT - GROUP_NODE_IMAGE_HEIGHT);
			m_rcImage.right = rc.left + GROUP_NODE_IMAGE_WIDTH;
			m_rcImage.bottom = rc.top + GROUP_NODE_IMAGE_HEIGHT;			
			 
			if(!m_pImage.IsEmpty())
			{				
				UI_IMAGE_ITEM Item;
				Item.pGraphic = &m_pImage;
				Item.dwSubCount = 2;
				CBlueRenderEngineUI::DoPaintGroupNode(hDc, pManager, m_rcImage, 
					TREENODE_TYPE_GROUP, &Item, m_bExpanded); 
			} else
			{
				CBlueRenderEngineUI::DoPaintGroupNode(hDc, pManager, m_rcImage, 
					TREENODE_TYPE_GROUP, m_pTreeView->GetDefaultGrpImageId(), m_bExpanded );
			}
			 
		}

		//绘制文字背景
		RECT rcTextBkgnd = m_rcItem;
		rcTextBkgnd.left += GROUP_NODE_IMAGE_WIDTH;
		DrawItemBkground( hDc, rcTextBkgnd );

		//绘制文字
		RECT rcText;
		rcText.left = rc.left + GROUP_NODE_IMAGE_WIDTH + TREE_NODE_VERT_DISTANCE;
		rcText.right = rc.right;
		rcText.top = rc.top;
		rcText.bottom = rcText.top + GROUP_NODE_TEXT_HEIGHT;
		if (bIsShowCount)
		{
			TCHAR szTemp[64] = {0};
			::swprintf(szTemp, _T("%s(%d/%d)"), m_szName, m_dwOnlineCount, m_dwLeafCount);
			CBlueRenderEngineUI::DoPaintPrettyText(hDc, m_pTreeView->GetPaintManager(), rcText, szTemp, iTextColor, 
				  UICOLOR__INVALID, NULL, nLinks, DT_SINGLELINE | DT_VCENTER | DT_LEFT );
		} else
			CBlueRenderEngineUI::DoPaintPrettyText(hDc, m_pTreeView->GetPaintManager(), rcText, m_szName, iTextColor, 
				  UICOLOR__INVALID, m_rcLinks, nLinks, DT_SINGLELINE | DT_VCENTER | DT_LEFT );

	} else
	{
		if ((bInvalidate) && ((rc.bottom - rc.top) < (int) m_pTreeView->GetLeafNodeHeight()))
		{
			Invalidate();
			return;
		}
		//节点大小
		m_rcItem.left = rc.left;
		m_rcItem.top = rc.top;
		m_rcItem.bottom = m_rcItem.top + m_pTreeView->GetLeafNodeHeight();
		m_rcItem.right = rc.right;
		
		if (m_pTreeView->ShowCheckStatus())
		{			
			m_rcCheck.left = rc.left - CHECK_STATUS_IMAGE_WIDTH - 2;
			m_rcCheck.top = rc.top + (m_pTreeView->GetLeafNodeHeight() -CHECK_STATUS_IMAGE_HEIGHT) / 2;
			m_rcCheck.right = rc.left - 2;
			m_rcCheck.bottom = m_rcCheck.top + CHECK_STATUS_IMAGE_HEIGHT;
			CBlueRenderEngineUI::DoPaintCheckStatus(hDc, m_pTreeView->GetPaintManager(), m_rcCheck,
				  m_pTreeView->GetCheckStatusImageId(), GetCheckStatus());
		} 
		if (m_nExtraImageId > 0)
		{
			m_rcCheck.left = rc.left - CHECK_STATUS_IMAGE_WIDTH - 2;
			m_rcCheck.top = rc.top + (m_pTreeView->GetLeafNodeHeight() -CHECK_STATUS_IMAGE_HEIGHT) / 2;
			m_rcCheck.right = rc.left - 2;
			m_rcCheck.bottom = m_rcCheck.top + CHECK_STATUS_IMAGE_HEIGHT;
			CBlueRenderEngineUI::DoPaintGraphic(hDc, m_pTreeView->GetPaintManager(), m_rcCheck,
				m_nExtraImageId, TRUE);
		}
		//节点图片
		m_rcImage.left = rc.left;
		m_rcImage.top = rc.top;
		m_rcImage.right = rc.left + m_pTreeView->GetLeafNodeWidth();
		m_rcImage.bottom = rc.top + m_pTreeView->GetLeafNodeHeight();
 
		BOOL bGray = FALSE;
		if ((m_byteStatus == ONLINE_STATUS_TYPE_OFFLINE) ||
			(m_byteStatus == ONLINE_STATUS_TYPE_HIDE))
			bGray = TRUE;
		BOOL bPersonDraw = FALSE;
		if (m_pTreeView->ShowCustomPicture() && ((!m_pImage.IsEmpty()) || (!m_strPersonImageFile.empty())))
		{ 
			if (bGray)
			{
				if (m_pGrayImage.IsEmpty())
				{
					m_pGrayImage.LoadFromFile(m_strPersonImageFile.c_str(), TRUE);  
			    }
				if (!m_pGrayImage.IsEmpty())
				{
					m_pGrayImage.DrawToDc(hDc, m_rcImage);
					bPersonDraw = TRUE;
				}
			} else
			{
				if (m_pImage.IsEmpty())
			    {
				   m_pImage.LoadFromFile(m_strPersonImageFile.c_str(), FALSE);
				}
				if (!m_pImage.IsEmpty())
				{
					m_pImage.DrawToDc(hDc, m_rcImage);
					bPersonDraw = TRUE;
				}
			}
		}
		if (!bPersonDraw)
		{  
			CBlueRenderEngineUI::DoPaintGraphic(hDc, m_pTreeView->GetPaintManager(), m_rcImage,
				m_pTreeView->GetDefaultImage(bGray), TRUE);
 			 
		}
		//状态绘制
		switch(m_byteStatus)
		{
		case ONLINE_STATUS_TYPE_OFFLINE: //离线状态
		case ONLINE_STATUS_TYPE_HIDE:    //隐身状态 已经直接绘制灰色图片
			 
		case ONLINE_STATUS_TYPE_ONLINE: //在线状态，不做处理
			{
				break;
			}
		default: //其它状态
			{
				int w, h;
				DWORD dwImageId = m_pTreeView->GetStatusImage(w, h);
				if (dwImageId > 0)
				{
					RECT rcDraw;
					rcDraw.left = rc.left + m_pTreeView->GetLeafNodeWidth() - w;
					rcDraw.top = rc.top  + m_pTreeView->GetLeafNodeHeight() - h;
					rcDraw.right = rcDraw.left + w;
					rcDraw.bottom = rcDraw.top + h;
					int idx = STATUS_IMAGE_SEQ_LEAVE;
					if (m_byteStatus == ONLINE_STATUS_TYPE_BUSY)
						idx = STATUS_IMAGE_SEQ_BUSY;
					if (m_byteStatus == ONLINE_STATUS_TYPE_MOBILE)
						idx = STATUS_IMAGE_SEQ_MOBILE;
					//绘制状态图标
					CBlueRenderEngineUI::DoPaintGraphic(hDc, m_pTreeView->GetPaintManager(), rcDraw,
					              dwImageId, idx);					 
				}
				break;
			}
		}


		//背景（选中、focus状态）
		RECT rcTextBkgnd = m_rcItem;
		rcTextBkgnd.left = m_rcImage.right;
		DrawItemBkground( hDc, rcTextBkgnd );
		//绘制昵称
		RECT rcText;
		rcText.left = rc.left + m_pTreeView->GetLeafNodeWidth() + TREE_NODE_VERT_DISTANCE;
		rcText.right = rc.right;
		rcText.top = rc.top  + 1;
		SIZE lsize;
		nLinks = 0;
		::GetTextExtentPoint32(hDc, m_szName, (int)::_tcslen(m_szName), &lsize);
		rcText.bottom = rcText.top + lsize.cy;
		if (m_pTreeView->ShowPersonLabel() &&  m_szLabel
			&& m_pTreeView->GetLeafNodeHeight() == LEAF_NODE_IMAGE_HEIGHT_SMALL)
		{
			TCHAR szwText[MAX_PATH] = {0};
			::lstrcpy(szwText, m_szName);
			if (m_pTreeView->ShowExtractData() && (!m_strExtra.IsEmpty()))
			{
				::lstrcat(szwText, L"-");
				::lstrcat(szwText, m_strExtra.GetData());
			}
			::lstrcat(szwText, L"--");
			::lstrcat(szwText, m_szLabel);
			CBlueRenderEngineUI::DoPaintPrettyText(hDc, m_pTreeView->GetPaintManager(), rcText, szwText, iTextColor, 
			                                       UICOLOR__INVALID, NULL, nLinks, DT_SINGLELINE);
		} else if (m_pTreeView->ShowExtractData() && (!m_strExtra.IsEmpty()))
		{
			TCHAR szwText[MAX_PATH] = {0};
			::lstrcpy(szwText, m_szName);
			::lstrcat(szwText, L"-");
			::lstrcat(szwText, m_strExtra.GetData());
			CBlueRenderEngineUI::DoPaintPrettyText(hDc, m_pTreeView->GetPaintManager(), rcText, szwText, iTextColor, 
			                                       UICOLOR__INVALID, NULL, nLinks, DT_SINGLELINE);
		} else
			CBlueRenderEngineUI::DoPaintPrettyText(hDc, m_pTreeView->GetPaintManager(), rcText, m_szName, iTextColor, 
			                                       UICOLOR__INVALID, NULL, nLinks, DT_SINGLELINE);
		//绘制个性签名
		if (m_pTreeView->ShowPersonLabel() && m_szLabel 
			&& m_pTreeView->GetLeafNodeHeight() == LEAF_NODE_IMAGE_HEIGHT_BIG )
		{
			nLinks = m_nLinksCount;
			rcText.top = rcText.bottom + 1;
			rcText.bottom = rcText.top + lsize.cy;
			iTextColor = UICOLOR_EDIT_TEXT_NORMAL;
			CBlueRenderEngineUI::DoPaintPrettyText(hDc, m_pTreeView->GetPaintManager(), rcText, m_szLabel, iTextColor,
				UICOLOR__INVALID, m_rcLinks, nLinks, DT_SINGLELINE);
		}
		if (m_rcItem.bottom > rc.bottom)
			m_rcItem.bottom = rc.bottom;
	}
}

void CTreeNodeItem::DrawItemBkground(HDC hdc, const RECT& rc)
{
	if (m_NodeType == TREENODE_TYPE_GROUP
		&& !m_pTreeView->GroupNodeSelState())
	{
		return;
	}

	BOOL bDraw = true;
	UITYPE_COLOR iBackColor = UICOLOR_CONTROL_BACKGROUND_NORMAL;
	if (m_NodeType == TREENODE_TYPE_LEAF && m_bIsFocus)
	{
		iBackColor = UICOLOR_CONTROL_BACKGROUND_HOVER;
	} 

	if (m_bSelected)
	{
		iBackColor = UICOLOR_CONTROL_BACKGROUND_SELECTED;
	}
		
	CBlueRenderEngineUI::DoFillRect(hdc,  m_pTreeView->GetPaintManager(), rc, iBackColor, FALSE);
}

//在此框内绘制
void CTreeNodeItem::Draw(HDC hdc, DWORD &dwTop, int &dwScrollPos, DWORD dwLeft, const RECT &rc, 
						 BOOL bNodeRoot, const BOOL &bIsShowCount)
{
	if (m_NodeType == TREENODE_TYPE_GROUP)  //分组节点绘制
	{
		if (!bNodeRoot)
		{
			if ((dwTop >= (int) (dwScrollPos + m_pTreeView->GetTreeViewTop())) &&  ((int)(dwTop - dwScrollPos) < rc.bottom))
			{
				RECT rcPaint = {dwLeft, dwTop - dwScrollPos, rc.right, rc.bottom};
				DrawItem(hdc, rcPaint, bIsShowCount);
			} else
			{
				m_rcItem.left = m_rcItem.right = m_rcItem.top = m_rcItem.bottom = 0;
				memset(&m_rcImage, 0, sizeof(RECT));
				memset(&m_rcCheck, 0, sizeof(RECT));
			}
			dwTop = dwTop +  GROUP_NODE_TEXT_HEIGHT + TREE_NODE_VERT_DISTANCE;
		}
		if (m_bExpanded) //绘制子节点
		{
			dwLeft += TREE_NODE_HORZ_DISTANCE;
			for (DWORD i = 0; i < m_dwNodeCount; i ++)
			{
				m_Nodes[i]->Draw(hdc, dwTop, dwScrollPos, dwLeft, rc, FALSE, bIsShowCount);
			}
		}
	} else if (m_NodeType == TREENODE_TYPE_LEAF)
	{		
		//计算区域
		if ((dwTop >= (int) (dwScrollPos + m_pTreeView->GetTreeViewTop())) && ((int) (dwTop - dwScrollPos) < rc.bottom))
		{
			RECT rcPaint = {dwLeft, dwTop - dwScrollPos, rc.right, rc.bottom};
			DrawItem(hdc, rcPaint, bIsShowCount);
		} else
		{
			m_rcItem.left = m_rcItem.right = m_rcItem.top  = m_rcItem.bottom = 0;
			memset(&m_rcImage, 0, sizeof(RECT));
			memset(&m_rcCheck, 0, sizeof(RECT));
		}
		dwTop = dwTop +  m_pTreeView->GetLeafNodeHeight() + TREE_NODE_VERT_DISTANCE;
	}
}

//设置状态
void CTreeNodeItem::SetStatus(BYTE byteStatus) 
{
	if (m_byteStatus != byteStatus)
	{
		m_byteStatus = byteStatus;
		if (m_pParent)
			m_pParent->OnNodeChange();
		Invalidate();
	}
}

void CTreeNodeItem::SetAllUserStatus(BYTE byteStatus)
{
	if (m_NodeType == TREENODE_TYPE_GROUP)
	{
		for (DWORD i = 0; i < m_dwNodeCount; i ++)
		{
			m_Nodes[i]->SetAllUserStatus(byteStatus); 
		}
	} else
	{
		m_byteStatus = byteStatus;
	}
}

DWORD CTreeNodeItem::GetNodeId()
{
	return m_dwNodeId;
}

void CTreeNodeItem::SetNodeId(DWORD dwNodeId)
{
	m_dwNodeId = dwNodeId;
}

//清除子节点
void CTreeNodeItem::Clear()
{
	for (DWORD i = 0; i < m_dwNodeCount; i ++)
	{
		delete m_Nodes[i];
	}
	delete []m_Nodes;
	m_dwNodeCount = 0;
	m_Nodes = NULL;
}


void CTreeNodeItem::SetFocus(BOOL bIsFocus)
{
	if (m_bIsFocus != bIsFocus)
	{
		m_bIsFocus = bIsFocus;
		//Invalidate();
		if (m_pTreeView)
		{
			HDC hdc = ::GetDC(m_pTreeView->GetPaintManager()->GetPaintWindow());
			DrawItem(hdc, m_rcItem, FALSE, TRUE);  
			::ReleaseDC(m_pTreeView->GetPaintManager()->GetPaintWindow(), hdc);
		}
	}
}

//是否被选择 包括子节点
CTreeNodeItem *CTreeNodeItem::Selected()
{
	if (m_bSelected)
		return this;
	CTreeNodeItem *pItem = NULL;
	for (DWORD i = 0; i < m_dwNodeCount; i ++)
	{
		pItem = m_Nodes[i]->Selected();
		if (pItem)
			break;
	}
	return pItem; 
}

//更新界面
void CTreeNodeItem::Invalidate()
{
	if (m_pTreeView)
		m_pTreeView->InvalidateItem(m_rcItem);
}

//设置是否被选择
void CTreeNodeItem::SetSelect(BOOL bSelected)
{
	if (m_bSelected != bSelected)
	{
		m_bSelected = bSelected;
		Invalidate();
	}
}

RECT CTreeNodeItem::GetImageRect()
{
	return m_rcImage;
}

//这个座标点是否在本节点内 包括子节点
CTreeNodeItem *CTreeNodeItem::XYInItem(int x, int y)
{
	if (XYInRect(x, y, m_rcItem))
	{
		if (m_rcItem.top < (int) m_pTreeView->GetTreeViewTop())
			return NULL;
		return this;
	} else if (m_pTreeView->ShowCheckStatus())
	{
		if (XYInRect(x, y, m_rcCheck))
		{
			if (m_rcCheck.top < (int) m_pTreeView->GetTreeViewTop())
				return NULL;
			return this;
		}		
	}
	if (m_bExpanded)
	{
		CTreeNodeItem *pItem = NULL;
		for (DWORD i = 0; i < m_dwNodeCount; i ++)
		{
			pItem = m_Nodes[i]->XYInItem(x, y);
			if (pItem)
				return pItem;
		}
	}
	return NULL;
}

////清除焦点 不更新界面
void CTreeNodeItem::ClearFocus()
{
	m_bIsFocus = FALSE;
	for(DWORD i = 0; i < m_dwNodeCount; i ++)
	{
		m_Nodes[i]->ClearFocus();
	}
}

	
//清除选择 不更新界面
void CTreeNodeItem::ClearSelected()
{
	m_bSelected = FALSE;
	for(DWORD i = 0; i < m_dwNodeCount; i ++)
	{
		m_Nodes[i]->ClearSelected();
	}
}

//获取下一个节点
CTreeNodeItem *CTreeNodeItem::GetNextNode()
{
	if (m_dwCurrSeq >= m_dwNodeCount)
		return NULL;
	return m_Nodes[m_dwCurrSeq ++];
}

//获取第一个节点
CTreeNodeItem *CTreeNodeItem::GetFirstNode()
{
	m_dwCurrSeq = 0;
	return GetNextNode();
}

//获取第iIndex个节点
CTreeNodeItem *CTreeNodeItem::GetChildNode( int iIndex )
{
	if ((iIndex >= 0) && (iIndex < (int) m_dwNodeCount))
	{
		return m_Nodes[iIndex];
	}
	return NULL;
}

void CTreeNodeItem::SetGraph(const char *szFileName)
{
	m_strPersonImageFile = szFileName; 
}

 
//设置节点名称
void CTreeNodeItem::SetNodeName(const char *szName)
{
	memset(m_szName, 0, sizeof(TCHAR) * MAX_NODE_NAME_SIZE);
	if (szName)
	{
#ifdef _UNICODE
		CStringConversion::StringToWideChar(szName, m_szName, MAX_NODE_NAME_SIZE);
#else
		strncpy(m_szName, szName, MAX_NODE_NAME_SIZE - 1);
#endif
	}
}

void CTreeNodeItem::SetNodeName(const TCHAR *szName) //设置节点名称
{
	memset(m_szName, 0, sizeof(TCHAR) * MAX_NODE_NAME_SIZE);
	if (szName)
	{
		::lstrcpyn(m_szName, szName, MAX_NODE_NAME_SIZE - 1);
	}
}


void CTreeNodeItem::QuickSort(LPSKIN_COMPARENODE pCompare, int nLow, int nHigh)
{
	int i = nLow, j = nHigh;
	CTreeNodeItem *pPivot, *pTemp = NULL;
	pPivot = m_Nodes[i];
	while (i < j)
	{
		while ((i < j ) && (pCompare(m_Nodes[j]->GetNodeType(), m_Nodes[j]->GetStatus(), m_Nodes[j]->GetData(),
			                         pPivot->GetNodeType(), pPivot->GetStatus(), pPivot->GetData()) >= 0))
			j--;
		m_Nodes[i] = m_Nodes[j];
		while ((i < j) && (pCompare(m_Nodes[i]->GetNodeType(), m_Nodes[i]->GetStatus(), m_Nodes[i]->GetData(),
			                        pPivot->GetNodeType(), pPivot->GetStatus(), pPivot->GetData()) <= 0))
			i++;
		m_Nodes[j] = m_Nodes[i];
	}
	m_Nodes[i] = pPivot;
	if (i > nLow)
		QuickSort(pCompare, nLow, i);
	if (i < nHigh)
		QuickSort(pCompare, i + 1, nHigh);
}

void CTreeNodeItem::Sort(LPSKIN_COMPARENODE pCompare, BOOL bRecursived)
{
	if (m_dwNodeCount <= 0)
		return;
	QuickSort(pCompare, 0, m_dwNodeCount - 1);
	if (bRecursived) //子节点排序
	{
		for (DWORD i = 0; i < m_dwNodeCount; i ++)
		{
			if (m_Nodes[i]->GetNodeType() == TREENODE_TYPE_GROUP) //如果是分组节点，则排序
			{
				m_Nodes[i]->Sort(pCompare, bRecursived);
			}
		}
	}
}

//设置个性签名
void CTreeNodeItem::SetLabel(const TCHAR *szLabel)
{
	if (m_szLabel)
		delete []m_szLabel;
	m_szLabel = NULL;
	if (szLabel)
	{
		int nLen = ::lstrlen(szLabel);
		if (nLen > 0)
		{
			m_szLabel = new TCHAR[nLen + 3];
			memset(m_szLabel, 0, sizeof(TCHAR) * (nLen + 3));
			::lstrcpy(m_szLabel, L"(");
			::lstrcat(m_szLabel, szLabel);
			::lstrcat(m_szLabel, L")");
		}
	}
}

//设置个性签名
void CTreeNodeItem::SetLabel(const char *szLabel)
{
	if (m_szLabel)
		delete []m_szLabel;
	m_szLabel = NULL;
	if (szLabel)
	{
		int nLen = strlen(szLabel);
		if (nLen > 0)
		{
			char szTemp[256] = {0};
			sprintf(szTemp, "(%s)", szLabel);
			nLen = strlen(szTemp);
			m_szLabel = new TCHAR[nLen  + 1];
			memset(m_szLabel, 0, sizeof(TCHAR) * (nLen + 1));
#ifdef _UNICODE
			CStringConversion::StringToWideChar(szTemp, m_szLabel, nLen);
#else
			strncpy(m_szLabel, szLabel, nLen);
#endif
		}
	}
}


BOOL CTreeNodeItem::IsExpanded()
{
	return m_bExpanded;
}


//设置节点数据
void CTreeNodeItem::SetNodeData(void *pData)
{
	m_pData = pData;
}

//加入一个新节点
CTreeNodeItem *CTreeNodeItem::AddChildNode(const DWORD dwId, const TCHAR *szName, CTreeNodeType NodeType, const TCHAR *szLabel,
		                void *pData, const TCHAR *szImageFileName, const TCHAR *szExtraData)
{
	if (m_NodeType != TREENODE_TYPE_GROUP)
		return NULL; 
	CTreeNodeItem **pTemp = new CTreeNodeItem *[m_dwNodeCount + 1];
	int nPos = 0;
	if (NodeType == TREENODE_TYPE_LEAF) //插入到前面
	{
		//复制以前的数据
		for(DWORD i = 0; i < m_dwNodeCount; i ++)
		{
			pTemp[i + 1] = m_Nodes[i];
		}
	} else
	{
		//复制以前的数据
		for(DWORD i = 0; i < m_dwNodeCount; i ++)
		{
			pTemp[i] = m_Nodes[i];
		}
		nPos = m_dwNodeCount;
	}
	m_dwNodeCount ++;
    //删除以前的数据
	delete []m_Nodes;
	m_Nodes = pTemp;
	m_Nodes[nPos] = new CTreeNodeItem(this, NodeType, m_pTreeView);
	m_Nodes[nPos]->SetLabel(szLabel);
	m_Nodes[nPos]->SetNodeId(dwId);
	m_Nodes[nPos]->SetNodeName(szName);
	m_Nodes[nPos]->SetNodeData(pData);
	m_Nodes[nPos]->SetExtraData(szExtraData);
	if (szImageFileName)
	{
		char szFileName[MAX_PATH] = {0};
		CStringConversion::WideCharToString(szImageFileName, szFileName, MAX_PATH - 1);
	    m_Nodes[nPos]->SetGraph(szFileName);
	}
	OnNodeChange();
	return m_Nodes[nPos];
}

BOOL CTreeNodeItem::DeleteNodeById(const int nId, CTreeNodeType tnType)
{
	if (m_NodeType != TREENODE_TYPE_GROUP)
		return NULL; 
	int idx = -1; 
	for (DWORD i = 0; i < m_dwNodeCount; i ++)
	{
		if ((m_Nodes[i]->GetNodeType() == tnType) && (m_Nodes[i]->GetNodeId() == nId))
		{
			idx = i;
			break; 
		}
		if ((m_Nodes[i]->GetNodeType() == TREENODE_TYPE_GROUP))
		{
			if (m_Nodes[i]->DeleteNodeById(nId, tnType))
				break;
		}
	} //end for (DWORD i = 0...
	if (idx >= 0)
	{
		DeleteNode(idx);
		OnNodeChange();
		return TRUE;
	}
	return FALSE;
}

//增减一个节点
CTreeNodeItem *CTreeNodeItem::AdjustChildNode(const TCHAR *szName, CTreeNodeType NodeType, void *pData, BOOL bAdd, BOOL bRecursive)
{
	if (m_NodeType != TREENODE_TYPE_GROUP)
		return NULL; 
	int idx = -1;
	if (bAdd)
	{
		CTreeNodeItem *pItem = NULL;
		for (DWORD i = 0; i < m_dwNodeCount; i ++)
		{ 
			if (::stricmp(m_pTreeView->GetNodeKey(m_Nodes[i]->GetNodeType(), m_Nodes[i]->GetData()),
				m_pTreeView->GetNodeKey(m_Nodes[i]->GetNodeType(), pData)) == 0)
			{
				m_Nodes[i]->SetNodeName(szName);
				pItem = m_Nodes[i];
				break;
			} //end if (::stricmp(...
			if (bRecursive && (m_Nodes[i]->GetNodeType() == TREENODE_TYPE_GROUP))
			{
				pItem = m_Nodes[i]->AdjustChildNode(szName, NodeType, pData, bAdd, bRecursive);
				if (pItem)
					break;
			}
		} //end for (DWORD i = 0...
		/*if (!pItem && (NodeType != TREENODE_TYPE_GROUP))
		{
			CTreeNodeItem **pTemp = new CTreeNodeItem *[m_dwNodeCount + 1]; 
			int nPos = 0;
			if (NodeType == TREENODE_TYPE_LEAF) //插入到前面
			{
				//复制以前的数据
				for(DWORD i = 0; i < m_dwNodeCount; i ++)
				{
					pTemp[i + 1] = m_Nodes[i];
				}
			} else
			{
				//复制以前的数据
				for(DWORD i = 0; i < m_dwNodeCount; i ++)
				{
					pTemp[i] = m_Nodes[i];
				}
				nPos = m_dwNodeCount;
			}	 
			m_dwNodeCount ++;
		    //删除以前的数据
			delete []m_Nodes;
			m_Nodes = pTemp;
			m_Nodes[nPos] = new CTreeNodeItem(this, NodeType, m_pTreeView); 
			m_Nodes[nPos]->SetNodeName(szName);
			m_Nodes[nPos]->SetNodeData(pData);
			OnNodeChange();
			return m_Nodes[nPos];
		} else
			return pItem;*/
		return pItem;
	} else 
	{
		for (DWORD i = 0; i < m_dwNodeCount; i ++)
		{
			if (m_Nodes[i]->GetNodeType() == NodeType)
			{
				if (::stricmp(m_pTreeView->GetNodeKey(m_Nodes[i]->GetNodeType(), m_Nodes[i]->GetData()),
					m_pTreeView->GetNodeKey(NodeType, pData)) == 0)
				{ 
					idx = i;
					break;
				} //end if (::stricmp(...
			}
			if (bRecursive && (m_Nodes[i]->GetNodeType() == TREENODE_TYPE_GROUP))
			{
				m_Nodes[i]->AdjustChildNode(szName, NodeType, pData, bAdd, bRecursive); 
			}
		} //end for (DWORD i = 0...
		if (idx >= 0)
		{
			DeleteNode(idx);
			OnNodeChange();
			return NULL;
		}
	}

	return NULL;
}

//加入一个新节点
CTreeNodeItem * CTreeNodeItem::AddChildNode(DWORD dwId, const char *szName, CTreeNodeType NodeType, const char *szLabel, void *pData)
{
	if (m_NodeType != TREENODE_TYPE_GROUP)
		return NULL; 
	CTreeNodeItem **pTemp = new CTreeNodeItem *[m_dwNodeCount + 1];
	int nPos = 0;
	if (NodeType == TREENODE_TYPE_LEAF) //插入到前面
	{
		//复制以前的数据
		for(DWORD i = 0; i < m_dwNodeCount; i ++)
		{
			pTemp[i + 1] = m_Nodes[i];
		}
	} else
	{
		//复制以前的数据
		for(DWORD i = 0; i < m_dwNodeCount; i ++)
		{
			pTemp[i] = m_Nodes[i];
		}
		nPos = m_dwNodeCount;
	}
	m_dwNodeCount ++;
    //删除以前的数据
	delete []m_Nodes;
	m_Nodes = pTemp;
	m_Nodes[nPos] = new CTreeNodeItem(this, NodeType, m_pTreeView);
	m_Nodes[nPos]->SetNodeId(dwId);
	m_Nodes[nPos]->SetLabel(szLabel);
	m_Nodes[nPos]->SetNodeName(szName);
	m_Nodes[nPos]->SetNodeData(pData);
	OnNodeChange();
	return m_Nodes[nPos];
}

//删除一个节点
BOOL CTreeNodeItem::DeleteNode(CTreeNodeItem *pNode)
{
	for (DWORD i = 0; i < m_dwNodeCount; i ++)
	{
		if (m_Nodes[i] == pNode)
		{
			for (DWORD j = i; j < m_dwNodeCount - 1; j ++)
			{
				m_Nodes[j] = m_Nodes[j + 1];
			}
			if (m_pTreeView)
				m_pTreeView->OnDeleteNode(pNode);
			delete pNode;
			m_dwNodeCount --;
			OnNodeChange();
			Invalidate();
			return TRUE;
		}
	}
	return FALSE;
}

//删除节点
BOOL CTreeNodeItem::DeleteNode(int iIndex)
{
	if ((iIndex >= 0) && (iIndex < (int) m_dwNodeCount))
	{
		return DeleteNode(m_Nodes[iIndex] );
	}
	return false;
}

DWORD CTreeNodeItem::GetChildCount()
{
	return m_dwNodeCount;
}

//展开节点
void CTreeNodeItem::Expanded(BOOL bRecursived)
{
	if (!m_bExpanded)
	{
		if (m_dwNodeCount > 0)
		{
			m_bExpanded = TRUE;
			if (bRecursived)
			{
				for (DWORD i = 0; i < m_dwNodeCount; i ++)
				{
					m_Nodes[i]->Expanded(bRecursived);
				}
			}
			Invalidate();
		} //end if (m_dwNodeCount > 0)
	} //end if (!m_bExpanded)
}

TCHAR *CTreeNodeItem::GetName()
{
	return m_szName;
}

TCHAR *CTreeNodeItem::GetLabel()
{
	return m_szLabel;
}

BYTE  CTreeNodeItem::GetStatus() const
{
	return m_byteStatus;
}

CTreeNodeType CTreeNodeItem::GetNodeType() const
{
	return m_NodeType;
}

 
//收缩节点
void CTreeNodeItem::Reduce()
{
	if (m_bExpanded)
	{
		m_bExpanded = FALSE;
		Invalidate();
	}
}

CTreeNodeItem *CTreeNodeItem::GetParent()
{
	return m_pParent;
}

//CUITreeView
CUITreeView::CUITreeView(void):
             m_pFocusNode(NULL),
		     m_IconType(TREENODE_ICON_TYPE_BIG),
			 m_dwLeafNodeWidth(LEAF_NODE_IMAGE_WIDTH_BIG),
			 m_dwLeafNodeHeight(LEAF_NODE_IMAGE_HEIGHT_BIG),
			 m_pGroupMenu(NULL),
			 m_pLeafMenu(NULL),
			 m_pSelectNode(NULL),
			 m_pDragExpandNode(NULL),
			 m_bShowStatData(FALSE), 
			 m_pFreeNodeFun(NULL),
			 m_pGetNodeKeyFun(NULL),
			 m_dwScrollLine(20),
			 m_bShowCheckStatus(FALSE),
			 m_dwGroupImageId(0), 
			 m_dwBigStatusImageId(0),
			 m_dwSmallStatusImageId(0),
			 m_bShowExtraData(FALSE),
			 m_bShowCustomPicture(TRUE),
			 m_dwDefaultImageId(0),
			 m_bShowPersonLabel(TRUE),
			 m_dwCheckStatusImageId(0),
			 m_iInitPos(0),
			 m_bGroupNodeSelState(false)
{
	m_RootNode = new CTreeNodeItem(NULL, TREENODE_TYPE_GROUP, this); 
}

CUITreeView::~CUITreeView(void)
{
	delete m_RootNode;
	DestroyAllMenu();
}

//IUITreeViewApp 虚函数
 

//清除所有节点
void CUITreeView::Clear()
{
	m_pFocusNode = NULL;
	m_pSelectNode = NULL;
	m_RootNode->Clear();
}

int CUITreeView::GetStatusImage(int &w, int &h)
{
	if (m_IconType == TREENODE_ICON_TYPE_SMALL)
	{
		w = 8;
		h = 8;
		return m_dwSmallStatusImageId;
	} else
	{
		w = 16;
		h = 16;
		return m_dwBigStatusImageId; 
	}
}

DWORD CUITreeView::GetDefaultGrpImageId()
{
	return m_dwGroupImageId;
}

 
BOOL CUITreeView::ShowCheckStatus() const
{
	return m_bShowCheckStatus && (m_dwCheckStatusImageId > 0);
}

void CUITreeView::EnableGroupNodeSelState(BOOL bEnable)
{
	m_bGroupNodeSelState = bEnable;
}


BOOL CUITreeView::GroupNodeSelState() const
{
	return m_bGroupNodeSelState;
}

 
//设置释放数据回调
void CUITreeView::SetFreeNodeFun(LPSKIN_FREE_NODE_EXDATA pFun)
{
	m_pFreeNodeFun = pFun;
}

//设置关键词
void CUITreeView::SetGetNodeKeyFun(LPSKIN_GET_TREE_NODE_KEY pFun)
{
	m_pGetNodeKeyFun = pFun;
}

//释放节点扩展数据
BOOL CUITreeView::FreeNodeExtData(TreeNodeType nodeType, void **pData)
{
	if (m_pFreeNodeFun)
		return m_pFreeNodeFun(nodeType, pData);
	return FALSE;
}

CTreeNodeItem *CUITreeView::GetNode()
{
	return m_RootNode;
}

//CControlUI 虚函数
LPCTSTR CUITreeView::GetClass() const
{
	return L"UITREEVIEW";
}

void CUITreeView::Init()
{
	CContainerUI::Init();
	//vertical scrollbar
	EnableScrollBar(UISB_VERT, true);
	LoadAllMenu();
}

void CUITreeView::LoadAllMenu()
{
	DestroyAllMenu();
 
	if (m_pManager)
	{
		if (!m_strGroupMenu.IsEmpty())
		{
			m_pGroupMenu = m_pManager->LoadMenuUI(m_strGroupMenu);
			if (m_pGroupMenu)
			{
				m_pGroupMenu->SetManager(m_pManager, NULL);
				m_pGroupMenu->SetOwner(this);	
			} 
		}
		if (!m_strLeafMenu.IsEmpty())
		{
			m_pLeafMenu = m_pManager->LoadMenuUI(m_strLeafMenu);
			if (m_pLeafMenu)
			{
				m_pLeafMenu->SetManager(m_pManager, NULL);
				m_pLeafMenu->SetOwner(this);
			}
		} //end if (!m_strLeafMenu.IsEmpty())
	} //end if (m_pManager)
}

void CUITreeView::DestroyAllMenu()
{
	if (m_pGroupMenu != NULL)
	{
		if (m_pManager)
			m_pManager->ReleaseMenuUI(&m_pGroupMenu); 
	}
	if (m_pLeafMenu)
	{
		if (m_pManager)
			m_pManager->ReleaseMenuUI(&m_pLeafMenu); 
	}
}

void CUITreeView::InvalidateItem(RECT &rc)
{
	AdjustScrollBarRect();
	Invalidate();
}

//调整滚动条
void CUITreeView::AdjustScrollBarPos(int &dwScrollPos, BOOL bNext)
{
	int dwTop = m_rcItem.top;
	m_RootNode->AdjustScrollPos(dwTop, dwScrollPos);
}

//滚动到某结点展现
void CUITreeView::ScrollToNodeByKey(const char *szKey)
{
	DWORD dwPos = 0;
	if (m_RootNode->CalcHeightToKey(szKey, dwPos))
	{
		//
		m_iInitPos = (int) dwPos;
		MoveToInitPos();
	}
}

void CUITreeView::MoveToInitPos()
{
	if (m_iInitPos > 0)
	{
		int cy = m_rcItem.bottom - m_rcItem.top;
		if (cy > 0)
		{
			if (m_iInitPos > cy)
				m_iInitPos = m_iInitPos  - (cy / 2); //居中
		
			AdjustScrollBarPos(m_iInitPos, TRUE);
			SetScrollPos(UISB_VERT, m_iInitPos);
			Invalidate();
			m_iInitPos = 0;
		} //end if (cy > 0)
	} //end if (m_iInitPos > 0)
}

void CUITreeView::Notify(TNotifyUI &msg)
{
	int iPos = GetScrollPos(UISB_VERT);
	if (msg.sType == _T("lineup"))
	{
		iPos = iPos - GetLeafNodeHeight() - TREE_NODE_VERT_DISTANCE;
		AdjustScrollBarPos(iPos, TRUE);
		SetScrollPos(UISB_VERT, iPos);
		Invalidate();
	} else if (msg.sType == _T("linedown"))
	{
		iPos = iPos + GetLeafNodeHeight() + TREE_NODE_VERT_DISTANCE;
		AdjustScrollBarPos(iPos, TRUE);
		SetScrollPos(UISB_VERT, iPos);
		Invalidate();
	} else if (msg.sType == _T("pageup"))
	{
		iPos = iPos - m_pVScrollBar->GetScrollPage();
		AdjustScrollBarPos(iPos, TRUE);
		SetScrollPos(UISB_VERT, iPos);
		Invalidate();
	} else if (msg.sType == _T("pagedown"))
	{
		iPos = iPos + m_pVScrollBar->GetScrollPage();
		AdjustScrollBarPos(iPos, TRUE);
		SetScrollPos(UISB_VERT, iPos);
		Invalidate();
	} else if (msg.sType == _T("thumbtrack"))
	{
		iPos = (int)msg.wParam;
		AdjustScrollBarPos(iPos, TRUE);
		SetScrollPos(UISB_VERT, iPos);
		Invalidate();
	} else
	{
		CContainerUI::Notify(msg);
	}
}

 
DWORD CUITreeView::GetLeafNodeWidth()
{
	return m_dwLeafNodeWidth;
}

DWORD CUITreeView::GetLeafNodeHeight()
{
	return m_dwLeafNodeHeight;
}

void CUITreeView::SetNodeIconType(CTreeNodeIconType IconType)
{
	if (m_IconType != IconType)
	{
		m_IconType = IconType;
		if (m_IconType == TREENODE_ICON_TYPE_BIG)
		{
			m_dwLeafNodeWidth = LEAF_NODE_IMAGE_WIDTH_BIG;
			m_dwLeafNodeHeight = LEAF_NODE_IMAGE_HEIGHT_BIG;

		} else if (m_IconType == TREENODE_ICON_TYPE_SMALL)
		{
			m_dwLeafNodeWidth = LEAF_NODE_IMAGE_WIDTH_SMALL;
			m_dwLeafNodeHeight = LEAF_NODE_IMAGE_HEIGHT_SMALL;
		} 
	}
}


CTreeNodeIconType CUITreeView::GetNodeIconType() const
{
	return m_IconType;
}

void CUITreeView::SetPos(RECT rc)
{
	CContainerUI::SetPos(rc);//contaier position
	AdjustScrollBarRect();//srcollbar and client area
}

void CUITreeView::AdjustScrollBarRect()
{
	//scrollbar
	ASSERT(m_pVScrollBar != NULL);
	DWORD dwTop = 0;
	m_RootNode->CalcHeight(dwTop);
	m_pVScrollBar->SetScrollRange(0, dwTop > 0 ? dwTop - 1 : 0);
	int cyScroll = m_rcItem.bottom - m_rcItem.top;
	m_pVScrollBar->SetScrollPage(cyScroll);
	CRect rcScrollBar(m_rcItem);
	m_rcClient = m_rcItem;
	if (IsScrollBarVisible(UISB_VERT))
	{
		rcScrollBar.left = rcScrollBar.right - SB_WIDTH;
		m_pVScrollBar->SetPos(rcScrollBar);
		m_rcClient.right -= SB_WIDTH;
		//初始化位置
		MoveToInitPos();
	}
}


SIZE CUITreeView::EstimateSize(SIZE szAvailable)
{
	return CSize( 0, 0 );
}

//事件处理
void CUITreeView::Event(TEventUI& event)
{
	switch(event.Type)
	{
		case UIEVENT_BUTTONDOWN:
			{
				CTreeNodeItem *pNode = m_RootNode->XYInItem(event.ptMouse.x, event.ptMouse.y);
				BOOL bDid = FALSE;
				if (pNode && ShowCheckStatus())
				{
					RECT rc = pNode->GetCheckRect();
					if (XYInRect(event.ptMouse.x, event.ptMouse.y, rc))
					{
						bDid = TRUE;
					}
				}
				if (!bDid)
				{
					SelectNode(pNode);
					if (m_pSelectNode)
					{
						m_pSelectNode->SetSelect(TRUE);
						if (!m_pSelectNode->IsExpanded())
							m_pSelectNode->Expanded(FALSE);
						else
							m_pSelectNode->Reduce();
					}					
				}
				break;
			}
		case UIEVENT_DBLCLICK:
			{
				CTreeNodeItem *pNode = m_RootNode->XYInItem(event.ptMouse.x, event.ptMouse.y);
				SelectNode(pNode);
				if (m_pSelectNode)
				{
					m_pSelectNode->SetSelect(TRUE);
					if (!m_pSelectNode->IsExpanded())
						m_pSelectNode->Expanded(FALSE);
					else
						m_pSelectNode->Reduce();
					if (m_pManager)
					{
						m_pManager->SendNotify(this, _T("lbdblclick"));
					}
				}
				break;
			}
		case UIEVENT_BUTTONUP:
			{
				CTreeNodeItem *pNode = m_RootNode->XYInItem(event.ptMouse.x, event.ptMouse.y);
				SelectNode(pNode);
				if (pNode)
				{
					if (ShowCheckStatus())
					{
						RECT rc = pNode->GetCheckRect();
						if (XYInRect(event.ptMouse.x, event.ptMouse.y, rc))
						{
							int nStatus = pNode->GetCheckStatus();
							if (nStatus != CHECK_STATUS_NORMAL)
								pNode->SetCheckStatus(CHECK_STATUS_NORMAL);
							else
								pNode->SetCheckStatus(CHECK_STATUS_CHECKED);
							if (m_pManager)
								m_pManager->SendNotify(this, _T("statuschange"));
						} else 	if (m_pManager)//end if (XYInRect(...
							m_pManager->SendNotify(this, _T("click"));						
					} else
					{
						RECT rc = pNode->GetImageRect();
						if (XYInRect(event.ptMouse.x, event.ptMouse.y, rc))
						{							
							if (m_pManager)
							{
								if (pNode->GetNodeType() == TREENODE_TYPE_GROUP)
									m_pManager->SendNotify(this, _T("treegroupclick"));
								else
									m_pManager->SendNotify(this, _T("imageclick"));
							}
						} else 	if (m_pManager)
							m_pManager->SendNotify(this, _T("click"));
					} //end else if (ShowCheckStatus())
				} //end if (pNode)
			}
			break;
		case UIEVENT_RBUTTONUP:
			{
				CTreeNodeItem *pNode = m_RootNode->XYInItem(event.ptMouse.x, event.ptMouse.y);
				SelectNode(pNode);
				if (pNode)
				{
					if (pNode->GetNodeType() == TREENODE_TYPE_GROUP)
					{
						if (m_pGroupMenu)
						{
							if (m_pManager)
							{
								m_pManager->SendNotify(m_pGroupMenu, _T("beforemenupop"));
							}	
							POINT pt = {event.ptMouse.x, event.ptMouse.y};
							::ClientToScreen(m_pManager->GetPaintWindow(), &pt);
							m_pGroupMenu->TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y);
						}  //end if (m_pGroupMenu)
					} else
					{
						if (m_pLeafMenu)
						{
							if (m_pManager)
								m_pManager->SendNotify(m_pLeafMenu, _T("beforemenupop"));
							POINT pt = {event.ptMouse.x, event.ptMouse.y};
							::ClientToScreen(m_pManager->GetPaintWindow(), &pt);
							m_pLeafMenu->TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y);
						} //end if (m_pLeafMenu)
					} //end else if (pNode->GetNodeType() == 
				} else 
				{
					if (m_pPopMenu)
					{
						if (m_pManager)
							m_pManager->SendNotify(m_pPopMenu, _T("beforemenupop"));
						POINT pt = {event.ptMouse.x, event.ptMouse.y};
						::ClientToScreen(m_pManager->GetPaintWindow(), &pt);
						m_pPopMenu->TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y);
					}
				}//end if (pNode)
				if (pNode && m_pManager)
				{
					m_pManager->SendNotify(this, _T("rbuttonclick"), (WPARAM)pNode);
				}
				break;
			}
		case UIEVENT_MOUSEMOVE:
			{
				CTreeNodeItem *pNode = m_RootNode->XYInItem(event.ptMouse.x, event.ptMouse.y);
				if (m_pFocusNode != pNode)
				{
					if (m_pFocusNode)
						m_pFocusNode->SetFocus(FALSE);
					m_pFocusNode = pNode;
					if (pNode != NULL)
					{
						if (pNode->GetLabel())
							SetToolTip(pNode->GetLabel());
						else
							SetToolTip(L"");
					} else
						SetToolTip(L"");
				    //if (m_pManager)
					//   m_pManager->SendNotify(this, _T("mousehover"));
				}
				if (m_pFocusNode)
				{
					m_pFocusNode->SetFocus(TRUE);
				}
				break;
			}
		case UIEVENT_DRAG:
			{
				CTreeNodeItem *pNode = m_RootNode->XYInItem(event.ptMouse.x, event.ptMouse.y);
				if (pNode)
				{
					if (!pNode->IsExpanded())
					{
						pNode->Expanded(FALSE);
						if (m_pDragExpandNode)
							m_pDragExpandNode->Reduce();
						m_pDragExpandNode = pNode;
					} //end if (!pNode->
				} //end if (pNode)
				break;
			}
		case UIEVENT_DRAGEND:
			{
				if (event.pSender != NULL) 
				{
					CTreeNodeItem *pParent = m_RootNode->XYInItem(event.ptMouse.x, event.ptMouse.y); 
					if (pParent && pParent->GetNodeType() == TREENODE_TYPE_GROUP)
					{
						if (event.pSender != this)
						{
							CUITreeView *pSrc = dynamic_cast<CUITreeView *>(event.pSender);
							if (pSrc)
							{
								CTreeNodeItem *pNode = pSrc->GetSelected();
								if (pNode)
								{
									//
									pParent->AddChildNode(pNode->GetNodeId(), pNode->GetName(), pNode->GetNodeType(),
										NULL, NULL, NULL, NULL);
								}
							} //end if (pSrc)
						} else
						{
							//
						} //end 
					}
				} //end if (event.pSender != NULL
				m_pDragExpandNode = NULL;
				break;
			}
		case UIEVENT_MOUSEHOVER:
			{
				if (m_pManager)
					m_pManager->SendNotify(this, _T("mousehover"));
				break;
			}
		case UIEVENT_MOUSELEAVE:
			{
				if (m_pManager)
					m_pManager->SendNotify(this, _T("mouseleave"));
				break;
			}
		case UIEVENT_SCROLLWHEEL: //
			{
				WORD l = LOWORD(event.wParam);
				switch(l)
				{
					case SB_LINEDOWN:
						{	
							int iPos = GetScrollPos(UISB_VERT);
							iPos = iPos +  GetLeafNodeHeight() + TREE_NODE_VERT_DISTANCE;
							AdjustScrollBarPos(iPos, TRUE);
							SetScrollPos( UISB_VERT, iPos);
							Invalidate();
							break;
						}
					case SB_LINEUP:
						{
							int iPos = GetScrollPos(UISB_VERT);
							iPos = iPos - GetLeafNodeHeight() - TREE_NODE_VERT_DISTANCE;
							AdjustScrollBarPos(iPos, TRUE);
							SetScrollPos( UISB_VERT, iPos);
							Invalidate();
							break;
						}
				}
				break;
			}
		case UIEVENT_RBUTTONDOWN:
			break;
		default: 
			CContainerUI::Event(event); //父类 
	}
}

CPaintManagerUI *CUITreeView::GetPaintManager()
{
	return m_pManager;
}

CTreeNodeItem *CUITreeView::GetSelected()
{
	return m_pSelectNode;
}

void CUITreeView::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcsicmp(pstrName, L"groupimageid") == 0)
	{
		m_dwGroupImageId = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, L"defaultimageid") == 0)
	{
		m_dwDefaultImageId = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, L"grayimageid") == 0)
	{
		m_dwDefaultGrayImageId = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, L"bigstatusimage") == 0)
	{
		m_dwBigStatusImageId = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, L"smallstatusimage") == 0)
	{
		m_dwSmallStatusImageId = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, L"showcheckstatus") == 0)
	{
		if (_tcsicmp(pstrValue, L"true") == 0)
			m_bShowCheckStatus = TRUE;
		else
			m_bShowCheckStatus = FALSE;
	} else if (_tcsicmp(pstrName, L"checkstatusimageid") == 0)
	{
		m_dwCheckStatusImageId = _ttoi(pstrValue);
	} else if (_tcsicmp(pstrName, L"icontype") == 0)
	{
		if (_tcsicmp(pstrValue, L"small") == 0)
			SetNodeIconType(TREENODE_ICON_TYPE_SMALL);
		else
			SetNodeIconType(TREENODE_ICON_TYPE_BIG);
	} else if (_tcsicmp(pstrName, L"groupmenu") == 0)
	{
		m_strGroupMenu = pstrValue;
	} else if (_tcsicmp(pstrName, L"leafmenu") == 0)
	{
		m_strLeafMenu = pstrValue;
	} else if (_tcsicmp(pstrName, L"showextradata") == 0)
	{
		if (_tcsicmp(pstrValue, L"true") == 0)
			m_bShowExtraData = TRUE;
		else
			m_bShowExtraData = FALSE;
	} else if (_tcsicmp(pstrName, L"showcustompic") == 0)
	{
		if (_tcsicmp(pstrValue, L"false") == 0)
			m_bShowCustomPicture = FALSE;
		else
			m_bShowCustomPicture = TRUE;
	} else if (_tcsicmp(pstrName, L"showpersonlabel") == 0)
	{
		m_bShowPersonLabel = (_tcsicmp(pstrValue, L"true") == 0);
	} else
	{
		CContainerUI::SetAttribute(pstrName, pstrValue);
	}
}

BOOL CUITreeView::ShowExtractData()
{
	return m_bShowExtraData;
}

//
BOOL CUITreeView::GetUserCount(void *pParentNode, DWORD &dwTotalCount, DWORD &dwOnlineCount)
{
	if (pParentNode)
	{
		CTreeNodeItem *pItem = static_cast<CTreeNodeItem *>(pParentNode);
		pItem->StatChildNode(dwTotalCount, dwOnlineCount);
	} else
		m_RootNode->StatChildNode(dwTotalCount, dwOnlineCount);
	return TRUE;
}

BOOL CUITreeView::DeleteNodeById(const int nId, CTreeNodeType tnType)
{
	//
	return m_RootNode->DeleteNodeById(nId, tnType);
}

CTreeNodeItem *CUITreeView::SelectNode(CTreeNodeItem* pNode)
{
	CTreeNodeItem* pPreSelected = m_pSelectNode;
	if (m_pSelectNode != pNode)
	{
		if (m_pSelectNode)
		{
			m_pSelectNode->SetSelect(FALSE);
		}
		m_pSelectNode = pNode;
		if (m_pSelectNode)
		{
			m_pSelectNode->SetSelect(TRUE);
		}
		if (m_pManager)
		{
			m_pManager->SendNotify(this, _T("itemselect"));
		}
	}
	return pPreSelected;
}

//获取默认的图像
DWORD CUITreeView::GetDefaultImage(BOOL bGray)
{
	if (bGray)
		return m_dwDefaultGrayImageId;
	else
		return m_dwDefaultImageId;
}

void CUITreeView::SetDefaultImage(const char *szFileName)
{
	//
}

//获取当前焦点节点
CTreeNodeItem *CUITreeView::GetFocusNode()
{
	return m_pFocusNode;
}

//设置是否显示统计数据
void CUITreeView::SetShowStatData(BOOL bIsShow)
{
	if (m_bShowStatData != bIsShow)
	{
		m_bShowStatData = bIsShow;
		Invalidate();
	}
}

void CUITreeView::OnDeleteNode(CTreeNodeItem *pNode)
{
	if (pNode == m_pSelectNode)
		m_pSelectNode = NULL;
	if (pNode == m_pFocusNode)
		m_pFocusNode = NULL;
}

const char *CUITreeView::GetNodeKey(TreeNodeType nodeType, const void *pData)
{
	if (m_pGetNodeKeyFun)
		return m_pGetNodeKeyFun(nodeType, pData);
	return NULL;
}

BOOL CUITreeView::GetNodeByKey(void *pParentNode, const char *szKey, TCHAR *szName, int *nNameLen,
	                           void **pSelNode, CTreeNodeType *tnType, void **pData)
{
	CTreeNodeItem *pItem = NULL;
	if (pParentNode)
	{
		pItem = (CTreeNodeItem *)pParentNode;
	} else
	{
		pItem = m_RootNode;
	}
	return pItem->GetNodeByKey(szKey, szName, nNameLen, pSelNode, tnType, pData);
}

CTreeNodeItem *CUITreeView::AdjustChildNode(void *pParentNode, const TCHAR *szName, 
	                             CTreeNodeType NodeType, void *pData, BOOL bAdd, BOOL bRecursive)
{
	CTreeNodeItem *pItem = NULL;
	if (pParentNode)
	{
		pItem = (CTreeNodeItem *)pParentNode;
	} else
	{
		pItem = m_RootNode;
	}
	if (pItem->AdjustChildNode(szName, NodeType, pData, bAdd, bRecursive) == NULL)
	{
		if (bAdd)
			return pItem->AddChildNode(0, szName, NodeType, NULL, pData, NULL, NULL);
	} // 
	return NULL;
}

BOOL CUITreeView::SortTree(void *pNode, LPSKIN_COMPARENODE pCompare,
	                            BOOL bRecursive, BOOL bParent)
{
	if (pNode)
	{
		CTreeNodeItem *pItem = (CTreeNodeItem *)pNode;
		if (bParent)
		{
			CTreeNodeItem *pParentNode = pItem->GetParent();
			if (pParentNode)
				pParentNode->Sort(pCompare, bRecursive);
		} else
			pItem->Sort(pCompare, bRecursive);
	} else
	{
		m_RootNode->Sort(pCompare, bRecursive);
	}
	return TRUE;
}

DWORD CUITreeView::GetTreeViewTop()
{
	return m_rcItem.top;
}

DWORD CUITreeView::GetCheckStatusImageId()
{
	return m_dwCheckStatusImageId;
}

//是否显示扩展数据
void CUITreeView::SetShowExtraData(BOOL bIsShow) //
{
	m_bShowExtraData = bIsShow;
}

//
LPVOID CUITreeView::SetExtraData(const char *szKey, const TCHAR *szExtraData, BOOL bMulti)
{
	return m_RootNode->UpdateExtraData(szKey, szExtraData, bMulti);
}

//
LPVOID CUITreeView::SetImageFile(const char *szKey, const char *szImageFileName, BOOL bMulti)
{
	return m_RootNode->UpdateImageFile(szKey, szImageFileName, bMulti);
}

BOOL CUITreeView::SetTreeViewStatusOffline()
{
	m_RootNode->SetAllUserStatus(ONLINE_STATUS_TYPE_OFFLINE);
	return TRUE;
}

BOOL CUITreeView::ShowCustomPicture()
{
	return m_bShowCustomPicture;
}
	 
//
LPVOID CUITreeView::SetExtraImageFile(const char *szKey, const int nImageId, BOOL bMulti)
{
	return m_RootNode->UpdateExtraImageFile(szKey, nImageId, bMulti);
}

void CUITreeView::SelectedAll(void *pNode, BOOL bRecursive)
{
	CTreeNodeItem *pItem = m_RootNode;
	if (pNode)
		pItem = (CTreeNodeItem *)pNode;
	pItem->SelectAll(bRecursive);
}

void CUITreeView::UnSelected(void *pNode, BOOL bRecursive)
{
	CTreeNodeItem *pItem = m_RootNode;
	if (pNode)
		pItem = (CTreeNodeItem *)pNode;
	pItem->UnSelected(bRecursive);
}

void CUITreeView::DeleteSelected(BOOL bRecursive)
{
	m_RootNode->DeleteSelected(bRecursive);
}

BOOL CUITreeView::ShowPersonLabel()
{
	return m_bShowPersonLabel;
}

//
BOOL CUITreeView::GetTreeNodeById(const DWORD dwId, CTreeNodeType tnType, void **pNode, void **pData)
{
	return m_RootNode->GetNodeById(dwId, tnType, pNode, pData);
}

void CUITreeView::GetSelectUsers(std::string &strUsers, BOOL bRecursive)
{
	m_RootNode->GetSelectUserList(strUsers, bRecursive);
}

void CUITreeView::DoPaint(HDC hDC, const RECT& rcPaint)
{
	RECT rcTemp = {0};
	if (!::IntersectRect(&rcTemp, &rcPaint, &m_rcItem)) 
		return;
	//Border and background
	if (HasBorder())
	{
		UINT uState = 0;
		if(IsFocused()) uState |= UISTATE_FOCUSED;
		CBlueRenderEngineUI::DoPaintEditBox(hDC, m_pManager, m_rcItem, uState);
	} 
	if (m_nBkgImageId > 0) 
		CBlueRenderEngineUI::DoPaintGraphic(hDC, m_pManager, m_rcItem, m_nBkgImageId, m_stretchFixed, m_nStretchMode);
	if (m_nBkgLeftImageId > 0)
	{
		RECT rc = m_rcItem;
		rc.bottom -= m_szLeft.cy;
		rc.left += m_szLeft.cx;
		CBlueRenderEngineUI::DoPaintGraphic(hDC, m_pManager, rc, m_nBkgLeftImageId, PA_BOTTOMLEFT);
	} 
	if (m_nBkgRightImageId > 0)
	{
		RECT rc = m_rcItem;
		rc.right -= m_szRight.cx;
		rc.bottom -= m_szRight.cy;
		CBlueRenderEngineUI::DoPaintGraphic(hDC, m_pManager, rc, m_nBkgRightImageId, PA_BOTTOMRIGHT);
	}
	//scrollbar
	int iScrollPos = 0;
	RECT rc = m_rcClient;
	//处理边界值
	rc.left += BORDER_CX;
	if (ShowCheckStatus())
		rc.left += CHECK_STATUS_BORDER;
	rc.bottom -= BORDER_CY;
	rc.top += BORDER_CY;
	rc.right -= BORDER_CX;
	if (IsScrollBarVisible(UISB_VERT))
	{
		m_pVScrollBar->DoPaint(hDC, rcPaint);
		iScrollPos = m_pVScrollBar->GetScrollPos();
		rc.right -= SB_WIDTH;
	}
	//all nodes
	ASSERT( m_RootNode ); 
	DWORD dwTop = rc.top;
	int iNewScrollPos = iScrollPos;
	m_RootNode->Draw(hDC, dwTop, iNewScrollPos, rc.left, rc, TRUE, m_bShowStatData);
}

//导出一个文件
void CUITreeView::SaveToFile(char *szFileName, BYTE byteFileType)
{
	ofstream ofs;
#ifdef _UNICODE
	TCHAR szTemp[MAX_PATH] = {0};
	CStringConversion::StringToWideChar(szFileName, szTemp, MAX_PATH);
	ofs.open(szTemp, std::ios::out | std::ios::binary);
#endif
	if (ofs.is_open())
	{
		m_RootNode->SaveToStream(ofs, "", byteFileType);
		ofs.close();
	}
}

#pragma warning(default:4996)
