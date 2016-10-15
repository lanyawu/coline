#include <time.h>
#include <Commonlib/systemutils.h>
#include <xml/tinyxml.h>
#include "MsgMgrImpl.h"
#include "../IMCommonLib/MessageList.h"

const char MESSAGE_TABLE_NAME[] = "messages";
const char CREATE_MESSAGE_TABLE_SQL[] = "create table messages(id INTEGER PRIMARY KEY, \
										 msgtype VARCHAR(16),\
	                                     fromname VARCHAR(128)  COLLATE NOCASE,\
										 toname VARCHAR(128)  COLLATE NOCASE,\
										 timestamp INTEGER,\
										 body TEXT,\
										 msg TEXT);create index if not exists msg_to_idx on messages(toname);\
										 create index if not exists msg_from_idx on messages(fromname);";

const char GROUP_TABLE_NAME[] = "groups";
const char CREATE_GROUP_TABLE_SQL[] = "create table groups(id INTEGER PRIMARY KEY, \
									  grpid VARCHAR(64) COLLATE NOCASE,\
									  grpdspname VARCHAR(256) COLLATE NOCASE,\
									  creator VARCHAR(62) COLLATE NOCASE);";
const char CHAT_CONTACTS_TABLE_NAME[] = "chat_contacts";
const char CREATE_CHAT_CONTACTS_TABLE_SQL[] = "create table chat_contacts(id INTEGER PRIMARY KEY, \
											username VARCHAR(128) COLLATE NOCASE,\
											realname VARCHAR(128) COLLATE NOCASE)";
//短信记录
const char CREATE_SMS_MESSAGE_TABLE_SQL[] = "create table sms2(id INTEGER PRIMARY KEY,\
											sender VARCHAR(32) COLLATE NOCASE,receiver VARCHAR(32) COLLATE NOCASE,\
											timestamp INTEGER, msg TEXT,guid VARCHAR(40),staus VARCHAR(1));\
											create index if not exists msg_to_idx on sms2(guid)";
//传真记录
const char CREATE_FAX_MESSAGE_TABLE_SQL[] = "create table fax(id INTEGER PRIMARY KEY, receiver VARCHAR(32) COLLATE NOCASE,\
											timestamp INTEGER, msg TEXT);";
#pragma warning(disable:4996)

CMsgMgrImpl::CMsgMgrImpl(void):
             m_pDBOP(NULL)
{
	//
}


CMsgMgrImpl::~CMsgMgrImpl(void)
{
	if (m_pDBOP)
		delete m_pDBOP;
	m_pDBOP = NULL;
}

//IUnknown
STDMETHODIMP CMsgMgrImpl::QueryInterface(REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IMsgMgr)))
	{
		*ppv = (IMsgMgr *) this;
		_AddRef();
		return S_OK;
	}  
	return E_NOINTERFACE;
}

BOOL CMsgMgrImpl::CheckDBOPValid()
{
	CGuardLock::COwnerLock guard(m_DbLock);
	if (m_pDBOP)
	{
		if (!m_pDBOP->TableIsExists(MESSAGE_TABLE_NAME))
			m_pDBOP->Execute(CREATE_MESSAGE_TABLE_SQL);
		if (!m_pDBOP->TableIsExists(GROUP_TABLE_NAME))
			m_pDBOP->Execute(CREATE_GROUP_TABLE_SQL);
		if (!m_pDBOP->TableIsExists(CHAT_CONTACTS_TABLE_NAME))
			m_pDBOP->Execute(CREATE_CHAT_CONTACTS_TABLE_SQL);
		if (!m_pDBOP->TableIsExists("sms2"))
			m_pDBOP->Execute(CREATE_SMS_MESSAGE_TABLE_SQL);
		if (!m_pDBOP->TableIsExists("fax"))
			m_pDBOP->Execute(CREATE_FAX_MESSAGE_TABLE_SQL);
		return TRUE;
	}
	return FALSE;
}

void CMsgMgrImpl::InitChatContactList()
{
	m_ContactList.clear();
	if (m_pDBOP)
	{

	}
}

void CMsgMgrImpl::CheckChatContactIsExists(const char *szFromName)
{

}

