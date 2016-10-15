#include "GroupItem.h"

#pragma warning(disable:4996)

CGroupItem::CGroupItem(const char *szGrpId, HWND hWnd):
	        m_hWnd(hWnd),
			m_bInitAckMenu(FALSE)
{
	if (szGrpId)
		m_strGrpId = szGrpId;
}


CGroupItem::~CGroupItem(void)
{
}


void CGroupItem::SetCreator(const char *szCreator)
{
	if (szCreator)
		m_strCreator = szCreator;
	else
		m_strCreator = "";
}

void CGroupItem::SetOwnerHWND(HWND hWnd)
{
	m_hWnd = hWnd;
}

void CGroupItem::SetDispName(const char *szDspName)
{
	m_strDspName = szDspName;
}

const char *CGroupItem::GetGroupId()
{
	return m_strGrpId.c_str();
}

const char *CGroupItem::GetCreator()
{
	return m_strCreator.c_str();
}

const char *CGroupItem::GetDispName()
{
	return m_strDspName.c_str();
}


void CGroupItem::SetNewUserList(IUserList *pUsers)
{
	m_UserList.Clear();
	LPORG_TREE_NODE_DATA pData;
	while (SUCCEEDED(pUsers->PopBackUserInfo(&pData)))
	{
		if (!m_UserList.AddUserInfo(pData, FALSE, TRUE))
		{
			if (pData->szDisplayName)
				delete [] pData->szDisplayName;
			delete pData;
		}
	}
}

BOOL CGroupItem::GetUserList(IUserList *pUsers)
{
	m_UserList.CopyTo(pUsers, TRUE);
	return TRUE;
}

BOOL CGroupItem::DeleteGroupUser(const char *szUserId)
{
	return m_UserList.DeleteUserInfo(szUserId);
}

BOOL CGroupItem::AddGroupUser(const char *szUserId, const char *szDspName)
{
	LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
	memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
	strncpy(pData->szUserName, szUserId, MAX_USER_NAME_SIZE - 1);
	if (szDspName)
	{
		pData->szDisplayName = new char[::strlen(szDspName) + 1];
		strcpy(pData->szDisplayName, szDspName);
		pData->szDisplayName[::strlen(szDspName)] = '\0';
	}
	if (!m_UserList.AddUserInfo(pData, FALSE, TRUE))
	{
		if (pData->szDisplayName)
			delete []pData->szDisplayName;
		delete pData;
		return FALSE;
	} else
		return TRUE;
}

BOOL CGroupItem::IsGroupFrame(const char *szGrpId)
{
	if (!m_strGrpId.empty() && szGrpId)
		return (::stricmp(szGrpId, m_strGrpId.c_str()) == 0);
	return FALSE;
}

HWND CGroupItem::GetHWND()
{
	return m_hWnd;
}

BOOL CGroupItem::DrawUsersToUI()
{
	return FALSE;
}

#pragma warning(default:4996)
