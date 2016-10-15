#include "MessageList.h"

#pragma warning(disable:4996)

CMessageList::CMessageList(void) 
{
	//
}


CMessageList::~CMessageList(void)
{
	Clear();
}

int GetDate(const char **p)
{
	const char *p1 = *p;
	while (*p1)
	{
		if ((*p1 == '/') || (*p1 == '-') || (*p1 == ' ')
			|| (*p1 == ':'))
			break;
		p1 ++;
	}
	char szTmp[6] = {0};
	int nSize = p1 - *p;
	if ((nSize > 0) && (nSize < 5))
	{
		strncpy(szTmp, *p, nSize);
		if (*p1)
			p1 ++;
		*p = p1;		
		return ::atoi(szTmp);
	}
	return 0;
}

int GetTime(const char **p)
{
	const char *p1 = *p;
	while (*p1)
	{
		if (*p1 == ':')
			break;
		p1 ++;
	}
	char szTmp[6] = {0};
	int nSize = p1 - *p;
	if ((nSize > 0) && (nSize < 5))
	{
		strncpy(szTmp, *p, nSize);
		if (*p1)
			p1 ++;
		*p = p1;
		return ::atoi(szTmp);
	}
	return 0;
}

int  GetDays(int y, int m, int d)
{
	static int months[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	int nDays = (y - 2000) * 365;
	nDays += (y - 2000) / 4 + 1;
	for (int i = 0; i < m - 1; i ++)
	{
		nDays += months[i];
	}
	if ((m > 2) && ((y % 4) == 0))
		nDays ++;
	nDays += d;
	return nDays;
}

void SplitDays(int nDays, int &y, int &m, int &d)
{
	static int months[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	y = nDays / 365;
	nDays -= ((y / 4) + 1);
	y = nDays / 365;
	nDays %= 365;
	m = 1;
	bool bIsLeap = ((y % 4) == 0);
    for (int i = 0; i < 11; i ++)
	{
		if (bIsLeap && (m == 3))
			nDays --;
		if ((m == 2) && bIsLeap && nDays == 29)
			break;
		if (nDays <= months[i])
			break;	
		nDays -= months[i];
		m ++;	
	}
	d = nDays;
}

void SplitTime(const int nStamp, int &h, int &m, int &s)
{
	h = nStamp / 3600;
	m = (nStamp % 3600) / 60;
	s = (nStamp % 60);
}

BOOL CMessageList::StringToTimeStamp(const char *szTime, int &nStamp)
{
	const char *p = szTime;
	int y = GetDate(&p);
	int m = GetDate(&p);
	int d = GetDate(&p);
	int h = GetTime(&p);
	int min = GetTime(&p);
	int sec = GetTime(&p);
	if ((y > 0) && (m > 0) && (d > 0))
	{
		int nDays = GetDays(y, m, d);
		nStamp = nDays * 24 * 3600;
		nStamp += (h * 3600);
		nStamp += (min * 60);
		nStamp += sec;
		return TRUE;
	}
	return FALSE;
}

BOOL CMessageList::TimeStampToString(const int nStamp, std::string &strTime)
{
	int y, m, d;
	SplitDays(nStamp / 24 / 3600, y, m, d);
	int h, min, sec;
	int nSec = (nStamp % (3600 * 24));
	SplitTime(nSec, h, min, sec); 
	char szTmp[64] = {0};
	sprintf(szTmp, "%d-%0.2d-%0.2d %d:%d:%d", y + 2000, m, d, h, min, sec);
	strTime = szTmp;
	return TRUE;
}


//IUnknown
STDMETHODIMP CMessageList::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IMessageList)))
	{
		*ppv = (IMessageList *) this;
		_AddRef();
		return S_OK;
	}  
	return E_NOINTERFACE;
}

STDMETHODIMP CMessageList::Clear()
{
	std::vector<CMessageItem *>::iterator it;
	for (it = m_List.begin(); it != m_List.end(); it ++)
	{
		delete (*it);
	}
	m_List.clear();
	return S_OK;
}

//IMessageList
STDMETHODIMP CMessageList::AddMsg(const int nMsgId, const char *szType, const char *szFromName,
		               const char *szToName, const char *szTime, const char *szMsg)
{
	CMessageItem *pItem = new CMessageItem();
	pItem->nMsgId = nMsgId;
	if (szFromName)
		pItem->strFromName = szFromName;
	if (szType)
		pItem->strType = szType;
	if (szToName)
		pItem->strToName = szToName;
	if (szTime)
		pItem->strTime = szTime;
	if (szMsg)
		pItem->strMsg = szMsg;
	m_List.push_back(pItem);
	return S_OK;
}

STDMETHODIMP CMessageList::AddRawMsg(const int nMsgId, const char *szRawMsg)
{
	return AddMsg(nMsgId, NULL, NULL, NULL, NULL, szRawMsg);
}

//
STDMETHODIMP_(DWORD) CMessageList::GetCount()
{
	return m_List.size();
}
	//
STDMETHODIMP CMessageList::GetMsg(const int nIdx, int *nMsgId, IAnsiString *szType, IAnsiString *strFromName,
		               IAnsiString *strToName, IAnsiString *strTime, IAnsiString *strMsg)
{
	if ((nIdx < 0) || (nIdx >= (int)m_List.size()))
		return E_FAIL;
	std::vector<CMessageItem *>::iterator it;
	int i = 0;
	HRESULT hr = E_FAIL;
	for (it = m_List.begin(); it != m_List.end(); it ++)
	{
		if (i == nIdx)
		{
			if (nMsgId)
				*nMsgId = (*it)->nMsgId;
			strFromName->SetString((*it)->strFromName.c_str());
			szType->SetString((*it)->strType.c_str());
			strToName->SetString((*it)->strToName.c_str());
			strTime->SetString((*it)->strTime.c_str());
			strMsg->SetString((*it)->strMsg.c_str());
			hr = S_OK;
			break;
		} //end if (i == nIdx)
		i ++;
	} //end for (it = m_List.begin()...
	return hr;
}

STDMETHODIMP CMessageList::GetRawMsg(const int nIdx, int *nMsgId, IAnsiString *strRawMsg)
{
	if ((nIdx < 0) || (nIdx >= (int)m_List.size()))
		return E_FAIL;
	std::vector<CMessageItem *>::iterator it;
	int i = 0;
	HRESULT hr = E_FAIL;
	for (it = m_List.begin(); it != m_List.end(); it ++)
	{
		if (i == nIdx)
		{
			if (nMsgId)
				*nMsgId = (*it)->nMsgId;
			strRawMsg->SetString((*it)->strMsg.c_str());
			hr = S_OK;
			break;
		} //end if (i == nIdx)
		i ++;
	} //end for (it = m_List.begin()...
	return hr;
}

#pragma warning(default:4996)