//IMsgMgr
//init 
STDMETHODIMP CMsgMgrImpl::InitMsgMgr(const char *szFileName, const char *szUserName)
{
	if (m_pDBOP)
		delete m_pDBOP;
	m_pDBOP = NULL;
	m_pDBOP = new CSqliteDBOP(szFileName, szUserName);
	if (CheckDBOPValid())
		return S_OK;
	else
	{
		delete m_pDBOP;
		std::string strBakFile = szFileName;
		char szTime[64] = {0};
		CSystemUtils::DateTimeToStr((DWORD)time(NULL), szTime, FALSE);
		strBakFile += "_";
		strBakFile += szTime;
		strBakFile +=".bak";
		CSystemUtils::MoveFilePlus(szFileName, strBakFile.c_str(), TRUE);
		m_pDBOP = new CSqliteDBOP(szFileName, szUserName);
		if (CheckDBOPValid())
			return S_OK;
	}
	return E_FAIL;
}

 
STDMETHODIMP CMsgMgrImpl::SaveMsg(const char *szType, const char *szFromName, const char *szToName,
		                 const char *szTime, const char *szMsg, const char *szBody)
{
	if (m_pDBOP)
	{
		std::string strSql;
		if (::stricmp(szType, "sms") == 0)
		{
			strSql = "insert into sms2(sender,receiver,timestamp,msg) values('";
			strSql += szFromName;
			strSql += "','";
			strSql += szToName;
			strSql += "','";
			int nStamp = 0;
			CMessageList::StringToTimeStamp(szTime, nStamp);
			char *szTmp = new char[32];
			memset(szTmp, 0, 20);
			strSql += ::itoa(nStamp, szTmp, 10);
			delete []szTmp;
			int nSize = ::strlen(szBody);
			szTmp = new char[nSize * 2 + 1];
			memset(szTmp, 0, nSize * 2 + 1);
			strSql += "','";
			strSql += CSqliteDBOP::StrToSqlStr(szBody, szTmp);
			strSql += "')";
		} else if (::stricmp(szType, "fax") == 0)
		{
			strSql = "insert into fax(receiver,timestamp,msg) values('"; 
			strSql += szToName;
			strSql += "','";
			int nStamp = 0;
			CMessageList::StringToTimeStamp(szTime, nStamp);
			char *szTmp = new char[32];
			memset(szTmp, 0, 20);
			strSql += ::itoa(nStamp, szTmp, 10);
			delete []szTmp;
			int nSize = ::strlen(szBody);
			szTmp = new char[nSize * 2 + 1];
			memset(szTmp, 0, nSize * 2 + 1);
			strSql += "','";
			strSql += CSqliteDBOP::StrToSqlStr(szBody, szTmp);
			strSql += "')";
		} else
		{
			strSql = "insert into messages(msgtype,fromname,toname,timestamp,body,msg) values(\"";
			strSql += szType;
			strSql += "\",\"";
			int nSize = ::strlen(szFromName);
			char *szTmp = new char[nSize * 2 + 1];
			memset(szTmp, 0, nSize * 2 + 1);
			strSql += CSqliteDBOP::StrToSqlStr(szFromName, szTmp);
			delete []szTmp;
			strSql += "\",\"";
			nSize = ::strlen(szToName);
			szTmp = new char[nSize * 2 + 1];
			memset(szTmp, 0, nSize * 2 + 1);
			strSql += CSqliteDBOP::StrToSqlStr(szToName, szTmp);
			delete []szTmp;
			strSql += "\",\"";
			nSize = 20;
			szTmp = new char[nSize * 2 + 1];
			memset(szTmp, 0, nSize * 2 + 1);
			int nStamp;
			CMessageList::StringToTimeStamp(szTime, nStamp);
			strSql += ::itoa(nStamp, szTmp, 10);
			delete []szTmp;
			strSql += "\",\"";
			nSize = ::strlen(szBody);
			szTmp = new char[nSize * 2 + 1];
			memset(szTmp, 0, nSize * 2 + 1);
			strSql += CSqliteDBOP::StrToSqlStr(szBody, szTmp);
			delete []szTmp;
			strSql += "\",\"";
			nSize = ::strlen(szMsg);
			szTmp = new char[nSize * 2 + 1];
			memset(szTmp, 0, nSize * 2 + 1);
			strSql += CSqliteDBOP::StrToSqlStr(szMsg, szTmp);
			delete []szTmp;
			strSql += "\")";	
		}
		CGuardLock::COwnerLock guard(m_DbLock);
		if (m_pDBOP->Execute(strSql.c_str()))
			return S_OK;
	}
	return E_FAIL;
}

