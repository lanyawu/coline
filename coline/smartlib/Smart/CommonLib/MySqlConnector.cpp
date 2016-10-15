#include <stdio.h>
#include <string.h>
#include <commonlib/debuglog.h>
#include <errmsg.h>
#include <commonlib/MySqlConnector.h>
 
//////////////////////////////////////////////////////////////////////////////
// class CDbConn

//
//���캯��
//
CDbConn::CDbConn() :
	     m_bConnected(false) 
{
	//
}

//
//���캯��
//��ʼ�����Ӳ���������������
//
CDbConn::CDbConn(const char *szHostName, short nPort, const char *szUserName, 
	     const char *szPassword, const char *szDbName, const char *szCharSet) :
	     m_bConnected(false) 
{
	m_nPort = nPort;
	m_strHostName = szHostName;
	m_strUserName = szUserName;
	m_strPassword = szPassword;
    m_strDbName = szDbName;
	m_strCharSet = szCharSet;
}

//
//��������
//
CDbConn::~CDbConn()
{
}

char *CDbConn::GetSafeSqlString(const char *szSql, char *szDest)
{
	char *szTemp = szDest;
	if (szSql) 
	{
		while (*szSql != '\0')
		{
			if (*szSql == '\'')
			{
				*szTemp = '\'';
				szTemp ++;
			} 
			*szTemp = *szSql;
			szTemp ++;
			szSql ++;
		}
	}
    return szDest;
}

//
//�������ݿ����ӣ����޸����Ӳ���
//
BOOL CDbConn::Open(const char *szHostName, short nPort, const char *szUserName,
	const char *szPassword, const char *szDbName, const char *szCharSet)
{
	m_nPort = nPort;
	m_strHostName = szHostName;
	m_strUserName = szUserName;
	m_strPassword = szPassword;
    m_strDbName = szDbName;
	m_strCharSet = szCharSet; 
	return TRUE;
}

//
//����ԭ�����Ӳ����������ݿ�����
//
BOOL CDbConn::Open()
{
	//����һ������
    short nPort = m_nPort;
	std::string strHostName = m_strHostName;
	std::string strDbName = m_strDbName;
	std::string strUserName = m_strUserName;
	std::string strPwd = m_strPassword;
	std::string strCharSet = m_strCharSet;
	return Open(strHostName.c_str(), nPort, strUserName.c_str(), strPwd.c_str(), strDbName.c_str(), strCharSet.c_str());
}

//
//����ԭ�����Ӳ������½������ݿ�����
//
BOOL CDbConn::ReOpen()
{
	//����һ������
    short nPort = m_nPort;
	std::string strHostName = m_strHostName;
	std::string strDbName = m_strDbName;
	std::string strUserName = m_strUserName;
	std::string strPwd = m_strPassword;
	std::string strCharSet = m_strCharSet;
	return Open(strHostName.c_str(), nPort, strUserName.c_str(), strPwd.c_str(), strDbName.c_str(), strCharSet.c_str());
}
int CDbConn::GetLastInsertId()
{
	return 0;
}

//
//ִ��SQL����
//
BOOL CDbConn::Query(const char *szSQLCmd, BOOL bGetResult)
{
	if (!m_bConnected) 
		Open();
	return InternalQuery(szSQLCmd, -1, bGetResult); 
}

//
//ִ��SQL�������'\0'�ַ�����ָ��SQL����
//
BOOL CDbConn::Query(const char *szSQLCmd, int nSQLCmdLen, BOOL bGetResult)
{ 
	if (!m_bConnected) 
		Open();
	return InternalQuery(szSQLCmd, nSQLCmdLen, bGetResult); 
}

//////////////////////////////////////////////////////////////////////////////
// class CMySQLConn

//
//���캯��
//
CMySQLConn::CMySQLConn() : 
	        m_pRes(NULL)
{
}

//
//���캯��
//��ʼ�����Ӳ���������������
//
CMySQLConn::CMySQLConn(const char *szHostName, short nPort, const char *szUserName, 
	         const char *szPassword, const char *szDbName, const char *szCharSet):
	        CDbConn(szHostName, nPort, szUserName, szPassword, szDbName, szCharSet),
	        m_pRes(NULL),
			m_pRow(NULL)
{
}

//
//��������
//
CMySQLConn::~CMySQLConn()
{
	Close();
}

//
//�������ݿ�
//ע�⣺���ʧ�ܣ����׳��쳣
//
BOOL CMySQLConn::Open(const char *szHostName, short nPort, const char *szUserName, 
	               const char *szPassword, const char *szDbName, const char *szCharSet)
{
	CDbConn::Open(szHostName, nPort, szUserName, szPassword, szDbName, szCharSet);

	Close();

	if (mysql_init(&m_stMySQL) == NULL)
	{
		PRINTDEBUGLOG(dtInfo, "init mysql error");
		return FALSE;
	}
	if (mysql_real_connect(&m_stMySQL, szHostName, szUserName, szPassword, 
		szDbName, nPort, NULL, 0) == NULL)
	{
		PRINTDEBUGLOG(dtInfo, "connect db error, host: %s, username: %s pwd: %s dbname: %s, port: %d",
			szHostName, szUserName, szPassword, szDbName, nPort);
		return FALSE;
	}
	mysql_set_character_set(&m_stMySQL, szCharSet);
	m_bConnected = TRUE;
	return m_bConnected;
}

//
//�ر����ݿ�
//
void CMySQLConn::Close()
{
	FreeMySQLRes();

	if (m_bConnected)
	{
		mysql_close(&m_stMySQL);
		m_bConnected = FALSE;
	}
}

