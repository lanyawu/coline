#include "PendingMsgList.h"
#include "InterfaceAnsiString.h"
#include <Commonlib/stringutils.h>
#include <map>
#pragma warning(disable:4996)

//CUserMessageTip
CUserMessageTip::CUserMessageTip():
                 m_nMsgCount(0)
{
}

CUserMessageTip::~CUserMessageTip()
{
}

STDMETHODIMP CUserMessageTip::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IUserPendMessageTip)))
	{
		*ppv = (IUserPendMessageTip *) this;
		_AddRef();
		return S_OK;
	} 
	return E_NOINTERFACE;
}

STDMETHODIMP CUserMessageTip::SetUserName(const char *szUserName)
{
	if (szUserName)
		m_strUserName = szUserName;
	else
		m_strUserName.clear();
	return S_OK;
}

STDMETHODIMP CUserMessageTip::SetMessageTip(const char *szMsgTip)
{
	if (szMsgTip)
		m_strMsgTip = szMsgTip;
	else
		m_strMsgTip.clear();
	return S_OK;
}

STDMETHODIMP CUserMessageTip::SetMsgCount(const int nMsgCount)
{
	m_nMsgCount = nMsgCount;
	return S_OK;
}

STDMETHODIMP_(int) CUserMessageTip::GetMsgCount()
{
	return m_nMsgCount;
}

//
void CUserMessageTip::IncCount()
{
	m_nMsgCount++;
}

//
STDMETHODIMP CUserMessageTip::GetUserName(IAnsiString *szUserName)
{
	if (!m_strUserName.empty())
	{
		return szUserName->SetString(m_strUserName.c_str());
	}
	return E_FAIL;
}

const char *CUserMessageTip::GetUserName()
{
	return m_strUserName.c_str();
}

STDMETHODIMP CUserMessageTip::GetMessageTip(IAnsiString *strMsgTip)
{
	if (!m_strMsgTip.empty())
		return strMsgTip->SetString(m_strMsgTip.c_str());
	return E_FAIL;
}

//CUserMessageTipList
CUserMessageTipList::CUserMessageTipList()
{
}

CUserMessageTipList::~CUserMessageTipList()
{
	std::list<IUserPendMessageTip *>::iterator it;
	for (it = m_TipList.begin(); it != m_TipList.end(); it ++)
	{
		(*it)->Release();
	}
	m_TipList.clear();
}

STDMETHODIMP CUserMessageTipList::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IUserPendMessageTipList)))
	{
		*ppv = (IUserPendMessageTipList *) this;
		_AddRef();
		return S_OK;
	} 
	return E_NOINTERFACE;
}

STDMETHODIMP CUserMessageTipList::GetFrontMessage(IUserPendMessageTip *pTip)
{
	if (!m_TipList.empty())
	{
		IUserPendMessageTip *p = m_TipList.front();
		m_TipList.pop_front();
		if (p)
		{
			CInterfaceAnsiString strTmp;
			if (SUCCEEDED(p->GetMessageTip(&strTmp)))
				pTip->SetMessageTip(strTmp.GetData());
			if (SUCCEEDED(p->GetUserName(&strTmp)))
				pTip->SetUserName(strTmp.GetData());
			pTip->SetMsgCount(p->GetMsgCount());
			p->Release();
			return S_OK;
		}
	}
	return E_FAIL;
}

STDMETHODIMP_(int) CUserMessageTipList::GetPendMsgTipCount()
{
	return m_TipList.size();
}

STDMETHODIMP CUserMessageTipList::AddPendMsgTip(IUserPendMessageTip *pTip, BOOL bCopy)
{
	IUserPendMessageTip *pItem = NULL;
	if (bCopy)
	{
		CUserMessageTip *p = new CUserMessageTip();
		p->QueryInterface(__uuidof(IUserPendMessageTip), (void **)&pItem);
		CInterfaceAnsiString strTmp;
		if (SUCCEEDED(pTip->GetMessageTip(&strTmp)))
			pItem->SetMessageTip(strTmp.GetData());
		if (SUCCEEDED(pTip->GetUserName(&strTmp)))
			pItem->SetUserName(strTmp.GetData());
		pItem->SetMsgCount(pTip->GetMsgCount());
	} else
		pItem = pTip;
	if (pItem)
	{
		m_TipList.push_back(pItem);
		return S_OK;
	}
	return E_FAIL;
}

//CPendingMsgList
CPendingMsgList::CPendingMsgList(void)
{
}


CPendingMsgList::~CPendingMsgList(void)
{
}

BOOL CPendingMsgList::AddItem(const char *szFromName, const char *szType, const char *szBuf)
{
	if (szFromName && szType && szBuf)
	{
		CPendingMsgItem *pItem = new CPendingMsgItem();
		pItem->m_strFromName = szFromName;
		pItem->m_strType = szType;
		pItem->m_strProtocol = szBuf;
		CGuardLock::COwnerLock guard(m_Lock);
		m_PendingList.push_back(pItem);
		return TRUE;
	}
	return FALSE;
}

BOOL CPendingMsgList::GetFrontProtocolByName(const char *szFromName, const char *szType,
	                                          IAnsiString *strProtocol, BOOL bPop)
{
	if (szFromName && szType)
	{
		CGuardLock::COwnerLock guard(m_Lock);
		std::vector<CPendingMsgItem *>::iterator it;
		for (it = m_PendingList.begin(); it != m_PendingList.end(); it ++)
		{
			if ((::stricmp(szFromName, (*it)->m_strFromName.c_str()) == 0)
				&& (::stricmp(szType, (*it)->m_strType.c_str()) == 0))
			{
				if (strProtocol)
				{
					strProtocol->SetString((*it)->m_strProtocol.c_str());
					if (bPop)
						m_PendingList.erase(it);
				}
				return TRUE;
			} //end if ((::stricmp(szFromName...
		} //end for (it = m_PendingList.begin
	} //end if (szFromName && szType)
	return FALSE;
}

//
BOOL CPendingMsgList::GetMessageTipList(IUserPendMessageTipList *pList)
{
	std::map<CAnsiString_, CUserMessageTip *> list;
	std::map<CAnsiString_, CUserMessageTip *>::iterator ltIt;
	CGuardLock::COwnerLock guard(m_Lock);
	std::vector<CPendingMsgItem *>::iterator it;
	for (it = m_PendingList.begin(); it != m_PendingList.end(); it ++)
	{
		ltIt = list.find((*it)->m_strFromName.c_str());
		if (ltIt == list.end())
		{
			CUserMessageTip *p = new CUserMessageTip();
			p->SetUserName((*it)->m_strFromName.c_str());
			p->IncCount();
			list.insert(std::pair<CAnsiString_, CUserMessageTip *>((*it)->m_strFromName.c_str(), p));
		} else
			ltIt->second->IncCount();
	}
	for (ltIt = list.begin(); ltIt != list.end(); ltIt ++)
	{
		pList->AddPendMsgTip(ltIt->second, FALSE);
	}
	list.clear();
	return TRUE;
}

BOOL CPendingMsgList::GetLastProtocol(IAnsiString *strFromName, IAnsiString *strType)
{
	CGuardLock::COwnerLock guard(m_Lock);
	if (!m_PendingList.empty())
	{
		CPendingMsgItem *pItem = m_PendingList.back();
		strFromName->SetString(pItem->m_strFromName.c_str());
		strType->SetString(pItem->m_strType.c_str());
		return TRUE;
	}
	return FALSE;
}

#pragma warning(default:4996)