STDMETHODIMP_(int) CMsgMgrImpl::GetMsgCount(const char *szType, const char *szFromName)
{
	int nCount = 0;
	if (m_pDBOP)
	{
		std::string strSql;
		if (::stricmp(szType, "sms") == 0)
		{
			strSql = "select count(id) from sms2 where sender='";
			strSql += szFromName;
			strSql += "' or receiver='";
			strSql += szFromName;
			strSql += "'";
		} else if (::stricmp(szType, "fax") == 0)
		{
			strSql = "select count(id) from fax where receiver='"; 
			strSql += szFromName;
			strSql += "'";
		} else
		{
			strSql = "select count(id) from messages \
								  where msgtype=\"";
			strSql += szType;
			strSql += "\" and (fromname=\"";
			int nSize = ::strlen(szFromName);
			char *szTmp = new char[nSize * 2 + 1];
			memset(szTmp, 0, nSize * 2 + 1);
			strSql += CSqliteDBOP::StrToSqlStr(szFromName, szTmp);
			strSql += "\" or toname=\"";
			strSql += szTmp;
			delete [] szTmp;
			nSize = 32;
			szTmp = new char[nSize];
			memset(szTmp, 0, nSize);
		    strSql += "\")";
			delete []szTmp;
		}
		char **szResult;
		int nRow, nCol; 	
		CGuardLock::COwnerLock guard(m_DbLock);
		if (m_pDBOP->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			if ((nRow > 0) && (szResult[1] != NULL))
				nCount = ::atoi(szResult[1]);
		}
		CSqliteDBOP::Free_Result(szResult);
	}
	return nCount;
}

STDMETHODIMP CMsgMgrImpl::GetMsgById(const char *szType, const int nMsgId, IMessageList *pList)
{
BOOL bSucc = FALSE;
	if (m_pDBOP && pList)
	{
		std::string strSql;
		char szMsgId[16] = {0};
		::itoa(nMsgId, szMsgId, 10);
		if (::stricmp(szType, "sms") == 0)
		{
			strSql = "select id,sender,receiver,timestamp,msg from sms2 where id='";
			strSql += szMsgId;
			strSql += "'";
		} else if (::stricmp(szType, "fax") == 0)
		{
			strSql = "select id,receiver,timestamp,msg from fax where id='";
			strSql += szMsgId;
			strSql += "'";
		} else
		{
			strSql = "select id,fromname,toname,timestamp,msg from messages \
								  where id='";
			strSql += szMsgId;
			strSql += "'";
		}
		//
		char **szResult;
		int nRow, nCol;
		std::string strTime;
		CGuardLock::COwnerLock guard(m_DbLock);
		if (m_pDBOP->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			for (int i = 1; i <= nRow; i ++)
			{
				CMessageList::TimeStampToString(::atoi(szResult[i * nCol + 3]), strTime);
				pList->AddMsg(::atoi(szResult[i * nCol]), szType, szResult[i * nCol + 1],
					szResult[i * nCol + 2], strTime.c_str(), szResult[i * nCol + 4]);
			}
			bSucc = TRUE;
		}
		CSqliteDBOP::Free_Result(szResult);
	}
	if (bSucc)
		return S_OK;
	return E_FAIL;
}

