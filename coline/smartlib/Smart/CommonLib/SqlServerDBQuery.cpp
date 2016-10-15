#ifdef _SQLSERVER_ADO__

#include <objbase.h>
#include <Commonlib/DebugLog.h>
#include <Commonlib/SqlServerDBQuery.h>

#pragma warning(disable:4996)

CSqlServerDBQuery::CSqlServerDBQuery(void):
                   m_lpConnection(NULL),
				   m_iGIT(NULL),
				   m_dwConnectionCookie(0)
{
	::CoInitialize(NULL);
}

CSqlServerDBQuery::~CSqlServerDBQuery(void)
{
	Close();
}

char * CSqlServerDBQuery::SafeSQLString(char * szSQL)
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

void CSqlServerDBQuery::Close()
{
	if( m_lpConnection != NULL )
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

bool CSqlServerDBQuery::LoadFromFile(const char *szFileName)
{
	sprintf_s(m_strConnectString, 255, "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=%s;Persist Security Info=False", szFileName);
	return ConnectDB();
}

bool CSqlServerDBQuery::Connect(const char *strDbHost, const WORD wHostPort, const char *strUserName, const char *strUserPwd, 
								const char *strDbName, const char *szDSN,  BOOL bMysql)
{
	m_bMysql = bMysql;
	if (!bMysql)
 		sprintf_s(m_strConnectString, 511, "Driver={SQL Server};Server=%s,%d;UID=%s;PWD=%s;Database=%s",
			strDbHost, wHostPort, strUserName, strUserPwd, strDbName);
	else
	{
		sprintf_s(m_strConnectString, 511, "DSN=%s;Server=%s;Database=%s", szDSN, strDbHost, strDbName);
		m_strUserName = strUserName;
		m_strUserPwd = strUserPwd;
	}
	return ConnectDB();
}

bool CSqlServerDBQuery::ConnectDB()
{
	PRINTDEBUGLOG(dtError, "连接数据库");
	Close();//先关闭以前的连接

	HRESULT hr = CoCreateInstance(CLSID_StdGlobalInterfaceTable,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IGlobalInterfaceTable,
		(void **)&m_iGIT);

	if(FAILED(hr))
	{
		PRINTDEBUGLOG(dtError,  "初始化全局接口表失败");
		return false;
	}

	try
	{
		
		CLSID clsid;
		hr = CLSIDFromProgID(OLESTR("ADODB.Connection"), &clsid);
		
		hr=CoCreateInstance(clsid,NULL,CLSCTX_INPROC_SERVER,__uuidof(_Connection),(LPVOID *) &m_lpConnection);
		if(FAILED(hr))
		{
			PRINTDEBUGLOG(dtError,  "初始化数据库ADO对象失败！");
			return false;
		}		
		

		if( SUCCEEDED(hr) )
		{
			if (m_bMysql)
			{
				
				m_lpConnection->Open(m_strConnectString, m_strUserName.c_str(), m_strUserPwd.c_str(), adModeUnknown);
			} else
				m_lpConnection->Open( m_strConnectString, "", "", adModeUnknown );
		}
		else
		{
			return false;
		}

		hr = m_iGIT->RegisterInterfaceInGlobal(m_lpConnection,__uuidof(_Connection),&m_dwConnectionCookie);
		if(FAILED(hr))
		{
			PRINTDEBUGLOG(dtError,  "向全局接口表注册m_lpConnection时失败 ！");
			m_lpConnection = NULL;
			return false;
		}
	}
	catch(_com_error e)
	{
		PRINTDEBUGLOG(dtError, "数据库访问异常!!错误代码：%d---错误信息：%s---描述：%s",
                     e.Error(),e.ErrorMessage(),(LPCSTR)e.Description());
		if( m_lpConnection->State ) m_lpConnection->Close();
		m_lpConnection = NULL;
		return false;
	}
    
	return true;
}

bool CSqlServerDBQuery::Execute(char *lpQueryString, _RecordsetPtr &pRecordset)
{
	for( int i=0; i< 3; i++ )
	{
		try
		{

			pRecordset->Open( _variant_t( lpQueryString ), _variant_t( (IDispatch*)m_lpConnection, true ), adOpenStatic, adLockOptimistic, adCmdText );
			return true;
		}
		catch(_com_error e)
		{
			PRINTDEBUGLOG(dtError, "数据库访问异常!!错误代码：%d---错误信息：%s---描述：%s SQL: %s ",
						 e.Error(),e.ErrorMessage(),(LPCSTR)e.Description(), lpQueryString);
			if( pRecordset->State ) pRecordset->Close();
			//尝试重新连接数据库
			ConnectDB();
		}
	}
	return false;
}

//执行存储过程
bool CSqlServerDBQuery::ExecuteProc(char * lpQueryString, _RecordsetPtr &pRecordset)
{
	for( int i=0; i< 3; i++ )
	{
		try
		{
			pRecordset->Open( _variant_t( lpQueryString ), _variant_t( (IDispatch*)m_lpConnection, true ), adOpenStatic, adLockOptimistic, adCmdUnknown);
			return true;
		}
		catch(_com_error e)
		{
			PRINTDEBUGLOG(dtError, "数据库访问异常!!错误代码：%d---错误信息：%s---描述：%s SQL: %s ",
						 e.Error(),e.ErrorMessage(),(LPCSTR)e.Description(), lpQueryString);
			if( pRecordset->State ) pRecordset->Close();
			//尝试重新连接数据库
			ConnectDB();
		}
	}
	return false;
}

bool CSqlServerDBQuery::Execute(char *lpQueryString)
{
	for( int i=0; i< 3; i++ )
	{
		try
		{
			_variant_t RecordsAffected = -1;
			m_lpConnection->Execute((_bstr_t)(lpQueryString), &RecordsAffected, adExecuteNoRecords | adCmdText);
			return true;
		}
		catch(_com_error e)
		{
			PRINTDEBUGLOG(dtError, "数据库访问异常!!错误代码：%d---错误信息：%s---描述：%s",
						 e.Error(),e.ErrorMessage(),(LPCSTR)e.Description());
			//尝试重新连接数据库
			ConnectDB();
		}
	}
	return false;
}

bool CSqlServerDBQuery::OpenTable(char *strTableName, _RecordsetPtr &pRecordset)
{
	try
	{
		pRecordset->Open( _variant_t(strTableName ),
			_variant_t( (IDispatch*)m_lpConnection, true ), adOpenStatic, adLockOptimistic, adCmdTable );
		return true;
	}
	catch(_com_error e)
	{
		return false;
	}
}

//class CDBQueryBufferMan
CDBQueryBufferMan::CDBQueryBufferMan(DWORD dwCount)
{
	m_hDbNotifyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (dwCount == 0)
		dwCount = 1;
	if (dwCount > MAX_DB_COUNT)
		dwCount = MAX_DB_COUNT;
	for (DWORD i = 0; i < dwCount; i ++)
	{
		CSqlServerDBQuery *pQuery = new CSqlServerDBQuery();
		InsertDBQuery(pQuery);
	}
}

CDBQueryBufferMan::~CDBQueryBufferMan()
{
	SetEvent(m_hDbNotifyEvent);
	CloseHandle(m_hDbNotifyEvent);
	CGuardLock::COwnerLock guard(m_DbLock);
	while(!m_DBList.empty())
	{
		CSqlServerDBQuery *pQuery = m_DBList.back();
		delete pQuery;
		m_DBList.pop_back();
	}
}

bool CDBQueryBufferMan::OpenDatabase(char *lpServer,WORD wPort, char * lpUID, char * lpPWD, char *lpDatabase)
{
	bool b = true;
	CGuardLock::COwnerLock guard(m_DbLock);
	vector<CSqlServerDBQuery *>::iterator it;
	for (it = m_DBList.begin(); it != m_DBList.end(); it ++)
	{
		b = b && (*it)->Connect(lpServer, wPort, lpUID, lpPWD, lpDatabase);
	}
	return b;
}

void CDBQueryBufferMan::InsertDBQuery(CSqlServerDBQuery *lpDbQuery)
{
	CGuardLock::COwnerLock guard(m_DbLock);
	m_DBList.push_back(lpDbQuery);
	SetEvent(m_hDbNotifyEvent);
}

CSqlServerDBQuery *CDBQueryBufferMan::GetDBQuery()
{
	CSqlServerDBQuery *pDb = NULL;
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

#endif
