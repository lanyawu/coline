#include <commonlib/debuglog.h>
#include "LogMgrApi.h"

static CLogMgr *m_pLogMgr = NULL;

 

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
			 break;
		case DLL_THREAD_ATTACH:
			 break;
		case DLL_THREAD_DETACH:
			 break;
		case DLL_PROCESS_DETACH:
			 break;
    }
    return TRUE;
}

BOOL CALLBACK LM_InitLogMgr(const char *szLogFileName)
{
	if (m_pLogMgr)
	{
		delete m_pLogMgr;
	}
	m_pLogMgr = new CLogMgr();
	return m_pLogMgr->InitLogMgr(szLogFileName);
}

void CALLBACK LM_DestroyLogMgr()
{
	if (!m_pLogMgr)
	{
		delete m_pLogMgr;
	}
	m_pLogMgr = NULL;
}

BOOL CALLBACK LM_AddLog(int *nLogId, const int nParentLogId, const char *szTitle)
{
	if (m_pLogMgr)
	{
		return m_pLogMgr->AddLog(*nLogId, nParentLogId, szTitle);
	}
	return FALSE;
}


BOOL CALLBACK LM_UpdateLog(const int nLogId, const char *pBuf, const int nSize, const char *szAffix, BOOL bSaveLog)
{
	if (m_pLogMgr)
	{
		return m_pLogMgr->UpdateLog(nLogId, pBuf, nSize, szAffix, bSaveLog);
	}
	return FALSE;
}

BOOL CALLBACK LM_UpdateLogPlainText(const int nLogId, const char *pBuf, const int nSize)
{
	if (m_pLogMgr)
		return m_pLogMgr->UpdateLogPlainText(nLogId, pBuf, nSize);
	return FALSE;
}

BOOL CALLBACK LM_IncReadLogTime(const int nLogId)
{
	if (m_pLogMgr)
		return m_pLogMgr->IncReadLogTime(nLogId);
	return FALSE;
}

BOOL CALLBACK LM_UpdateLogLines(const int nLogId, const int nLines)
{
	if (m_pLogMgr)
		return m_pLogMgr->UpdateLogLines(nLogId, nLines);
	return FALSE;
}

BOOL CALLBACK LM_UpdateTitle(const int nLogId, const char *szTitle)
{
	if (m_pLogMgr)
		return m_pLogMgr->UpdateTitle(nLogId, szTitle);
	return FALSE;
}

BOOL CALLBACK LM_UpdateParentLogId(const int nLogId, const int nParentLogId)
{
	if (m_pLogMgr)
		return m_pLogMgr->UpdateParentLogId(nLogId, nParentLogId);
	return FALSE;
}

BOOL CALLBACK LM_AddWisdom(const int nFromLogId, const int nRow, const char *szWisdom)
{
	if (m_pLogMgr)
		return m_pLogMgr->AddWisdom(nFromLogId, nRow, szWisdom);
	return FALSE;
}

BOOL CALLBACK LM_GetAllWisdom(LPWISDOM_ITEM *ppItems, int *nCount)
{
	if (m_pLogMgr)
		return m_pLogMgr->GetAllWisdom(ppItems, *nCount);
	return FALSE;
}

BOOL CALLBACK LM_AddComment(int *nCommentId, const int nLogId, const int nStart, const int nLength, 
		             const char *szComment, const int nCommentSize)
{
	if (m_pLogMgr)
		return m_pLogMgr->AddComment(*nCommentId, nLogId, nStart, nLength, szComment, nCommentSize);
	return FALSE;
}

BOOL CALLBACK LM_DeleteComment(const int nCommentId)
{
	if (m_pLogMgr)
		return m_pLogMgr->DeleteComment(nCommentId);
	return FALSE;
}

BOOL CALLBACK LM_DeleteCommentBySel(const int nLogId, const int nStart, const int nLength)
{
	if (m_pLogMgr)
		return m_pLogMgr->DeleteCommentBySel(nLogId, nStart, nLength);
	return FALSE;
}

BOOL CALLBACK LM_GetComments(const int nLogId, LPLOG_COMMENT_ITEM *ppItems, int *nCount)
{
	if (m_pLogMgr)
		return m_pLogMgr->GetComments(nLogId, ppItems, *nCount);
	return FALSE;
}

BOOL CALLBACK LM_DeleteCommentItems(LPLOG_COMMENT_ITEM pItems)
{
	if (pItems)
	{
		delete []pItems;
		return TRUE;
	}
	return FALSE;
}

BOOL CALLBACK LM_GetCommentText(const int nCommentId, char **pBuf, int *nBufSize)
{
	if (m_pLogMgr)
		return m_pLogMgr->GetCommentText(nCommentId, pBuf, *nBufSize);
	return FALSE;
}

BOOL CALLBACK LM_DeleteString(char *pBuf)
{
	if (pBuf)
	{
		delete []pBuf;
		return TRUE;
	}
	return FALSE;
}

BOOL CALLBACK LM_DeleteInt(int *pInt)
{
	if (pInt)
	{
		delete []pInt;
		return TRUE;
	}
	return FALSE;
}

BOOL CALLBACK LM_DeleteWisdoms(LPWISDOM_ITEM pItems)
{
	if (pItems)
	{
		delete pItems;
		return TRUE;
	}
	return FALSE;
}

