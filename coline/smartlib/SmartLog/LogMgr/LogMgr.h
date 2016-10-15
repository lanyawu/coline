#ifndef __LOGMGR_H_____
#define __LOGMGR_H_____

#include "LogDb.h"
#include "AffixDb.h"

class CLogMgr
{
public:
	CLogMgr(void);
	~CLogMgr(void);
public:
	BOOL InitLogMgr(const char *szLogName);

	//增加一个日志
	BOOL AddLog(int &nLogId, const int nParentLogId, const char *szTitle);
	BOOL UpdateLog(const int nLogId, const char *pBuf, const int nBufSize, const char *szAffix, BOOL bSave);
	BOOL UpdateLogPlainText(const int nLogId, const char *pBuf, const int nSize);
	BOOL GetLog(const int nLogId, char **szTitle, char **pBuf, int &nBufSize, 
		         char *szCreateDate, char *szLastModiDate, int &nReadTime, int &nCurrLines);
	BOOL UpdateTitle(const int nLogId, const char *szTitle);
	BOOL IncReadLogTime(const int nLogId);
	BOOL UpdateLogLines(const int nLogId, const int nLines);
	BOOL UpdateParentLogId(const int nLogId, const int nParentLogId);
	//获取节点
 	BOOL GetNodeItems(const int nParentLogId,  LPLOG_NODE_ITEM *ppItems, int &nCount);
    BOOL SearchText(const char *szSubText, LPLOG_NODE_ITEM *ppItems, int &nCount);
    BOOL DeleteLog(const int nLogId);
	BOOL DeleteAffix(const int nAffixId);
	BOOL AddAffix(const int nLogId, const char *szAffixName);
	BOOL GetAffixList(const int nLogId, int **nAffixList, int &nAffixCount);
	BOOL GetAffixInfo(const int nAffixId, char *szAffixName);
    BOOL SaveAsAffix(const int nAffixId, const char *szFileName);
	//
    BOOL AddWisdom(const int nFromLogId, const int nRow, const char *szWisdom);
    BOOL GetAllWisdom(LPWISDOM_ITEM *ppItems, int &nCount);
	//
	//备注相关
	BOOL AddComment(int &nCommentId, const int nLogId, const int nStart, const int nLength, 
		             const char *szComment, const int nCommentSize);
    BOOL GetComments(const int nLogId, LPLOG_COMMENT_ITEM *ppItems, int &nCount);
	BOOL GetCommentText(const int nCommentId, char **pBuf, int &nBufSize);
    BOOL DeleteComment(const int nCommentId);
	BOOL DeleteCommentBySel(const int nLogId, const int nStart, const int nLength);
	//
	BOOL AddMemoLog(const char *szMemo);
	//
	BOOL AddPersonAccount(const float fIncoming, const float fPayout, const char *szAddr, const char *szItem, 
		                    const char *szComment, const char *szUserName, const char *szUserDate);
	//
	//add auto
	BOOL AddAutoOil(const float fPrice, const float fCapacity, const float fTotal, const char *szAddDate,
		             const float fKilometre, const char *szComment);
	BOOL AddAutoFee(const float fPrice, const float fCount, const float fTotal, const char *szFeeDate,
		             const char *szComment);
	BOOL CheckPassword(const char *szPassword);
	BOOL SetPassword(const char *szOldPassword, const char *szNewPassword);
	//
	//
	BOOL AddWorkFlow(const char *szTimeSect, const char *szMemo);
private:
	std::string m_strLogDbName;
	BOOL m_bValid;
	CLogDb   *m_LogDb;
	CAffixDb *m_AffixDb;
};

#endif
