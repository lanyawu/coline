#include "InterfaceUserList.h"

#pragma warning(disable:4996)

CInterfaceUserList::CInterfaceUserList(void)
{
}


CInterfaceUserList::~CInterfaceUserList(void)
{
	Clear();
}

void CInterfaceUserList::Clear()
{
	LPORG_TREE_NODE_DATA pData;
	while (!m_UserList.empty())
	{
		pData = m_UserList.back();
		delete pData;
		m_UserList.pop_back();
	}
	while(!m_DeptList.empty())
	{
		pData = m_DeptList.back();
		delete pData;
		m_DeptList.pop_back();
	}
}

//IUnknown
STDMETHODIMP CInterfaceUserList::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if (::IsEqualGUID(riid, IID_IUnknown) || ::IsEqualGUID(riid, __uuidof(IUserList)))
	{
		_AddRef();
		*ppv = (IUserList *) this;
		return S_OK;
	}
	return E_NOINTERFACE;
}

BOOL CInterfaceUserList::CompareSub(IUserList *pCompare, IUserList *pAdd, IUserList *pSub)
{
	std::list<LPORG_TREE_NODE_DATA>::iterator it;
	pCompare->CopyTo(pSub, TRUE);
	LPORG_TREE_NODE_DATA pData;
	for (it = m_UserList.begin(); it != m_UserList.end(); it ++)
	{
		if (FAILED(pSub->PopsUserByName((*it)->szUserName, &pData)))
		{
			pAdd->AddUserInfo((*it), TRUE);
		} else
		{
			if (pData->szDisplayName)
				delete []pData->szDisplayName;
			delete pData;
		} 
	}
	return TRUE;
}

STDMETHODIMP CInterfaceUserList::PopsUserByName(const char *szUserName, LPORG_TREE_NODE_DATA *pData)
{
	std::list<LPORG_TREE_NODE_DATA>::iterator it;
	for (it = m_UserList.begin(); it != m_UserList.end(); it ++)
	{
		if (::stricmp(szUserName, (*it)->szUserName) == 0)
		{
			*pData = (*it);
			m_UserList.erase(it);
			return S_OK;
		}
	}
	return E_FAIL;
}

STDMETHODIMP_(BOOL) CInterfaceUserList::UserIsExists(const char *szUserName)
{
	std::list<LPORG_TREE_NODE_DATA>::iterator it;
	for (it = m_UserList.begin(); it != m_UserList.end(); it ++)
	{
		if (::stricmp(szUserName, (*it)->szUserName) == 0)
			return TRUE;
	}
	return FALSE;
}

//
STDMETHODIMP CInterfaceUserList::AddUserInfo(LPORG_TREE_NODE_DATA pData, BOOL bCopy) 
{
	LPORG_TREE_NODE_DATA pItem = pData;
	if (bCopy)
	{
		pItem = new ORG_TREE_NODE_DATA();
		memset(pItem, 0, sizeof(ORG_TREE_NODE_DATA));
		pItem->bOpened = pData->bOpened;
		pItem->id = pData->id;
		pItem->nDisplaySeq = pData->nDisplaySeq;
		pItem->nStamp = pData->nStamp;
		pItem->pid = pData->pid;
		strcpy(pItem->szUserName, pData->szUserName);
		strcpy(pItem->szCell, pData->szCell);
		if (pData->szDisplayName)
		{
			int nSize = ::strlen(pData->szDisplayName);
			pItem->szDisplayName = new char[nSize + 1];
			strcpy(pItem->szDisplayName, pData->szDisplayName);
			pItem->szDisplayName[nSize] = '\0';
		}
	}
	m_UserList.push_back(pItem);
	return S_OK;
}

