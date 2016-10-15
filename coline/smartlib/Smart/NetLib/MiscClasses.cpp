#include <Netlib/MiscClasses.h>
#include <CommonLib/DebugLog.h>

CSafeUdpMessageDeque::CSafeUdpMessageDeque()
{
	srand(GetTickCount()); //初始化随机数
}

CSafeUdpMessageDeque::~CSafeUdpMessageDeque()
{
	Clear();
}

void CSafeUdpMessageDeque::Clear()
{
	CGuardLock::COwnerLock guard(m_Lock);
	LPUDPMESSAGEITEM pItem;
	while(!m_List.empty())
	{
		pItem = m_List.back();
		delete pItem;
		m_List.pop_back();
	}
}

void CSafeUdpMessageDeque::Insert(LPUDPMESSAGEITEM pItem)
{
	if (m_List.size() > MAX_MSGBUFFERSIZE)
	{	
		CGuardLock::COwnerLock guard(m_Lock);
		if (m_List.size() > MAX_MSGBUFFERSIZE) //丢弃前面一些
		{
			PRINTDEBUGLOG(dtInfo, "缓存队列数据过多，现在数据: %d 条", m_List.size());
			DWORD dwCount;
			LPUDPMESSAGEITEM pChuckItem;
			for(dwCount = 0; dwCount < MAX_CHUCK_SIZE; dwCount ++)
			{
				if (m_List.empty())
					break;
				pChuckItem = m_List.front();
				m_List.pop_front();
				delete pChuckItem;
			}
		}
	}

	CGuardLock::COwnerLock guard(m_Lock);
	m_List.push_back(pItem);
}

void CSafeUdpMessageDeque::InsertFront(LPUDPMESSAGEITEM pItem)
{
	CGuardLock::COwnerLock guard(m_Lock);
	m_List.push_front(pItem);
}

LPUDPMESSAGEITEM CSafeUdpMessageDeque::GetItem()
{
	if (m_List.empty())
		return NULL;	

	LPUDPMESSAGEITEM pItem = NULL;
	CGuardLock::COwnerLock guard(m_Lock);	
	if (!m_List.empty())
	{
		pItem = m_List.front();
		m_List.pop_front();
	}
	return pItem;
}


// class CSafeUdpMessageVector
CSafeUdpMessageVector::CSafeUdpMessageVector()
{
	m_List.clear();
}

CSafeUdpMessageVector::~CSafeUdpMessageVector()
{
	Clear();
}

void CSafeUdpMessageVector::Insert(LPUDPMESSAGEITEM pItem)
{
	CGuardLock::COwnerLock guard(m_Lock);
	m_List.push_back(pItem);
}

void CSafeUdpMessageVector::Clear()
{
	CGuardLock::COwnerLock guard(m_Lock);
	LPUDPMESSAGEITEM pItem;
	while(!m_List.empty())
	{
		pItem = m_List.back();
		delete pItem;
		m_List.pop_back();
	}
}

LPUDPMESSAGEITEM CSafeUdpMessageVector::GetItem()
{	
	LPUDPMESSAGEITEM pItem = NULL;
	m_Lock.Lock();	

	if (!m_List.empty())
	{
		pItem = m_List.back();
		m_List.pop_back();
	}
	m_Lock.UnLock();

	if (!pItem)
	{
		try
		{
			pItem = new UDPMESSAGEITEM;
		}catch(...)
		{
			pItem = NULL;
			PRINTDEBUGLOG(dtError, "UDPPROCESSOR 申请内存失败");
		}
	}
	if (pItem)
		memset(pItem, 0, sizeof(UDPMESSAGEITEM));
	return pItem;
}
