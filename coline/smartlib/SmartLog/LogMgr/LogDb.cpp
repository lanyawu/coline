#include <time.h>
#include <commonlib/systemutils.h>
#include <crypto/crypto.h>
#include "LogDb.h"

#define AFFIX_EXT_NAME  ".aff"  //扩展名
#define AFFIX_TABLE_NAME         "affixdb"      //附件表名
#define LOGLIST_TABLE_NAME       "loglist"      //日志列表
#define MODIFY_LOG_HISTORY       "modifyhistory" //历史记录
#define WISDOM_TABLE_NAME        "wisdom"   //
#define PARAM_TABLE_NAME         "params"
#define LOG_COMMENT_TABLE_NAME   "logcomment"
#define SCHEDULE_TABLE_NAME      "schedulelog"
#define MEMO_TABLE_NAME          "memolog"  //
#define WORKFLOW_TABLE_NAME      "workflow"
#define ACCOUNT_TABLE_NAME       "account" 
#define OPERATELOG_TABLE_NAME    "operatelog"
#define PERSONACCOUNT_TABLE_NAME "personaccount"
#define AUTOOIL_TABLE_NAME       "autooil"
#define AUTOFEE_TABLE_NAME       "autofee"
//#define PERSONDETAIL_TABLE_NAME  "
#define AFFIX_CREATE_TABLE_SQL   "create table affixdb(dbname VARCHAR(256));"
#define LOGLIST_CREATE_TABLE_SQL "create table loglist(id INTEGER PRIMARY KEY, parentlog INTEGER, logname VARCHAR(256),\
                                  createdate VARCHAR(64), lastmodidate VARCHAR(64), readtimes INTEGER, currlines INTEGER, orderseq INTEGER,\
                                  srclen INTEGER, logtext BLOB, plaintext BLOB);" //日志表记录
#define MODIFY_LOG_HISTORY_TABLE_SQL "create table modifyhistory(id INTEGER PRIMARY KEY, logid INTEGER, modifydate VARCHAR(64),\
                                       logtext BLOB);" //
#define WISDOM_CREATE_TABLE_SQL  "create table wisdom(id INTEGER PRIMARY KEY, fromlogid INTEGER, nrow INTEGER,\
                                       createdate VARCHAR(64), wisdomtext VARCHAR(256));"
#define LOG_COMMENT_CREATE_TABLE_SQL "create table logcomment(id INTEGER PRIMARY KEY, logid INTEGER, nstart INTEGER, nlength INTEGER,\
                                       commentdate VARCHAR(64), commenttext BLOB);" 
#define SCHEDULE_CREATE_TABLE_SQL  "create table schedulelog(id INTEGER PRIMARY KEY, createdate VARCHAR(64), finisheddate VARCHAR(64), \
                                       importance VARCHAR(64), shedulelog BLOB, finishstatus VARCHAR(64));"
#define MEMO_CREATE_TABLE_SQL  "create table memolog(id INTEGER PRIMARY KEY, createdate VARCHAR(64), memo BLOB);"

#define WORKFLOW_CREATE_TABLE_SQL "create table workflow(id INTEGER PRIMARY KEY, timesect VARCHAR(64), workdate VARCHAR(64), memo BLOB);"

#define ACCOUNT_CREATE_TABLE_SQL "create table account(id INTEGER PRIMARY KEY, account VARCHAR(64), password VARCHAR(64), memo VARCHAR(256));"

#define OPERATELOG_CREATE_TABLE_SQL "create table operatelog(id INTEGER PRIMARY KEY, atype VARCHAR(64), title VARCHAR(256), \
                                        operateresult VARCHAR(64), operatetime VARCHAR(64));"
#define PERSONACCOUNT_CREATE_TABLE_SQL "create table personaccount(id INTEGER PRIMARY KEY, incoming REAL, payout REAL, addr VARCHAR(256),\
                                         item VARCHAR(64), comment VARCHAR(256), usedate VARCHAR(64), username VARCHAR(32), createdate VARCHAR(64));"
#define AUTOOIL_CREATE_TABLE_SQL  "create table autooil(id INTEGER PRIMARY KEY, price REAL, capacity REAL, total REAL, adddate VARCHAR(64),\
                                      kilometre REAL, comment VARCHAR(128));"
#define AUTOFEE_CREATE_TABLE_SQL  "create table autofee(id INTEGER PRIMARY KEY, price REAL, quantity REAL, total REAL, feedate VARCHAR(64),\
                                      comment VARCHAR(256));"
#define PARAM_CREATE_TABLE_SQL  "create table params(id INTEGER PRIMARY KEY, paramname VARCHAR(128), paramvalue VARCHAR(128), comment VARCHAR(256));"
#pragma warning(disable:4996)

CLogDb::CLogDb(void):
        m_DBOP(NULL)
{
}

