#include <commonlib/systemutils.h>
#include "AffixDb.h"

#define AFFIX_TABLE_NAME         "affix"      //¸½¼þÁÐ±í
#define AFFIX_CREATE_TABLE_SQL   "create table affix(id INTEGER PRIMARY KEY, logid INTEGER, affixname VARCHAR(256),\
                                         logtext BLOB);" //
#pragma warning(disable:4996)

CAffixDb::CAffixDb(void):
           m_DBOP(NULL)
{

}

CAffixDb::~CAffixDb(void)
{
	if (m_DBOP)
		delete m_DBOP;
	m_DBOP = NULL;
}

BOOL CAffixDb::InitAffixDb(const char *szLogName, const char *szLogKey)
{
	try
	{
		m_strDbName = szLogName;
		m_DBOP = new CSqliteDBOP(szLogName, szLogKey);
		if (!m_DBOP->TableIsExists(AFFIX_TABLE_NAME))
			m_DBOP->Execute(AFFIX_CREATE_TABLE_SQL);
		return TRUE;
	} catch(...)
	{
		m_DBOP = NULL;
	}
	return FALSE;
}

BOOL CAffixDb::AddAffix(const int nLogId, const char *szAffixName)
{
	BOOL bSucc = FALSE;
	if (CSystemUtils::FileIsExists(szAffixName))
	{
		char szTmp[16] = {0};
		char szFileName[MAX_PATH] = {0};
		CSystemUtils::ExtractFileName(szAffixName, szFileName, MAX_PATH);
		std::string strSql = "insert into ";
		strSql += AFFIX_TABLE_NAME;
		strSql += "(logid,affixname,logtext) values(";
		strSql += ::itoa(nLogId, szTmp, 10);
		strSql += ",'";
		strSql += szFileName;
		strSql += "',?)";
        FILE *fp = fopen(szAffixName, "r+b");
		if (fp)
		{
			::fseek(fp, 0, SEEK_END);
			int nSize = ::ftell(fp);
			if (nSize > 0)
			{
				char *pData = new char[nSize];
				fseek(fp, 0, SEEK_SET);
				fread(pData, sizeof(char), nSize, fp);
				bSucc = m_DBOP->InsertBlob(strSql.c_str(), pData, nSize);
				delete []pData;
			}
			fclose(fp);
		}
	}
	return bSucc;
}

BOOL CAffixDb::GetAffixList(const int nLogId, int **nAffixList, int &nAffixCount)
{
	std::string strSql = "select id from affix where logid=";
	char szTmp[16] = {0};
	strSql += ::itoa(nLogId, szTmp, 10);
	char **szResult = NULL;
	int nRow, nCol;
	BOOL bSucc = FALSE;
	CGuardLock::COwnerLock guard(m_Lock);
	if (m_DBOP->Open(strSql.c_str(), &szResult, nRow, nCol))
	{
		nAffixCount = nRow;
		if (nRow > 0)
		{
			*nAffixList = new int[nAffixCount];
			memset(*nAffixList, 0, nRow * sizeof(int));
			for (int i = 0; i <nRow; i ++)
			{
				if (szResult[i + 1])
					(*nAffixList)[i] = atoi(szResult[i + 1]);
			}
			bSucc = TRUE;
		}
	}
	m_DBOP->Free_Result(szResult);
	return bSucc;
}

BOOL CAffixDb::GetAffixInfo(const int nAffixId, char *szAffixName)
{
	std::string strSql = "select affixname from affix where id=";
	char szTmp[16] = {0};
	strSql += ::itoa(nAffixId, szTmp, 10);
	char **szResult = NULL;
	int nRow, nCol;
	BOOL bSucc = FALSE;
	CGuardLock::COwnerLock guard(m_Lock);
	if (m_DBOP->Open(strSql.c_str(), &szResult, nRow, nCol))
	{
		if (nRow > 0)
		{
			if (szResult[1])
			{
				strcpy(szAffixName, szResult[1]);
				bSucc = TRUE;
			}
		}
	}
	m_DBOP->Free_Result(szResult);
	return bSucc;
}

BOOL CAffixDb::SaveAsAffix(const int nAffixId, const char *szFileName)
{
	std::string strSql = "select logtext from affix where id=";
	char szTmp[16] = {0};
	strSql += ::itoa(nAffixId, szTmp, 10);
	char *pData = NULL;
	DWORD dwSize = 0;
	BOOL bSucc = FALSE;
	CGuardLock::COwnerLock guard(m_Lock);
	if (m_DBOP->GetBlob(strSql.c_str(), &pData, dwSize, 0))
	{
		if (dwSize > 0)
		{
			FILE *fp = fopen(szFileName, "a+b");
			if (fp)
			{
				fwrite(pData, sizeof(char), dwSize, fp);
				fclose(fp);
				bSucc = TRUE;
			}
		}
		m_DBOP->Free_Blob(&pData);
	}
	return bSucc;
}

BOOL CAffixDb::DeleteAffix(const int nAffixId)
{
	std::string strSql = "delete from affix where id=";
	char szTmp[16] = {0};
	strSql += ::itoa(nAffixId, szTmp, 10);
	CGuardLock::COwnerLock guard(m_Lock);
	return m_DBOP->Execute(strSql.c_str());
}

BOOL CAffixDb::DeleteByLogId(const int nLogId)
{
	std::string strSql = "delete from affix where logid=";
	char szTmp[16] = {0};
	strSql += ::itoa(nLogId, szTmp, 10);
	CGuardLock::COwnerLock guard(m_Lock);
	return m_DBOP->Execute(strSql.c_str());
}

#pragma warning(default:4996)