//get message 
STDMETHODIMP CMsgMgrImpl::GetMsg(const char *szType, const char *szFromName, const int nPage, 
		                const int nPageCount, IMessageList *pList)
{
	BOOL bSucc = FALSE;
	if (m_pDBOP && pList)
	{
		std::string strSql;
		if (::stricmp(szType, "sms") == 0)
		{
			strSql = "select id,sender,receiver,timestamp,msg from sms2 where sender='";
			strSql += szFromName;
			strSql += "' or receiver='";
			strSql += szFromName;
			strSql += "'";
			strSql += "  limit ";
		    char szTmp[32] = {0};
			strSql += ::itoa(nPageCount * (nPage - 1), szTmp, 10);
			strSql += ",";
			memset(szTmp, 0, 32);
			strSql += ::itoa(nPageCount, szTmp, 10);
		} else if (::stricmp(szType, "fax") == 0)
		{
			strSql = "select id,receiver,timestamp,msg from fax where receiver='";
			strSql += szFromName;
			strSql += "'";
			strSql += "  limit ";
		    char szTmp[32] = {0};
			strSql += ::itoa(nPageCount * (nPage - 1), szTmp, 10);
			strSql += ",";
			memset(szTmp, 0, 32);
			strSql += ::itoa(nPageCount, szTmp, 10);
		} else
		{
			strSql = "select id,fromname,toname,timestamp,msg from messages \
								  where msgtype=\"";
			strSql += szType;
			strSql += "\" and (fromname=\"";
			int nSize = ::strlen(szFromName);
			char *szTmp = new char[nSize * 2 + 1];
			memset(szTmp, 0, nSize * 2 + 1);
			strSql += CSqliteDBOP::StrToSqlStr(szFromName, szTmp);
			strSql += "\" or toname=\"";
			strSql += szTmp;
			delete [] szTmp;
			nSize = 32;
			szTmp = new char[nSize];
			memset(szTmp, 0, nSize);
		    strSql += "\") limit ";
			strSql += ::itoa(nPageCount * (nPage - 1), szTmp, 10);
			strSql += ",";
			memset(szTmp, 0, nSize);
			strSql += ::itoa(nPageCount, szTmp, 10);
			delete[]szTmp;
		}
		//
		char **szResult;
		int nRow, nCol;
		std::string strTime;
		CGuardLock::COwnerLock guard(m_DbLock);
		if (m_pDBOP->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			for (int i = 1; i <= nRow; i ++)
			{
				CMessageList::TimeStampToString(::atoi(szResult[i * nCol + 3]), strTime);
				pList->AddMsg(::atoi(szResult[i * nCol]), szType, szResult[i * nCol + 1],
					szResult[i * nCol + 2], strTime.c_str(), szResult[i * nCol + 4]);
			}
			bSucc = TRUE;
		}
		CSqliteDBOP::Free_Result(szResult);
	}
	if (bSucc)
		return S_OK;
	return E_FAIL;
}

STDMETHODIMP CMsgMgrImpl::GetRawMsg(const char *szType, const char *szFromName, const int nPage, 
		                const int nPageCount, IMessageList *pList)
{
	BOOL bSucc = FALSE;
	if (m_pDBOP && pList)
	{
		std::string strSql;
		if (::stricmp(szType, "sms") == 0)
		{
			strSql = "select id,msg from sms2 where sender='";
			strSql += szFromName;
			strSql += "' or receiver='";
			strSql += szFromName;
			strSql += "' limit ";
			char szTmp[16] = {0};
			strSql += ::itoa(nPageCount * (nPage - 1), szTmp, 10);
			strSql += ",";
			memset(szTmp, 0, 16);
			strSql += ::itoa(nPageCount, szTmp, 10);
		} else if (::stricmp(szType, "fax") == 0)
		{
			strSql = "select id,msg from sms2 where receiver='";
			strSql += szFromName;
			strSql += "' limit ";
			char szTmp[16] = {0};
			strSql += ::itoa(nPageCount * (nPage - 1), szTmp, 10);
			strSql += ",";
			memset(szTmp, 0, 16);
		} else
		{
			strSql = "select id,msg from messages \
								  where msgtype=\"";
			strSql += szType;
			strSql += "\" and (fromname=\"";
			int nSize = ::strlen(szFromName);
			char *szTmp = new char[nSize * 2 + 1];
			memset(szTmp, 0, nSize * 2 + 1);
			strSql += CSqliteDBOP::StrToSqlStr(szFromName, szTmp);
			strSql += "\" or toname=\"";
			strSql += szTmp;
			delete [] szTmp;
			nSize = 32;
			szTmp = new char[nSize];
			memset(szTmp, 0, nSize);
		    strSql += "\") limit ";
			strSql += ::itoa(nPageCount * (nPage - 1), szTmp, 10);
			strSql += ",";
			memset(szTmp, 0, nSize);
			strSql += ::itoa(nPageCount, szTmp, 10);
			delete[]szTmp;
		}
		//
		CGuardLock::COwnerLock guard(m_DbLock);
		char **szResult;
		int nRow, nCol;
		if (m_pDBOP->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			for (int i = 1; i <= nRow; i ++)
			{
				pList->AddRawMsg(::atoi(szResult[i * nCol]), szResult[i * nCol + 1]);
			}
			bSucc = TRUE;
		}
		CSqliteDBOP::Free_Result(szResult);
	}
	if (bSucc)
		return S_OK;
	return E_FAIL;
}