CLogDb::~CLogDb(void)
{
	if (m_DBOP)
		delete m_DBOP;
	m_DBOP = NULL;
}

BOOL CLogDb::InitLogDb(const char *szLogName, const char *szLogKey)
{
	if (!m_DBOP)
	{
		try
		{
			m_DBOP = new CSqliteDBOP(szLogName, szLogKey);
			m_strDbName = szLogName;
			if (!m_DBOP->TableIsExists(LOGLIST_TABLE_NAME))
				m_DBOP->Execute(LOGLIST_CREATE_TABLE_SQL);	
			if (!m_DBOP->TableIsExists(MODIFY_LOG_HISTORY))
				m_DBOP->Execute(MODIFY_LOG_HISTORY_TABLE_SQL);
			if (!m_DBOP->TableIsExists(AFFIX_TABLE_NAME))
				m_DBOP->Execute(AFFIX_CREATE_TABLE_SQL);
			if (!m_DBOP->TableIsExists(WISDOM_TABLE_NAME))
				m_DBOP->Execute(WISDOM_CREATE_TABLE_SQL);
			if (!m_DBOP->TableIsExists(LOG_COMMENT_TABLE_NAME))
				m_DBOP->Execute(LOG_COMMENT_CREATE_TABLE_SQL);
			if (!m_DBOP->TableIsExists(SCHEDULE_TABLE_NAME))
				m_DBOP->Execute(SCHEDULE_CREATE_TABLE_SQL);
			if (!m_DBOP->TableIsExists(MEMO_TABLE_NAME))
				m_DBOP->Execute(MEMO_CREATE_TABLE_SQL);
			if (!m_DBOP->TableIsExists(ACCOUNT_TABLE_NAME))
				m_DBOP->Execute(ACCOUNT_CREATE_TABLE_SQL);
			if (!m_DBOP->TableIsExists(OPERATELOG_TABLE_NAME))
				m_DBOP->Execute(OPERATELOG_CREATE_TABLE_SQL);
			if (!m_DBOP->TableIsExists(PERSONACCOUNT_TABLE_NAME))
				m_DBOP->Execute(PERSONACCOUNT_CREATE_TABLE_SQL);
			if (!m_DBOP->TableIsExists(AUTOOIL_TABLE_NAME))
				m_DBOP->Execute(AUTOOIL_CREATE_TABLE_SQL);
			if(!m_DBOP->TableIsExists(AUTOFEE_TABLE_NAME))
				m_DBOP->Execute(AUTOFEE_CREATE_TABLE_SQL);
			if (!m_DBOP->TableIsExists(WORKFLOW_TABLE_NAME))
				m_DBOP->Execute(WORKFLOW_CREATE_TABLE_SQL);
			if (!m_DBOP->TableIsExists(PARAM_TABLE_NAME))
			{
				m_DBOP->Execute(PARAM_CREATE_TABLE_SQL);
				m_DBOP->Execute("insert into params(paramname,paramvalue,comment) values('password','','密码')");
			}
			return TRUE;
		} catch(...)
		{
			m_DBOP = NULL;

		}
	}
	return FALSE;
}

//获取附件文件名称
BOOL CLogDb::GetAffixDbName(std::string &strDbName)
{
	static char szGetAffixDbSql[] = "select dbname from affixdb";
	char **szResult = NULL;
	int nRow, nCol;
	BOOL bSucc = FALSE;
	CGuardLock::COwnerLock guard(m_Lock);
    if (m_DBOP->Open(szGetAffixDbSql, &szResult, nRow, nCol))
	{
		char szFileName[MAX_PATH] = {0};
		CSystemUtils::ExtractFilePath(m_strDbName.c_str(), szFileName, MAX_PATH - 1);
		if (nRow > 0)
		{
			if (szResult[1])
				strcat(szFileName, szResult[1]);
			if (CSystemUtils::FileIsExists(szFileName))
			{
				strDbName = szFileName;
				bSucc = TRUE;
			}
		} else
		{
			char szTmp[MAX_PATH] = {0};
			char szExt[MAX_PATH] = {0};
			char szDbName[MAX_PATH] = {0};
			CSystemUtils::ExtractFileName(m_strDbName.c_str(), szTmp, MAX_PATH - 1);
			CSystemUtils::ExtractFileExtName(szTmp, szExt, MAX_PATH - 1);
			int nNameSize = (int)::strlen(szTmp) - (int)::strlen(szExt) - 1;
			if (nNameSize <= 0)
				nNameSize = (int)::strlen(szTmp);
			strncpy(szDbName, szTmp, nNameSize);
			strcat(szDbName, AFFIX_EXT_NAME);
			std::string strSql = "insert into affixdb(dbname) values('";
			strSql += szDbName;
			strSql += "')";
			CGuardLock::COwnerLock guard(m_Lock);
			bSucc = m_DBOP->Execute(strSql.c_str());
			if (bSucc)
			{
				strDbName = szFileName;
				strDbName += szDbName;
			}
		}

	}
	m_DBOP->Free_Result(szResult);
	return bSucc;
}