int CMySQLConn::GetLastInsertId()
{
	return (int) ::mysql_insert_id(&m_stMySQL);
}

//
//�ͷŲ�ѯ�����(m_pRes)
//
void CMySQLConn::FreeMySQLRes()
{
	if (m_pRes)
	{
		mysql_free_result(m_pRes);
		m_pRes = NULL;
	}
}

//
//ִ��SQL����
//ע�⣺���ʧ�ܣ����׳��쳣
//
//������
//  pSQLCmd:	Ҫִ�е�SQL����
//  nSQLCmdLen:	��SQL�а���'\0'ʱ����ָ��SQL�ĳ���(Ϊ-1ʱ��ʾ��ָ������)
//  bGetResult:	�Ƿ�ϣ�����ؽ����
//���أ�
//  true:	�ɹ�
//  false:	ʧ��
//
BOOL CMySQLConn::InternalQuery(const char *szSQLCmd, int nSQLCmdLen, BOOL bGetResult)
{
	int nRet;
	if (!m_bConnected) 
		return FALSE;
	for (int nTimes = 0; nTimes < 2; nTimes++)
	{
		//ִ��SQL
		if (nSQLCmdLen == -1)
			nRet = mysql_query(&m_stMySQL, szSQLCmd);
		else
			nRet = mysql_real_query(&m_stMySQL, szSQLCmd, nSQLCmdLen);

		//���ִ��SQLʧ��
		if (nRet)
		{
			//������״Σ����Ҵ�������Ϊ���Ӷ�ʧ������������			
			if (nTimes == 0)
			{		
				int nErrNo = mysql_errno(&m_stMySQL);		
				if (nErrNo == CR_SERVER_GONE_ERROR || nErrNo == CR_SERVER_LOST)
				{
					ReOpen();
					continue;
				}
			}
			PRINTDEBUGLOG(dtInfo, "query error:%s  SQL: %s", ::mysql_error(&m_stMySQL), szSQLCmd);
			return FALSE;
		} else  //���ִ��SQL�ɹ�
			break;
	} //end for(...
	if (bGetResult)
	{
		FreeMySQLRes();
		m_pRes = mysql_store_result(&m_stMySQL);
	}
	return TRUE;
}

//
//����Query������ȡ��һ������
//
//���أ�
//  true:	�ɹ�
//  false:	ʧ��
//
BOOL CMySQLConn::FetchRow()
{
	if (m_pRes)
	{
		m_pRow = mysql_fetch_row(m_pRes);
		return (m_pRow != NULL);
	} else
		return FALSE;
}

BOOL CMySQLConn::MoveFirstRow()
{
	if (m_pRes)
	{
		::mysql_data_seek(m_pRes, 0);		
		return TRUE;
	}
	return FALSE;
}

BOOL CMySQLConn::MoveLastRow()
{
	return TRUE;
}

//
//����FetchRow������ȡĳ������
//
//������
//  nFieldIndex:	�к�(0-based)
//���أ�
//  ����ָ��
//
char * CMySQLConn::GetFieldData(int nFieldIndex)
{
	if (m_bConnected && m_pRes && nFieldIndex >= 0) 
		return m_pRow[nFieldIndex];
	else
		return NULL;
}

//
//��ѯ�󷵻ؼ�¼����
//
int CMySQLConn::GetRecordCount()
{
	if (m_pRes)
		return (int) mysql_num_rows(m_pRes);
	else
		return 0;
}

//class CDbConnectionPool
CDbConnectionPool::CDbConnectionPool(int dwCount)
{
	m_bTerminated = FALSE;
	m_hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	CGuardLock::COwnerLock guard(m_DbLock);
	for (int i = 0; i < dwCount; i ++)
	{
		CMySQLConn *pCon = new CMySQLConn();
		m_DBList.push_back(pCon);
	}
}

CDbConnectionPool::~CDbConnectionPool()
{
	m_bTerminated = true;
	SetEvent(m_hEvent);

	CGuardLock::COwnerLock guard(m_DbLock); 
	std::vector<CMySQLConn *>::iterator it;
	for (it = m_DBList.begin(); it != m_DBList.end(); it ++)
	{
		delete (*it);
	}
	m_DBList.clear();
	::CloseHandle(m_hEvent);
	m_hEvent = NULL;
}

BOOL CDbConnectionPool::OpenDatabase(const char *szHostName, short nPort, const char *szUserName, const char *szUserPwd, 
                              const char * szDatabase, const char *szCharSet)
{
	BOOL b = TRUE;
	CGuardLock::COwnerLock guard(m_DbLock);
	std::vector<CMySQLConn *>::iterator it;
	for (it = m_DBList.begin(); it != m_DBList.end(); it ++)
	{
		b = (*it)->Open(szHostName, nPort, szUserName, szUserPwd, szDatabase, szCharSet);
	}
	return b;
}

void CDbConnectionPool::InsertDBQuery(CMySQLConn *pDbQuery)
{
	m_DbLock.Lock();
	m_DBList.push_back(pDbQuery);
	m_DbLock.UnLock();
	//
	SetEvent(m_hEvent);
}

CMySQLConn *CDbConnectionPool::GetDBConnection()
{
	CMySQLConn *pDb = NULL;
	while(!m_bTerminated)
	{
		while (m_DBList.empty() && (!m_bTerminated))
		{
			::WaitForSingleObject(m_hEvent, INFINITE);
		}
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
