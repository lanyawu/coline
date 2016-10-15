#include <commonlib/Schedule.h>
#include <vector>
CSchedule::CSchedule(void):
           m_bTerminated(FALSE),
		   m_dwCurrId(0)
{
	m_hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hThread = ::CreateThread(NULL, 0, ExecuteScheduleThread, this, 0, NULL);
}

CSchedule::~CSchedule(void)
{
	m_bTerminated = TRUE;
	SetEvent(m_hEvent);
	::WaitForSingleObject(m_hThread, 5000);
	Clear();
	CloseHandle(m_hThread);
	CloseHandle(m_hEvent);
}

//增加一个任务
DWORD CSchedule::AddSchedule(LPSCHEDULEFUNC pFun, LPVOID lpParam, DWORD dwInterval, BOOL bIsSuspended, 
							 DWORD dwTimes) 
{
	if (dwInterval < SCHEDULE_MIN_INTERVAL)
		dwInterval = SCHEDULE_MIN_INTERVAL;
	
	LPSCHEDULE_ITEM pItem = new SCHEDULE_ITEM;
	memset(pItem, 0, sizeof(SCHEDULE_ITEM));
	pItem->bIsSuspended = bIsSuspended;
	pItem->dwInterval = dwInterval;
	pItem->dwNextExeTime = GetTickCount() + dwInterval;
	pItem->dwTimes = dwTimes;
	pItem->lpParam = lpParam;
	pItem->pFun = pFun;
	m_Lock.Lock();
	DWORD id = ++m_dwCurrId;
	pItem->dwScheduleId = id;
	m_List.insert(std::pair<DWORD, LPSCHEDULE_ITEM>(id, pItem));
	m_Lock.UnLock();
    if (!bIsSuspended)
		SetEvent(m_hEvent);
	return id;
}

//删除一个任务
void  CSchedule::DeleteSchedule(DWORD dwScheduleId)
{
	CGuardLock::COwnerLock guard(m_Lock);
	std::map<DWORD, LPSCHEDULE_ITEM>::iterator it = m_List.find(dwScheduleId);
	if (it != m_List.end())
	{
		delete [](*it).second;
		m_List.erase(it);
	}
}

//挂起一个任务
void  CSchedule::Suspend(DWORD dwScheduleId) 
{
	CGuardLock::COwnerLock guard(m_Lock);
	std::map<DWORD, LPSCHEDULE_ITEM>::iterator it = m_List.find(dwScheduleId);
	if (it != m_List.end())
	{
		(*it).second->bIsSuspended = TRUE;
	}
}

void  CSchedule::Resume(DWORD dwScheduleId)
{
	bool b = false;
	m_Lock.Lock();
	std::map<DWORD, LPSCHEDULE_ITEM>::iterator it = m_List.find(dwScheduleId);
	if (it != m_List.end())
	{
		(*it).second->bIsSuspended = FALSE;
		(*it).second->dwNextExeTime = GetTickCount() + (*it).second->dwInterval;
		b = true;
	}
	m_Lock.UnLock();
	if (b)
		SetEvent(m_hEvent);
}

void  CSchedule::Clear()
{
	CGuardLock::COwnerLock guard(m_Lock);
	std::map<DWORD, LPSCHEDULE_ITEM>::iterator it;
	for (it = m_List.begin(); it != m_List.end(); it ++)
	{
		delete [](*it).second;
	}
	m_List.clear();
}

void CSchedule::ExecuteSchedule(DWORD &dwWaitTime)
{
	std::vector<LPSCHEDULE_ITEM> tmp;
	LPSCHEDULE_ITEM pItem;
	DWORD dwNextTime = 0xFFFFFFFF;
	DWORD dwNow;
	std::map<DWORD, LPSCHEDULE_ITEM>::iterator it;
	dwNow = GetTickCount();
	m_Lock.Lock();
	for (it = m_List.begin(); it != m_List.end();)
	{
		if ((!((*it).second->bIsSuspended)) && (dwNow >= (*it).second->dwNextExeTime))
		{
			pItem = new SCHEDULE_ITEM();
			memmove(pItem, (*it).second, sizeof(SCHEDULE_ITEM));
			tmp.push_back(pItem);
			(*it).second->dwNextExeTime = dwNow + (*it).second->dwInterval;
			if ((*it).second->dwTimes > 0)
				(*it).second->dwTimes --;
		}
		if ((!((*it).second->bIsSuspended)) && (dwNextTime > (*it).second->dwNextExeTime))
			dwNextTime = (*it).second->dwNextExeTime;
		if ((*it).second->dwTimes == 0)
		{
			delete [](*it).second;
			it = m_List.erase(it);
		} else
			it ++;
	}
	m_Lock.UnLock();
	//execute

	while (!tmp.empty())
	{
		pItem = tmp.back();
		tmp.pop_back();
		pItem->pFun(pItem->lpParam);
		delete pItem;
	}
	dwNow = GetTickCount();
	if (dwNextTime > dwNow)
		dwWaitTime = dwNextTime - dwNow;
	else
		dwWaitTime = 0;
}

DWORD WINAPI CSchedule::ExecuteScheduleThread(LPVOID lpParam)
{
	CSchedule *pThis = (CSchedule *)lpParam;
	DWORD dwWaitTime = INFINITE;
	while(!pThis->m_bTerminated)
	{
		WaitForSingleObject(pThis->m_hEvent, dwWaitTime);
		if (pThis->m_bTerminated)
			break;
		dwWaitTime = INFINITE;
		pThis->ExecuteSchedule(dwWaitTime);
	}
	return 0;
}