BOOL CALLBACK LM_GetLog(const int nLogId, char **szTitle, char **pBuf, int *nBufSize, 
	         char *szCreateDate, char *szLastModiDate, int *nReadTimes, int *nCurrLines)
{
	if (m_pLogMgr)
	{
		return m_pLogMgr->GetLog(nLogId, szTitle, pBuf, *nBufSize, szCreateDate, szLastModiDate, *nReadTimes, *nCurrLines);
	}
	return FALSE;
}

BOOL CALLBACK LM_GetChildNodes(const int nParentLog, LPLOG_NODE_ITEM *ppItems, int *nCount)
{
	if (m_pLogMgr)
	{
		return m_pLogMgr->GetNodeItems(nParentLog, ppItems, *nCount);
	}
	return FALSE;
}

BOOL CALLBACK LM_SearchText(const char *szSubText, LPLOG_NODE_ITEM *ppItems, int *nCount)
{
	if (m_pLogMgr)
		return m_pLogMgr->SearchText(szSubText, ppItems, *nCount);
	return FALSE;
}

BOOL CALLBACK LM_DeleteLog(const int nLogId)
{
	if (m_pLogMgr)
	{
		return m_pLogMgr->DeleteLog(nLogId);
	}
	return FALSE;
}

BOOL CALLBACK LM_DeleteNodeItems(LPLOG_NODE_ITEM pItems)
{
	if (pItems)
	{
		delete []pItems;
		return TRUE;
	}
	return FALSE;
}

BOOL CALLBACK LM_DeleteAffix(const int nAffixId)
{
	if (m_pLogMgr)
	{
		return m_pLogMgr->DeleteAffix(nAffixId);
	}
	return FALSE;
}

BOOL CALLBACK LM_AddAffix(const int nLogId, const char *szAffixName)
{
	if (m_pLogMgr)
	{
		return m_pLogMgr->AddAffix(nLogId, szAffixName);
	}
	return FALSE;
}

BOOL CALLBACK LM_GetAffixList(const int nLogId, int **nAffixList, int &nAffixCount)
{
	if (m_pLogMgr)
	{
		return m_pLogMgr->GetAffixList(nLogId, nAffixList, nAffixCount);
	}
	return FALSE;
}

BOOL CALLBACK LM_GetAffixInfo(const int nAffixId, char *szAffixName)
{
	if (m_pLogMgr)
	{
		return m_pLogMgr->GetAffixInfo(nAffixId, szAffixName);
	}
	return FALSE;
}

BOOL CALLBACK LM_SaveAsAffix(const int nAffixId, const char *szFileName)
{
	if (m_pLogMgr)
	{
		return m_pLogMgr->SaveAsAffix(nAffixId, szFileName);
	}
	return FALSE;
}

BOOL CALLBACK LM_AddMemoLog(const char *szMemo)
{
	if (m_pLogMgr)
		return m_pLogMgr->AddMemoLog(szMemo);
	return FALSE;
}

BOOL CALLBACK LM_AddPersonAccount(const char *szIncoming, const char *szPayout, const char *szAddr, const char *szItem, 
		                    const char *szComment, const char *szUserName, const char *szUserDate)
{
	if (m_pLogMgr)
	{
		float fIncoming = ::atof(szIncoming);
		float fPayout = ::atof(szPayout);		
		return m_pLogMgr->AddPersonAccount(fIncoming, fPayout, szAddr, szItem, szComment, szUserName, szUserDate);
	}
	return FALSE;
}

BOOL CALLBACK LM_AddAutoOil(const char *szPrice, const char *szCapacity, const char *szTotal, const char *szAddDate,
		             const char *szKilometre, const char *szComment)
{
	if (m_pLogMgr)
	{
		float fPrice = ::atof(szPrice);
		float fCapacity = ::atof(szCapacity);
		float fTotal = ::atof(szTotal);
		float fKilometre = ::atof(szKilometre);
		return m_pLogMgr->AddAutoOil(fPrice, fCapacity, fTotal, szAddDate, fKilometre, szComment);
	}
	return FALSE;
}

BOOL CALLBACK LM_AddAutoFee(const char *szPrice, const char *szCount, const char *szTotal, const char *szFeeDate,
		             const char *szComment)
{
	if (m_pLogMgr)
	{
		float fPrice = ::atof(szPrice);
		float fCount = ::atof(szCount);
		float fTotal = ::atof(szTotal);
		return m_pLogMgr->AddAutoFee(fPrice, fCount, fTotal, szFeeDate, szComment);
	}
	return FALSE;
}

BOOL CALLBACK LM_CheckPassword(const char *szPassword)
{
	if (m_pLogMgr)
		return m_pLogMgr->CheckPassword(szPassword);
	return FALSE;
}

BOOL CALLBACK LM_SetPassword(const char *szOldPassword, const char *szNewPassword)
{
	if (m_pLogMgr)
		return m_pLogMgr->SetPassword(szOldPassword, szNewPassword);
	return FALSE;
}

BOOL CALLBACK LM_AddWorkFlow(const char *szTimeSect, const char *szMemo)
{
	if (m_pLogMgr)
		return m_pLogMgr->AddWorkFlow(szTimeSect, szMemo);
	return FALSE;
}
