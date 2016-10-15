#include <Commonlib/GuardLock.h>

CGuardLock::CGuardLock(void)
{
	::InitializeCriticalSection(&m_CriSec);
}

CGuardLock::~CGuardLock(void)
{
	::DeleteCriticalSection(&m_CriSec);
}

void CGuardLock::Lock()
{
	::EnterCriticalSection(&m_CriSec);
}


void CGuardLock::UnLock()
{
	::LeaveCriticalSection(&m_CriSec);
}

CGuardLock::COwnerLock::COwnerLock(CGuardLock &Lock):
                        m_Lock(Lock)
{
	m_Lock.Lock();
}

CGuardLock::COwnerLock::~COwnerLock()
{
	m_Lock.UnLock();
}