BOOL CLogDb::GetNodeItems(const int nParentLogId,  LPLOG_NODE_ITEM *ppItems, int &nCount)
{
	std::string strSql = "select id,logname from ";
    strSql += LOGLIST_TABLE_NAME;
	strSql += " where parentlog=";
	char szTmp[16] = {0};
	BOOL bSucc = FALSE;
	strSql += ::itoa(nParentLogId, szTmp, 10);
	strSql += " order by orderseq,id";
	char **szResult = NULL;
	int nRow, nCol;
	CGuardLock::COwnerLock guard(m_Lock);
	if (m_DBOP->Open(strSql.c_str(), &szResult, nRow, nCol))
	{
		if (nRow > 0)
		{
			nCount = nRow;
			*ppItems = new LOG_NODE_ITEM[nCount];
			memset(*ppItems, 0, nCount * sizeof(LOG_NODE_ITEM));
			for (int i = 0; i < nRow; i ++)
			{
				(*ppItems)[i].nNodeId = ::atoi(szResult[(i + 1) * 2]);
				if (szResult[(i + 1) * 2 + 1])
					strncpy((*ppItems)[i].szNodeName, szResult[(i + 1) * 2 + 1], MAX_LOG_NODE_NAME_SIZE - 1);
			}
			bSucc = TRUE;
		}
	}
	m_DBOP->Free_Result(szResult);
	return bSucc;
}

BOOL CLogDb::UpdateLogPlainText(const int nLogId, const char *pBuf, const int nSize)
{
	char szTime[64] = {0};
	char szTmp[16] = {0};
	CSystemUtils::DateTimeToStr((DWORD)::time(NULL), szTime);
	std::string strSql;
	if (nLogId != 0) //update
	{
		strSql = "update ";
		strSql += LOGLIST_TABLE_NAME;
		strSql += " set plaintext=?,lastmodidate='";
		strSql += szTime;
		strSql += "' where id=";
		strSql += ::itoa(nLogId, szTmp, 10);
	}
	CGuardLock::COwnerLock guard(m_Lock);
	return m_DBOP->InsertBlob(strSql.c_str(), pBuf, nSize);
}

BOOL CLogDb::AddOperateLog(const char *szType, const char *szTitle, const char *szResult)
{
	//
	char szTime[64] = {0};
	CSystemUtils::DateTimeToStr((DWORD)::time(NULL), szTime);
	std::string strSql = "insert into ";
	strSql += OPERATELOG_TABLE_NAME;
	strSql += "(atype,title,operateresult,operatetime) values('";
	strSql += szType;
	strSql += "','";
	strSql += szTitle;
	strSql += "','";
	strSql += szResult;
	strSql += "','";
	strSql += szTime;
	strSql += "');";
	return m_DBOP->Execute(strSql.c_str());
}

BOOL CLogDb::UpdateLog(const int nLogId, const char *pBuf, const int nSize, BOOL bSaveLog)
{
	char szTime[64] = {0};
	char szTmp[16] = {0};
	CSystemUtils::DateTimeToStr((DWORD)::time(NULL), szTime);
	std::string strSql;
	//compress
	BYTE *pDest = NULL;
	UINT32 uDestLen = 0;
	UINT32 uSrcSize = nSize;
	if (!zlib_compress(&pDest, &uDestLen, (BYTE *)pBuf, uSrcSize))
	{
		uSrcSize = 0;
	}
	if (nLogId != 0) //update
	{
		char szTmp[16] = {0};
		strSql = "update ";
		strSql += LOGLIST_TABLE_NAME;
		strSql += " set srclen=";
		strSql += ::itoa(uSrcSize, szTmp, 10);
		strSql += ",logtext=?,lastmodidate='";
		strSql += szTime;
		strSql += "' where id=";
		strSql += ::itoa(nLogId, szTmp, 10);
	}
	CGuardLock::COwnerLock guard(m_Lock);
	std::string strTitle = "日志编号:";
	strTitle += ::itoa(nLogId, szTmp, 10);
	BOOL bSucc = FALSE;
	if (uSrcSize == 0)
		bSucc = m_DBOP->InsertBlob(strSql.c_str(), pBuf, nSize);
	else
		bSucc = m_DBOP->InsertBlob(strSql.c_str(), (char *)pDest, uDestLen);
	if (pDest)
		delete []pDest;
	if (bSucc)
	{		
		AddOperateLog("修改", strTitle.c_str(), "成功");
		return TRUE;
	} else
	{
		AddOperateLog("修改", strTitle.c_str(), "失败");
		return FALSE;
	}
}

