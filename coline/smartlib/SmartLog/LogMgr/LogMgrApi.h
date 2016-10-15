#include "LogMgr.h"

BOOL CALLBACK LM_InitLogMgr(const char *szLogFileName);
void CALLBACK LM_DestroyLogMgr();
BOOL CALLBACK LM_AddLog(int *nLogId, const int nParentLogId, const char *szTitle);
BOOL CALLBACK LM_UpdateLog(const int nLogId, const char *pBuf, const int nSize, const char *szAffix, BOOL bSaveLog);
BOOL CALLBACK LM_UpdateLogPlainText(const int nLogId, const char *pBuf, const int nSize);
BOOL CALLBACK LM_IncReadLogTime(const int nLogId);
BOOL CALLBACK LM_UpdateLogLines(const int nLogId, const int nLines);
BOOL CALLBACK LM_GetLog(const int nLogId, char **szTitle, char **pBuf, int *nBufSize, 
	         char *szCreateDate, char *szLastModiDate, int *nReadTimes, int *nCurrLines);
BOOL CALLBACK LM_GetChildNodes(const int nParentLog, LPLOG_NODE_ITEM *ppItems, int *nCount);
BOOL CALLBACK LM_SearchText(const char *szSubText, LPLOG_NODE_ITEM *ppItems, int *nCount);
BOOL CALLBACK LM_DeleteString(char *pBuf);
BOOL CALLBACK LM_DeleteInt(int *pInt);
BOOL CALLBACK LM_DeleteNodeItems(LPLOG_NODE_ITEM pItems);
BOOL CALLBACK LM_DeleteWisdoms(LPWISDOM_ITEM pItems);
BOOL CALLBACK LM_DeleteCommentItems(LPLOG_COMMENT_ITEM pItems);
BOOL CALLBACK LM_UpdateTitle(const int nLogId, const char *szTitle);
BOOL CALLBACK LM_UpdateParentLogId(const int nLogId, const int nParentLogId);
BOOL CALLBACK LM_DeleteLog(const int nLogId);
BOOL CALLBACK LM_DeleteAffix(const int nAffixId);
BOOL CALLBACK LM_AddAffix(const int nLogId, const char *szAffixName);
BOOL CALLBACK LM_GetAffixList(const int nLogId, int **nAffixList, int &nAffixCount);
BOOL CALLBACK LM_GetAffixInfo(const int nAffixId, char *szAffixName);
BOOL CALLBACK LM_SaveAsAffix(const int nAffixId, const char *szFileName);
BOOL CALLBACK LM_AddWisdom(const int nFromLogId, const int nRow, const char *szWisdom);
BOOL CALLBACK LM_GetAllWisdom(LPWISDOM_ITEM *ppItems, int *nCount);
BOOL CALLBACK LM_AddComment(int *nCommentId, const int nLogId, const int nStart, const int nLength, 
		             const char *szComment, const int nCommentSize);
BOOL CALLBACK LM_GetComments(const int nLogId, LPLOG_COMMENT_ITEM *ppItems, int *nCount);
BOOL CALLBACK LM_GetCommentText(const int nCommentId, char **pBuf, int *nBufSize);
BOOL CALLBACK LM_DeleteComment(const int nCommentId);
BOOL CALLBACK LM_DeleteCommentBySel(const int nLogId, const int nStart, const int nLength);

BOOL CALLBACK LM_AddMemoLog(const char *szMemo);

BOOL CALLBACK LM_AddPersonAccount(const char *szIncoming, const char *szPayout, const char *szAddr, const char *szItem, 
		                    const char *szComment, const char *szUserName, const char *szUserDate);
BOOL CALLBACK LM_AddAutoOil(const char *szPrice, const char *szCapacity, const char *szTotal, const char *szAddDate,
		             const char *szKilometre, const char *szComment);
BOOL CALLBACK LM_AddAutoFee(const char *szPrice, const char *szCount, const char *szTotal, const char *szFeeDate,
		             const char *szComment);
BOOL CALLBACK LM_CheckPassword(const char *szPassword);
BOOL CALLBACK LM_SetPassword(const char *szOldPassword, const char *szNewPassword);
//
BOOL CALLBACK LM_AddWorkFlow(const char *szTimeSect, const char *szMemo);