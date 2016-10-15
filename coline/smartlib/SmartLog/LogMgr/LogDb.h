#ifndef ___LOGDB_H____
#define ___LOGDB_H____
#include <string>
#include <commonlib/sqlitedbop.h>
#include <commonlib/guardlock.h>

#define MAX_LOG_NODE_NAME_SIZE 256

#pragma pack(push)
#pragma pack(1)
typedef struct CLogNodeItem
{
	int nNodeId;
    char szNodeName[MAX_LOG_NODE_NAME_SIZE];
}LOG_NODE_ITEM, *LPLOG_NODE_ITEM;

typedef struct CWisdomItem
{
	int Id;
	int nFromLogId;
	int nRow;
	char szWisdom[256];
}WISDOM_ITEM, *LPWISDOM_ITEM;

typedef struct CLogCommentItem
{
	int nStart;
	int nLength;
	int nCommendId;
}LOG_COMMENT_ITEM, *LPLOG_COMMENT_ITEM;

#pragma pack(pop)

class CLogDb
{
public:
	CLogDb(void);
	~CLogDb(void);
public:
	BOOL InitLogDb(const char *szLogName, const char *szLogKey);
	BOOL GetAffixDbName(std::string &strDbName); //获取附件文件名称
	BOOL DeleteLog(const int nLogId);
	BOOL UpdateTitle(const int nLogId, const char *szTitle);
	BOOL SearchText(const char *szSubText, LPLOG_NODE_ITEM *ppItems, int &nCount);
	BOOL UpdateParentLogId(const int nLogId, const int nParentLogId);
	BOOL UpdateLog(const int nLogId, const char *pBuf, const int nSize, BOOL bSaveLog);
	BOOL UpdateLogPlainText(const int nLogId, const char *pBuf, const int nSize);
	BOOL GetNodeItems(const int nParentLogId,  LPLOG_NODE_ITEM *ppItems, int &nCount);
	BOOL AddLog(int &nLogId, const int nParentLogId, const char *szTitle);
    BOOL GetLog(const int nLogId, char **szTitle, char **pBuf, int &nBufSize, char *szCreateDate, char *szLastModiDate,
		         int &nReadTimes, int &nCurrLine);
	//
	BOOL IncReadLogTime(const int nLogId);
	BOOL UpdateLogLines(const int nLogId, const int nLines);
	//
    BOOL AddWisdom(const int nFromLogId, const int nRow, const char *szWisdom);
    BOOL GetAllWisdom(LPWISDOM_ITEM *ppItems, int &nCount);
	//备注相关
	BOOL AddComment(int &nCommentId, const int nLogId, const int nStart, const int nLength, 
		             const char *szComment, const int nCommentSize);
    BOOL GetComments(const int nLogId, LPLOG_COMMENT_ITEM *ppItems, int &nCount);
	BOOL GetCommentText(const int nCommentId, char **pBuf, int &nBufSize);
	BOOL DeleteComment(const int nCommentId);
	BOOL DeleteCommentBySel(const int nLogId, const int nStart, const int nLength);
	//
	//BOOL AddSchedule(
	BOOL AddMemoLog(const char *szMemo);
	//add personaccount
	BOOL AddPersonAccount(const float fIncoming, const float fPayout, const char *szAddr, const char *szItem, 
		                    const char *szComment, const char *szUserName, const char *szUserDate);
	//add auto
	BOOL AddAutoOil(const float fPrice, const float fCapacity, const float fTotal, const char *szAddDate,
		             const float fKilometre, const char *szComment);
	BOOL AddAutoFee(const float fPrice, const float fCount, const float fTotal, const char *szFeeDate,
		             const char *szComment);
	//
	BOOL CheckPassword(const char *szPassword);
	BOOL SetPassword(const char *szOldPassword, const char *szNewPassword);
	//
	BOOL AddWorkFlow(const char *szTimeSect, const char *szMemo);
private:
	BOOL WisdomExists(const char *szSafeWisdom);
	BOOL AddOperateLog(const char *szType, const char *szTitle, const char *szResult);
private:
	CSqliteDBOP *m_DBOP;
	CGuardLock m_Lock;
	std::string m_strDbName; //自身的数据库名称
};

#endif