BOOL CLogDb::AddLog(int &nLogId, const int nParentLogId, const char *szTitle)
{
	int nTitleSize = (int) ::strlen(szTitle);
	char *szSafeTitle = new char[nTitleSize * 2 + 1];
	memset(szSafeTitle, 0, nTitleSize * 2 + 1);
	char szTime[64] = {0};
	char szTmp[16] = {0};
	CSystemUtils::DateTimeToStr((DWORD)::time(NULL), szTime);
	std::string strSql;
	strSql = "insert into ";
	strSql += LOGLIST_TABLE_NAME;
	strSql += "(parentlog,logname,orderseq,createdate) values(";
	strSql += ::itoa(nParentLogId, szTmp, 10);
	strSql += ",'";
	strSql += CSqliteDBOP::StrToSqlStr(szTitle, szSafeTitle);
	strSql += "','999999999','";
	strSql += szTime;
	strSql += "')"; 
 
	delete []szSafeTitle;
	CGuardLock::COwnerLock guard(m_Lock);
	BOOL bSucc = m_DBOP->Execute(strSql.c_str());
	if (bSucc)
	{
		nLogId = m_DBOP->LastInsertRowId();
		std::string strTitle = "日志编号：";
		strTitle += ::itoa(nLogId, szTmp, 10);
		strTitle += "  标题：";
		strTitle += szTitle;
		AddOperateLog("增加", strTitle.c_str(), "成功");
	} else
	{
		AddOperateLog("增加", szTitle, "失败");
	}
	return bSucc;
}

BOOL CLogDb::DeleteLog(const int nLogId)
{
	std::string strSql = "delete from loglist where id=";
	char szTmp[16] = {0};
	strSql += ::itoa(nLogId, szTmp, 10);
	CGuardLock::COwnerLock guard(m_Lock);
	std::string strTitle = "日志编号:";
	strTitle += ::itoa(nLogId, szTmp, 10);
	if (m_DBOP->Execute(strSql.c_str()))
	{
		AddOperateLog("删除", strTitle.c_str(), "成功");
		return TRUE;
	} else
	{
		AddOperateLog("删除", strTitle.c_str(), "失败");
		return FALSE;
	}
}

BOOL CLogDb::UpdateTitle(const int nLogId, const char *szTitle)
{
	std::string strSql = "update loglist set logname='";
	strSql += szTitle;
	strSql += "' where id=";
	char szTmp[16] = {0};
	strSql += ::itoa(nLogId, szTmp, 10);
	std::string strTitle = "日志编号：";
	strTitle += ::itoa(nLogId, szTmp, 10);
	strTitle += "  标题：";
	strTitle += szTitle;
	CGuardLock::COwnerLock guard(m_Lock);
	if (m_DBOP->Execute(strSql.c_str()))
	{
		AddOperateLog("修改标题", strTitle.c_str(), "成功");
		return TRUE;
	} else
	{
		AddOperateLog("修改标题", strTitle.c_str(), "失败");
		return FALSE;
	}
}

BOOL CLogDb::UpdateParentLogId(const int nLogId, const int nParentLogId)
{
	std::string strSql = "update loglist set parentlog=";
	char szTmp[16] = {0};
	strSql += ::itoa(nParentLogId, szTmp, 10);
	strSql += " where id=";
	strSql += ::itoa(nLogId, szTmp, 10);
	std::string strTitle = "日志编号：";
	strTitle += ::itoa(nLogId, szTmp, 10);
	strTitle += " 修改父节点号为：";
	strTitle += ::itoa(nParentLogId, szTmp, 10);
	CGuardLock::COwnerLock guard(m_Lock);
	if (m_DBOP->Execute(strSql.c_str()))
	{
		AddOperateLog("修改父节点", strTitle.c_str(), "成功");
		return TRUE;
	} else
	{
		AddOperateLog("修改父节点", strTitle.c_str(), "失败");
		return FALSE;
	}
}

BOOL CLogDb::SearchText(const char *szSubText, LPLOG_NODE_ITEM *ppItems, int &nCount)
{
	std::string strSql = "select id, logname from ";
	strSql += LOGLIST_TABLE_NAME;
	strSql += " where plaintext like '%";
	strSql += szSubText;
	strSql += "%'";
	char **szResult = NULL;
	int nRow, nCol;
	BOOL bSucc = FALSE;
	CGuardLock::COwnerLock guard(m_Lock);
	if (m_DBOP->Open(strSql.c_str(), &szResult, nRow, nCol))
	{
		if (nRow > 0)
		{
			nCount = nRow;
			*ppItems = new LOG_NODE_ITEM[nCount];
			memset(*ppItems, 0, nCount * sizeof(LOG_NODE_ITEM));
			for (int i = 0; i < nRow; i ++)
			{
				(*ppItems)[i].nNodeId = ::atoi(szResult[(i + 1) * 2]);
				if (szResult[(i + 1) * 2 + 1])
					strncpy((*ppItems)[i].szNodeName, szResult[(i + 1) * 2 + 1], MAX_LOG_NODE_NAME_SIZE - 1);
			}
			bSucc = TRUE;
		}
	}
	m_DBOP->Free_Result(szResult);
	std::string strTitle = "搜索<";
	strTitle += szSubText;
	strTitle += "> 结果数：";
	char szTmp[16] = {0};
	strTitle += ::itoa(nCount, szTmp, 10);
	AddOperateLog("搜索", strTitle.c_str(), "成功");
	return bSucc;
}

