#ifndef __SQLSERVERDBQUERY_H__
#define __SQLSERVERDBQUERY_H__

#ifdef _SQLSERVER_ADO__
#include <vector>
#include <string>
#include <Commonlib/GuardLock.h>

#import "D:\Program Files\Common Files\system\ado\msado15.dll" no_namespace rename ("EOF", "adoEOF")

#define MAX_DB_COUNT 10
const DWORD OFFLINE_MESSAGE_SAVETIME = 7 * 24 * 60 * 60; //离线消息默认保存天数

using namespace std;
class COMMONLIB_API CSqlServerDBQuery
{
public:
	CSqlServerDBQuery(void);
public:
	~CSqlServerDBQuery(void);

private:
	bool ConnectDB();	
	void Close();
public:
	static char * SafeSQLString(char * szSQL);
	bool Connect(const char *strDbHost,const  WORD wHostPort, const char *strUserName, const  char *strUserPwd, 
		const char *strDbName, const char *szDSN = NULL, BOOL bMysql = FALSE);
	bool LoadFromFile(const char *szFileName);
	bool OpenTable(char *strTableName, _RecordsetPtr &pRecordset);
    bool Execute(char *lpQueryString, _RecordsetPtr& pRecordset);
	bool ExecuteProc(char *lpQueryString, _RecordsetPtr &pRecordset);
	bool Execute(char *lpQueryString); //没有结果集的操作
	bool Connected() { return (m_lpConnection != NULL); };
private:
	char					m_strConnectString[512];
	_Connection *			m_lpConnection;
	DWORD					m_dwConnectionCookie;
    IGlobalInterfaceTable   *m_iGIT;
	std::string m_strUserName;
	std::string m_strUserPwd;
	BOOL  m_bMysql;
};

//class CDBQueryBufferMan 数据库查询池
class CDBQueryBufferMan
{
public:
	CDBQueryBufferMan(DWORD dwCount);
	~CDBQueryBufferMan();
private:
	vector<CSqlServerDBQuery *> m_DBList;
    CGuardLock m_DbLock;
	HANDLE m_hDbNotifyEvent;
public:
	bool OpenDatabase(char *lpServer,WORD wPort, char *lpUID, char *lpPWD, char * lpDatabase);
	void InsertDBQuery(CSqlServerDBQuery *lpDbQuery);
	CSqlServerDBQuery *GetDBQuery();
};

#endif
#endif