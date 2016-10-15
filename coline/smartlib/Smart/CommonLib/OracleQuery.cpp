#include <Commonlib/DebugLog.h>
#include <Commonlib/stringutils.h>
#include <Commonlib/oraclequery.h>

#pragma warning(disable:4996)

COracleQuery::COracleQuery(void):
              m_lpConnection(NULL),
		      m_iGIT(NULL),
		      m_dwConnectionCookie(0)
{
	::CoInitialize(NULL);
}


COracleQuery::~COracleQuery(void)
{
	Close();
}

char * COracleQuery::SafeSQLString(char * szSQL)
{
	if (strlen(szSQL) == 0)
		return szSQL;
	char *szNew = new char[strlen(szSQL) * 2];
	memset(szNew, 0, strlen(szSQL) * 2);
	char *szCurr = (char *)szSQL;
	char *szNewCurr = szNew;
	while(*szCurr)
	{
		*szNewCurr++ = *szCurr;
		if (szCurr == "'")
			*szNewCurr ++ = '\'';
		szCurr ++;
	}
	strcpy((char *)szSQL, szNew);
	delete []szNew;
	return szSQL;
}

void COracleQuery::Close()
{
	if (m_lpConnection != NULL)
	{
		if(m_lpConnection->State ) 
		   m_lpConnection->Close();
		m_lpConnection = NULL;
	}
	if(m_iGIT !=  NULL)
	{
		m_iGIT->RevokeInterfaceFromGlobal(m_dwConnectionCookie);
		m_iGIT->Release();
		m_iGIT = NULL;
	}
}

BOOL COracleQuery::Connect(const char *strDbHost,const  WORD wHostPort, const char *strUserName, const  char *strUserPwd, 
                            const char *strDbName, const char *szDSN)
{
	memset(m_strConnectString, 0, MAX_CONNECTSTRING_LENGTH);
	//"Provider=OraOLEDB.Oracle.1;User ID=HR;Password=sa;Data Source=(DESCRIPTION=(ADDRESS=(PROTOCOL=TCP)(HOST=192.168.1.188)(PORT=1521))(CONNECT_DATA=(SERVICE_NAME = CISS)));Persist Security Info=False
	sprintf_s(m_strConnectString, MAX_CONNECTSTRING_LENGTH - 1, "Provider=OraOLEDB.Oracle.1;User ID=%s;Password=%s;Data Source=(DESCRIPTION=(ADDRESS=(PROTOCOL=TCP)(HOST=%s)(PORT=%d))(CONNECT_DATA=(SERVICE_NAME=%s)));Persist Security Info=False",
			strUserName, strUserPwd, strDbHost, wHostPort, strDbName);
	if (ConnectDB())
	{
		return TRUE;
	} else
	{
		PRINTDEBUGLOG(dtInfo, "connect string:%s", m_strConnectString);
		memset(m_strConnectString, 0, MAX_CONNECTSTRING_LENGTH);
		sprintf_s(m_strConnectString, MAX_CONNECTSTRING_LENGTH - 1, "Provider=MSDAORA.1;Data Source=%s;User ID=%s;Password=%s",strDbName, strUserName, strUserPwd);
	    return ConnectDB();
	}
}

BOOL COracleQuery::ConnectDB()
{
	::CoInitialize(NULL); 
	Close();//先关闭以前的连接 
	HRESULT hr = CoCreateInstance(CLSID_StdGlobalInterfaceTable,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IGlobalInterfaceTable,
		(void **)&m_iGIT);

	if(FAILED(hr))
	{
		PRINTDEBUGLOG(dtError,  "初始化全局接口表失败");
		return FALSE;
	} 
	try
	{ 
		CLSID clsid;
		hr = CLSIDFromProgID(OLESTR("ADODB.Connection"), &clsid);
		LPOLESTR pOleStr = NULL;
		HRESULT hr = ::StringFromCLSID(clsid, &pOleStr);
		if(SUCCEEDED(hr)) 
		{
			char szTmp[MAX_PATH] = {0};
			CStringConversion::WideCharToString(pOleStr, szTmp, MAX_PATH - 1); 
			PRINTDEBUGLOG(dtInfo, "ADODB CLSID:%s", szTmp);
		}
		hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, __uuidof(_Connection),(LPVOID *) &m_lpConnection);
		if (FAILED(hr))
		{
			LPVOID lpMsgBuf = NULL;
			::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, hr, 
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
			char szTmp[MAX_PATH] = {0};
			CStringConversion::WideCharToString((TCHAR *)lpMsgBuf, szTmp, MAX_PATH - 1);
			PRINTDEBUGLOG(dtError,  "初始化数据库ADO对象失败！%s", szTmp);
			::LocalFree(lpMsgBuf);
			
			return FALSE;
		}	  
		if( SUCCEEDED(hr) )
		{ 
			//PRINTDEBUGLOG(dtInfo, "open connectstring:%s", m_strConnectString);
			m_lpConnection->Open(m_strConnectString, "", "", adModeUnknown );
		} else
		{
			return FALSE;
		} 
		hr = m_iGIT->RegisterInterfaceInGlobal(m_lpConnection, __uuidof(_Connection), &m_dwConnectionCookie);
		if (FAILED(hr))
		{
			PRINTDEBUGLOG(dtError,  "向全局接口表注册m_lpConnection时失败 ！");
			m_lpConnection = NULL;
			return FALSE;
		}
	} catch(_com_error e)
	{
		PRINTDEBUGLOG(dtError, "数据库访问异常!!错误代码：%d---错误信息：%s---描述：%s",
                     e.Error(),e.ErrorMessage(),(LPCSTR)e.Description());
		if (m_lpConnection->State) 
			m_lpConnection->Close();
		m_lpConnection = NULL;
		return FALSE;
	}
    
	return TRUE;
}