STDMETHODIMP CInterfaceUserList::AddDeptInfo(LPORG_TREE_NODE_DATA pData, BOOL bCopy) 
{
	LPORG_TREE_NODE_DATA pItem = pData;
	if (bCopy)
	{ 
		pItem = new ORG_TREE_NODE_DATA();
		memset(pItem, 0, sizeof(ORG_TREE_NODE_DATA));
		pItem->bOpened = pData->bOpened;
		pItem->id = pData->id;
		pItem->nDisplaySeq = pData->nDisplaySeq;
		pItem->nStamp = pData->nStamp;
		pItem->pid = pData->pid;
		strcpy(pItem->szUserName, pData->szUserName);
		if (pData->szDisplayName)
		{
			int nSize = ::strlen(pData->szDisplayName);
			pItem->szDisplayName = new char[nSize + 1];
			strcpy(pItem->szDisplayName, pData->szDisplayName);
			pItem->szDisplayName[nSize] = '\0';
		}
	}
	m_DeptList.push_back(pItem);
	return S_OK;
}

STDMETHODIMP CInterfaceUserList::PopBackUserInfo(LPORG_TREE_NODE_DATA *pData) 
{
	if (!m_UserList.empty())
	{
		*pData = m_UserList.back();
		m_UserList.pop_back();
		return S_OK;
	}
	return E_FAIL;
}

STDMETHODIMP CInterfaceUserList::PopBackDeptInfo(LPORG_TREE_NODE_DATA *pData) 
{
	if (!m_DeptList.empty())
	{
		*pData = m_DeptList.back();
		m_DeptList.pop_back();
		return S_OK;
	}
	return E_FAIL;
}

STDMETHODIMP CInterfaceUserList::PopFrontUserInfo(LPORG_TREE_NODE_DATA *pData)
{
	if (!m_UserList.empty())
	{
		*pData = m_UserList.front();
		m_UserList.pop_front();
		return S_OK;
	}
	return E_FAIL;
}

STDMETHODIMP CInterfaceUserList::PopFrontDeptInfo(LPORG_TREE_NODE_DATA *pData)
{
	if (!m_DeptList.empty())
	{
		*pData = m_DeptList.front();
		m_DeptList.pop_front();
		return S_OK;
	}
	return E_FAIL;
}

STDMETHODIMP_(DWORD) CInterfaceUserList::GetUserCount()
{
	return m_UserList.size();
}

STDMETHODIMP_(DWORD) CInterfaceUserList::GetDeptCount()
{
	return m_DeptList.size();
}

STDMETHODIMP CInterfaceUserList::SetDeptId(const char *szPid) 
{
	if (szPid)
		m_strPid = szPid;
	else
		m_strPid = "";
	return S_OK;
}

STDMETHODIMP CInterfaceUserList::GetDeptId(IAnsiString *strPid)
{
	if ((!m_strPid.empty()) && strPid)
	{
		strPid->SetString(m_strPid.c_str());
		return S_OK;
	}
	return E_FAIL;
		
}

BOOL CInterfaceUserList::DeleteUserInfo(const char *szUserName)
{
	std::list<LPORG_TREE_NODE_DATA>::iterator it;
	for (it = m_UserList.begin(); it != m_UserList.end(); it ++)
	{
		if (stricmp((*it)->szUserName, szUserName) == 0)
		{
			if ((*it)->szDisplayName)
				delete [](*it)->szDisplayName;
			delete (*it);
			m_UserList.erase(it);
			return TRUE;
		}
	}
	return FALSE;
}

 
STDMETHODIMP CInterfaceUserList::CopyTo(IUserList *pDst, BOOL bCopy)
{
	std::list<LPORG_TREE_NODE_DATA>::iterator it;
	for (it = m_UserList.begin(); it != m_UserList.end(); it ++)
	{ 
		pDst->AddUserInfo((*it), bCopy);
	}
	for (it = m_DeptList.begin(); it != m_DeptList.end(); it ++)
	{
		pDst->AddDeptInfo((*it), bCopy);
	} 
	return S_OK;
}

BOOL CInterfaceUserList::AddUserInfo(LPORG_TREE_NODE_DATA pData, BOOL bCopy, BOOL bChkExists)
{
	if (bChkExists && UserIsExists(pData->szUserName))
		return FALSE;
	return SUCCEEDED(AddUserInfo(pData, bCopy));

}

#pragma warning(default:4996)
