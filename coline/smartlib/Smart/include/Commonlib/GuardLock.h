#ifndef __GUARDLOCK_H__
#define __GUARDLOCK_H__

#include <Commonlib/Types.h>

class COMMONLIB_API CGuardLock
{
public:
	CGuardLock(void);
	~CGuardLock(void);
	class COwnerLock
	{
	public:
		COwnerLock(CGuardLock &Lock);
		~COwnerLock();
	private:
		CGuardLock &m_Lock;
	};
public:
	void Lock();
	void UnLock();
private:
	CRITICAL_SECTION m_CriSec;
};


#endif