BOOL COracleQuery::LoadFromFile(const char *szFileName)
{
	return FALSE;
}

BOOL COracleQuery::OpenTable(const char *strTableName, _RecordsetPtr &pRecordset)
{
	return FALSE;
}

BOOL COracleQuery::Execute(const char *lpQueryString, _RecordsetPtr& pRecordset)
{
	for( int i=0; i< 3; i++ )
	{
		try
		{

			pRecordset->Open(_variant_t( lpQueryString ), _variant_t( (IDispatch*)m_lpConnection, true ), 
				adOpenStatic, adLockOptimistic, adCmdText);
			return TRUE;
		} catch(_com_error e)
		{
			PRINTDEBUGLOG(dtError, "数据库访问异常!!错误代码：%d---错误信息：%s---描述：%s SQL: %s ",
						 e.Error(),e.ErrorMessage(),(LPCSTR)e.Description(), lpQueryString);
			if (pRecordset->State) 
				pRecordset->Close();
			//尝试重新连接数据库
			ConnectDB();
		}
	}
	return FALSE;
}

BOOL COracleQuery::ExecuteProc(const char *lpQueryString, _RecordsetPtr &pRecordset)
{
	return FALSE;
}


int COracleQuery::GetLastInsertId()
{
	//select seq_atable.currval from dual
	return 0;
}
  
//没有结果集的操作
BOOL COracleQuery::Execute(const char *lpQueryString) 
{
	for( int i=0; i< 3; i++ )
	{
		try
		{
			_variant_t RecordsAffected = -1;
			m_lpConnection->Execute((_bstr_t)(lpQueryString), &RecordsAffected, adExecuteNoRecords | adCmdText);
			return TRUE;
		} catch(_com_error e)
		{
			PRINTDEBUGLOG(dtError, "数据库访问异常!!错误代码：%d---错误信息：%s---描述：%s",
						 e.Error(),e.ErrorMessage(),(LPCSTR)e.Description());
			//尝试重新连接数据库
			ConnectDB();
		}
	}
	return FALSE;
}

//class CDBQueryBufferMan
COracleDBQueryBufferMan::COracleDBQueryBufferMan(DWORD dwCount)
{
	m_hDbNotifyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (dwCount == 0)
		dwCount = 1;
	if (dwCount > MAX_DB_COUNT)
		dwCount = MAX_DB_COUNT;
	for (DWORD i = 0; i < dwCount; i ++)
	{
		COracleQuery *pQuery = new COracleQuery();
		InsertDBQuery(pQuery);
	}
}

COracleDBQueryBufferMan::~COracleDBQueryBufferMan()
{
	SetEvent(m_hDbNotifyEvent);
	CloseHandle(m_hDbNotifyEvent);
	CGuardLock::COwnerLock guard(m_DbLock);
	while(!m_DBList.empty())
	{
		COracleQuery *pQuery = m_DBList.back();
		delete pQuery;
		m_DBList.pop_back();
	}
}

bool COracleDBQueryBufferMan::OpenDatabase(const char *lpServer, const WORD wPort, const char * lpUID, const char * lpPWD, const char *lpDatabase)
{
	bool b = true;
	CGuardLock::COwnerLock guard(m_DbLock);
	std::vector<COracleQuery *>::iterator it;
	for (it = m_DBList.begin(); it != m_DBList.end(); it ++)
	{
		b = b && (*it)->Connect(lpServer, wPort, lpUID, lpPWD, lpDatabase);
	}
	return b;
}

void COracleDBQueryBufferMan::InsertDBQuery(COracleQuery *lpDbQuery)
{
	CGuardLock::COwnerLock guard(m_DbLock);
	m_DBList.push_back(lpDbQuery);
	SetEvent(m_hDbNotifyEvent);
}


COracleQuery *COracleDBQueryBufferMan::GetDBQuery()
{
	COracleQuery *pDb = NULL;
	while(true)
	{
		while (m_DBList.empty())
			WaitForSingleObject(m_hDbNotifyEvent, 10);
		m_DbLock.Lock();
		if (!m_DBList.empty())
		{
			pDb = m_DBList.back();
			m_DBList.pop_back();
		}
		m_DbLock.UnLock();
		if (pDb)
			break;
	}
	return pDb;
}

#pragma warning(default:4996)