BOOL CLogDb::IncReadLogTime(const int nLogId)
{
	char szTmp[16] = {0};
	std::string strSql = "update ";
	strSql += LOGLIST_TABLE_NAME;
	strSql += " set readtimes=readtimes + 1 where id=";
	strSql += ::itoa(nLogId, szTmp, 10);
	std::string strTitle = "日志编号：";
	strTitle += ::itoa(nLogId, szTmp, 10);
	CGuardLock::COwnerLock guard(m_Lock);
	BOOL bSucc = m_DBOP->Execute(strSql.c_str());
	AddOperateLog("阅读", strTitle.c_str(), "成功");
	return bSucc;
}

BOOL CLogDb::UpdateLogLines(const int nLogId, const int nLines)
{
	char szTmp[16] = {0};
	std::string strSql = "update ";
	strSql += LOGLIST_TABLE_NAME;
	strSql += " set currlines=";
	strSql += ::itoa(nLines, szTmp, 10);
	strSql += " where id=";
	strSql += ::itoa(nLogId, szTmp, 10);
	return m_DBOP->Execute(strSql.c_str());
}

BOOL CLogDb::GetLog(const int nLogId, char **szTitle, char **pBuf, int &nBufSize, 
					char *szCreateDate, char *szLastModiDate, int &nReadTimes, int &nCurrLine)
{
	char **szResult = NULL;
	int nRow, nCol;
	char szTmp[16] = {0};
	std::string strSql = "select logname,createdate,lastmodidate, readtimes, currlines, srclen from ";
	strSql += LOGLIST_TABLE_NAME;
	strSql += " where id=";
	strSql += ::itoa(nLogId, szTmp, 10);
	BOOL bSucc = FALSE;
	nReadTimes = 0;
	nCurrLine = 0;
	CGuardLock::COwnerLock guard(m_Lock);
	if (m_DBOP->Open(strSql.c_str(), &szResult, nRow, nCol))
	{
		if (nRow > 0)
		{
			int nTitleSize = 0;
			int nSrcLen = 0;
			if (szResult[6]) //logname
				nTitleSize = (int)::strlen(szResult[6]);
			nBufSize = 0;
			if (szResult[7])
				strcpy(szCreateDate, szResult[7]);
			if (szResult[8])
				strcpy(szLastModiDate, szResult[8]);
			if (nTitleSize > 0)
			{
				*szTitle = new char[nTitleSize + 1];
				strcpy(*szTitle, szResult[6]);
				(*szTitle)[nTitleSize] = '\0';
			}
			if (szResult[9])
				nReadTimes = atoi(szResult[9]);
			if (szResult[10])
				nCurrLine = atoi(szResult[10]);
			if (szResult[11])
				nSrcLen = atoi(szResult[11]);
			strSql = "select logtext from loglist where id=";
			strSql += ::itoa(nLogId, szTmp, 10);
			DWORD dwSize = 0;
			char *szTmpBuf = NULL;
			bSucc = m_DBOP->GetBlob(strSql.c_str(), &szTmpBuf, dwSize, 0);
			if (bSucc)
			{
				if (nSrcLen == 0)
				{
					nBufSize = dwSize;
					*pBuf = new char[nBufSize];
					memmove((*pBuf), szTmpBuf, nBufSize);
				} else
				{
					char *pTmpBuf = new char[nSrcLen];
					UINT32 uDstSize = nSrcLen;
					if (zlib_uncompress((BYTE *)pTmpBuf, &uDstSize, (BYTE *)szTmpBuf, dwSize))
					{
						nBufSize = uDstSize;
						(*pBuf) = pTmpBuf;
					} else
					{
						delete []pTmpBuf;
						nBufSize = 0;
					}
				}
			} else
				nBufSize = 0;
			if (szTmpBuf)
				delete []szTmpBuf;
		}
	}
	m_DBOP->Free_Result(szResult);
	return bSucc;
}