//nMsg != 0 delete one message
//nMsg == 0 delete all message from == szFromName and type == szType
STDMETHODIMP CMsgMgrImpl::ClearMsg(const int nMsgId, const char *szType, const char *szFromName)
{
	if (m_pDBOP)
	{ 
		std::string strSql;
		if (::stricmp(szType, "sms") == 0)
		{
			strSql = "delete  from sms2 where id='";
			char szId[32] = {0};
			::itoa(nMsgId, szId, 10);
			strSql += szId;
			strSql += "'";
		} else
		{
			if ((nMsgId == 0) && (szType != NULL) && (szFromName != NULL))
			{
				strSql = "delete from messages where msgtype=\"";
				strSql += szType;
				strSql += "\" and (fromname=\"";
				int nSize = ::strlen(szFromName);
				char *szTmp = new char[nSize * 2 + 1];
				memset(szTmp, 0, (nSize * 2 + 1));
				strSql += CSqliteDBOP::StrToSqlStr(szFromName, szTmp);
				strSql += "\" or toname=\"";
			    strSql += szTmp;
				delete []szTmp;
				strSql += "\")";
			} else
			{
				strSql = "delete from messages where id=";
				char szTmp[16] = {0};
				strSql += ::itoa(nMsgId, szTmp, 10);
			}
		}
		CGuardLock::COwnerLock guard(m_DbLock);
		if (m_pDBOP->Execute(strSql.c_str()))
			return S_OK;
	}
	return E_FAIL;
}

//
STDMETHODIMP_ (int) CMsgMgrImpl::GetSearchMsgCount(const char *szType, const char *szKey, const char *szFromName)
{
	int nCount = 0;
	if (m_pDBOP)
	{
		std::string strSql;
		BOOL bDid = FALSE;
		if (szType)
		{
			if (::stricmp(szType, "sms") == 0)
			{
				strSql = "select count(id) from sms2 where msg like \'%%";
				strSql += szKey;
				strSql += "%%'";
				if (szFromName && strlen(szFromName) > 0)
				{
					strSql += " and (sender='";
					strSql += szFromName;
					strSql += "' or receiver='";
					strSql += szFromName;
					strSql += "')";
				}
				bDid = TRUE;
			} else if (::stricmp(szType, "fax") == 0)
			{
					strSql = "select count(id) from sms2 where msg like \'%%";
				strSql += szKey;
				strSql += "%%'";
				if (szFromName && strlen(szFromName) > 0)
				{
					strSql += " and (receiver='";
					strSql += szFromName;
					strSql += "')";
				}
				bDid = TRUE;
			} 
		}
		if (!bDid)
		{
			strSql = "select count(id)  from messages \
								  where body like \"%%";
			int nSize = ::strlen(szKey);
			char *szTmp = new char[nSize * 2 + 1];
			memset(szTmp, 0, nSize * 2 + 1);
			strSql += CSqliteDBOP::StrToSqlStr(szKey, szTmp);
			delete [] szTmp;
			strSql += "%%\"";
			if (szType && (::strlen(szType) > 0))
			{
				strSql += " and msgtype=\"";
				strSql += szType;
				strSql += "\"";
			} 
			if (szFromName && (::strlen(szFromName) > 0))
			{
				strSql += " and (fromname=\"";
				int nSize = ::strlen(szFromName);
				char *szTmp = new char[nSize * 2 + 1];
				strSql += CSqliteDBOP::StrToSqlStr(szFromName, szTmp);
				strSql += "\" or toname=\"";
			    strSql += szTmp;
				delete []szTmp;
				strSql += "\")";
			}
		}
		CGuardLock::COwnerLock guard(m_DbLock);
		char **szResult;
		int nRow, nCol;
		std::string strTime;
		if (m_pDBOP->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			if ((nRow > 0) && (szResult[1] != NULL))
				nCount = ::atoi(szResult[1]);
		}
		CSqliteDBOP::Free_Result(szResult);
	}
	return nCount;
}

