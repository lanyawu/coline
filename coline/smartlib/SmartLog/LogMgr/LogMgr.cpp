#include <string>
#include "LogMgr.h"

#define LOG_MGR_SQLITE_DB_KEY "w~x@y$z^"
CLogMgr::CLogMgr(void):        
         m_LogDb(NULL),
		 m_bValid(FALSE),
		 m_AffixDb(NULL)
{

}

CLogMgr::~CLogMgr(void)
{
	if (m_LogDb)
		delete m_LogDb;
	if (m_AffixDb)
		delete m_AffixDb;
	m_LogDb = NULL;
	m_AffixDb = NULL;
}

BOOL CLogMgr::InitLogMgr(const char *szLogName)
{
	m_strLogDbName = szLogName;
	m_LogDb = new CLogDb();
	std::string strAffixName;
	if (m_LogDb->InitLogDb(szLogName, LOG_MGR_SQLITE_DB_KEY))
	{
		if (m_LogDb->GetAffixDbName(strAffixName))
		{
			m_AffixDb = new CAffixDb();
			return m_AffixDb->InitAffixDb(strAffixName.c_str(), LOG_MGR_SQLITE_DB_KEY);
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CLogMgr::UpdateLog(const int nLogId, const char *pBuf, const int nBufSize, const char *szAffix, BOOL bSave)
{
	if (m_LogDb && m_AffixDb && m_bValid)
	{
		if (m_LogDb->UpdateLog(nLogId, pBuf, nBufSize, bSave))
		{
			if (szAffix) //增加附件
			{
				std::string strAffix = szAffix;
				int nPos = 0;
				int nOffset = 0;
				do 
				{
					nPos = (int) strAffix.find("|", nOffset);
					std::string strTmp = strAffix.substr(nOffset, nPos - nOffset);
					m_AffixDb->AddAffix(nLogId, strTmp.c_str());
					if (nPos != std::string::npos)
					{
						nOffset = nPos + 1;				    
					}
				} while (nPos != std::string::npos);
			}
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CLogMgr::UpdateLogPlainText(const int nLogId, const char *pBuf, const int nSize)
{
	if (m_LogDb  && m_bValid)
		return m_LogDb->UpdateLogPlainText(nLogId, pBuf, nSize);
	return FALSE;
}

//增加一个日志
BOOL CLogMgr::AddLog(int &nLogId, const int nParentLogId, const char *szTitle)
{
	if (m_LogDb  && m_bValid)
	{
		return m_LogDb->AddLog(nLogId, nParentLogId, szTitle);
	}
	return FALSE;
}

BOOL CLogMgr::AddWisdom(const int nFromLogId, const int nRow, const char *szWisdom)
{
	if (m_LogDb  && m_bValid)
		return m_LogDb->AddWisdom(nFromLogId, nRow, szWisdom);
	return FALSE;
}

BOOL CLogMgr::GetAllWisdom(LPWISDOM_ITEM *ppItems, int &nCount)
{
	if (m_LogDb  && m_bValid)
		return m_LogDb->GetAllWisdom(ppItems, nCount);
	return FALSE;
}

//备注相关
BOOL CLogMgr::AddComment(int &nCommentId, const int nLogId, const int nStart, const int nLength, 
	             const char *szComment, const int nCommentSize)
{
	if (m_LogDb  && m_bValid)
		return m_LogDb->AddComment(nCommentId, nLogId, nStart, nLength, szComment, nCommentSize);
	return FALSE;
}

BOOL CLogMgr::GetComments(const int nLogId, LPLOG_COMMENT_ITEM *ppItems, int &nCount)
{
	if (m_LogDb  && m_bValid)
		return m_LogDb->GetComments(nLogId, ppItems, nCount);
	return FALSE;
}

BOOL CLogMgr::DeleteComment(const int nCommentId)
{
	if (m_LogDb  && m_bValid)
		return m_LogDb->DeleteComment(nCommentId);
	return FALSE;
}

BOOL CLogMgr::DeleteCommentBySel(const int nLogId, const int nStart, const int nLength)
{
	if (m_LogDb  && m_bValid)
		return m_LogDb->DeleteCommentBySel(nLogId, nStart, nLength);
	return FALSE;
}

BOOL CLogMgr::GetCommentText(const int nCommentId, char **pBuf, int &nBufSize)
{
	if (m_LogDb && m_bValid)
		return m_LogDb->GetCommentText(nCommentId, pBuf, nBufSize);
	return FALSE;
}

BOOL CLogMgr::GetLog(const int nLogId, char **szTitle, char **pBuf, int &nBufSize, 
	         char *szCreateDate, char *szLastModiDate, int &nReadTimes, int &nCurrLines)
{
	//
	if (m_LogDb && m_bValid)
		return m_LogDb->GetLog(nLogId, szTitle, pBuf, nBufSize, szCreateDate, szLastModiDate, nReadTimes, nCurrLines);
	return FALSE;
}

BOOL CLogMgr::IncReadLogTime(const int nLogId)
{
	if (m_LogDb && m_bValid)
		return m_LogDb->IncReadLogTime(nLogId);
	return FALSE;
}

BOOL CLogMgr::UpdateLogLines(const int nLogId, const int nLines)
{
	if (m_LogDb && m_bValid)
		return m_LogDb->UpdateLogLines(nLogId, nLines);
	return FALSE;
}

BOOL CLogMgr::DeleteLog(const int nLogId)
{
	//
	if (m_LogDb && m_AffixDb && m_bValid)
	{
		if (m_LogDb->DeleteLog(nLogId))
			return m_AffixDb->DeleteByLogId(nLogId);
	}
	return FALSE;
}

BOOL CLogMgr::UpdateTitle(const int nLogId, const char *szTitle)
{
	if (m_LogDb && m_bValid)
		return m_LogDb->UpdateTitle(nLogId, szTitle);
	return FALSE;
}

BOOL CLogMgr::UpdateParentLogId(const int nLogId, const int nParentLogId)
{
	if (m_LogDb && m_bValid)
		return m_LogDb->UpdateParentLogId(nLogId, nParentLogId);
	return FALSE;
}

BOOL CLogMgr::DeleteAffix(const int nAffixId)
{
	//
	if (m_AffixDb && m_bValid)
	{
		return m_AffixDb->DeleteAffix(nAffixId);
	}
	return FALSE;
}

BOOL CLogMgr::AddMemoLog(const char *szMemo)
{
	if (m_LogDb && m_bValid)
		return m_LogDb->AddMemoLog(szMemo);
	return FALSE;
}

BOOL CLogMgr::AddPersonAccount(const float fIncoming, const float fPayout, const char *szAddr, const char *szItem, 
		                    const char *szComment, const char *szUserName, const char *szUserDate)
{
	if (m_LogDb && m_bValid)
		return m_LogDb->AddPersonAccount(fIncoming, fPayout, szAddr, szItem, szComment, szUserName, szUserDate);
	return FALSE;
}

//add auto
BOOL CLogMgr::AddAutoOil(const float fPrice, const float fCapacity, const float fTotal, const char *szAddDate,
	             const float fKilometre, const char *szComment)
{
	if (m_LogDb && m_bValid)
		return m_LogDb->AddAutoOil(fPrice, fCapacity, fTotal, szAddDate, fKilometre, szComment);
	return FALSE;
}

BOOL CLogMgr::AddWorkFlow(const char *szTimeSect, const char *szMemo)
{
	if (m_LogDb)
		return m_LogDb->AddWorkFlow(szTimeSect, szMemo);
	return FALSE;
}

BOOL CLogMgr::AddAutoFee(const float fPrice, const float fCount, const float fTotal, const char *szFeeDate,
	             const char *szComment)
{
	if (m_LogDb && m_bValid)
		return m_LogDb->AddAutoFee(fPrice, fCount, fTotal, szFeeDate, szComment);
	return FALSE;
}

//获取节点
BOOL CLogMgr::GetNodeItems(const int nParentLogId,  LPLOG_NODE_ITEM *ppItems, int &nCount)
{
	if (m_LogDb && m_bValid)
		return m_LogDb->GetNodeItems(nParentLogId, ppItems, nCount);
	return FALSE;
}

BOOL CLogMgr::SearchText(const char *szSubText, LPLOG_NODE_ITEM *ppItems, int &nCount)
{
	if (m_LogDb && m_bValid)
		return m_LogDb->SearchText(szSubText, ppItems, nCount);
	return FALSE;
}

BOOL CLogMgr::AddAffix(const int nLogId, const char *szAffixName)
{
	//
	if (m_AffixDb && m_bValid)
		return m_AffixDb->AddAffix(nLogId, szAffixName);
	return FALSE;
}

BOOL CLogMgr::GetAffixList(const int nLogId, int **nAffixList, int &nAffixCount)
{
	//
	if (m_AffixDb && m_bValid)
		return m_AffixDb->GetAffixList(nLogId, nAffixList, nAffixCount);
	return FALSE;
}

BOOL CLogMgr::GetAffixInfo(const int nAffixId, char *szAffixName)
{
	//
	if (m_AffixDb && m_bValid)
		return m_AffixDb->GetAffixInfo(nAffixId, szAffixName);
	return FALSE;
}

BOOL CLogMgr::SaveAsAffix(const int nAffixId, const char *szFileName)
{
	//
	if (m_AffixDb && m_bValid)
		return m_AffixDb->SaveAsAffix(nAffixId, szFileName);
	return FALSE;
}

BOOL CLogMgr::CheckPassword(const char *szPassword)
{
	if (m_LogDb)
		m_bValid = m_LogDb->CheckPassword(szPassword);
	return m_bValid;
}

BOOL CLogMgr::SetPassword(const char *szOldPassword, const char *szNewPassword)
{
	if (m_LogDb  && m_bValid)
		return m_LogDb->SetPassword(szOldPassword, szNewPassword);
	return FALSE;
}