BOOL CLogDb::WisdomExists(const char *szSafeWisdom)
{
	//
	char **szResult;
	int nRow, nCol;
	std::string strSql = "select id from ";
	strSql += WISDOM_TABLE_NAME;
	strSql += " where wisdomtext='";
	strSql += szSafeWisdom;
	strSql += "'";
	BOOL bSucc = FALSE;
	if (m_DBOP->Open(strSql.c_str(), &szResult, nRow, nCol))
	{
		if (nRow > 0)
			bSucc = TRUE;
		m_DBOP->Free_Result(szResult);
	}
	return bSucc;
}

//备注相关
BOOL CLogDb::AddComment(int &nCommentId, const int nLogId, const int nStart, const int nLength, 
						const char *szComment, const int nCommentSize)
{
	//
	std::string strSql;
	BOOL bUpdate = FALSE;
	char szTmp[16] = {0};
	if (nCommentId != 0)
	{
		strSql = "select id from ";
		strSql += LOG_COMMENT_TABLE_NAME;
		strSql += " where id=";
		strSql += ::itoa(nCommentId, szTmp, 10);
	    char **szResult = NULL;
	    int nRow, nCol;
		CGuardLock::COwnerLock guard(m_Lock);
		if (m_DBOP->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			if (nRow > 0)
				bUpdate = TRUE;
			m_DBOP->Free_Result(szResult);
		}
	}
	std::string strType;
	std::string strTitle = "日志编号：";
	strTitle += ::itoa(nLogId, szTmp, 10);

	if (bUpdate)
	{
		strSql = "update ";
		strSql += LOG_COMMENT_TABLE_NAME;
		strSql += " set commenttext=?,nstart='";
		strSql += ::itoa(nStart, szTmp, 10);
		strSql += "', nlength='";
		strSql += ::itoa(nLength, szTmp, 10);
		strSql += " where id=";
		strSql += ::itoa(nCommentId, szTmp, 10);
        strType = "修改备注";
	} else
	{
		strSql = "insert into ";
		strSql += LOG_COMMENT_TABLE_NAME;
		strSql += "(logid,nstart,nlength,commentdate,commenttext) values(";
		strSql += ::itoa(nLogId, szTmp, 10);
		strSql += ",";
		strSql += ::itoa(nStart, szTmp, 10);
		strSql += ",";
		strSql += ::itoa(nLength, szTmp, 10);
		strSql += ",'";
		char szTime[64] = {0};
		CSystemUtils::DateTimeToStr((DWORD)::time(NULL), szTime);
		strSql += szTime;
		strSql += "',?)";
		strType = "增加备注";
	}
	//
	CGuardLock::COwnerLock guard(m_Lock);
    if (m_DBOP->InsertBlob(strSql.c_str(), szComment, nCommentSize))
	{
		if (nCommentId == 0)
			nCommentId = m_DBOP->LastInsertRowId();
		strTitle += " 备注号：";
		strTitle += ::itoa(nCommentId, szTmp, 10);
		AddOperateLog(strType.c_str(), strTitle.c_str(), "成功");
		return TRUE;
	} else
	{
		AddOperateLog(strType.c_str(), strTitle.c_str(), "失败");
		return FALSE;
	}
}

BOOL CLogDb::GetComments(const int nLogId, LPLOG_COMMENT_ITEM *ppItems, int &nCount)
{
	//
	std::string strSql = "select id, nstart, nlength from ";
	strSql += LOG_COMMENT_TABLE_NAME;
	strSql += " where logid=";
	char szTmp[16] = {0};
	strSql += ::itoa(nLogId, szTmp, 10);
	char **szResult = NULL;
	int nRow, nCol;
	BOOL bSucc = FALSE;
	CGuardLock::COwnerLock guard(m_Lock);
	if (m_DBOP->Open(strSql.c_str(), &szResult, nRow, nCol))
	{
		if (nRow > 0)
		{
			nCount = nRow;
			*ppItems = new LOG_COMMENT_ITEM[nCount];
			memset((*ppItems), 0, nCount * sizeof(LOG_COMMENT_ITEM));
			for (int i = 0; i < nRow; i ++)
			{
				(*ppItems)[i].nCommendId = ::atoi(szResult[(i + 1) * 3]);
				(*ppItems)[i].nStart = ::atoi(szResult[(i + 1) * 3 + 1]);
				(*ppItems)[i].nLength = ::atoi(szResult[(i + 1) * 3 + 2]);
			}
			bSucc = TRUE;
		}
		m_DBOP->Free_Result(szResult);
	}
	return bSucc;
}

BOOL CLogDb::GetCommentText(const int nCommentId, char **pBuf, int &nBufSize)
{
	std::string strSql = "select commenttext from ";
	strSql += LOG_COMMENT_TABLE_NAME;
	strSql += " where id=";
	char szTmp[16] = {0};
	strSql += ::itoa(nCommentId, szTmp, 10);
	DWORD dwSize = 0;
    CGuardLock::COwnerLock guard(m_Lock);
	if (m_DBOP->GetBlob(strSql.c_str(), pBuf, dwSize, 0))
	{
		nBufSize = dwSize;
		return TRUE;
	}
	return FALSE;
}