//
STDMETHODIMP CMsgMgrImpl::SearchMsg(const char *szKey, const char *szType, const char *szFromName,
		                   const int nPage, const int nPageCount, IMessageList *pList)
{
	BOOL bSucc = FALSE;
	if (m_pDBOP && pList)
	{
		std::string strSql = "select id,msgtype,fromname,toname,timestamp,msg from messages \
							  where body like \"%%";
		int nSize = ::strlen(szKey);
		char *szTmp = new char[nSize * 2 + 1];
		memset(szTmp, 0, nSize * 2 + 1);
		strSql += CSqliteDBOP::StrToSqlStr(szKey, szTmp);
		delete [] szTmp;
		strSql += "%%\"";
		if (szType && (::strlen(szType) > 0))
		{
			strSql += " and msgtype=\"";
			strSql += szType;
			strSql += "\"";
		}
		if (szFromName && (::strlen(szFromName) > 0))
		{
			strSql += " and (fromname=\"";
			int nSize = ::strlen(szFromName);
			char *szTmp = new char[nSize * 2 + 1];
			strSql += CSqliteDBOP::StrToSqlStr(szFromName, szTmp);
			strSql += "\" or toname=\"";
		    strSql += szTmp;
			delete []szTmp;
			strSql += "\")";
		}
	 
		nSize = 32;
		szTmp = new char[nSize];
		memset(szTmp, 0, nSize);
	    strSql += " limit ";
		strSql += ::itoa(nPageCount * (nPage - 1), szTmp, 10);
		strSql += ",";
		memset(szTmp, 0, nSize);
		strSql += ::itoa(nPageCount, szTmp, 10);
		delete[]szTmp;
		//
		CGuardLock::COwnerLock guard(m_DbLock);
		char **szResult;
		int nRow, nCol;
		std::string strTime;
		if (m_pDBOP->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			for (int i = 1; i <= nRow; i ++)
			{
				CMessageList::TimeStampToString(::atoi(szResult[i * nCol + 4]), strTime);
				pList->AddMsg(::atoi(szResult[i * nCol]), szResult[i * nCol + 1], szResult[i * nCol + 2],
					szResult[i * nCol + 3], strTime.c_str(), szResult[i * nCol + 5]);
			}
			bSucc = TRUE;
		}
		CSqliteDBOP::Free_Result(szResult);
	}
	if (bSucc)
		return S_OK;
	return E_FAIL;
}

STDMETHODIMP CMsgMgrImpl::SearchRawMsg(const char *szKey, const char *szType, const char *szFromName,
		                   const int nPage, const int nPageCount, IMessageList *pList)
{
	BOOL bSucc = FALSE;
	if (m_pDBOP && pList)
	{
		std::string strSql = "select id,msg from messages \
							  where body like \"%%";
		int nSize = ::strlen(szKey);
		char *szTmp = new char[nSize * 2 + 1];
		memset(szTmp, 0, nSize * 2 + 1);
		strSql += CSqliteDBOP::StrToSqlStr(szKey, szTmp);
		delete [] szTmp;
		strSql += "%%\"";
		if (szType && (::strlen(szType) > 0))
		{
			strSql += " and msgtype=\"";
			strSql += szType;
			strSql += "\"";
		}
		if (szFromName && (::strlen(szFromName) > 0))
		{
			strSql += " and fromname=\"";
			int nSize = ::strlen(szFromName);
			char *szTmp = new char[nSize * 2 + 1];
			strSql += CSqliteDBOP::StrToSqlStr(szFromName, szTmp);
			delete []szTmp;
			strSql += "\"";
		}
	 
		nSize = 32;
		szTmp = new char[nSize];
		memset(szTmp, 0, nSize);
	    strSql += " limit ";
		strSql += ::itoa(nPageCount * (nPage - 1), szTmp, 10);
		strSql += ",";
		memset(szTmp, 0, nSize);
		strSql += ::itoa(nPageCount, szTmp, 10);
		delete[]szTmp;
		//
		CGuardLock::COwnerLock guard(m_DbLock);
		char **szResult;
		int nRow, nCol;
		if (m_pDBOP->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			for (int i = 1; i <= nRow; i ++)
			{
				pList->AddRawMsg(::atoi(szResult[i * nCol]), szResult[i * nCol + 1]);
			}
			bSucc = TRUE;
		}
		CSqliteDBOP::Free_Result(szResult);
	}
	if (bSucc)
		return S_OK;
	return E_FAIL;
}

