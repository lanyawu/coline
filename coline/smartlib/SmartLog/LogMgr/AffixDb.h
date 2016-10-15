#ifndef __AFFIXDB_H____
#define __AFFIXDB_H____
#include <string>
#include <commonlib/sqlitedbop.h>
#include <commonlib/guardlock.h>

class CAffixDb
{
public:
	CAffixDb(void);
	~CAffixDb(void);
public:
	BOOL InitAffixDb(const char *szLogName, const char *szLogKey);
	BOOL AddAffix(const int nLogId, const char *szAffixName);
	BOOL GetAffixList(const int nLogId, int **nAffixList, int &nAffixCount);
	BOOL GetAffixInfo(const int nAffixId, char *szAffixName);
	BOOL DeleteAffix(const int nAffixId);
	BOOL DeleteByLogId(const int nLogId);
	BOOL SaveAsAffix(const int nAffixId, const char *szFileName);
private:
	std::string m_strDbName;
	CSqliteDBOP *m_DBOP;
	CGuardLock m_Lock;
};

#endif