BOOL CLogDb::DeleteComment(const int nCommentId)
{
	std::string strSql = "delete from ";
	strSql += LOG_COMMENT_TABLE_NAME;
	strSql += " where id=";
	char szTmp[16] = {0};
	strSql += ::itoa(nCommentId, szTmp, 10);
	CGuardLock::COwnerLock guard(m_Lock);
	return m_DBOP->Execute(strSql.c_str());
}

BOOL CLogDb::AddMemoLog(const char *szMemo)
{
	std::string strSql = "insert into memolog(createdate,memolog) values('";
	char szTime[64] = {0};
	strSql += CSystemUtils::DateTimeToStr((DWORD)::time(NULL), szTime);
	strSql += "',?)";
	DWORD nLen = (DWORD)::strlen(szMemo);
	CGuardLock::COwnerLock guard(m_Lock);
	return m_DBOP->InsertBlob(strSql.c_str(), szMemo, nLen);
}

BOOL CLogDb::DeleteCommentBySel(const int nLogId, const int nStart, const int nLength)
{
	std::string strSql = "delete from ";
	strSql += LOG_COMMENT_TABLE_NAME;
	char szTmp[16] = {0};
	strSql += " where logid=";
	strSql += ::itoa(nLogId, szTmp, 10);
	strSql += " and nstart=";
	strSql += ::itoa(nStart, szTmp, 10);
	strSql += " and nlength=";
	strSql += ::itoa(nLength, szTmp, 10);
	CGuardLock::COwnerLock guard(m_Lock);
	return m_DBOP->Execute(strSql.c_str());
}

BOOL CLogDb::AddWisdom(const int nFromLogId, const int nRow, const char *szWisdom)
{
	int nWisdomSize = (int) ::strlen(szWisdom);
	char *szSafeWisdom = new char[nWisdomSize * 2 + 1];
	memset(szSafeWisdom, 0, nWisdomSize * 2 + 1);
	CSqliteDBOP::StrToSqlStr(szWisdom, szSafeWisdom);
	if (!WisdomExists(szSafeWisdom))
	{
		char szTime[64] = {0};
		char szTmp[16] = {0};
		CSystemUtils::DateTimeToStr((DWORD)::time(NULL), szTime);
		std::string strSql;
		strSql = "insert into ";
		strSql += WISDOM_TABLE_NAME;
		strSql += "(fromlogid,nrow,createdate,wisdomtext) values(";
		strSql += ::itoa(nFromLogId, szTmp, 10);
		strSql += ",";
		strSql += ::itoa(nRow, szTmp, 10);
		strSql += ",'";
		strSql += szTime;
		strSql += "','";
		strSql += szSafeWisdom;
		strSql += "')"; 
	 
		delete []szSafeWisdom;
		CGuardLock::COwnerLock guard(m_Lock);
		return m_DBOP->Execute(strSql.c_str());
	}
	return FALSE;
}

BOOL CLogDb::GetAllWisdom(LPWISDOM_ITEM *ppItems, int &nCount)
{
	std::string strSql = "select id,fromlogid,nrow,wisdomtext from ";
    strSql += WISDOM_TABLE_NAME;
	BOOL bSucc = FALSE; 
	char **szResult = NULL;
	int nRow, nCol;
	if (m_DBOP->Open(strSql.c_str(), &szResult, nRow, nCol))
	{
		if (nRow > 0)
		{
			nCount = nRow;
			*ppItems = new WISDOM_ITEM[nCount];
			memset(*ppItems, 0, nCount * sizeof(WISDOM_ITEM));
			for (int i = 0; i < nRow; i ++)
			{
				(*ppItems)[i].Id = ::atoi(szResult[(i + 1) * 4]);
				(*ppItems)[i].nFromLogId = ::atoi(szResult[(i + 1) * 4 + 1]);
				(*ppItems)[i].nRow = ::atoi(szResult[(i + 1) * 4 + 2]);
				if (szResult[(i + 1) * 4 + 3])
					strncpy((*ppItems)[i].szWisdom, szResult[(i + 1) * 4 + 3], 255);
			}
			bSucc = TRUE;
		}
	}
	m_DBOP->Free_Result(szResult);
	return bSucc;
}