//
STDMETHODIMP CMsgMgrImpl::SaveGroupInfo(const char *szGrpId, const char *szGrpDspName, const char *szCreator)
{
	if (szGrpId && szGrpDspName && m_pDBOP)
	{
		CGuardLock::COwnerLock guard(m_DbLock);
		BOOL bSucc = TRUE;
		std::string strSql = "select grpid, grpdspname from groups where grpid='";
		strSql += szGrpId;
		strSql += "'";
		char **szResult;
		int nCol, nRow;
		BOOL bExists = FALSE, bUpdate = TRUE;
		if (m_pDBOP->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			if (nRow > 0)
			{
				bExists = TRUE;
				if (stricmp(szResult[nCol + 1], szGrpDspName) == 0)
					bUpdate = FALSE;
			}
			CSqliteDBOP::Free_Result(szResult);
		}
		strSql = "";
		if (!bExists)
		{
			strSql = "insert into groups(grpid,grpdspname,creator) values('";
			strSql += szGrpId;
			strSql += "','";
			strSql += szGrpDspName;
			strSql += "','";
			if (szCreator)
				strSql += szCreator;
			strSql += "')";
		} else if (bUpdate)
		{
			strSql = "update groups set grpdspname='";
			strSql += szGrpDspName;
			strSql += "',creator='";
			if (szCreator)
				strSql += szCreator;
			strSql += "' where grpid='";
			strSql += szGrpId;
			strSql += "'";
		}
		if (!strSql.empty())
		{
			bSucc = m_pDBOP->Execute(strSql.c_str());
		}
		if (bSucc)
			return S_OK;
	}
	return E_FAIL;
}

//
STDMETHODIMP CMsgMgrImpl::GetSmsUserList(IUserList *pList)
{
	return E_FAIL;
}

//
STDMETHODIMP CMsgMgrImpl::GetGroups(IUserList *pList)
{
	if (m_pDBOP && pList)
	{
		CGuardLock::COwnerLock guard(m_DbLock);
		std::string strSql = "select grpid, grpdspname from groups";
		char **szResult = NULL;
		int nRow, nCol;
		if (m_pDBOP->Open(strSql.c_str(), &szResult, nRow, nCol))
		{
			int nSize;
			for (int i = 1; i <= nRow; i ++)
			{				
				if (szResult[i * nCol] && szResult[i * nCol + 1])
				{
					LPORG_TREE_NODE_DATA pData = new ORG_TREE_NODE_DATA();
					memset(pData, 0, sizeof(ORG_TREE_NODE_DATA));
					strncpy(pData->szUserName, szResult[i * nCol], 63);
					nSize = ::strlen(szResult[i * nCol + 1]);
					pData->szDisplayName = new char[nSize + 1];
					strcpy(pData->szDisplayName, szResult[i * nCol + 1]);
					pData->szDisplayName[nSize] = '\0';
					pList->AddUserInfo(pData, FALSE);
				} // end if (szResult[i * nCol]
			} //end for (int i..
			return S_OK;
		} //end if (m_pDBOP->Open
	} //end if (m_pDBOP && pList)
	return E_FAIL;
}

#pragma warning(default:4996)