BOOL CLogDb::AddPersonAccount(const float fIncoming, const float fPayout, const char *szAddr, const char *szItem, 
		                    const char *szComment, const char *szUserName, const char *szUserDate)
{ 
	char szTmp[512] = {0};
	std::string strSql = "insert into ";
	strSql += PERSONACCOUNT_TABLE_NAME;
	strSql += "(incoming,payout,addr,item,comment,usedate,username,createdate) values(";
	sprintf(szTmp, "'%0.2f','%0.2f','", fIncoming, fPayout);
	strSql += szTmp;
	if (szAddr)
		strSql += szAddr;
	strSql += "','";
	if (szItem)
		strSql += szItem;
    strSql += "','";
	if (szComment)
		strSql += szComment;
	strSql += "','";
	if (szUserDate)
		strSql += szUserDate;
	strSql += "','";
	if (szUserName)
		strSql += szUserName;
	strSql += "','";
	strSql += CSystemUtils::DateTimeToStr((DWORD)::time(NULL), szTmp);
	strSql += "')";
	CGuardLock::COwnerLock guard(m_Lock);
	return m_DBOP->Execute(strSql.c_str());
}

//add auto
BOOL CLogDb::AddAutoOil(const float fPrice, const float fCapacity, const float fTotal, const char *szAddDate,
	             const float fKilometre, const char *szComment)
{
	char szTmp[512] = {0};
	std::string strSql = "insert into ";
	strSql += AUTOOIL_TABLE_NAME;
	strSql += "(price,capacity,total,kilometre,adddate,comment) values(";
	sprintf(szTmp, "'%0.2f','%0.2f','%0.2f','%0.2f','", fPrice, fCapacity, fTotal, fKilometre);
	strSql += szTmp;
	if (szAddDate)
		strSql += szAddDate;
	strSql += "','";
	if (szComment)
		strSql += szComment;
	strSql += "')";
	CGuardLock::COwnerLock guard(m_Lock);
	return m_DBOP->Execute(strSql.c_str());
}

BOOL CLogDb::AddAutoFee(const float fPrice, const float fCount, const float fTotal, const char *szFeeDate,
	             const char *szComment)
{
	char szTmp[512] = {0};
	std::string strSql = "insert into ";
	strSql += AUTOFEE_TABLE_NAME;
	strSql += "(price,quantity,total,feedate,comment) values(";
	sprintf(szTmp, "'%0.2f','%0.2f','%0.2f','", fPrice, fCount, fTotal);
	strSql += szTmp;
	if (szFeeDate)
		strSql += szFeeDate;
	strSql += "','";
	if (szComment)
		strSql += szComment;
	strSql += "')";
	CGuardLock::COwnerLock guard(m_Lock);
	return m_DBOP->Execute(strSql.c_str());
}

BOOL CLogDb::CheckPassword(const char *szPassword)
{
	std::string strSql = "select paramvalue from params where paramname='password'";
	char **szResult = NULL;
	int nRow, nCol;
	BOOL bSucc = FALSE;
	CGuardLock::COwnerLock guard(m_Lock);
	if (m_DBOP->Open(strSql.c_str(), &szResult, nRow, nCol))
	{
		bSucc = TRUE;
		if (nRow > 0)
		{
			if (szResult[1])
			{
				if (szPassword)
				{
					if (::stricmp(szPassword, szResult[1]) != 0)
						bSucc = FALSE;
				} else
					bSucc = FALSE;
			}
		}
		m_DBOP->Free_Result(szResult);
	}
	if (bSucc)
		AddOperateLog("登陆", "登陆系统", "成功");
	else
	{
		if (szPassword && strlen(szPassword) > 0)
		{
			std::string strTitle = "登陆密码:";
			strTitle += szPassword;
			AddOperateLog("登陆", strTitle.c_str(), "失败");
		}
	}
	return bSucc;
}

BOOL CLogDb::SetPassword(const char *szOldPassword, const char *szNewPassword)
{
	if (CheckPassword(szOldPassword))
	{
		CGuardLock::COwnerLock guard(m_Lock);
		std::string strSql = "update params set paramvalue='";
		if (szNewPassword)
			strSql += szNewPassword;
		strSql += "' where paramname='password'";
		std::string strTitle = "修改系统密码,原密码:";
		strTitle += szOldPassword;
		strTitle += " 新密码:";
		strTitle += szNewPassword;
		AddOperateLog("修改密码", strTitle.c_str(), "成功");
		return m_DBOP->Execute(strSql.c_str());
	} else
	{
		AddOperateLog("修改密码", "修改系统密码", "失败");
	}
	return FALSE;
}

BOOL CLogDb::AddWorkFlow(const char *szTimeSect, const char *szMemo)
{
	std::string strSql = "insert into workflow(timesect,workdate,memo) values('";
	if (szTimeSect)
		strSql += szTimeSect;
	char szDate[64] = {0};
	CSystemUtils::DateToStr((DWORD)::time(NULL), szDate);
	strSql += "','";
	strSql += szDate;
	strSql += "',?)";
	return m_DBOP->InsertBlob(strSql.c_str(), szMemo, (DWORD)::strlen(szMemo));
}

#pragma warning(default:4996)
